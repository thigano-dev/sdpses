/**
 * @file	uart_private.h
 * @brief	UART(Universal Asynchronous Receiver/Transmitter) private
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

#ifndef SDPSES_DEVICE_UART_PRIVATE_H_INCLUDED_
#define SDPSES_DEVICE_UART_PRIVATE_H_INCLUDED_

#include "uart.h"

struct Uart {
	struct Uart* (*destroy)(struct Uart* self);

	int (*setup)(struct Uart* self, const SerialParams* params);

	int (*get)(struct Uart* self, uint8_t* data);
	int (*put)(struct Uart* self, uint8_t data);
	int (*read)(struct Uart* self, uint8_t data_buff[], unsigned int data_count);
	int (*write)(struct Uart* self, const uint8_t data_buff[], unsigned int data_count);

	void (*clear)(struct Uart* self);
	int (*flush)(struct Uart* self);

	unsigned int (*getFramePeriodUsec)(const struct Uart* self);
	bool (*overrunErrorOccurred)(const struct Uart* self);
	bool (*framingErrorOccurred)(const struct Uart* self);
	bool (*parityErrorOccurred)(const struct Uart* self);
};

int Uart_ctor(struct Uart* self);
void Uart_dtor(struct Uart* self);

#endif /* SDPSES_DEVICE_UART_PRIVATE_H_INCLUDED_ */
