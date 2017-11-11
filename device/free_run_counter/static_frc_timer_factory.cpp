/**
 * @file	static_frc_timer_factory.cpp
 * @brief	static free-running counter timer factory
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

#include "system_parameter_definition.h"
#include "static_frc_timer_factory.h"

#if defined(__NIOS2__)
#  include "nios_timer.h" /*!< only count-down */
#elif defined(__MICROBLAZE__)
#  include "mb_timer.h"
#else
#  include "cclock_timer.h"
#endif

namespace sdpses {

namespace device {

StaticFrcTimerFactory::StaticFrcTimerFactory()
{
}

StaticFrcTimerFactory::~StaticFrcTimerFactory()
{
}

Timer& StaticFrcTimerFactory::getInstance()
{
#if defined(__NIOS2__)
	static NiosTimer timer(FREE_RUN_TIMER_BASE, FREE_RUN_TIMER_FREQ);
#elif defined(__MICROBLAZE__)
	static MbTimer timer(FREE_RUN_TIMER_BASE, FREE_RUN_TIMER_FREQ);
#else
	static CclockTimer timer;
#endif

	return timer;
}

} /* namespace device */

} /* namespace sdpses */
