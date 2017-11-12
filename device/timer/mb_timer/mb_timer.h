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

namespace sdpses {

namespace device {

/**
 * @class	MbTimer
 * @brief	MbTimer class
 * @note	Don't inherit from this class.
 */
class MbTimer : public Timer {

public:
	MbTimer(uint32_t base_addr, uint32_t freq, uint32_t ic_base, uint32_t irq);
	MbTimer(uint32_t base_addr, uint32_t freq);
	~MbTimer();

	int setup(const CountParams& count_params);

	void start();
	void stop();

	uint32_t readCounter() const;
	uint32_t getFrequency() const;

	int setupInterrupt(GenCallbackFunc callback_func, void* callback_arg);
	void enableInterrupt();
	void disableInterrupt();

private:
	MbTimer();
	MbTimer(const MbTimer&);
	MbTimer& operator=(const MbTimer&);

	static const uint32_t kTMR_NUM0;
	static const uint32_t kTMR_NUM1;

	const uint32_t kBASE_ADDR;
	const uint32_t kFREQ;
	const uint32_t kIC_BASE;
	const uint32_t kIRQ;
	const uint32_t kIRQ_MASK;

	uint32_t interruptFlags_;

	GenCallbackFunc callbackFunc_;
	void* callbackArg_;

	static void interruptHandler(void* context);
};

} /* namespace device */

} /* namespace sdpses */

#endif /* SDPSES_DEVICE_MB_TIMER_H_INCLUDED_ */
