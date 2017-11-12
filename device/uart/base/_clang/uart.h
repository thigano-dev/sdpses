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

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "serial_params.h"

struct Uart;

struct Uart* Uart_destroy(struct Uart* self);

int Uart_setup(struct Uart* self, const SerialParams* params);

int Uart_get(struct Uart* self, uint8_t* data);
int Uart_put(struct Uart* self, uint8_t data);
int Uart_read(struct Uart* self, uint8_t data_buff[], unsigned int data_count);
int Uart_write(struct Uart* self, const uint8_t data_buff[], unsigned int data_count);

void Uart_clear(struct Uart* self);
int Uart_flush(struct Uart* self);

unsigned int Uart_getFramePeriodUsec(const struct Uart* self);
bool Uart_overrunErrorOccurred(const struct Uart* self);
bool Uart_framingErrorOccurred(const struct Uart* self);
bool Uart_parityErrorOccurred(const struct Uart* self);

#endif /* SDPSES_DEVICE_UART_H_INCLUDED_ */
