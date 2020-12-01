#pragma once

#include <types.h>

struct list_struct_;
typedef struct list_struct_ list_t;
struct list_item_struct_;
typedef struct list_item_struct_ list_item_t;

list_t* list_init();

void list_destroy(list_t* list);

void list_add(list_t* list, void* payload);

void list_remove(list_t* list, size_t index);

void list_clear(list_t* list);

void* list_get(list_t* list, size_t index);

size_t list_size(list_t* list);

void list_rotate(list_t* list);
