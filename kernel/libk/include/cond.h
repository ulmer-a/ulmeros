#pragma once

#include <types.h>
#include <mutex.h>
#include <list.h>

#define COND_MAGIC 0xbeefdead
#define COND_INITIALIZER { .lock = 0, .magic = MUTEX_MAGIC }

struct _cv_struct
{
  size_t magic;
  list_t* waiting_tasks;
  mutex_t waiting_tasks_lock;
};

typedef struct _cv_struct cond_t;

void cond_init(cond_t* cond);

void cond_wait(cond_t* cond, mutex_t* mtx);

void cond_signal(cond_t* cond);
