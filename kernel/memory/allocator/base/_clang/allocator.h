/**
 * @file	allocator.h
 * @brief	memory allocator
 * @author	Tsuguyoshi Higano
 * @date	Nov 14, 2017
 *
 * @par Project
 * Software Development Platform for Small-scale Embedded Systems (SDPSES)
 *
 * @copyright (c) Tsuguyoshi Higano, 2017
 *
 * @par License
 * Released under the MIT license@n
 * http://opensource.org/licenses/mit-license.php
 */

#ifndef SDPSES_KERNEL_MEMORY_ALLOCATOR_H_INCLUDED_
#define SDPSES_KERNEL_MEMORY_ALLOCATOR_H_INCLUDED_

#include <stddef.h>

#ifdef __cplusplus
#include <new>
extern "C" {
#endif

void* Allocator_allocate(size_t size);
void Allocator_deallocate(void* ptr);

unsigned long Allocator_totalAllocationRequests(void);
unsigned long Allocator_totalDeallocationRequests(void);

#ifdef __cplusplus
}
#endif

#endif /* SDPSES_KERNEL_MEMORY_ALLOCATOR_H_INCLUDED_ */
