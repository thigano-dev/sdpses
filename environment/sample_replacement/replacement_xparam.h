/**
 * @file	replacement_xparam.h
 * @brief	replacement for xilinx parameters
 */

#ifndef REPLACEMENT_XPARAM_H_INCLUDED_
#define REPLACEMENT_XPARAM_H_INCLUDED_

/*--- XILINX MicroBlaze ------------------------------------------------------*/
#include "xparameters.h"

/*! mpu */
#define MPU_TYPE				"MicroBlaze"
#define MPU_VER					XPAR_MICROBLAZE_HW_VER
#define MPU_FREQ				XPAR_CPU_CORE_CLOCK_FREQ_HZ
#define MPU_RESET_ADDR			0

/*! cache */
#if (XPAR_MICROBLAZE_0_USE_ICACHE == 1)
#define MPU_ICACHE_SIZE			XPAR_MICROBLAZE_0_CACHE_BYTE_SIZE
#else
#define MPU_ICACHE_SIZE			0
#endif

#if (XPAR_MICROBLAZE_0_USE_DCACHE == 1)
#define MPU_DCACHE_SIZE			XPAR_MICROBLAZE_0_DCACHE_BYTE_SIZE
#else
#define MPU_DCACHE_SIZE			0
#endif

/*! peripherals */
#define INTC_BASE				XPAR_INTC_0_BASEADDR

#define FREE_RUN_TIMER_BASE		XPAR_TMRCTR_0_BASEADDR
#define FREE_RUN_TIMER_FREQ		XPAR_TMRCTR_0_CLOCK_FREQ_HZ

#define UART_BASE				XPAR_UARTLITE_0_BASEADDR
#define UART_INTC_BASE			XPAR_INTC_0_BASEADDR
#define UART_IRQ				XPAR_INTC_0_UARTLITE_0_VEC_ID

#endif /* REPLACEMENT_XPARAM_H_INCLUDED_ */
