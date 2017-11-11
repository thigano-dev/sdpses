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
	const FreeRunCounter* const freeRunCounter = FreeRunCounter_getInstance();

	// wait
	freeRunCounter->waitUsec(5);

	// timeout
	const uint32_t baseCount = freeRunCounter->now();
	const uint32_t timeoutCount = freeRunCounter->convertMsecToCount(100);
	while (...) {
		if (freeRunCounter->timeout(baseCount, timeoutCount)) { break; }
	}

	// measure
	const uint32_t startCount = freeRunCounter->now();
	(proces something)
	const uint32_t endCount = freeRunCounter->now();
	const uint32_t durationTimeMsec = freeRunCounter->measureDurationMsec(startCount, endCount);
	@endcode
 */

#ifndef SDPSES_DEVICE_FREE_RUN_COUNTER_H_INCLUDED_
#define SDPSES_DEVICE_FREE_RUN_COUNTER_H_INCLUDED_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct FreeRunCounter FreeRunCounter;

/**
 * @brief	Get instance
 * @return	instance
 */
const FreeRunCounter* FreeRunCounter_getInstance(void);

/**
 * @struct	FreeRunCounter
 * @brief	FreeRunCounter struct
 * @note	Direct instantiation is prohibited.
 */
struct FreeRunCounter {

	uint32_t (*now)(void);

	uint32_t (*convertNsecToCount)(uint32_t nsec);
	uint32_t (*convertUsecToCount)(uint32_t usec);
	uint32_t (*convertMsecToCount)(uint32_t msec);

	bool (*timeout)(uint32_t base_count, uint32_t timeout_count);

	void (*waitNsec)(uint32_t nsec);
	void (*waitUsec)(uint32_t usec);
	void (*waitMsec)(uint32_t msec);

	uint32_t (*measureDurationNsec)(uint32_t start_count, uint32_t end_count);
	uint32_t (*measureDurationUsec)(uint32_t start_count, uint32_t end_count);
	uint32_t (*measureDurationMsec)(uint32_t start_count, uint32_t end_count);
};

#endif /* SDPSES_DEVICE_FREE_RUN_COUNTER_H_INCLUDED_ */
