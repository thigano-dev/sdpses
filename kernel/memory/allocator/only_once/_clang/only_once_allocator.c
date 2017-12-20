/**
 * @file	only_once_allocator.c
 * @brief	only once allocator
 * @author	Tsuguyoshi Higano
 * @date	Dec 04, 2017
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

#include <stdint.h>
#include <stddef.h>

#include "allocator_private.h"
#include "only_once_allocator.h"
#include "only_once_allocator_cfg.h"
#include "lib_assert.h"

static struct Allocator allocator_;

#if defined(ONLY_ONCE_ALLOCATOR_MEMORY_POOL_BASE)
static uint8_t* const memoryPool_ = (uint8_t*)ONLY_ONCE_ALLOCATOR_MEMORY_POOL_BASE;
#else
static uint8_t memoryPool_[kONLY_ONCE_ALLOCATOR_SIZE_MAX];
#endif

static uint8_t* next_ = 0;
static uint8_t* end_ = 0;

static size_t totalAllocatedSize_ = 0;

static void* allocate(size_t size);
static void deallocate(void* ptr);

static inline uintptr_t next_aligned_address(const uintptr_t addr) {
	return ((addr + (kALIGNMENT_UNIT - 1)) & ~(uintptr_t)(kALIGNMENT_UNIT - 1));
}

/**
 * @brief	Initialize
 * @return	none
 */
void OnlyOnceAllocator_initialize(void)
{
	allocator_.allocate = allocate;
	allocator_.deallocate = deallocate;
	Allocator_initialize(&allocator_);

	next_ = (uint8_t*)next_aligned_address((uintptr_t)&memoryPool_[0]);
	end_ = &memoryPool_[kONLY_ONCE_ALLOCATOR_SIZE_MAX];
	totalAllocatedSize_ = 0;
}

/**
 * @brief	Terminate
 * @return	none
 */
void OnlyOnceAllocator_terminate(void)
{
	totalAllocatedSize_ = 0;
	Allocator_terminate();
}

/**
 * @brief	Get total number of memory allocation requests
 * @return	total number of memory allocation requests
 */
unsigned long OnlyOnceAllocator_totalAllocationRequests(void)
{
	return Allocator_totalAllocationRequests();
}

/**
 * @brief	Get total number of memory deallocation requests
 * @return	total number of memory deallocation requests
 */
unsigned long OnlyOnceAllocator_totalDeallocationRequests(void)
{
	return Allocator_totalDeallocationRequests();
}

/**
 * @brief	Get total size of allocated memory
 * @return	total size of allocated memory
 */
size_t OnlyOnceAllocator_totalAllocatedSize(void)
{
	return totalAllocatedSize_;
}

/**
 * @brief	Get total capacity of allocatable memory
 * @return	total capacity of allocatable memory
 */
size_t OnlyOnceAllocator_allocatableSizeMax(void)
{
	return (kONLY_ONCE_ALLOCATOR_SIZE_MAX & ~(uintptr_t)(kALIGNMENT_UNIT - 1));
}

/**
 * @brief	Allocate memory block
 * @param	size			size in bytes
 * @retval	!=NULL			success (a pointer to the memory block)
 * @retval	NULL			failure
 */
static void* allocate(const size_t size)
{
	ASSERT_(((uintptr_t)next_ + size) <= (uintptr_t)end_);

	void* const ptr = next_;
	next_ = (uint8_t*)next_aligned_address((uintptr_t)next_ + size);
	totalAllocatedSize_ += (next_ - (uint8_t*)ptr);

	return ptr;
}

/**
 * @brief	Deallocate memory block
 * @param	ptr				pointer to the memory block
 * @return	none
 */
static void deallocate(void* const ptr)
{
	/*! @attention This memory allocator does not support release of memory. */
	ASSERT_(kASSERT_FAILURE);

	(void)ptr;
}
