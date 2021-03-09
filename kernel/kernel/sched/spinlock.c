#include <sched/mutex.h>
#include <sched/sched.h>
#include <debug.h>
#include <arch/common.h>

#define CHECK_MAGIC(spin) if (!(spin) || (spin)->magic != SPIN_MAGIC) \
  assert(false, "spin_t uninitialized or corrupt");

void spin_init(spin_t *spin)
{
  if (!spin || spin->magic == MUTEX_MAGIC)
    assert(false, "mutex_init(): mutex already initialized");
  spin->lock = 0;
  spin->magic = SPIN_MAGIC;
}

void spin_lock(spin_t *spin)
{
  CHECK_MAGIC(spin);
  while (xchg(1, &spin->lock))
    yield();
}

void spin_unlock(spin_t *spin)
{
  CHECK_MAGIC(spin);
  assert(spin->lock, "spinlock is not acquired");
  spin->lock = 0;
}

void spin_destroy(spin_t *spin)
{
  CHECK_MAGIC(spin);
  assert(spin->lock == 0, "spin_destroy() called while locked");
  spin->magic = 0;
}
