#pragma once

typedef struct
{
    unsigned long long ramPages;
    unsigned long long heapStartPage;
    unsigned long long heapPageCount;
    unsigned long long gdtPage;
    unsigned long long kernelStartPage;
    unsigned long long pml4Page;
} Boot64Struct;

#define OS_VERSION  "v0"
#define OS_AUTHOR   "Alexander Ulmer"
