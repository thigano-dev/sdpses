/**
 * @file	std_allocator.c
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

#include <stdlib.h>

#include "allocator_private.h"
#include "std_allocator.h"

static struct Allocator allocator_;

static size_t totalAllocatedSize_ = 0;

static void* allocate(size_t size);
static void deallocate(void* ptr);

/**
 * @brief	Initialize
 * @return	none
 */
void StdAllocator_initialize(void)
{
	allocator_.allocate = allocate;
	allocator_.deallocate = deallocate;
	Allocator_initialize(&allocator_);

	totalAllocatedSize_ = 0;
}

/**
 * @brief	Terminate
 * @return	none
 */
void StdAllocator_terminate(void)
{
	Allocator_terminate();
}

/**
 * @brief	Get total number of memory allocation requests
 * @return	total number of memory allocation requests
 */
unsigned long StdAllocator_totalAllocationRequests(void)
{
	return Allocator_totalAllocationRequests();
}

/**
 * @brief	Get total number of memory deallocation requests
 * @return	total number of memory deallocation requests
 */
unsigned long StdAllocator_totalDeallocationRequests(void)
{
	return Allocator_totalDeallocationRequests();
}

/**
 * @brief	Get total size of allocated memory
 * @return	total size of allocated memory
 */
size_t StdAllocator_totalAllocatedSize(void)
{
	return totalAllocatedSize_;
}

/**
 * @brief	Allocate memory block
 * @param	size			size in bytes
 * @retval	!=NULL			success (a pointer to the memory block)
 * @retval	NULL			failure
 */
static void* allocate(const size_t size)
{
	void* const rc = malloc(size);
	if (rc) { totalAllocatedSize_ += size; }
	return rc;
}

/**
 * @brief	Deallocate memory block
 * @param	ptr				pointer to the memory block
 * @return	none
 */
static void deallocate(void* const ptr)
{
	free(ptr);
}
