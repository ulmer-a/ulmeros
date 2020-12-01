#include <mutex.h>
#include <arch.h>
#include <sched.h>
#include <memory.h>

mutex_t* mutex_alloc()
{
  mutex_t* mtx = kmalloc(sizeof(mutex_t));
  mutex_init(mtx);
  return mtx;
}

void mutex_init(mutex_t* mtx)
{
  if (!mtx || mtx->magic == MUTEX_MAGIC)
    assert(false, "mutex_init(): already already initialized");

  mtx->magic = MUTEX_MAGIC;
  mtx->lock = 0;
}

void mutex_lock(mutex_t* mtx)
{
  if (!mtx || mtx->magic != MUTEX_MAGIC)
    assert(false, "mutex_lock(): uninitialized");

  while (xchg(1, &mtx->lock))
    yield();
}

void mutex_unlock(mutex_t* mtx)
{
  if (!mtx || mtx->magic != MUTEX_MAGIC)
    assert(false, "mutex_unlock(): uninitialized");

  mtx->lock = 0;
}

int mutex_locked(mutex_t* mtx)
{
  if (!mtx || mtx->magic != MUTEX_MAGIC)
    assert(false, "mutex_locked(): uninitialized");

  return mtx->lock;
}