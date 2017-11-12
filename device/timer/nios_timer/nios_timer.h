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

namespace sdpses {

namespace device {

/**
 * @class	NiosTimer
 * @brief	NiosTimer class
 * @note	Don't inherit from this class.
 */
class NiosTimer : public Timer {

public:
	NiosTimer(uint32_t base_addr, uint32_t freq, uint32_t ic_id, uint32_t irq);
	NiosTimer(uint32_t base_addr, uint32_t freq);
	~NiosTimer();

	int setup(const CountParams& count_params);

	void start();
	void stop();

	uint32_t readCounter() const;
	uint32_t getFrequency() const;

	int setupInterrupt(GenCallbackFunc callback_func, void* callback_arg);
	void enableInterrupt();
	void disableInterrupt();

private:
	NiosTimer();
	NiosTimer(const NiosTimer&);
	NiosTimer& operator=(const NiosTimer&);

	const uint32_t kBASE_ADDR;
	const uint32_t kFREQ;
	const uint32_t kIC_ID;
	const uint32_t kIRQ;

	uint32_t controlFlags_;

	GenCallbackFunc	callbackFunc_;
	void*			callbackArg_;

	static void interruptServiceRoutine(void* isr_context);
};

} /* namespace device */

} /* namespace sdpses */

#endif /* SDPSES_DEVICE_NIOS_TIMER_H_INCLUDED_ */
