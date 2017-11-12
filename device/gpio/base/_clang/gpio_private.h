/**
 * @file	gpio_private.h
 * @brief	GPIO(General-purpose input/output) private
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

#ifndef SDPSES_DEVICE_GPIO_PRIVATE_H_INCLUDED_
#define SDPSES_DEVICE_GPIO_PRIVATE_H_INCLUDED_

#include "gpio.h"

struct Gpio {
	struct Gpio* (*destroy)(struct Gpio* self);

	void (*writeData)(struct Gpio* self, uint32_t data);
	uint32_t (*readData)(struct Gpio* self);

	void (*writeDirection)(struct Gpio* self, uint32_t direction);
	uint32_t (*readDirection)(struct Gpio* self);

	int (*setupInterrupt)(struct Gpio* self, uint32_t interrupt_bits,
			Gpio_CallbackFunc callback_func, void* callback_arg);

	void (*enableMultipleInterrupts)(struct Gpio* self, uint32_t bitmask);
	void (*disableMultipleInterrupts)(struct Gpio* self, uint32_t bitmask);

	void (*enableInterrupt)(struct Gpio* self);
	void (*disableInterrupt)(struct Gpio* self);
};

int Gpio_ctor(struct Gpio* const self);
void Gpio_dtor(struct Gpio* const self);

#endif /* SDPSES_DEVICE_GPIO_PRIVATE_H_INCLUDED_ */
