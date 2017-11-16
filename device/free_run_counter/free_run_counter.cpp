/**
 * @file	free_run_counter.cpp
 * @brief	free-running counter
 * @author	Tsuguyoshi Higano
 * @date	Nov 17, 2017
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
#include "static_frc_timer_factory.h"
#include "timer.h"
#include "lib_assert.h"

namespace sdpses {

namespace device {

/**
 * @brief	Get instance
 * @return	instance
 */
const FreeRunCounter& FreeRunCounter::getInstance()
{
	static const FreeRunCounter instance;
	return instance;
}

/**
 * @brief	Constructor
 */
FreeRunCounter::FreeRunCounter()
	: timer_(StaticFrcTimerFactory::getInstance())
	, kCOUNTS_PER_1024NSEC((timer_.getFrequency() + (976562 - 1)) / 976562)
	, kCOUNTS_PER_USEC((timer_.getFrequency() + (1000000 - 1)) / 1000000)
	, kCOUNTS_PER_MSEC((timer_.getFrequency() + (1000 - 1)) / 1000)
	, kMEASUREMENT_UNIT_1024NSEC(timer_.getFrequency() / 976562)
	, kMEASUREMENT_UNIT_USEC(timer_.getFrequency() / 1000000)
	, kMEASUREMENT_UNIT_MSEC(timer_.getFrequency() / 1000)
{
#if defined(USE_FRC_COUNT_UP_TYPE_)
	const Timer::CountMethod countMethod = Timer::kCOUNT_METHOD_UP;
#else
	const Timer::CountMethod countMethod = Timer::kCOUNT_METHOD_DOWN;
#endif
	const uint32_t loadCountValue = 0xFFFFFFFF;
	const Timer::CountParams countParams(countMethod, Timer::kRELOAD_ENABLE, loadCountValue);
	timer_.setup(countParams);
	timer_.start();
}

/**
 * @brief	Destructor
 */
FreeRunCounter::~FreeRunCounter()
{
}

/**
 * @brief	Return the difference between counts
 * @param	start_count		start counter value
 * @param	end_count		end counter value
 * @return	the difference between counts
 */
inline uint32_t FreeRunCounter::diff_count(const uint32_t start_count, const uint32_t end_count) const {
#if defined(USE_FRC_COUNT_UP_TYPE_)
	return (end_count - start_count);
#else
	return (start_count - end_count);
#endif
}

/**
 * @brief	Get current counter value
 * @return	counter value
 */
uint32_t FreeRunCounter::now() const
{
	return timer_.readCounter();
}

/**
 * @brief	Convert nanoseconds to count
 * @param	nsec			nanoseconds
 * @return	relative counter value
 */
uint32_t FreeRunCounter::convertNsecToCount(const uint32_t nsec) const
{
	ASSERT_(nsec < ((UINT32_MAX - (1024 - 1)) / kCOUNTS_PER_1024NSEC));
	return (((kCOUNTS_PER_1024NSEC * nsec) + (1024 - 1)) >> 10);
}

/**
 * @brief	Convert microseconds to count
 * @param	usec			microseconds
 * @return	relative counter value
 */
uint32_t FreeRunCounter::convertUsecToCount(const uint32_t usec) const
{
	ASSERT_(usec < (UINT32_MAX / kCOUNTS_PER_USEC));
	return (kCOUNTS_PER_USEC * usec);
}

/**
 * @brief	Convert milliseconds to count
 * @param	msec			milliseconds
 * @return	relative counter value
 */
uint32_t FreeRunCounter::convertMsecToCount(const uint32_t msec) const
{
	ASSERT_(msec < (UINT32_MAX / kCOUNTS_PER_MSEC));
	return (kCOUNTS_PER_MSEC * msec);
}

/**
 * @brief	Timeout
 * @param	base_count		base counter value
 * @param	timeout_count	count until timeout(relative counter value)
 * @retval	true			timed out
 * @retval	false			not timed out
 */
bool FreeRunCounter::timeout(const uint32_t base_count, const uint32_t timeout_count) const
{
	if (diff_count(base_count, timer_.readCounter()) < timeout_count) { return false; }
	return true;
}

/**
 * @brief	Wait [nanoseconds]
 * @param	nsec			nanoseconds
 * @return	none
 */
void FreeRunCounter::waitNsec(const uint32_t nsec) const
{
	ASSERT_(nsec < ((UINT32_MAX - (1024 - 1)) / kCOUNTS_PER_1024NSEC));

	const uint32_t baseCount = timer_.readCounter();
	const uint32_t timeoutCount = (((kCOUNTS_PER_1024NSEC * nsec) + (1024 - 1)) >> 10);
	while (diff_count(baseCount, timer_.readCounter()) < timeoutCount);
}

/**
 * @brief	Wait [microseconds]
 * @param	usec			microseconds
 * @return	none
 */
void FreeRunCounter::waitUsec(const uint32_t usec) const
{
	ASSERT_(usec < (UINT32_MAX / kCOUNTS_PER_USEC));

	const uint32_t baseCount = timer_.readCounter();
	const uint32_t timeoutCount = (kCOUNTS_PER_USEC * usec);
	while (diff_count(baseCount, timer_.readCounter()) < timeoutCount);
}

/**
 * @brief	Wait [milliseconds]
 * @param	msec			milliseconds
 * @return	none
 */
void FreeRunCounter::waitMsec(const uint32_t msec) const
{
	ASSERT_(msec < (UINT32_MAX / kCOUNTS_PER_MSEC));

	const uint32_t baseCount = timer_.readCounter();
	const uint32_t timeoutCount = (kCOUNTS_PER_MSEC * msec);
	while (diff_count(baseCount, timer_.readCounter()) < timeoutCount);
}

/**
 * @brief	Measure time duration between counter values [nanoseconds]
 * @param	start_count		start counter value
 * @param	end_count		end counter value
 * @return	nanoseconds
 */
uint32_t FreeRunCounter::measureDurationNsec(const uint32_t start_count, const uint32_t end_count) const
{
	const uint32_t diffCount = diff_count(start_count, end_count);
	if (diffCount & 0xFFC00000UL) {
		return (((diffCount + (kMEASUREMENT_UNIT_1024NSEC - 1)) / kMEASUREMENT_UNIT_1024NSEC) << 10);
	}
	return (((diffCount << 10) + (kMEASUREMENT_UNIT_1024NSEC - 1)) / kMEASUREMENT_UNIT_1024NSEC);
}

/**
 * @brief	Measure time duration between counter values [microseconds]
 * @param	start_count		start counter value
 * @param	end_count		end counter value
 * @return	microseconds
 */
uint32_t FreeRunCounter::measureDurationUsec(const uint32_t start_count, const uint32_t end_count) const
{
	const uint32_t diffCount = diff_count(start_count, end_count);
	return ((diffCount + (kMEASUREMENT_UNIT_USEC - 1)) / kMEASUREMENT_UNIT_USEC);
}

/**
 * @brief	Measure time duration between counter values [milliseconds]
 * @param	start_count		start counter value
 * @param	end_count		end counter value
 * @return	milliseconds
 */
uint32_t FreeRunCounter::measureDurationMsec(const uint32_t start_count, const uint32_t end_count) const
{
	const uint32_t diffCount = diff_count(start_count, end_count);
	return ((diffCount + (kMEASUREMENT_UNIT_MSEC - 1)) / kMEASUREMENT_UNIT_MSEC);
}

} /* namespace device */

} /* namespace sdpses */
