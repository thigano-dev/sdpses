/**
 * @file	nios_timer.h
 * @brief	Altera Avalon Timer
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

#ifndef SDPSES_DEVICE_NIOS_TIMER_H_INCLUDED_
#define SDPSES_DEVICE_NIOS_TIMER_H_INCLUDED_

#include "timer.h"

struct NiosTimer;

size_t NiosTimer_sizeOf(void);

struct NiosTimer* NiosTimer_create(uint32_t base_addr, uint32_t freq);
struct NiosTimer* NiosTimer_createWithInterrupt(uint32_t base_addr,
		uint32_t freq, uint32_t ic_id, uint32_t irq);
struct Timer* NiosTimer_destroy(struct Timer* self);

int NiosTimer_ctor(struct NiosTimer* instance, uint32_t base_addr, uint32_t freq);
int NiosTimer_ctorWithInterrupt(struct NiosTimer* instance,
		uint32_t base_addr, uint32_t freq, uint32_t ic_id, uint32_t irq);
void NiosTimer_dtor(struct NiosTimer* instance);

int NiosTimer_setup(struct Timer* self, const TimerCountParams* params);

void NiosTimer_start(struct Timer* self);
void NiosTimer_stop(struct Timer* self);

uint32_t NiosTimer_readCounter(const struct Timer* self);
uint32_t NiosTimer_getFrequency(const struct Timer* self);

int NiosTimer_setupInterrupt(struct Timer* self,
		GenCallbackFunc callback_func, void* callback_arg);
void NiosTimer_enableInterrupt(struct Timer* self);
void NiosTimer_disableInterrupt(struct Timer* self);

#endif /* SDPSES_DEVICE_NIOS_TIMER_H_INCLUDED_ */
