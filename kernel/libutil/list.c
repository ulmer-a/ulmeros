#include <util/list.h>
#include <util/system.h>

#define SANITY_CHECK(list)                        \
  assert((list) && (list)->magic == LIST_MAGIC,  \
    "list is invalid or corrupt")

struct _list_item_struct
{
  list_item_t* next;
  list_item_t* prev;
  void* payload;
};

void list_init(list_t* list)
{
  assert(list, "list is null");

  list->first  = NULL;
  list->last   = NULL;
  list->items  = 0;
  list->magic  = LIST_MAGIC;
}

void list_clear(list_t* list)
{
  SANITY_CHECK(list);

  while (list->items > 0)
    list_remove(list, 0);
}

size_t list_add(list_t* list, void* payload)
{
  SANITY_CHECK(list);

  list_item_t* item = kmalloc(sizeof(list_item_t));
  item->payload = payload;
  item->next = NULL;
  item->prev = list->last;

  if (list->last)
    list->last->next = item;
  else
    list->first = item;
  list->last = item;

  list->items++;
  return list->items - 1;
}

static list_item_t* list_get_index(list_t* list, size_t index)
{
  SANITY_CHECK(list);

  size_t i = 0;
  for (list_item_t* item = list->first;
       item != NULL;
       item = item->next)
  {
    if (i == index)
    {
      return item;
    }

    i++;
  }
  return NULL;
}


void* list_pop_front(list_t* list)
{
  SANITY_CHECK(list);

  if (list->items == 0)
    return NULL;

  list_item_t* item = list->first;
  list_remove(list, 0);
  return item;
}

void list_remove(list_t* list, size_t index)
{
  SANITY_CHECK(list);

  list_item_t* item = list_get_index(list, index);
  list_it_remove(list, item);
}

void* list_get(list_t* list, size_t index)
{
  SANITY_CHECK(list);

  list_item_t* item = list_get_index(list, index);
  if (item == NULL)
    return NULL;
  return item->payload;
}

size_t list_size(list_t* list)
{
  SANITY_CHECK(list);

  return list->items;
}

void list_rotate(list_t* list)
{
  SANITY_CHECK(list);

  if (list->items < 2)
    return;

  list_item_t* item = list->first;
  item->next->prev = NULL;
  item->prev = list->last;
  list->first = item->next;
  item->next = NULL;
  list->last->next = item;
  list->last = item;
}

void *list_it_get(list_item_t *it)
{
  return it->payload;
}

list_item_t *list_it_next(list_item_t *it)
{
  return it->next;
}

list_item_t *list_it_front(list_t *list)
{
  SANITY_CHECK(list);
  return list->first;
}

list_item_t *list_it_back(list_t *list)
{
  SANITY_CHECK(list);
  return list->last;
}

void list_it_remove(list_t* list, list_item_t *item)
{
  SANITY_CHECK(list);
  assert(item, "list iterator is null");

  if (item->prev)
    item->prev->next = item->next;
  if (item->next)
    item->next->prev = item->prev;
  if (list->first == item)
    list->first = item->next;
  if (list->last == item)
    list->last = item->prev;
  list->items--;

  kfree(item);
}

list_item_t *list_find(list_t *list, void *item)
{
  SANITY_CHECK(list);

  for (list_item_t* it = list->first;
       it != NULL;
       it = it->next)
  {
    if (it->payload == item)
      return it;
  }

  return NULL;
}

void list_destroy(list_t *list)
{
  SANITY_CHECK(list);
  list_clear(list);
  list->magic = 0;
}
