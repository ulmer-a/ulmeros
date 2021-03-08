#include <sched/task.h>
#include <sched/proc.h>
#include <sched/sched.h>
#include <util/list.h>
#include <sched/mutex.h>
#include <mm/memory.h>
#include <mm/vspace.h>
#include <debug.h>

static size_t task_count = 0;
static list_t task_list;
static mutex_t task_list_lock;

static void proc_delete(proc_t* proc)
{
  /* destroy locks, lists, vspace and loader */
  loader_release(proc->loader);
  list_destroy(&proc->task_list);
  mutex_destroy(&proc->heap_lock);
  mutex_destroy(&proc->stack_list_lock);
  mutex_destroy(&proc->task_list_lock);
  vspace_delete(proc->vspace);

  /* delete all the user stacks */
  // TODO: delete stacks
}

static void task_delete(task_t *task)
{
  assert(task && task->state == TASK_KILLED, "invalid task_t*");

  /* remove the task from the corresponding process' task list */
  proc_t* process = task->process;
  if (process)
  {
    /* remove the task from the process' list of tasks */
    mutex_lock(&process->task_list_lock);
    list_item_t* it = list_find(&process->task_list, task);
    assert(it, "cleanup task_list: task has already been removed");
    list_it_remove(&process->task_list, it);
    int proc_dead = list_size(&process->task_list) == 0;
    mutex_unlock(&process->task_list_lock);

    /* after the process' last thread has been removed,
     * delete the process as well. */
    if (proc_dead)
    {
      debug(TASK, "deleting process %zu\n", process->pid);
      proc_delete(process);
    }
  }

  /* release the memory occupied by the task */
  kfree(task->kstack_base);
  kfree(task);
}

static void cleanup_task_func()
{
  while (task_count > 0)
  {
    /* iterate through the list of tasks, remove
     * and delete any one with state TASK_KILLED */
    mutex_lock(&task_list_lock);
    list_item_t* it = list_it_front(&task_list);
    while (it != LIST_IT_END)
    {
      list_item_t* next = list_it_next(it);
      task_t* task = list_it_get(it);

      if (task->state == TASK_KILLED)
      {
        debug(TASK, "deleting task #%zu\n", task->tid);
        task_delete(task);
        list_it_remove(&task_list, it);
        atomic_add(&task_count, -1);
      }
      it = next;
    }
    mutex_unlock(&task_list_lock);

    yield();
  }

  debug(TASKLIST, "no more tasks alive!\n");

  // TODO: shutdown the system
}

void tl_insert(task_t* task)
{
  mutex_lock(&task_list_lock);
  list_add(&task_list, task);
  mutex_unlock(&task_list_lock);
  atomic_add(&task_count, 1);
}

void tl_setup()
{
  list_init(&task_list);
  mutex_init(&task_list_lock);

  task_t* cleanup_task = create_kernel_task(cleanup_task_func);
  sched_insert(cleanup_task);
}
