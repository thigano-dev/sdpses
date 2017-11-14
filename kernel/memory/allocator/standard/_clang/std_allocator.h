/**
 * @file	std_allocator.h
 * @brief	standard allocator
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

#ifndef SDPSES_KERNEL_MEMORY_STD_ALLOCATOR_H_INCLUDED_
#define SDPSES_KERNEL_MEMORY_STD_ALLOCATOR_H_INCLUDED_

#include "allocator.h"

#ifdef __cplusplus
extern "C" {
#endif

void StdAllocator_initialize(void);
void StdAllocator_terminate(void);

unsigned long StdAllocator_totalAllocationRequests(void);
unsigned long StdAllocator_totalDeallocationRequests(void);

size_t StdAllocator_totalAllocatedSize(void);

#ifdef __cplusplus
}
#endif

#endif /* SDPSES_KERNEL_MEMORY_STD_ALLOCATOR_H_INCLUDED_ */
