#pragma once

#include <types.h>

#define PAGE_SIZE 4096

struct vspace_struct_;
typedef struct vspace_struct_ vspace_t;

/**
 * @brief initialize the kernel page tables
 */
void _init vspace_init_kernel();

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
 * @param user 1 to enable user space
 * @return errno
 */
int vspace_map(vspace_t* vspace, size_t virt_page,
               size_t phys_page, int user, int shared);

/**
 * @brief vspace_unmap
 * @param vspace the address space
 * @param virt_page page to unmap
 * @return errno
 */
int vspace_unmap(vspace_t* vspace, size_t virt_page);
