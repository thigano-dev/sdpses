/**
 * @file	frc_timer_factory.h
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

#ifndef SDPSES_DEVICE_FRC_TIMER_FACTORY_H_INCLUDED_
#define SDPSES_DEVICE_FRC_TIMER_FACTORY_H_INCLUDED_

namespace sdpses {

namespace device {

class Timer;

/**
 * @class	FrcTimerFactory
 * @brief	FrcTimerFactory class
 * @note	Don't inherit from this class.
 */
class FrcTimerFactory {

public:
	~FrcTimerFactory();

	static Timer& getInstance();

private:
	FrcTimerFactory();
	FrcTimerFactory(const FrcTimerFactory&);
	FrcTimerFactory& operator=(const FrcTimerFactory&);
};

} /* namespace device */

} /* namespace sdpses */

#endif /* SDPSES_DEVICE_FRC_TIMER_FACTORY_H_INCLUDED_ */
