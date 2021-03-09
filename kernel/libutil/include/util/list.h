#pragma once

#include <util/types.h>

#ifdef ARCH_X86_64
#define LIST_MAGIC ((unsigned long)0xabffdeadbeefabff)
#else
#define LIST_MAGIC ((unsigned long)0xbadbeef0ul)
#endif

struct _list_item_struct;
typedef struct _list_item_struct list_item_t;

typedef struct _list_struct
{
  size_t items;
  size_t magic;
  list_item_t* first;
  list_item_t* last;
} list_t;

#define LIST_INITIALIZER {    \
  .items = 0,                 \
  .magic = LIST_MAGIC,        \
  .first = NULL,              \
  .last = NULL                \
}

list_item_t* list_it_front(list_t* list);
list_item_t* list_it_back(list_t* list);
list_item_t* list_it_next(list_item_t* it);
list_item_t* list_it_prev(list_item_t* it);
void list_it_remove(list_t *list, list_item_t* it);
void* list_it_get(list_item_t* it);

#define LIST_IT_END NULL

/* meta operations */
void list_init(list_t* list);
void list_destroy(list_t* list);
size_t list_size(list_t* list);

/* basic list operations */
void* list_get(list_t* list, size_t index);
list_item_t* list_find(list_t* list, void* item);
size_t list_add(list_t* list, void* payload);
void list_remove(list_t* list, size_t index);

/* advanced list operations */
void list_clear(list_t* list);
void list_rotate(list_t* list);

/* queue operations */
void* list_pop_front(list_t* list);
void* list_pop_back(list_t* list);


