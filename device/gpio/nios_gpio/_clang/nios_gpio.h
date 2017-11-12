/**
 * @file	nios_gpio.h
 * @brief	Altera Avalon PIO
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

#ifndef SDPSES_DEVICE_NIOS_GPIO_H_INCLUDED_
#define SDPSES_DEVICE_NIOS_GPIO_H_INCLUDED_

#include "gpio.h"

typedef enum {
	kNIOS_GPIO_TRG_LEVEL,
	kNIOS_GPIO_TRG_EDGE
} NiosGpio_InterruptTrigger;

struct NiosGpio;

size_t NiosGpio_sizeOf(void);

struct NiosGpio* NiosGpio_create(uint32_t base_addr);
struct NiosGpio* NiosGpio_createWithInterrupt(uint32_t base_addr,
		uint32_t ic_id, uint32_t irq, NiosGpio_InterruptTrigger int_trg);
struct Gpio* NiosGpio_destroy(struct Gpio* self);

int NiosGpio_ctor(struct NiosGpio* instance, uint32_t base_addr);
int NiosGpio_ctorWithInterrupt(struct NiosGpio* instance,
		uint32_t base_addr, uint32_t ic_id, uint32_t irq, NiosGpio_InterruptTrigger int_trg);
void NiosGpio_dtor(struct NiosGpio* instance);

void NiosGpio_writeData(struct Gpio* self, uint32_t data);
uint32_t NiosGpio_readData(struct Gpio* self);

void NiosGpio_setDataBit(struct Gpio* self, uint32_t bitmask);
void NiosGpio_clearDataBit(struct Gpio* self, uint32_t bitmask);

void NiosGpio_writeDirection(struct Gpio* self, uint32_t direction);
uint32_t NiosGpio_readDirection(struct Gpio* self);

void NiosGpio_setOutputBit(struct Gpio* self, uint32_t bitmask);
void NiosGpio_setInputBit(struct Gpio* self, uint32_t bitmask);

int NiosGpio_setupInterrupt(struct Gpio* self, uint32_t interrupt_bits,
		Gpio_CallbackFunc callback_func, void* callback_arg);

void NiosGpio_enableMultipleInterrupts(struct Gpio* self, uint32_t bitmask);
void NiosGpio_disableMultipleInterrupts(struct Gpio* self, uint32_t bitmask);

void NiosGpio_enableInterrupt(struct Gpio* self);
void NiosGpio_disableInterrupt(struct Gpio* self);

#endif /* SDPSES_DEVICE_NIOS_GPIO_H_INCLUDED_ */
