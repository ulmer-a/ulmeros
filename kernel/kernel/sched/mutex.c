#include <sched/mutex.h>
#include <sched/sched.h>
#include <sched/task.h>
#include <arch/common.h>
#include <debug.h>

#define CHECK_MAGIC(mtx) if (!mtx || mtx->magic != MUTEX_MAGIC) \
  assert(false, "mutex_t uninitialized or corrupt");

void mutex_init(mutex_t* mtx)
{
  if (!mtx || mtx->magic == MUTEX_MAGIC)
    assert(false, "mutex_init(): mutex already initialized");

  mtx->magic = MUTEX_MAGIC;
  mtx->lock = 0;
  mtx->held_by = NULL;

  list_init(&mtx->waiting_tasks);
  spin_init(&mtx->waiting_tasks_lock);
}

void mutex_lock(mutex_t* mtx)
{
  CHECK_MAGIC(mtx);

  while (xchg(1, &mtx->lock))
  {
    /* lock the mutex's waiting tasks list and
     * put the task asleep before adding it to
     * the list. this way it can only be woken
     * up when it has already been put asleep. */
    assert(current_task, "mutex_lock(): invalid current_task");
    spin_lock(&mtx->waiting_tasks_lock);
    current_task->state = TASK_SLEEPING;
    list_add(&mtx->waiting_tasks, current_task);
    yield();
    spin_unlock(&mtx->waiting_tasks_lock);
  }

  mtx->held_by = current_task;
}

void mutex_unlock(mutex_t* mtx)
{
  CHECK_MAGIC(mtx);
  mtx->held_by = NULL;
  mtx->lock = 0;

  /* wake up the first task on the list, if any. */
  spin_lock(&mtx->waiting_tasks_lock);
  if (list_size(&mtx->waiting_tasks) > 0)
  {
    task_t* task = list_get(&mtx->waiting_tasks, 0);
    assert(task, "mutex wakeup: invalid task ptr");
    assert(task->state == TASK_SLEEPING, "mutex wakeup: task is not sleeping");
    task->state = TASK_RUNNING;
  }
  spin_unlock(&mtx->waiting_tasks_lock);
}

int mutex_locked(mutex_t* mtx)
{
  CHECK_MAGIC(mtx);
  return mtx->lock;
}

int mutex_held(mutex_t *mtx)
{
  CHECK_MAGIC(mtx);
  return mtx->held_by == current_task;
}

