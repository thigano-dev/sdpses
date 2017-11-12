/**
 * @file	fixed_queue8.h
 * @brief	fixed-size queue8
 * @author	Tsuguyoshi Higano
 * @date	Nov 13, 2017
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

#ifndef SDPSES_CONTAINER_FIXED_QUEUE8_H_INCLUDED_
#define SDPSES_CONTAINER_FIXED_QUEUE8_H_INCLUDED_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct FixedQueue8;
typedef struct FixedQueue8 FixedQueue8;

size_t FixedQueue8_sizeOf(void);

FixedQueue8* FixedQueue8_create(size_t size_max);
FixedQueue8* FixedQueue8_destroy(FixedQueue8* self);

int FixedQueue8_ctor(FixedQueue8* self, size_t size_max);
void FixedQueue8_dtor(FixedQueue8* self);

void FixedQueue8_clear(FixedQueue8* self);
void FixedQueue8_push(FixedQueue8* self, uint8_t element);
void FixedQueue8_pop(FixedQueue8* self);
uint8_t FixedQueue8_peek(const FixedQueue8* self);

bool FixedQueue8_empty(const FixedQueue8* self);
bool FixedQueue8_full(const FixedQueue8* self);

size_t FixedQueue8_size(const FixedQueue8* self);
size_t FixedQueue8_availableSize(const FixedQueue8* self);
size_t FixedQueue8_maxSize(const FixedQueue8* self);

#endif /* SDPSES_CONTAINER_FIXED_QUEUE8_H_INCLUDED_ */
