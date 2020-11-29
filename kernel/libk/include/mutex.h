#pragma once

#include <types.h>

#define MUTEX_MAGIC 0xdeadbeef
#define MUTEX_INITIALIZER { .lock = 0, .magic = MUTEX_MAGIC }

struct _mutex_struct
{
  size_t lock;
  size_t magic;
};

typedef struct _mutex_struct mutex_t;

/**
 * @brief mutex_alloc allocate and initialize a mutex
 * @return pointer to an initialized mutex
 */
mutex_t* mutex_alloc();

/**
 * @brief mutex_init initialize a mutex object
 * @param mtx pointer to the mutex to be initialized
 */
void mutex_init(mutex_t *mtx);
void mutex_lock(mutex_t* mtx);
void mutex_unlock(mutex_t* mtx);
void mutex_destroy(mutex_t* mtx);
