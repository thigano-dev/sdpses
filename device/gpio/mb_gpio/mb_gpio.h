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

namespace sdpses {

namespace device {

/**
 * @class	MbGpio
 * @brief	MbGpio class
 * @note	Don't inherit from this class.
 */
class MbGpio : public Gpio {

public:
	MbGpio(uint32_t base_addr, uint32_t ic_base, uint32_t irq);
	explicit MbGpio(uint32_t base_addr);
	~MbGpio();

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
	MbGpio();
	MbGpio(const MbGpio&);
	MbGpio& operator=(const MbGpio&);

	static const uint32_t kDIRECTION_OUTPUT;
	static const uint32_t kDIRECTION_INPUT;

	static const uint32_t kPORTINT_DISABLE;
	static const uint32_t kPORTINT_ENABLE;

	const uint32_t kBASE_ADDR;
	const uint32_t kIC_BASE;
	const uint32_t kIRQ;
	const uint32_t kIRQ_MASK;

	uint32_t interruptFlags_;

	CallbackFunc	callbackFunc_;
	void*			callbackArg_;

	static void interruptHandler(void* context);
};

} /* namespace device */

} /* namespace sdpses */

#endif /* SDPSES_DEVICE_MB_GPIO_H_INCLUDED_ */
