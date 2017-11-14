/**
 * @file	allocator.c
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

#include "allocator_private.h"
#include "lib_assert.h"

static const struct Allocator* allocator_ = NULL;

static unsigned long totalAllocationRequests_ = 0;
static unsigned long totalDeallocationRequests_ = 0;

/**
 * @brief	Initialize
 * @param	allocator		Allocator*
 * @return	none
 */
void Allocator_initialize(const struct Allocator* const allocator)
{
	allocator_ = allocator;

	totalAllocationRequests_ = 0;
	totalDeallocationRequests_ = 0;
}

/**
 * @brief	Terminate
 * @return	none
 */
void Allocator_terminate(void)
{
	totalAllocationRequests_ = 0;
	totalDeallocationRequests_ = 0;
}

/**
 * @brief	Allocate memory block
 * @param	size			size in bytes
 * @retval	!=NULL			success (a pointer to the memory block)
 * @retval	NULL			failure
 */
void* Allocator_allocate(const size_t size)
{
	ASSERT_(allocator_ != NULL);

	void* const allocatedPointer = allocator_->allocate(size);
	if (allocatedPointer != NULL) {
		totalAllocationRequests_++;
	}

	return allocatedPointer;
}

/**
 * @brief	Deallocate memory block
 * @param	ptr				pointer to a memory block
 * @return	none
 */
void Allocator_deallocate(void* const ptr)
{
	ASSERT_(allocator_ != NULL);

	totalDeallocationRequests_++;
	allocator_->deallocate(ptr);
}

/**
 * @brief	Get total number of memory allocation requests
 * @return	total number of memory allocation requests
 */
unsigned long Allocator_totalAllocationRequests(void)
{
	return totalAllocationRequests_;
}

/**
 * @brief	Get total number of memory deallocation requests
 * @return	total number of memory deallocation requests
 */
unsigned long Allocator_totalDeallocationRequests(void)
{
	return totalDeallocationRequests_;
}
