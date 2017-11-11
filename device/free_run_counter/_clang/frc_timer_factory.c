/**
 * @file	frc_timer_factory.c
 * @brief	free-running counter timer factory
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
#include "frc_timer_factory.h"

#if defined(__NIOS2__)
#  include "nios_timer.h" /*!< only count-down */
#elif defined(__MICROBLAZE__)
#  include "mb_timer.h"
#else
#  include "cclock_timer.h"
#endif

struct Timer* FrcTimerFactory_getInstance(void)
{
#if defined(__NIOS2__)
	struct NiosTimer* const instance = NiosTimer_create(FREE_RUN_TIMER_BASE, FREE_RUN_TIMER_FREQ);
#elif defined(__MICROBLAZE__)
	struct MbTimer* const instance = MbTimer_create(FREE_RUN_TIMER_BASE, FREE_RUN_TIMER_FREQ);
#else
	struct CclockTimer* const instance = CclockTimer_create();
#endif

	return (struct Timer*)instance;
}
