#include <sched/mutex.h>
#include <sched/task.h>
#include <debug.h>

#ifndef BOOT32
extern task_t* current_task;
#endif

static void yield() {}
static size_t xchg(size_t val, size_t* mem)
{
  size_t ret = *mem;
  *mem = val;
  return ret;
}

void mutex_init(mutex_t* mtx)
{
  if (!mtx || mtx->magic == MUTEX_MAGIC)
    assert(false, "mutex_init(): already already initialized");

  mtx->magic = MUTEX_MAGIC;
  mtx->lock = 0;
}

int mutex_held(mutex_t* mtx)
{
  if (!mtx || mtx->magic != MUTEX_MAGIC)
    assert(false, "mutex_lock(): uninitialized");

  return mtx->lock;
}

void mutex_lock(mutex_t* mtx)
{
  if (!mtx || mtx->magic != MUTEX_MAGIC)
    assert(false, "mutex_lock(): uninitialized");

  while (xchg(1, &mtx->lock))
  {
    yield();
  }

#ifndef BOOT32
  mtx->held_by = current_task;
#endif
}

void mutex_unlock(mutex_t* mtx)
{
  if (!mtx || mtx->magic != MUTEX_MAGIC)
    assert(false, "mutex_unlock(): uninitialized");

#ifndef BOOT32
  mtx->held_by = NULL;
#endif

  mtx->lock = 0;
}

int mutex_locked(mutex_t* mtx)
{
  if (!mtx || mtx->magic != MUTEX_MAGIC)
    assert(false, "mutex_locked(): uninitialized");

  return mtx->lock;
}

int mutex_held_by(mutex_t *mtx, task_t *task)
{
  if (!mtx || mtx->magic != MUTEX_MAGIC)
    assert(false, "mutex_hel_by(): uninitialized");
  return (mtx->held_by == task);
}
