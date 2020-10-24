#pragma once

#include <types.h>

void ktask_init();
void mktask();

typedef enum
{
  Running,
  Sleeping,
  ToBeDestroyed
} task_state_t;

typedef struct
{
    size_t r15;
    size_t r14;
    size_t r13;
    size_t r12;
    size_t r11;
    size_t r10;
    size_t r9;
    size_t r8;
    size_t rdi;
    size_t rsi;
    size_t rdx;
    size_t rcx;
    size_t rbx;
    size_t rax;
    size_t rbp;
    size_t gs;
    size_t fs;
    size_t irq;
    size_t error;
    size_t rip;
    size_t cs;
    size_t rflags;
    size_t rsp;
    size_t ss;
} __attribute__((packed)) task_context_t;

typedef struct
{
    size_t tid;
    task_state_t state;

} task_t;
