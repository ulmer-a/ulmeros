#pragma once

#include <util/types.h>

#define PG_USER   BIT(0)
#define PG_KERNEL 0
#define PG_WRITE  BIT(1)
#define PG_NOEXEC BIT(2)

struct vspace_struct_;
typedef struct vspace_struct_ vspace_t;

extern vspace_t _kernel_vspace;
#define VSPACE_KERNEL (&_kernel_vspace)

void vspace_init(vspace_t* vspace);

void *get_virtual(uint64_t phys_addr);

void vspace_apply(vspace_t* vspace);
void vspace_map(vspace_t* vspace, size_t virt, size_t phys, int flags);
void vspace_unmap(vspace_t* vspace, size_t virt);
