#pragma once

#ifdef BOOT32
#include "../../arch/x86_64/boot32/boot32.h"
#else
#include <mm/memory.h>
#include <debug.h>
#endif
