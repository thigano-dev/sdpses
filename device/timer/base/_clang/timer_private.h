/**
 * @file	timer_private.h
 * @brief	timer private
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

#ifndef SDPSES_DEVICE_TIMER_PRIVATE_H_INCLUDED_
#define SDPSES_DEVICE_TIMER_PRIVATE_H_INCLUDED_

#include "timer.h"

struct Timer {
	struct Timer* (*destroy)(struct Timer* self);

	int (*setup)(struct Timer* self, const TimerCountParams* params);

	void (*start)(struct Timer* self);
	void (*stop)(struct Timer* self);

	uint32_t (*readCounter)(const struct Timer* self);
	uint32_t (*getFrequency)(const struct Timer* self);

	int (*setupInterrupt)(struct Timer* self,
			GenCallbackFunc callback_func, void* callback_arg);
	void (*enableInterrupt)(struct Timer* self);
	void (*disableInterrupt)(struct Timer* self);
};

int Timer_ctor(struct Timer* const self);
void Timer_dtor(struct Timer* const self);

#endif /* SDPSES_DEVICE_TIMER_PRIVATE_H_INCLUDED_ */
