#pragma once

typedef unsigned long long uint64_t;
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;

#define BIT(x) (1 << (x))
#define PAGE_SIZE 4096

extern uint64_t ramSize;

extern void clearBss();
extern void printMessage(const char *msg);

extern void* getEmptyPage();
extern uint32_t getHeapSize();
extern uint32_t getHeapStart();

extern void memcpy(char *dest, const char *src, unsigned long n);
extern void memset(void* mem, char val, unsigned long size);

#define ENTRY64_ADDR ((void*)0x01000000) // @ 16MB
