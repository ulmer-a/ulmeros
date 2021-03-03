#include <sched/task.h>
#include <sched/proc.h>
#include <sched/sched.h>
#include <util/list.h>
#include <util/mutex.h>
#include <mm/memory.h>
#include <mm/vspace.h>
#include <debug.h>

static size_t task_count = 0;
static list_t task_list;
static mutex_t task_list_lock;

static void task_delete(task_t *task)
{
  assert(task && task->state == TASK_KILLED, "invalid task_t*");

  /* release kernel stack memory */
  size_t kstack_ppn = virt_to_ppn(VSPACE_KERNEL, task->kstack_base);
  free_page(kstack_ppn);

  kfree(task);
}

static void cleanup_task_func()
{
  while (task_count > 0)
  {
    break;
#ifdef DEBUG
    /* occasionally, check for heap corruption */
    //kheap_check_corrupt();
#endif

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
