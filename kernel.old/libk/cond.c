#include <cond.h>
#include <sched.h>

void cond_init(cond_t* cond)
{
  cond->magic = COND_MAGIC;
  cond->waiting_tasks = list_init();
  mutex_init(&cond->waiting_tasks_lock);
}

void cond_wait(cond_t* cond, mutex_t* mtx)
{
  assert(cond->magic == COND_MAGIC, "cond corruption");

  /* aquire the waiting tasks lock to be
   * able to modify the waiting tasks list*/
  mutex_lock(&cond->waiting_tasks_lock);

  /* the lock can be released because access
   * to the waiting tasks list will require
   * the waiting_tasks_lock */
  mutex_unlock(mtx);

  /* add the current thread to the list of
   * waiting threads. */
  list_add(cond->waiting_tasks, current_task);
  mutex_unlock(&cond->waiting_tasks_lock);

  /* go to sleep */
  current_task->waiting_for_lock = true;
  yield();

  /* reaquire the lock */
  mutex_lock(mtx);
}

void cond_signal(cond_t* cond)
{
  assert(cond->magic == COND_MAGIC, "cond corruption");

  /* pop the task to be released */
  mutex_lock(&cond->waiting_tasks_lock);
  task_t* thread = list_pop_front(cond->waiting_tasks);
  mutex_unlock(&cond->waiting_tasks_lock);

  if (thread == NULL)
    return;

  /* wake up the waiting thread */
  assert(thread->waiting_for_lock, "waiting thread is not waiting");
  thread->waiting_for_lock = false;
}
