#pragma once

#include <util/types.h>
#include <util/list.h>

struct _task_struct;
typedef struct _task_struct task_t;

typedef struct _spin_struct
{
    size_t lock;
    size_t magic;
} spin_t;

typedef struct _mutex_struct
{
  size_t lock;
  size_t magic;

  /* list of waiting tasks */
  list_t waiting_tasks;
  spin_t waiting_tasks_lock;

  task_t* held_by;
} mutex_t;

#define MUTEX_MAGIC 0xdeadbeef
#define SPIN_MAGIC  0xdead0ff0

#define SPIN_INITIALIZER {    \
  .lock = 0,                  \
  .magic = SPIN_MAGIC         \
}

#define MUTEX_INITIALIZER {               \
  .lock = 0,                              \
  .magic = MUTEX_MAGIC,                   \
  .waiting_tasks = LIST_INITIALIZER,      \
  .waiting_tasks_lock = SPIN_INITIALIZER, \
  .held_by = NULL                         \
}

void mutex_init(mutex_t *mtx);
int mutex_held(mutex_t* mtx);
void mutex_lock(mutex_t* mtx);
void mutex_unlock(mutex_t* mtx);
void mutex_destroy(mutex_t* mtx);

void spin_init(spin_t* spin);
void spin_lock(spin_t* spin);
void spin_unlock(spin_t* spin);
void spin_destroy(spin_t* spin);
