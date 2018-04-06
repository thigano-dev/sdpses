/**
 * @file	frc_timer_factory.cpp
 * @brief	free-running counter timer factory
 * @author	Tsuguyoshi Higano
 * @date	Apr 6, 2018
 *
 * @par Project
 * Software Development Platform for Small-scale Embedded Systems (SDPSES)
 *
 * @copyright (c) Tsuguyoshi Higano, 2017-2018
 *
 * @par License
 * Released under the MIT license@n
 * http://opensource.org/licenses/mit-license.php
 */

#include "system_parameter_definition.h"
#include "frc_timer_factory.h"

#if defined(__NIOS2__)
#  include "nios_timer.h" /*!< only count-down */
#elif defined(__MICROBLAZE__)
#  include "mb_timer.h"
#else
#  include "cclock_timer.h"
#endif

namespace sdpses {

namespace device {

FrcTimerFactory::FrcTimerFactory()
{
}

FrcTimerFactory::~FrcTimerFactory()
{
}

Timer& FrcTimerFactory::getInstance()
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
