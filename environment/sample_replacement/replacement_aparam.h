/**
 * @file	replacement_aparam.h
 * @brief	replacement for altera parameters
 */

#ifndef REPLACEMENT_APARAM_H_INCLUDED_
#define REPLACEMENT_APARAM_H_INCLUDED_

/*--- ALTERA Nios II ---------------------------------------------------------*/
#include "system.h"

/*! mpu */
#define MPU_TYPE				ALT_CPU_ARCHITECTURE
#define MPU_VER					ALT_CPU_CPU_IMPLEMENTATION
#define MPU_FREQ				ALT_CPU_CPU_FREQ
#define MPU_RESET_ADDR			ALT_CPU_RESET_ADDR
#define MPU_EXCEPTION_ADDR		ALT_CPU_EXCEPTION_ADDR
#define MPU_BIG_ENDIAN			ALT_CPU_BIG_ENDIAN

/*! cache */
#if defined(ALT_CPU_ICACHE_SIZE)
#  define MPU_ICACHE_SIZE		ALT_CPU_ICACHE_SIZE
#else
#  define MPU_ICACHE_SIZE		0
#endif

#if defined(ALT_CPU_DCACHE_SIZE)
#  define MPU_DCACHE_SIZE		ALT_CPU_DCACHE_SIZE
#else
#  define MPU_DCACHE_SIZE		0
#endif

/*! peripherals */
#define FREE_RUN_TIMER_BASE		TIMER_BASE
#define FREE_RUN_TIMER_FREQ		TIMER_FREQ

#define UART_IC_ID				UART_IRQ_INTERRUPT_CONTROLLER_ID

#endif /* REPLACEMENT_APARAM_H_INCLUDED_ */
