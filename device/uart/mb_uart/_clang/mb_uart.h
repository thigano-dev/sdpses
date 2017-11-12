/**
 * @file	mb_uart.h
 * @brief	Xilinx Uart Lite
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

#ifndef SDPSES_DEVICE_MB_UART_H_INCLUDED_
#define SDPSES_DEVICE_MB_UART_H_INCLUDED_

#include "uart.h"

typedef struct {
	unsigned int txBuffSz;
	unsigned int rxBuffSz;
} MbUartParams;

struct MbUart;

size_t MbUart_sizeOf(void);

struct MbUart* MbUart_create(uint32_t base_addr, uint32_t ic_base,
		uint32_t irq, const MbUartParams* uart_params);
struct Uart* MbUart_destroy(struct Uart* self);

int MbUart_ctor(struct MbUart* instance, uint32_t base_addr,
		uint32_t ic_base, uint32_t irq, const MbUartParams* uart_params);
void MbUart_dtor(struct MbUart* instance);

int MbUart_setup(struct Uart* self, const SerialParams* params);

int MbUart_get(struct Uart* self, uint8_t* data);
int MbUart_put(struct Uart* self, uint8_t data);
int MbUart_read(struct Uart* self, uint8_t data_buff[], unsigned int data_count);
int MbUart_write(struct Uart* self, const uint8_t data_buff[], unsigned int data_count);

void MbUart_clear(struct Uart* self);
int MbUart_flush(struct Uart* self);

unsigned int MbUart_getFramePeriodUsec(const struct Uart* self);
bool MbUart_overrunErrorOccurred(const struct Uart* self);
bool MbUart_framingErrorOccurred(const struct Uart* self);
bool MbUart_parityErrorOccurred(const struct Uart* self);

#endif /* SDPSES_DEVICE_MB_UART_H_INCLUDED_ */
