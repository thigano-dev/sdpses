/**
 * @file	allocator_private.h
 * @brief	allocator private
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

#ifndef SDPSES_KERNEL_MEMORY_ALLOCATOR_PRIVATE_H_INCLUDED_
#define SDPSES_KERNEL_MEMORY_ALLOCATOR_PRIVATE_H_INCLUDED_

#include "allocator.h"

struct Allocator {
	void* (*allocate)(size_t size);
	void (*deallocate)(void* ptr);
};

void Allocator_initialize(const struct Allocator* allocator);
void Allocator_terminate(void);

#endif /* SDPSES_KERNEL_MEMORY_ALLOCATOR_PRIVATE_H_INCLUDED_ */
