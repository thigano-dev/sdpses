/**
 * @file	only_once_allocator.h
 * @brief	only once allocator
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

#ifndef SDPSES_KERNEL_MEMORY_ONLY_ONCE_ALLOCATOR_H_INCLUDED_
#define SDPSES_KERNEL_MEMORY_ONLY_ONCE_ALLOCATOR_H_INCLUDED_

#include "allocator.h"

#ifdef __cplusplus
extern "C" {
#endif

void OnlyOnceAllocator_initialize(void);
void OnlyOnceAllocator_terminate(void);

unsigned long OnlyOnceAllocator_totalAllocationRequests(void);
unsigned long OnlyOnceAllocator_totalDeallocationRequests(void);

size_t OnlyOnceAllocator_totalAllocatedSize(void);
size_t OnlyOnceAllocator_allocatableSizeMax(void);

#ifdef __cplusplus
}
#endif

#endif /* SDPSES_KERNEL_MEMORY_ONLY_ONCE_ALLOCATOR_H_INCLUDED_ */
