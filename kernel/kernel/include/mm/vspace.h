#pragma once

#include <util/types.h>
#include <arch/common.h>
#include <sched/mutex.h>

#define PG_USER     BIT(0)
#define PG_WRITE    BIT(1)
#define PG_NOEXEC   BIT(2)

typedef struct _vspace_struct vspace_t;

extern vspace_t _vspace_kernel;
#define VSPACE_KERNEL (&_vspace_kernel)

/* get a pointer to the content of the
 * corresponding virtual address. */
void* phys_to_virt(void* phys_addr);

/* get a pointer to the content of the
 * corresponding page frame number. */
void* ppn_to_virt(size_t ppn);

/* resolve a virtual address to a physical one */
size_t virt_to_ppn(vspace_t *vspace, void* virt_addr);

/* initialize a new virtual address space. */
void vspace_init(vspace_t* vspace);

/* allocate and initialize a new virtual
 * address space. */
vspace_t* vspace_create();

void vspace_delete(vspace_t* vspace);

/* switch to the specified virtual address space. */
void vspace_apply(vspace_t* vspace);

/* map a single page into the specified virtual
 * address space. additional flags can be specified
 * to control read/write, user/supervisor and exec/noexec
 * permissions. */
void vspace_map(vspace_t* vspace, size_t virt, size_t phys, int flags);

/* unmap the specified page from the referenced virtual
 * address space. */
int vspace_unmap(vspace_t* vspace, size_t virt);

