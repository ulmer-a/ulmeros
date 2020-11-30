#include <memory.h>
#include <types.h>
#include <definitions.h>
#include <vspace.h>
#include <mutex.h>

struct hblock
{
    unsigned available;
    unsigned magic;
    struct hblock *prev;
    struct hblock *next;
    size_t size;
} __attribute__((packed));

#define HEAP_MAGIC  (0xabcdefabu)
#define HEADER_SIZE (sizeof(struct hblock))

static void*    kheap_start = ARCH_KHEAP_START;
static void*    kheap_break = NULL;
static mutex_t  kheap_mutex = MUTEX_INITIALIZER;

static struct hblock *heap_start = NULL;
static struct hblock *heap_last = NULL;

void kheap_init()
{
  debug(KHEAP, "initializing kernel heap @ %p", kheap_start);

  /* map the first heap page and initialize */
  size_t phys_page = page_alloc(0);
  size_t heap_start_virt = (size_t)kheap_start / PAGE_SIZE;
  vspace_map(VSPACE_KERNEL, heap_start_virt, phys_page, PG_KERNEL);
  kheap_break = kheap_start + PAGE_SIZE;

  /* */
}

static void* kbrk(ssize_t increment)
{
  // TODO initialize heap
  return NULL;
}

static void crop(struct hblock *block, size_t new_size)
{
  size_t excess_size = block->size - new_size;
  if (excess_size <= HEADER_SIZE) {
    // don't do anything
    return;
  }

  // update current block
  struct hblock *original_next = block->next;
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

void *kmalloc(size_t size)
{
  mutex_lock(&kheap_mutex);

  struct hblock *entry;
  for (entry = heap_start; entry != NULL; entry = entry->next)
  {
    if (entry->magic != HEAP_MAGIC) {
      assert(false, "kheap: heap corruption detected");
    }

    size_t required_size = HEADER_SIZE + size;
    if (entry->available && entry->size >= required_size)
    {
      entry->available = 0;
      crop(entry, required_size);
      mutex_unlock(&kheap_mutex);
      return entry + 1;
    }
  }

  struct hblock *blk = kbrk(size + HEADER_SIZE);
  if (blk == (void*)-1) {
    mutex_unlock(&kheap_mutex);
    return NULL;
  }

  blk->available = 0;
  blk->magic = HEAP_MAGIC;
  blk->next = NULL;
  blk->prev = heap_last;
  blk->size = HEADER_SIZE + size;

  if (heap_last != NULL)
    heap_last->next = blk;
  heap_last = blk;

  if (heap_start == NULL)
    heap_start = blk;

  mutex_unlock(&kheap_mutex);
  return blk + 1;
}

static void merge_blocks(struct hblock *first, struct hblock *second)
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
  if (ptr == NULL) {
    return;
  }

  if (heap_last == NULL || heap_start == NULL ||
        ptr < (void*)(heap_start) || ptr > (void*)(heap_last + 1)) {
    assert(false, "kheap: heap corruption detected");
  }

  mutex_lock(&kheap_mutex);

  struct hblock *tofree = (struct hblock *)ptr - 1;
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
