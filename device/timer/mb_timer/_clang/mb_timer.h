/**
 * @file	mb_timer.h
 * @brief	Xilinx timer/counter
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

#ifndef SDPSES_DEVICE_MB_TIMER_H_INCLUDED_
#define SDPSES_DEVICE_MB_TIMER_H_INCLUDED_

#include "timer.h"

struct MbTimer;

size_t MbTimer_sizeOf(void);

struct MbTimer* MbTimer_create(uint32_t base_addr, uint32_t freq);
struct MbTimer* MbTimer_createWithInterrupt(uint32_t base_addr,
		uint32_t freq, uint32_t ic_base, uint32_t irq);
struct Timer* MbTimer_destroy(struct Timer* self);

int MbTimer_ctor(struct MbTimer* instance,
		uint32_t base_addr, uint32_t freq);
int MbTimer_ctorWithInterrupt(struct MbTimer* instance, uint32_t base_addr,
		uint32_t freq, uint32_t ic_base, uint32_t irq);
void MbTimer_dtor(struct MbTimer* instance);

int MbTimer_setup(struct Timer* self, const TimerCountParams* params);

void MbTimer_start(struct Timer* self);
void MbTimer_stop(struct Timer* self);

uint32_t MbTimer_readCounter(const struct Timer* self);
uint32_t MbTimer_getFrequency(const struct Timer* self);

int MbTimer_setupInterrupt(struct Timer* self,
		GenCallbackFunc callback_func, void* callback_arg);
void MbTimer_enableInterrupt(struct Timer* self);
void MbTimer_disableInterrupt(struct Timer* self);

#endif /* SDPSES_DEVICE_MB_TIMER_H_INCLUDED_ */
