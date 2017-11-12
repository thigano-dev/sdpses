/**
 * @file	timer.h
 * @brief	timer
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

#ifndef SDPSES_DEVICE_TIMER_H_INCLUDED_
#define SDPSES_DEVICE_TIMER_H_INCLUDED_

#include <stdint.h>

#include "lib_callback.h"

namespace sdpses {

namespace device {

/**
 * @class	Timer
 * @brief	Timer is the abstract base class for timer.
 */
class Timer {

public:
	enum CountMethod {
		kCOUNT_METHOD_UP,
		kCOUNT_METHOD_DOWN,

		kCOUNT_METHOD_DEFAULT = kCOUNT_METHOD_DOWN
	};

	enum Reload {
		kRELOAD_DISABLE,
		kRELOAD_ENABLE,

		kRELOAD_DEFAULT = kRELOAD_ENABLE
	};

	struct CountParams {
		explicit CountParams(const CountMethod method = kCOUNT_METHOD_DEFAULT,
							 const Reload reload = kRELOAD_DEFAULT,
							 const uint32_t load_count_value = 0xFFFFFFFFUL)
			: method_(method)
			, reload_(reload)
			, loadCountValue_(load_count_value) {}
		~CountParams() {}

		const CountMethod	method_;
		const Reload		reload_;
		const uint32_t		loadCountValue_;
	};

	virtual ~Timer() {}

	/**
	 * @brief	Set up
	 * @param	params			CountParams
	 * @retval	0				success
	 * @retval	!=0				failure
	 */
	virtual int setup(const CountParams& params) = 0;

	/**
	 * @brief	Start counting
	 * @return	none
	 */
	virtual void start() = 0;

	/**
	 * @brief	Stop counting
	 * @return	none
	 */
	virtual void stop() = 0;

	/**
	 * @brief	Read counter value
	 * @return	counter value
	 */
	virtual uint32_t readCounter() const = 0;

	/**
	 * @brief	Get frequency
	 * @return	frequency
	 */
	virtual uint32_t getFrequency() const = 0;

	/**
	 * @brief	Set up interrupt
	 * @param	callback_func	callback function
	 * @param	callback_arg	arguments for the callback function
	 * @retval	0				success
	 * @retval	!=0				failure
	 */
	virtual int setupInterrupt(const GenCallbackFunc callback_func, void* const callback_arg) { return 1; }

	/**
	 * @brief	Enable interrupt
	 * @return	none
	 */
	virtual void enableInterrupt() {}

	/**
	 * @brief	Disable interrupt
	 * @return	none
	 */
	virtual void disableInterrupt() {}

protected:
	Timer() {}

private:
	Timer(const Timer&);
	Timer& operator=(const Timer&);
};

} /* namespace device */

} /* namespace sdpses */

#endif /* SDPSES_DEVICE_TIMER_H_INCLUDED_ */
