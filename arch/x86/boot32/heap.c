#include <util/types.h>

#include "boot32.h"

typedef struct hblock
{
  unsigned available;
  unsigned magic;
  struct hblock *prev;
  struct hblock *next;
  size_t size;
} __attribute__((packed)) hblock_t;

#define HEAP_MAGIC  (0xabcdefabu)
#define HEADER_SIZE (sizeof(struct hblock))

void* kheap_start_ = (void*)EARLY_HEAP_START;
void* kheap_break_ = (void*)EARLY_HEAP_START;

static struct hblock *heap_start = NULL;
static struct hblock *heap_last = NULL;

void kheap_print()
{
  debug("-- heap dump:\n");
  hblock_t *entry;
  for (entry = heap_start; entry != NULL; entry = entry->next)
  {
    if (entry->magic != HEAP_MAGIC) {
      assert(false, "kheap: heap corruption detected");
    }

    if (entry->available)
    {
      debug("  @ %p: %zu bytes (free)\n");
    }
    else
    {
      debug("  @ %p: %zu bytes\n", entry + 1, entry->size);
    }
  }
}

static void* kbrk(ssize_t increment)
{
  void *orig_brk = kheap_break_;
  kheap_break_ += increment;

  if (free_pages.bitmap)
  {
    size_t orig_brk_page = (uint64_t)orig_brk >> PAGE_SHIFT;
    size_t new_brk_page = ((uint64_t)kheap_break_ >> PAGE_SHIFT) + 1;
    for (size_t page = orig_brk_page; page <= new_brk_page; page++)
      bitmap_set(&free_pages, page);
  }

  return orig_brk;
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
  //debug("kmalloc(): size=%zu\n", size);

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
      return entry + 1;
    }
  }

  struct hblock *blk = kbrk(size + HEADER_SIZE);
  if (blk == (void*)-1) {
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
  debug("kfree(): %p\n", ptr);
  if (ptr == NULL) {
    return;
  }

  if (heap_last == NULL || heap_start == NULL ||
        ptr < (void*)(heap_start) || ptr > (void*)(heap_last + 1)) {
    assert(false, "kheap: heap corruption detected");
  }

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
}
