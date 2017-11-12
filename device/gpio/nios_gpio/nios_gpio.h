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

namespace sdpses {

namespace device {

/**
 * @class	NiosGpio
 * @brief	NiosGpio class
 * @note	Don't inherit from this class.
 */
class NiosGpio : public Gpio {

public:
	enum InterruptTrigger { kTRG_LEVEL, kTRG_EDGE };

	NiosGpio(uint32_t base_addr, uint32_t ic_id, uint32_t irq, InterruptTrigger int_trg = kTRG_LEVEL);
	explicit NiosGpio(uint32_t base_addr);
	~NiosGpio();

	void writeData(uint32_t data);
	uint32_t readData() const;

	void writeDirection(uint32_t direction);
	uint32_t readDirection() const;

	int setupInterrupt(uint32_t interrupt_bits,
			CallbackFunc callback_func, void* callback_arg);

	void enableMultipleInterrupts(uint32_t bitmask);
	void disableMultipleInterrupts(uint32_t bitmask);

	void enableInterrupt();
	void disableInterrupt();

private:
	NiosGpio();
	NiosGpio(const NiosGpio&);
	NiosGpio& operator=(const NiosGpio&);

	const uint32_t kBASE_ADDR;
	const uint32_t kIC_ID;
	const uint32_t kIRQ;
	const InterruptTrigger kINT_TRG;

	uint32_t interruptFlags_;

	CallbackFunc callbackFunc_;
	void* callbackArg_;

	static void interruptServiceRoutine(void* isr_context);
};

} /* namespace device */

} /* namespace sdpses */

#endif /* SDPSES_DEVICE_NIOS_GPIO_H_INCLUDED_ */
