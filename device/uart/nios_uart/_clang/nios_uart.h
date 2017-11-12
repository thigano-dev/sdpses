/**
 * @file	nios_uart.h
 * @brief	Altera Avalon UART
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

#ifndef SDPSES_DEVICE_NIOS_UART_H_INCLUDED_
#define SDPSES_DEVICE_NIOS_UART_H_INCLUDED_

#include <stdint.h>

#include "uart.h"

typedef struct {
	unsigned int txBuffSz;
	unsigned int rxBuffSz;
} NiosUartParams;

struct NiosUart;

size_t NiosUart_sizeOf(void);

struct NiosUart* NiosUart_create(uint32_t base_addr, uint32_t freq,
		uint32_t ic_id, uint32_t irq, const NiosUartParams* uart_params);
struct Uart* NiosUart_destroy(struct Uart* self);

int NiosUart_ctor(struct NiosUart* instance, uint32_t base_addr,
		uint32_t freq, uint32_t ic_id, uint32_t irq, const NiosUartParams* uart_params);
void NiosUart_dtor(struct NiosUart* instance);

int NiosUart_setup(struct Uart* self, const SerialParams* params);

int NiosUart_get(struct Uart* self, uint8_t* data);
int NiosUart_put(struct Uart* self, uint8_t data);
int NiosUart_read(struct Uart* self, uint8_t data_buff[], unsigned int data_count);
int NiosUart_write(struct Uart* self, const uint8_t data_buff[], unsigned int data_count);

void NiosUart_clear(struct Uart* self);
int NiosUart_flush(struct Uart* self);

unsigned int NiosUart_getFramePeriodUsec(const struct Uart* self);
bool NiosUart_overrunErrorOccurred(const struct Uart* self);
bool NiosUart_framingErrorOccurred(const struct Uart* self);
bool NiosUart_parityErrorOccurred(const struct Uart* self);

#endif /* SDPSES_DEVICE_NIOS_UART_H_INCLUDED_ */
