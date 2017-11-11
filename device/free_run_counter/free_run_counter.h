/**
 * @file	free_run_counter.h
 * @brief	free-running counter
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

/**
	@code sample
	// get instance
	const FreeRunCounter& freeRunCounter = FreeRunCounter::getInstance();

	// wait
	freeRunCounter.waitUsec(5);

	// timeout
	const uint32_t baseCount = freeRunCounter.now();
	const uint32_t timeoutCount = freeRunCounter.convertMsecToCount(100);
	while (...) {
		if (freeRunCounter.timeout(baseCount, timeoutCount)) { break; }
	}

	// measure
	const uint32_t startCount = freeRunCounter.now();
	(proces something)
	const uint32_t endCount = freeRunCounter.now();
	const uint32_t durationTimeMsec = freeRunCounter.measureDurationMsec(startCount, endCount);
	@endcode
 */

#ifndef SDPSES_DEVICE_FREE_RUN_COUNTER_H_INCLUDED_
#define SDPSES_DEVICE_FREE_RUN_COUNTER_H_INCLUDED_

#include <stdint.h>

namespace sdpses {

namespace device {

class Timer;

/**
 * @class	FreeRunCounter
 * @brief	FreeRunCounter class
 * @note	Don't inherit from this class.
 */
class FreeRunCounter {

public:
	~FreeRunCounter();

	static const FreeRunCounter& getInstance();

	uint32_t now() const;

	uint32_t convertNsecToCount(uint32_t nsec) const;
	uint32_t convertUsecToCount(uint32_t usec) const;
	uint32_t convertMsecToCount(uint32_t msec) const;

	bool timeout(uint32_t base_count, uint32_t timeout_count) const;

	void waitNsec(uint32_t nsec) const;
	void waitUsec(uint32_t usec) const;
	void waitMsec(uint32_t msec) const;

	uint32_t measureDurationNsec(uint32_t start_count, uint32_t end_count) const;
	uint32_t measureDurationUsec(uint32_t start_count, uint32_t end_count) const;
	uint32_t measureDurationMsec(uint32_t start_count, uint32_t end_count) const;

private:
	FreeRunCounter();
	FreeRunCounter(const FreeRunCounter&);
	FreeRunCounter& operator=(const FreeRunCounter&);

	uint32_t diff_count(uint32_t start_count, uint32_t end_count) const;

	Timer& timer_;

	const uint32_t kCOUNTS_PER_1024NSEC;
	const uint32_t kCOUNTS_PER_USEC;
	const uint32_t kCOUNTS_PER_MSEC;

	const uint32_t kMEASUREMENT_UNIT_1024NSEC;
	const uint32_t kMEASUREMENT_UNIT_USEC;
	const uint32_t kMEASUREMENT_UNIT_MSEC;
};

} /* namespace device */

} /* namespace sdpses */

#endif /* SDPSES_DEVICE_FREE_RUN_COUNTER_H_INCLUDED_ */
