/**
 * @file	mb_gpio.h
 * @brief	Xilinx General Purpose I/O
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

#ifndef SDPSES_DEVICE_MB_GPIO_H_INCLUDED_
#define SDPSES_DEVICE_MB_GPIO_H_INCLUDED_

#include "gpio.h"

struct MbGpio;

size_t MbGpio_sizeOf(void);

struct MbGpio* MbGpio_create(uint32_t base_addr);
struct MbGpio* MbGpio_createWithInterrupt(uint32_t base_addr, uint32_t ic_id, uint32_t irq);
struct Gpio* MbGpio_destroy(struct Gpio* self);

int MbGpio_ctor(struct MbGpio* instance, uint32_t base_addr);
int MbGpio_ctorWithInterrupt(struct MbGpio* instance,
		uint32_t base_addr, uint32_t ic_base, uint32_t irq);
void MbGpio_dtor(struct MbGpio* instance);

void MbGpio_writeData(struct Gpio* self, uint32_t data);
uint32_t MbGpio_readData(struct Gpio* self);

void MbGpio_setDataBit(struct Gpio* self, uint32_t bitmask);
void MbGpio_clearDataBit(struct Gpio* self, uint32_t bitmask);

void MbGpio_writeDirection(struct Gpio* self, uint32_t direction);
uint32_t MbGpio_readDirection(struct Gpio* self);

void MbGpio_setOutputBit(struct Gpio* self, uint32_t bitmask);
void MbGpio_setInputBit(struct Gpio* self, uint32_t bitmask);

int MbGpio_setupInterrupt(struct Gpio* self, uint32_t interrupt_bits,
		Gpio_CallbackFunc callback_func, void* callback_arg);

void MbGpio_enableMultipleInterrupts(struct Gpio* self, uint32_t bitmask);
void MbGpio_disableMultipleInterrupts(struct Gpio* self, uint32_t bitmask);

void MbGpio_enableInterrupt(struct Gpio* self);
void MbGpio_disableInterrupt(struct Gpio* self);

#endif /* SDPSES_DEVICE_MB_GPIO_H_INCLUDED_ */
