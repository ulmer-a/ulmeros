#pragma once

#include <task.h>


void _init sched_init();
void _init sched_start();

void sched_insert(task_t* task);

void yield();

extern task_t* current_task;
