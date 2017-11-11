/**
 * @file	device_io.h
 * @brief	device io
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

#ifndef SDPSES_DEVICE_DEVICE_IO_H_INCLUDED_
#define SDPSES_DEVICE_DEVICE_IO_H_INCLUDED_

#include <stdint.h>

/*--- ALTERA(intel) Nios II --------------------------------------------------*/
#if defined(__NIOS2__)
#include <io.h>
static inline uint8_t device_read_io8(const uintptr_t base_addr, const uint32_t offset_addr) {
	return IORD_8DIRECT(base_addr, offset_addr);
}
static inline void device_write_io8(const uintptr_t base_addr, const uint32_t offset_addr, const uint8_t value) {
	IOWR_8DIRECT(base_addr, offset_addr, value);
}
static inline uint16_t device_read_io16(const uintptr_t base_addr, const uint32_t offset_addr) {
	return IORD_16DIRECT(base_addr, offset_addr);
}
static inline void device_write_io16(const uintptr_t base_addr, const uint32_t offset_addr, const uint16_t value) {
	IOWR_16DIRECT(base_addr, offset_addr, value);
}
static inline uint32_t device_read_io32(const uintptr_t base_addr, const uint32_t offset_addr) {
	return IORD_32DIRECT(base_addr, offset_addr);
}
static inline void device_write_io32(const uintptr_t base_addr, const uint32_t offset_addr, const uint32_t value) {
	IOWR_32DIRECT(base_addr, offset_addr, value);
}

/*--- XILINX MicroBlaze ------------------------------------------------------*/
#elif defined(__MICROBLAZE__)
#include "xil_io.h"
static inline uint8_t device_read_io8(const uintptr_t base_addr, const uint32_t offset_addr) {
	return Xil_In8(base_addr + offset_addr);
}
static inline void device_write_io8(const uintptr_t base_addr, const uint32_t offset_addr, const uint8_t value) {
	Xil_Out8((base_addr + offset_addr), value);
}
static inline uint16_t device_read_io16(const uintptr_t base_addr, const uint32_t offset_addr) {
	return Xil_In16(base_addr + offset_addr);
}
static inline void device_write_io16(const uintptr_t base_addr, const uint32_t offset_addr, const uint16_t value) {
	Xil_Out16((base_addr + offset_addr), value);
}
static inline uint32_t device_read_io32(const uintptr_t base_addr, const uint32_t offset_addr) {
	return Xil_In32(base_addr + offset_addr);
}
static inline void device_write_io32(const uintptr_t base_addr, const uint32_t offset_addr, const uint32_t value) {
	Xil_Out32((base_addr + offset_addr), value);
}

/*--- Other Processor --------------------------------------------------------*/
#else
static inline uint8_t device_read_io8(const uintptr_t base_addr, const uint32_t offset_addr) {
	return *(volatile uint8_t*)(base_addr + offset_addr);
}
static inline void device_write_io8(const uintptr_t base_addr, const uint32_t offset_addr, const uint8_t value) {
	*(volatile uint8_t*)(base_addr + offset_addr) = value;
}
static inline uint16_t device_read_io16(const uintptr_t base_addr, const uint32_t offset_addr) {
	return *(volatile uint16_t*)(base_addr + offset_addr);
}
static inline void device_write_io16(const uintptr_t base_addr, const uint32_t offset_addr, const uint16_t value) {
	*(volatile uint16_t*)(base_addr + offset_addr) = value;
}
static inline uint32_t device_read_io32(const uintptr_t base_addr, const uint32_t offset_addr) {
	return *(volatile uint32_t*)(base_addr + offset_addr);
}
static inline void device_write_io32(const uintptr_t base_addr, const uint32_t offset_addr, const uint32_t value) {
	*(volatile uint32_t*)(base_addr + offset_addr) = value;
}

#endif

#endif /* SDPSES_DEVICE_DEVICE_IO_H_INCLUDED_ */
