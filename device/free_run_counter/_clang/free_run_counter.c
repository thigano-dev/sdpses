/**
 * @file	free_run_counter.c
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

#include "free_run_counter.h"
#include "frc_timer_factory.h"
#include "timer.h"
#include "lib_assert.h"
#include "lib_debug.h"

static int ctor(void);
//static void dtor(void);

static uint32_t now(void);
static uint32_t convertNsecToCount(uint32_t nsec);
static uint32_t convertUsecToCount(uint32_t usec);
static uint32_t convertMsecToCount(uint32_t msec);
static bool timeout(uint32_t base_count, uint32_t timeout_count);

static void waitNsec(uint32_t nsec);
static void waitUsec(uint32_t usec);
static void waitMsec(uint32_t msec);

static uint32_t measureDurationNsec(uint32_t start_count, uint32_t end_count);
static uint32_t measureDurationUsec(uint32_t start_count, uint32_t end_count);
static uint32_t measureDurationMsec(uint32_t start_count, uint32_t end_count);

static uint32_t countsPer1024Nsec_;
static uint32_t countsPerUsec_;
static uint32_t countsPerMsec_;
static uint32_t measurementUnit1024Nsec_;
static uint32_t measurementUnitUsec_;
static uint32_t measurementUnitMsec_;

static struct Timer* timer_;

static inline uint32_t diff_count(const uint32_t start_count, const uint32_t end_count) {
#if defined(USE_FRC_COUNT_UP_TYPE_)
	return (end_count - start_count);
#else
	return (start_count - end_count);
#endif
}

/**
 * @brief	Get instance
 * @return	instance
 */
const FreeRunCounter* FreeRunCounter_getInstance(void)
{
	static const FreeRunCounter instance = {
		now,
		convertNsecToCount,
		convertUsecToCount,
		convertMsecToCount,
		timeout,
		waitNsec,
		waitUsec,
		waitMsec,
		measureDurationNsec,
		measureDurationUsec,
		measureDurationMsec
	};
	static int initialized = 0;

	if (initialized) { return &instance; }

	if (ctor()) {
		DEBUG_PRINTF_("fatal error: FreeRunCounter_getInstance()\r\n");
		DYNAMIC_STOP_();
	} else {
		initialized = 1;
	}

	return &instance;
}

/**
 * @brief	Constructor
 * @retval	0				success
 * @retval	!=0				failure
 */
static int ctor(void)
{
	timer_ = FrcTimerFactory_getInstance();
	if (!timer_) {
		DEBUG_PRINTF_("error: FrcTimerFactory_getInstance()\r\n");
		return 1;
	}

#if defined(USE_FRC_COUNT_UP_TYPE_)
	const TimerCountMethod countMethod = kTIMER_COUNT_METHOD_UP;
#else
	const TimerCountMethod countMethod = kTIMER_COUNT_METHOD_DOWN;
#endif
	const uint32_t loadCountValue = 0xFFFFFFFFUL;
	const TimerCountParams params = { countMethod, kTIMER_RELOAD_ENABLE, loadCountValue };

	if (Timer_setup(timer_, &params)) {
		DEBUG_PRINTF_("error: Timer_setup()\r\n");
		return 1;
	}

	countsPer1024Nsec_	= ((Timer_getFrequency(timer_) + (976562 - 1)) / 976562);
	countsPerUsec_		= ((Timer_getFrequency(timer_) + (1000000 - 1)) / 1000000);
	countsPerMsec_		= ((Timer_getFrequency(timer_) + (1000 - 1)) / 1000);
	measurementUnit1024Nsec_	= (Timer_getFrequency(timer_) / 976562);
	measurementUnitUsec_		= (Timer_getFrequency(timer_) / 1000000);
	measurementUnitMsec_		= (Timer_getFrequency(timer_) / 1000);

	Timer_start(timer_);

	const uint32_t count1 = now();
	const uint32_t count2 = now();
	const uint32_t count3 = now();
	if ((count1 == count2) && (count1 == count3)) {
		DEBUG_PRINTF_("error: Timer_start()\r\n");
		return 1;
	}

	return 0;
}

/**
 * @brief	Destructor
 * @return	none
 */
//static void dtor(void) {}

/**
 * @brief	Get current counter value
 * @return	counter value
 */
static uint32_t now(void)
{
	return Timer_readCounter(timer_);
}

/**
 * @brief	Convert nanoseconds to count
 * @param	nsec			nanoseconds
 * @return	relative counter value
 */
static uint32_t convertNsecToCount(const uint32_t nsec)
{
	ASSERT_(nsec < ((UINT32_MAX - (1024 - 1)) / countsPer1024Nsec_));
	return (((countsPer1024Nsec_ * nsec) + (1024 - 1)) >> 10);
}

/**
 * @brief	Convert microseconds to count
 * @param	usec			microseconds
 * @return	relative counter value
 */
static uint32_t convertUsecToCount(const uint32_t usec)
{
	ASSERT_(usec < (UINT32_MAX / countsPerUsec_));
	return (countsPerUsec_ * usec);
}

/**
 * @brief	Convert milliseconds to count
 * @param	msec			milliseconds
 * @return	relative counter value
 */
static uint32_t convertMsecToCount(const uint32_t msec)
{
	ASSERT_(msec < (UINT32_MAX / countsPerMsec_));
	return (countsPerMsec_ * msec);
}

/**
 * @brief	Timeout
 * @param	base_count		base counter value
 * @param	timeout_count	count until timeout(relative counter value)
 * @retval	true			timed out
 * @retval	false			not timed out
 */
static bool timeout(const uint32_t base_count, const uint32_t timeout_count)
{
	if (diff_count(base_count, Timer_readCounter(timer_)) < timeout_count) { return false; }
	return true;
}

/**
 * @brief	Wait [nanoseconds]
 * @param	nsec			nanoseconds
 * @return	none
 */
static void waitNsec(const uint32_t nsec)
{
	ASSERT_(nsec < ((UINT32_MAX - (1024 - 1)) / countsPer1024Nsec_));

	const uint32_t baseCount = Timer_readCounter(timer_);
	const uint32_t timeoutCount = (((countsPer1024Nsec_ * nsec) + (1024 - 1)) >> 10);
	while (diff_count(baseCount, Timer_readCounter(timer_)) < timeoutCount);
}

/**
 * @brief	Wait [microseconds]
 * @param	usec			microseconds
 * @return	none
 */
static void waitUsec(const uint32_t usec)
{
	ASSERT_(usec < (UINT32_MAX / countsPerUsec_));

	const uint32_t baseCount = Timer_readCounter(timer_);
	const uint32_t timeoutCount = (countsPerUsec_ * usec);
	while (diff_count(baseCount, Timer_readCounter(timer_)) < timeoutCount);
}

/**
 * @brief	Wait [milliseconds]
 * @param	msec			milliseconds
 * @return	none
 */
static void waitMsec(const uint32_t msec)
{
	ASSERT_(msec < (UINT32_MAX / countsPerMsec_));

	const uint32_t baseCount = Timer_readCounter(timer_);
	const uint32_t timeoutCount = (countsPerMsec_ * msec);
	while (diff_count(baseCount, Timer_readCounter(timer_)) < timeoutCount);
}

/**
 * @brief	Measure time duration between counter values [nanoseconds]
 * @param	start_count		start counter value
 * @param	end_count		end counter value
 * @return	nanoseconds
 */
static uint32_t measureDurationNsec(const uint32_t start_count, const uint32_t end_count)
{
	const uint32_t diffCount = diff_count(start_count, end_count);
	if (diffCount & 0xFFC00000UL) {
		return (((diffCount + (measurementUnit1024Nsec_ - 1)) / measurementUnit1024Nsec_) << 10);
	}
	return (((diffCount << 10) + (measurementUnit1024Nsec_ - 1)) / measurementUnit1024Nsec_);
}

/**
 * @brief	Measure time duration between counter values [microseconds]
 * @param	start_count		start counter value
 * @param	end_count		end counter value
 * @return	microseconds
 */
static uint32_t measureDurationUsec(const uint32_t start_count, const uint32_t end_count)
{
	const uint32_t diffCount = diff_count(start_count, end_count);
	return ((diffCount + (measurementUnitUsec_ - 1)) / measurementUnitUsec_);
}

/**
 * @brief	Measure time duration between counter values [milliseconds]
 * @param	start_count		start counter value
 * @param	end_count		end counter value
 * @return	milliseconds
 */
static uint32_t measureDurationMsec(const uint32_t start_count, const uint32_t end_count)
{
	const uint32_t diffCount = diff_count(start_count, end_count);
	return ((diffCount + (measurementUnitMsec_ - 1)) / measurementUnitMsec_);
}
