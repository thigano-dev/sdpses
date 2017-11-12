/**
 * @file	gpio.h
 * @brief	GPIO(General-purpose input/output)
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

#ifndef SDPSES_DEVICE_GPIO_H_INCLUDED_
#define SDPSES_DEVICE_GPIO_H_INCLUDED_

#include <stddef.h>
#include <stdint.h>

/**
 * @brief	Callback Function
 * @param	callback_arg	arguments for the callback function
 * @param	status			interrupt status
 * @return	none
 */
typedef void (*Gpio_CallbackFunc)(void* callback_arg, uint32_t status);

struct Gpio;

struct Gpio* Gpio_destroy(struct Gpio* self);

void Gpio_writeData(struct Gpio* self, uint32_t data);
uint32_t Gpio_readData(struct Gpio* self);

void Gpio_setDataBit(struct Gpio* self, uint32_t bitmask);
void Gpio_clearDataBit(struct Gpio* self, uint32_t bitmask);

void Gpio_writeDirection(struct Gpio* self, uint32_t direction);
uint32_t Gpio_readDirection(struct Gpio* self);

void Gpio_setOutputBit(struct Gpio* self, uint32_t bitmask);
void Gpio_setInputBit(struct Gpio* self, uint32_t bitmask);

int Gpio_setupInterrupt(struct Gpio* self, uint32_t interrupt_bits,
		Gpio_CallbackFunc callback_func, void* callback_arg);

void Gpio_enableMultipleInterrupts(struct Gpio* self, uint32_t bitmask);
void Gpio_disableMultipleInterrupts(struct Gpio* self, uint32_t bitmask);

void Gpio_enableInterrupt(struct Gpio* self);
void Gpio_disableInterrupt(struct Gpio* self);

#endif /* SDPSES_DEVICE_GPIO_H_INCLUDED_ */
