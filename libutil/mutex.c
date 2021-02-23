#include <util/mutex.h>
#include <util/system.h>

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
