#pragma once

#include <util/types.h>

void debug(const char* fmt, ...);
void _assert(const char* function, const char* file, int line, const char* msg);

#ifdef DEBUG
#define assert(x, msg) if(!(x)) { _assert(__func__, __FILE__, __LINE__, msg); }
#else
#define assert(x, msg)
#endif
