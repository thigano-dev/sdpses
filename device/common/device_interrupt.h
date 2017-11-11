/**
 * @file	device_interrupt.h
 * @brief	device interrupt
 * @author	Tsuguyoshi Higano
 * @date	Nov 11, 2017
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

#ifndef SDPSES_DEVICE_DEVICE_INTERRUPT_H_INCLUDED_
#define SDPSES_DEVICE_DEVICE_INTERRUPT_H_INCLUDED_

#include <stdint.h>

/*--- ALTERA(intel) Nios II --------------------------------------------------*/
#if defined(__NIOS2__)
#include <sys/alt_irq.h>
typedef alt_isr_func interrupt_function_t;
typedef alt_irq_context interrupt_context_t;

static inline void device_interrupt_register(const uint32_t intc, const uint32_t irq,
		const interrupt_function_t func, void* const context) {
	alt_ic_isr_register(intc, irq, func, context, (void*)0);
}
static inline void device_interrupt_enable(const uint32_t intc, const uint32_t irq) {
	alt_ic_irq_enable(intc, irq);
}
static inline void device_interrupt_disable(const uint32_t intc, const uint32_t irq) {
	alt_ic_irq_disable(intc, irq);
}
static inline void device_interrupt_clear(const uint32_t intc, const uint32_t irq) {
	/* do nothing */
}
static inline interrupt_context_t device_interrupt_disable_all(void) {
	return alt_irq_disable_all();
}
static inline void device_interrupt_enable_all(const interrupt_context_t context) {
	alt_irq_enable_all(context);
}

/*--- XILINX MicroBlaze ------------------------------------------------------*/
#elif defined(__MICROBLAZE__)
#include "xintc_l.h"
typedef XInterruptHandler interrupt_function_t;
typedef int interrupt_context_t;

static inline void device_interrupt_register(const uint32_t intc, const uint32_t irq,
		const interrupt_function_t func, void* const context) {
	XIntc_RegisterHandler(intc, irq, func, context);
}
static inline void device_interrupt_enable(const uint32_t intc, const uint32_t irq) {
	XIntc_EnableIntr(intc, (1UL << irq));
}
static inline void device_interrupt_disable(const uint32_t intc, const uint32_t irq) {
	XIntc_DisableIntr(intc, (1UL << irq));
}
static inline void device_interrupt_clear(const uint32_t intc, const uint32_t irq) {
	XIntc_AckIntr(intc, (1UL << irq));
}
static inline interrupt_context_t device_interrupt_disable_all(void) {
	microblaze_disable_interrupts();
	return 0;
}
static inline void device_interrupt_enable_all(const interrupt_context_t context) {
	microblaze_enable_interrupts();
}

/*--- Unknown Processor Type -------------------------------------------------*/
#else
#error "unknown processor type."

#endif

#endif /* SDPSES_DEVICE_DEVICE_INTERRUPT_H_INCLUDED_ */
