#pragma once

#include <util/types.h>

struct _list_item_struct;
typedef struct _list_item_struct list_item_t;

struct _list_struct
{
  size_t items;
  size_t magic;
  list_item_t* first;
  list_item_t* last;
};
typedef struct _list_struct list_t;

list_item_t* list_it_front(list_t* list);
list_item_t* list_it_back(list_t* list);
list_item_t* list_it_next(list_item_t* it);
list_item_t* list_it_prev(list_item_t* it);
void* list_it_get(list_item_t* it);

#define LIST_IT_END NULL

void list_init(list_t* list);

int list_is_valid(list_t* list);

void list_destroy(list_t* list);

size_t list_add(list_t* list, void* payload);

void list_remove(list_t* list, size_t index);

void list_clear(list_t* list);

void* list_get(list_t* list, size_t index);

void* list_pop_front(list_t* list);

size_t list_size(list_t* list);

void list_rotate(list_t* list);
