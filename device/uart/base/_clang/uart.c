/**
 * @file	uart.c
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

#include "uart_private.h"

/**
 * @brief	Destroy
 * @param	self			Uart*
 * @return	Uart*
 */
struct Uart* Uart_destroy(struct Uart* const self)
{
	if (!self) { return NULL; }
	return self->destroy(self);
}

/**
 * @brief	Constructor
 * @param	self			Uart*
 * @retval	0				success
 * @retval	!=0				failure
 */
int Uart_ctor(struct Uart* const self)
{
	return 0;
}

/**
 * @brief	Destructor
 * @param	self			Uart*
 * @return	none
 */
void Uart_dtor(struct Uart* const self)
{
	if (!self) { return; }
}

/**
 * @brief	Set up
 * @param	self			Uart*
 * @param	params			SerialParams*
 * @retval	0				success
 * @retval	!=0				failure
 */
int Uart_setup(struct Uart* const self, const SerialParams* const params)
{
	return self->setup(self, params);
}

/**
 * @brief	Get a data
 * @param	self			Uart*
 * @param	data			pointer to a data
 * @retval	0				success
 * @retval	!=0				failure
 */
int Uart_get(struct Uart* const self, uint8_t* const data)
{
	return self->get(self, data);
}

/**
 * @brief	Put a data
 * @param	self			Uart*
 * @param	data			data
 * @retval	0				success
 * @retval	!=0				failure
 */
int Uart_put(struct Uart* const self, const uint8_t data)
{
	return self->put(self, data);
}

/**
 * @brief	Read data into buffer
 * @param	self			Uart*
 * @param	data_buff		data buffer
 * @param	data_count		number of data
 * @retval	0				success
 * @retval	!=0				failure
 */
int Uart_read(struct Uart* const self, uint8_t data_buff[], const unsigned int data_count)
{
	return self->read(self, data_buff, data_count);
}

/**
 * @brief	Write data buffer
 * @param	self			Uart*
 * @param	data_buff		data buffer
 * @param	data_count		number of data
 * @retval	0				success
 * @retval	!=0				failure
 */
int Uart_write(struct Uart* const self, const uint8_t data_buff[], const unsigned int data_count)
{
	return self->write(self, data_buff, data_count);
}

/**
 * @brief	Clear receive/transmit buffer and errors
 * @param	self			Uart*
 * @return	none
 */
void Uart_clear(struct Uart* const self)
{
	self->clear(self);
}

/**
 * @brief	Flush TX-Buffer
 * @param	self			Uart*
 * @retval	0				success
 * @retval	!=0				failure
 */
int Uart_flush(struct Uart* const self)
{
	return self->flush(self);
}

/**
 * @brief	Get frame period
 * @param	self			Uart*
 * @return	frame period
 */
unsigned int Uart_getFramePeriodUsec(const struct Uart* const self)
{
	return self->getFramePeriodUsec(self);
}

/**
 * @brief	Overrun error occurred
 * @param	self			Uart*
 * @retval	true			occurred
 * @retval	false			not occurred
 */
bool Uart_overrunErrorOccurred(const struct Uart* const self)
{
	return self->overrunErrorOccurred(self);
}

/**
 * @brief	Framing error occurred
 * @param	self			Uart*
 * @retval	true			occurred
 * @retval	false			not occurred
 */
bool Uart_framingErrorOccurred(const struct Uart* const self)
{
	return self->framingErrorOccurred(self);
}

/**
 * @brief	Parity error occurred
 * @param	self			Uart*
 * @retval	true			occurred
 * @retval	false			not occurred
 */
bool Uart_parityErrorOccurred(const struct Uart* const self)
{
	return self->parityErrorOccurred(self);
}
