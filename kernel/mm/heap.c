#include <util/types.h>
#include <util/mutex.h>
#include <mm/memory.h>
#include <mm/vspace.h>
#include <arch/common.h>
#include <debug.h>

typedef struct _hblock
{
  unsigned available;
  unsigned magic;
  struct _hblock *prev;
  struct _hblock *next;
#ifdef DEBUG
  const char* function;
  unsigned int line;
#endif
  size_t size;
} __attribute__((packed)) hblock_t;

#define HEAP_MAGIC  (0xabcdefabu)
#define HEADER_SIZE (sizeof(hblock_t))

void* kheap_start_ = (void*)KHEAP_START;
void* kheap_break_ = (void*)KHEAP_START;

static hblock_t *heap_start = NULL;
static hblock_t *heap_last = NULL;
static mutex_t  kheap_mutex = MUTEX_INITIALIZER;

void kheap_print()
{
  mutex_lock(&kheap_mutex);

  debug(KHEAP, "-- heap dump:\n");
  hblock_t *entry;
  for (entry = heap_start; entry != NULL; entry = entry->next)
  {
    if (entry->magic != HEAP_MAGIC) {
      assert(false, "kheap: heap corruption detected");
    }

    if (entry->available)
    {
      debug(KHEAP, "  @ %p: %zu bytes (free)\n");
    }
    else
    {
#ifdef DEBUG
      debug(KHEAP, "  @ %p: %zu bytes (%s:%u)\n", entry + 1, entry->size,
            entry->function, entry->line);
#else
      debug(KHEAP, "  @ %p: %zu bytes\n", entry + 1, entry->size);
#endif
    }
  }

  mutex_unlock(&kheap_mutex);
}

void kheap_check_corrupt()
{
  for (hblock_t* entry = kheap_start_;
       entry != NULL;
       entry = entry->next)
  {
    if (entry->magic != HEAP_MAGIC)
    {
      assert(false, "heap corruption detected!");
    }
  }
}

static void* kbrk(ssize_t increment)
{
  void *orig_brk = kheap_break_;
  kheap_break_ += increment;


  if (orig_brk > kheap_break_)
  {
    size_t first_unmap = (size_t)kheap_break_ / PAGE_SIZE;
    if ((size_t)kheap_break_ % PAGE_SIZE != 0)
      first_unmap += 1;
    size_t last_unmap = (size_t)orig_brk / PAGE_SIZE;

    for (size_t page = first_unmap; page <= last_unmap; page++)
      vspace_unmap(VSPACE_KERNEL, page);
  }
  else
  {
    size_t first_map = (size_t)orig_brk / PAGE_SIZE;
    if ((size_t)orig_brk % PAGE_SIZE != 0)
      first_map += 1;
    size_t last_map = (size_t)kheap_break_ / PAGE_SIZE;

    for (size_t page = first_map; page <= last_map; page++)
    {
      vspace_map(VSPACE_KERNEL, page, alloc_page(), PG_NOEXEC|PG_WRITE);
    }
  }

  return orig_brk;
}

static void crop(hblock_t *block, size_t new_size)
{
  size_t excess_size = block->size - new_size;
  if (excess_size <= HEADER_SIZE) {
    // don't do anything
    return;
  }

  // update current block
  hblock_t *original_next = block->next;
  block->size = new_size;
  block->next = (void*)block + new_size;

  // allocate new block
  block->next->available = 1;
  block->next->magic = HEAP_MAGIC;
  block->next->prev = block;
  block->next->next = original_next;
  block->next->size = excess_size;

  if (block->next->next != NULL) {
    // update the next block's prev pointer
    block->next->next->prev = block->next;
  } else {
    // update heap info
    heap_last = block->next;
  }
}

#ifdef DEBUG
void *_kmalloc(size_t size, unsigned line, const char* function)
#else
void *_kmalloc(size_t size)
#endif
{
#ifdef DEBUG
  debug(KHEAP, "kmalloc(): size %zd from %s():%u\n", size, function, line);
#endif

  mutex_lock(&kheap_mutex);

  hblock_t *entry;
  for (entry = heap_start; entry != NULL; entry = entry->next)
  {
    if (entry->magic != HEAP_MAGIC) {
      assert(false, "kheap: heap corruption detected");
    }

    size_t required_size = HEADER_SIZE + size;
    if (entry->available && entry->size >= required_size)
    {
      entry->available = 0;

#ifdef DEBUG
      entry->function = function;
      entry->line = line;
#endif

      crop(entry, required_size);
      mutex_unlock(&kheap_mutex);
      return entry + 1;
    }
  }

  hblock_t *blk = kbrk(size + HEADER_SIZE);
  if (blk == (void*)-1) {
    mutex_unlock(&kheap_mutex);
    return NULL;
  }

  blk->available = 0;
  blk->magic = HEAP_MAGIC;
  blk->next = NULL;
  blk->prev = heap_last;
  blk->size = HEADER_SIZE + size;

#ifdef DEBUG
  blk->function = function;
  blk->line = line;
#endif

  if (heap_last != NULL)
    heap_last->next = blk;
  heap_last = blk;

  if (heap_start == NULL)
    heap_start = blk;

  mutex_unlock(&kheap_mutex);

  return blk + 1;
}

static void merge_blocks(hblock_t *first, hblock_t *second)
{
  if (first->magic != HEAP_MAGIC || second->magic != HEAP_MAGIC) {
    assert(false, "kheap: heap corruption detected");
  }

  first->size += second->size;
  first->next = second->next;
  if (second->next != NULL) {
    second->next->prev = first;
  } else {
    heap_last = first;
  }

  second->magic = 0;
}

void kfree(void *ptr)
{
  debug(KHEAP, "kfree(): %p\n", ptr);
  if (ptr == NULL) {
    return;
  }

  if (heap_last == NULL || heap_start == NULL ||
        ptr < (void*)(heap_start) || ptr > (void*)(heap_last + 1)) {
    assert(false, "kheap: heap corruption detected");
  }

  mutex_lock(&kheap_mutex);

  hblock_t *tofree = (hblock_t *)ptr - 1;
  if (tofree->magic != HEAP_MAGIC || tofree->available) {
    assert(false, "kheap: heap corruption detected");
  }

  tofree->available = 1;

  // merge free blocks
  if (tofree->next != NULL && tofree->next->available)
    merge_blocks(tofree, tofree->next);
  if (tofree->prev != NULL && tofree->prev->available)
    merge_blocks(tofree->prev, tofree);

  // decrease heap size when possible
  while (heap_last != NULL && heap_last->available)
  {
    // invalidate
    heap_last->magic = 0;

    size_t decrement = heap_last->size;
    if (heap_last->prev != NULL) {
      heap_last->prev->next = NULL;
      heap_last = heap_last->prev;
    } else {
      heap_start = NULL;
      heap_last = NULL;
    }

    // decrease heap break
    kbrk(-decrement);
  }

  mutex_unlock(&kheap_mutex);
}
