#pragma once

#include <types.h>

#define PAGE_SIZE 4096

struct _vspace_struct;
typedef struct _vspace_struct vspace_t;

/**
 * @brief vspace_init
 * @return new address space
 */
vspace_t* vspace_init();

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
