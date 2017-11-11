/**
 * @file	static_frc_timer_factory.h
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

#ifndef SDPSES_DEVICE_STATIC_FRC_TIMER_FACTORY_H_INCLUDED_
#define SDPSES_DEVICE_STATIC_FRC_TIMER_FACTORY_H_INCLUDED_

namespace sdpses {

namespace device {

class Timer;

/**
 * @class	StaticFrcTimerFactory
 * @brief	StaticFrcTimerFactory class
 * @note	Don't inherit from this class.
 */
class StaticFrcTimerFactory {

public:
	~StaticFrcTimerFactory();

	static Timer& getInstance();

private:
	StaticFrcTimerFactory();
	StaticFrcTimerFactory(const StaticFrcTimerFactory&);
	StaticFrcTimerFactory& operator=(const StaticFrcTimerFactory&);
};

} /* namespace device */

} /* namespace sdpses */

#endif /* SDPSES_DEVICE_STATIC_FRC_TIMER_FACTORY_H_INCLUDED_ */
