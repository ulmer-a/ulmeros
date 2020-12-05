#pragma once

#include <types.h>

struct list_item_struct_;
typedef struct list_item_struct_ list_item_t;

struct list_struct_
{
  size_t items;
  size_t magic;
  list_item_t* first;
  list_item_t* last;
};
typedef struct list_struct_ list_t;


list_t* list_init();

int list_is_valid(list_t* list);

void list_init_without_alloc(list_t* list);

void list_destroy(list_t* list);

size_t list_add(list_t* list, void* payload);

void list_remove(list_t* list, size_t index);

void list_clear(list_t* list);

void* list_get(list_t* list, size_t index);

void* list_pop_front(list_t* list);

size_t list_size(list_t* list);

void list_rotate(list_t* list);
