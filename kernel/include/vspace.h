#pragma once

#include <types.h>

#define PAGE_SIZE 4096

struct vspace_struct_;
typedef struct vspace_struct_ vspace_t;

extern vspace_t vspace_kernel_;
#define VSPACE_KERNEL (&vspace_kernel_)

#define PG_KERNEL   0
#define PG_USER     BIT(0)
#define PG_READONLY BIT(1)
#define PG_NOEXEC   BIT(2)
#define PG_SHARED   BIT(3)

/**
 * @brief initialize the kernel page tables
 */
void _init vspace_init_kernel();

/**
 * @brief vspace_get_phys_ptr get a pointer to a mapping of
 * a specified physical address
 * @param phys_addr the phyiscal address
 * @return the pointer to the mapping
 */
void* vspace_get_phys_ptr(void* phys_addr);

/**
 * @brief vspace_get_page_ptr get a pointer to a mapping of
 * a specified physical page frame number
 * @param phys the physical page frame number
 * @return the pointer to the mapping
 */
void* vspace_get_page_ptr(size_t phys);

/**
 * @brief vspace_init
 * @return new address space
 */
vspace_t* vspace_init();

/**
 * @brief vspace_apply switch to the given address space
 * @param vspace the address space to switch to
 */
void vspace_apply(vspace_t* vspace);

/**
 * @brief vspace_duplicate
 * @return duplicated address space (cow)
 */
vspace_t* vspace_duplicate(vspace_t* original);

/**
 * @brief vspace_destroy
 * @param vspace
 */
void vspace_destroy(vspace_t* vspace);

/**
 * @brief vspace_map
 * @param vspace the address space
 * @param virt_page virtual page
 * @param phys_page physical page
 * @param flags various flags
 * @return errno
 */
int vspace_map(vspace_t* vspace, size_t virt_page,
               size_t phys_page, int flags);

/**
 * @brief vspace_unmap
 * @param vspace the address space
 * @param virt_page page to unmap
 * @return errno
 */
int vspace_unmap(vspace_t* vspace, size_t virt_page);
