#pragma once

#include <util/types.h>
#include <util/list.h>

#define MUTEX_MAGIC 0xdeadbeef
#define MUTEX_INITIALIZER { .lock = 0, .magic = MUTEX_MAGIC }

struct _mutex_struct
{
  size_t lock;
  size_t magic;
};

typedef struct _mutex_struct mutex_t;

void mutex_init(mutex_t *mtx);
int mutex_held(mutex_t* mtx);
void mutex_lock(mutex_t* mtx);
void mutex_unlock(mutex_t* mtx);
void mutex_destroy(mutex_t* mtx);
