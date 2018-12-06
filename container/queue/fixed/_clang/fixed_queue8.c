/**
 * @file	fixed_queue8.c
 * @brief	fixed-size queue8
 * @author	Tsuguyoshi Higano
 * @date	Dec 06, 2018
 *
 * @par Project
 * Software Development Platform for Small-scale Embedded Systems (SDPSES)
 *
 * @copyright (c) Tsuguyoshi Higano, 2017-2018
 *
 * @par License
 * Released under the MIT license@n
 * http://opensource.org/licenses/mit-license.php
 */

#include "allocator.h"

#include "fixed_queue8.h"
#include "lib_assert.h"
#include "lib_debug.h"

struct FixedQueue8 {
	size_t sizeMax;
	size_t head;
	size_t tail;
	size_t size;
	uint8_t* elements;
};

/**
 * @brief	Get the size of FixedQueue8
 * @return	the size of FixedQueue8
 */
size_t FixedQueue8_sizeOf(void)
{
	return sizeof(FixedQueue8);
}

/**
 * @brief	Create
 * @param	size_max		the maximum number of elements
 * @return	instance
 */
FixedQueue8* FixedQueue8_create(const size_t size_max)
{
	FixedQueue8* const instance = Allocator_allocate(sizeof(FixedQueue8));
	if (!instance) {
		FATAL_("Cannot allocate memory\r\n");
		return NULL;
	}

	if (FixedQueue8_ctor(instance, size_max)) {
		Allocator_deallocate(instance);
		return NULL;
	}

	return instance;
}

/**
 * @brief	Destroy
 * @param	self			FixedQueue8*
 * @return	FixedQueue8*
 */
FixedQueue8* FixedQueue8_destroy(FixedQueue8* const self)
{
	if (!self) { return NULL; }

	FixedQueue8_dtor(self);
	Allocator_deallocate(self);

	return NULL;
}

/**
 * @brief	Constructor
 * @param	self			FixedQueue8*
 * @param	size_max		the maximum number of elements
 * @retval	0				success
 * @retval	!=0				failure
 */
int FixedQueue8_ctor(FixedQueue8* const self, const size_t size_max)
{
	if (size_max == 0) { return 1; }

	self->sizeMax = size_max;
	self->elements = Allocator_allocate(sizeof(uint8_t) * size_max);
	if (!self->elements) {
		FATAL_("Cannot allocate memory\r\n");
		return 1;
	}

	FixedQueue8_clear(self);

	return 0;
}

/**
 * @brief	Destructor
 * @param	self			FixedQueue8*
 * @return	none
 */
void FixedQueue8_dtor(FixedQueue8* const self)
{
	if (!self) { return; }

	Allocator_deallocate(self->elements);
}

/**
 * @brief	Removes all elements
 * @param	self			FixedQueue8*
 * @return	none
 */
void FixedQueue8_clear(FixedQueue8* const self)
{
	self->head = 0;
	self->tail = 0;
	self->size = 0;
}

/**
 * @brief	Inserts a element
 * @param	self			FixedQueue8*
 * @param	element			element
 * @return	none
 *
 * @pre not full
 */
void FixedQueue8_push(FixedQueue8* const self, const uint8_t element)
{
	ASSERT_(self->size < self->sizeMax);

	self->elements[self->tail] = element;
	if (++self->tail == self->sizeMax) { self->tail = 0; }
	self->size++;
}

/**
 * @brief	Removes the next element
 * @brief	Remove next element
 * @param	self			FixedQueue8*
 * @return	none
 *
 * @pre not empty
 */
void FixedQueue8_pop(FixedQueue8* const self)
{
	ASSERT_(self->size > 0);

	if (++self->head == self->sizeMax) { self->head = 0; }
	self->size--;
}

/**
 * @brief	Returns the next element
 * @param	self			FixedQueue8*
 * @return	the next element
 *
 * @pre not empty
 */
uint8_t FixedQueue8_front(const FixedQueue8* const self)
{
	ASSERT_(self->size > 0);

	return self->elements[self->head];
}

/**
 * @brief	Is empty
 * @param	self			FixedQueue8*
 * @retval	true			empty
 * @retval	false			not empty
 */
bool FixedQueue8_empty(const FixedQueue8* const self)
{
	return (self->size) ? false : true;
}

/**
 * @brief	Is full
 * @param	self			FixedQueue8*
 * @retval	true			full
 * @retval	false			not full
 */
bool FixedQueue8_full(const FixedQueue8* const self)
{
	return (self->size < self->sizeMax) ? false : true;
}

/**
 * @brief	Returns the number of elements
 * @param	self			FixedQueue8*
 * @return	the number of elements
 */
size_t FixedQueue8_size(const FixedQueue8* const self)
{
	return self->size;
}

/**
 * @brief	Returns the number of storable elements
 * @param	self			FixedQueue8*
 * @return	the number of storable elements
 */
size_t FixedQueue8_availableSize(const FixedQueue8* const self)
{
	return (self->sizeMax - self->size);
}

/**
 * @brief	Returns the maximum number of elements
 * @param	self			FixedQueue8*
 * @return	the maximum number of elements
 */
size_t FixedQueue8_maxSize(const FixedQueue8* const self)
{
	return self->sizeMax;
}
