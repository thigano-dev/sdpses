/**
 * @file	gpio.cpp
 * @brief	GPIO(General-purpose input/output)
 * @author	Tsuguyoshi Higano
 * @date	Nov 13, 2017
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

#include "gpio.h"

namespace sdpses {

namespace device {

/**
 * @brief	Constructor
 */
Gpio::Gpio()
{
}

/**
 * @brief	Destructor
 */
Gpio::~Gpio()
{
}

/**
 * @brief	Set multiple bits
 * @param	bitmask			[1:set, 0:unaffected]
 * @return	none
 */
void Gpio::setDataBit(uint32_t bitmask)
{
	bitmask = (readData() | bitmask);
	writeData(bitmask);
}

/**
 * @brief	Clear multiple bits
 * @param	bitmask			[1:set to zero, 0:unaffected]
 * @return	none
 */
void Gpio::clearDataBit(uint32_t bitmask)
{
	bitmask = (readData() & ~bitmask);
	writeData(bitmask);
}

/**
 * @brief	Set the output direction
 * @param	bitmask			[1:output, 0:unaffected]
 * @return	none
 */
void Gpio::setOutputBit(uint32_t bitmask)
{
	bitmask = (readDirection() | bitmask);
	writeDirection(bitmask);
}

/**
 * @brief	Set the input direction
 * @param	bitmask			[1:input, 0:unaffected]
 * @return	none
 */
void Gpio::setInputBit(uint32_t bitmask)
{
	bitmask = (readDirection() & ~bitmask);
	writeDirection(bitmask);
}

} /* namespace device */

} /* namespace sdpses */
