/**
 * @file	uart.h
 * @brief	UART(Universal Asynchronous Receiver/Transmitter)
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

#ifndef SDPSES_DEVICE_UART_H_INCLUDED_
#define SDPSES_DEVICE_UART_H_INCLUDED_

#include <stdint.h>

#include "serial_params.h"

namespace sdpses {

namespace device {

/**
 * @class	Uart
 * @brief	Uart is the abstract base class for UART.
 */
class Uart {

public:
	virtual ~Uart() {}

	/**
	 * @brief	Set up
	 * @param	params			SerialParams
	 * @retval	0				success
	 * @retval	!=0				failure
	 */
	virtual int setup(const SerialParams& params) = 0;

	/**
	 * @brief	Get a data
	 * @param	data			pointer to a data
	 * @retval	0				success
	 * @retval	!=0				failure
	 */
	virtual int get(uint8_t* data) = 0;

	/**
	 * @brief	Put a data
	 * @param	data			data
	 * @retval	0				success
	 * @retval	!=0				failure
	 */
	virtual int put(uint8_t data) = 0;

	/**
	 * @brief	Read data into buffer
	 * @param	data_buff		data buffer
	 * @param	data_count		number of data
	 * @retval	0				success
	 * @retval	!=0				failure
	 */
	virtual int read(uint8_t data_buff[], unsigned int data_count) = 0;

	/**
	 * @brief	Write data buffer
	 * @param	data_buff		data buffer
	 * @param	data_count		number of data
	 * @retval	0				success
	 * @retval	!=0				failure
	 */
	virtual int write(const uint8_t data_buff[], unsigned int data_count) = 0;

	/**
	 * @brief	Clear receive/transmit buffer and errors
	 * @return	none
	 */
	virtual void clear() = 0;

	/**
	 * @brief	Flush TX-Buffer
	 * @retval	0				success
	 * @retval	!=0				failure
	 */
	virtual int flush() = 0;

	/**
	 * @brief	Get frame period
	 * @return	frame period
	 */
	virtual unsigned int getFramePeriodUsec() const = 0;

	/**
	 * @brief	Overrun error occurred
	 * @retval	true			occurred
	 * @retval	false			not occurred
	 */
	virtual bool overrunErrorOccurred() const = 0;

	/**
	 * @brief	Framing error occurred
	 * @retval	true			occurred
	 * @retval	false			not occurred
	 */
	virtual bool framingErrorOccurred() const = 0;

	/**
	 * @brief	Parity error occurred
	 * @retval	true			occurred
	 * @retval	false			not occurred
	 */
	virtual bool parityErrorOccurred() const = 0;

protected:
	Uart() {}

private:
	Uart(const Uart&);
	Uart& operator=(const Uart&);
};

} /* namespace device */

} /* namespace sdpses */

#endif /* SDPSES_DEVICE_UART_H_INCLUDED_ */
