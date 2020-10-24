#pragma once

typedef struct
{

} mutex_t;

void mutex_init(mutex_t* mtx);
void mutex_lock(mutex_t* mtx);
void mutex_unlock(mutex_t* mtx);
