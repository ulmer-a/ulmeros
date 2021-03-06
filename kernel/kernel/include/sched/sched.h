#pragma once

#include <sched/task.h>

/* initialize the scheduler's data structures. */
void sched_init();

/* enable the scheduler for the first time */
void sched_enable();

/* yield the time slice and run
 * schedule() in an interrupt context. */
void yield();

/* insert a thread of execution into
 * the scheduler. */
void sched_insert(task_t* task);

