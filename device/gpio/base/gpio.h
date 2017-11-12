/**
 * @file	gpio.h
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

#ifndef SDPSES_DEVICE_GPIO_H_INCLUDED_
#define SDPSES_DEVICE_GPIO_H_INCLUDED_

#include <stdint.h>

namespace sdpses {

namespace device {

/**
 * @class	Gpio
 * @brief	Gpio is the abstract base class for general purpose input/output.
 *
 * @attention direction
 * Bits set to 0 are input and bits set to 1 are output.
 */
class Gpio {

public:
	virtual ~Gpio();

	/**
	 * @brief	Write data
	 * @param	data			data
	 * @return	none
	 */
	virtual void writeData(uint32_t data) = 0;

	/**
	 * @brief	Read data
	 * @return	data
	 */
	virtual uint32_t readData() const = 0;

	void setDataBit(uint32_t bitmask);
	void clearDataBit(uint32_t bitmask);

	/**
	 * @brief	Write the input/output direction
	 * @param	direction		[1:output, 0:input]
	 * @return	none
	 */
	virtual void writeDirection(uint32_t direction) = 0;

	/**
	 * @brief	Read the input/output direction
	 * @return	direction [1:output, 0:input]
	 */
	virtual uint32_t readDirection() const = 0;

	void setOutputBit(uint32_t bitmask);
	void setInputBit(uint32_t bitmask);

	/**
	 * @brief	Callback Function
	 * @param	callback_arg	arguments for the callback function
	 * @param	status			interrupt status
	 * @return	none
	 */
	typedef void (*CallbackFunc)(void* callback_arg, uint32_t status);

	/**
	 * @brief	Set up interrupt
	 * @param	interrupt_bits	[1:enable, 0:disable]
	 * @param	callback_func	callback function
	 * @param	callback_arg	arguments for the callback function
	 * @retval	0				success
	 * @retval	!=0				failure
	 */
	virtual int setupInterrupt(uint32_t interrupt_bits,
			CallbackFunc callback_func, void* callback_arg) { return 1; }

	/**
	 * @brief	Enable multiple interrupts for each input port
	 * @param	bitmask			[1:enable, 0:unaffected]
	 * @return	none
	 */
	virtual void enableMultipleInterrupts(uint32_t bitmask) {}

	/**
	 * @brief	Disable multiple interrupts for each input port
	 * @param	bitmask			[1:disable, 0:unaffected]
	 * @return	none
	 */
	virtual void disableMultipleInterrupts(uint32_t bitmask) {}

	/**
	 * @brief	Enable interrupt
	 * @return	none
	 */
	virtual void enableInterrupt() {}

	/**
	 * @brief	Disable interrupt
	 * @return	none
	 */
	virtual void disableInterrupt() {}

protected:
	Gpio();

private:
	Gpio(const Gpio&);
	Gpio& operator=(const Gpio&);
};

} /* namespace device */

} /* namespace sdpses */

#endif /* SDPSES_DEVICE_GPIO_H_INCLUDED_ */
