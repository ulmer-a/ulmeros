#include <list.h>
#include <memory.h>

struct list_struct_
{
  size_t items;
  list_item_t* first;
  list_item_t* last;
};

struct list_item_struct_
{
  list_item_t* next;
  list_item_t* prev;
  void* payload;
};

list_t* list_init()
{
  list_t* list = kmalloc(sizeof(list_t));
  list->first  = NULL;
  list->last   = NULL;
  list->items  = 0;
  return list;
}

void list_destroy(list_t* list)
{
  while (list->items > 0)
    list_remove(list, 0);
  kfree(list);
}

void list_add(list_t* list, void* payload)
{
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
}

static list_item_t* list_get_index(list_t* list, size_t index)
{
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

void list_remove(list_t* list, size_t index)
{
  list_item_t* item = list_get_index(list, index);

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

void* list_get(list_t* list, size_t index)
{
  return list_get_index(list, index)->payload;
}

size_t list_size(list_t* list)
{
  return list->items;
}

void list_rotate(list_t* list)
{
  if (list->items < 2)
    return;

  list_item_t* item = list->first;
  item->next->prev = NULL;
  item->prev = list->last;
  list->first = item->next;
  item->next = NULL;
  list->last = item;
}
