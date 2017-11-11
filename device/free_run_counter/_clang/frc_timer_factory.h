/**
 * @file	frc_timer_factory.h
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

#ifndef SDPSES_DEVICE_FRC_TIMER_FACTORY_H_INCLUDED_
#define SDPSES_DEVICE_FRC_TIMER_FACTORY_H_INCLUDED_

#include <stddef.h>

struct Timer;

struct Timer* FrcTimerFactory_getInstance(void);

#endif /* SDPSES_DEVICE_FRC_TIMER_FACTORY_H_INCLUDED_ */
