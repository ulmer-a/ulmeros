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

static void waiting_lock(mutex_t* mtx)
{
  while (xchg(1, &mtx->waiting_tasks_lock))
    yield();
}

static void waiting_unlock(mutex_t* mtx)
{
  mtx->waiting_tasks_lock = 0;
}

void mutex_init(mutex_t* mtx)
{
  if (!mtx || mtx->magic == MUTEX_MAGIC)
    assert(false, "mutex_init(): already already initialized");

  list_init_without_alloc(&mtx->waiting_tasks);

  mtx->magic = MUTEX_MAGIC;
  mtx->lock = 0;
}

int mutex_held(mutex_t* mtx)
{
  if (!mtx || mtx->magic != MUTEX_MAGIC)
    assert(false, "mutex_lock(): uninitialized");

  return (mtx->holding == current_task);
}

void mutex_lock(mutex_t* mtx)
{
  if (!mtx || mtx->magic != MUTEX_MAGIC)
    assert(false, "mutex_lock(): uninitialized");

  if (mtx->holding && mtx->holding == current_task)
    assert(false, "mutex_lock(): already held by this task´");

  while (xchg(1, &mtx->lock))
  {
    if (current_task == NULL)
      return;

    waiting_lock(mtx);

    if (xchg(1, &mtx->lock) == 0)
    {
      waiting_unlock(mtx);
      break;
    }

    list_add(&mtx->waiting_tasks, current_task);
    waiting_unlock(mtx);
    current_task->waiting_for_lock = true;
    yield();
  }

  mtx->holding = current_task;
}

void mutex_unlock(mutex_t* mtx)
{
  if (!mtx || mtx->magic != MUTEX_MAGIC)
    assert(false, "mutex_unlock(): uninitialized");

  mtx->holding = NULL;
  mtx->lock = 0;

  if (current_task == NULL)
    return;

  waiting_lock(mtx);
  task_t* thread = list_pop_front(&mtx->waiting_tasks);
  waiting_unlock(mtx);

  if (thread == NULL)
    return;

  assert(thread->waiting_for_lock, "waiting thread is not waiting");
  thread->waiting_for_lock = false;
}

int mutex_locked(mutex_t* mtx)
{
  if (!mtx || mtx->magic != MUTEX_MAGIC)
    assert(false, "mutex_locked(): uninitialized");

  return mtx->lock;
}
