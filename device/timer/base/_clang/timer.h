/**
 * @file	timer.h
 * @brief	timer
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

#ifndef SDPSES_DEVICE_TIMER_H_INCLUDED_
#define SDPSES_DEVICE_TIMER_H_INCLUDED_

#include <stddef.h>
#include <stdint.h>

#include "lib_callback.h"

typedef enum {
	kTIMER_COUNT_METHOD_UP,
	kTIMER_COUNT_METHOD_DOWN,

	kTIMER_COUNT_METHOD_DEFAULT = kTIMER_COUNT_METHOD_DOWN
} TimerCountMethod;

typedef enum {
	kTIMER_RELOAD_DISABLE,
	kTIMER_RELOAD_ENABLE,

	kTIMER_RELOAD_DEFAULT = kTIMER_RELOAD_ENABLE
} TimerReload;

static const uint32_t kLOAD_COUNT_VALUE_DEFAULT = 0xFFFFFFFFUL;

typedef struct {
	TimerCountMethod method;
	TimerReload      reload;
	uint32_t         loadCountValue;
} TimerCountParams;

struct Timer;

struct Timer* Timer_destroy(struct Timer* self);

int Timer_setup(struct Timer* self, const TimerCountParams* params);

void Timer_start(struct Timer* self);
void Timer_stop(struct Timer* self);

uint32_t Timer_readCounter(const struct Timer* self);
uint32_t Timer_getFrequency(const struct Timer* self);

int Timer_setupInterrupt(struct Timer* self,
		GenCallbackFunc callback_func, void* callback_arg);
void Timer_enableInterrupt(struct Timer* self);
void Timer_disableInterrupt(struct Timer* self);

#endif /* SDPSES_DEVICE_TIMER_H_INCLUDED_ */
