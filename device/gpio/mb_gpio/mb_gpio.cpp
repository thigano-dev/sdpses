/**
 * @file	mb_gpio.cpp
 * @brief	Xilinx General Purpose I/O
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

#include "xintc_l.h"

#include "xgpio_l.h"
#include "mb_gpio.h"
#include "lib_assert.h"
#include "lib_debug.h"

namespace sdpses {

namespace device {

const uint32_t MbGpio::kDIRECTION_OUTPUT	= 0;
const uint32_t MbGpio::kDIRECTION_INPUT		= 1;

const uint32_t MbGpio::kPORTINT_DISABLE		= 0;
const uint32_t MbGpio::kPORTINT_ENABLE		= 1;

/**
 * @brief	Constructor
 * @param	base_addr		base address
 * @param	ic_base			intc base address
 * @param	irq				irq number
 */
MbGpio::MbGpio(const uint32_t base_addr, const uint32_t ic_base, const uint32_t irq)
	: kBASE_ADDR(base_addr)
	, kIC_BASE(ic_base)
	, kIRQ(irq)
	, kIRQ_MASK(1UL << irq)
	, interruptFlags_(0)
	, callbackFunc_(0)
	, callbackArg_(0)
{
	DEBUG_PRINTF_("<MicroBlaze GPIO parameters>\r\n");
	DEBUG_PRINTF_("  BASE_ADDR     : [H'%08lX]\r\n", base_addr);
	DEBUG_PRINTF_("  IC BASE       : [H'%08lX]\r\n", ic_base);
	DEBUG_PRINTF_("  IRQ           : [%lu]\r\n", irq);
	DEBUG_PRINTF_("\r\n");

	XGpio_WriteReg(kBASE_ADDR, XGPIO_GIE_OFFSET, ~XGPIO_GIE_GINTR_ENABLE_MASK);
}

/**
 * @brief	Constructor
 * @param	base_addr		base address
 */
MbGpio::MbGpio(const uint32_t base_addr)
	: kBASE_ADDR(base_addr)
	, kIC_BASE(0)
	, kIRQ(0)
	, kIRQ_MASK(0)
	, interruptFlags_(0)
	, callbackFunc_(0)
	, callbackArg_(0)
{
	DEBUG_PRINTF_("<MicroBlaze GPIO parameters>\r\n");
	DEBUG_PRINTF_("  BASE_ADDR     : [H'%08lX]\r\n", base_addr);
	DEBUG_PRINTF_("\r\n");

	XGpio_WriteReg(kBASE_ADDR, XGPIO_GIE_OFFSET, ~XGPIO_GIE_GINTR_ENABLE_MASK);
}

/**
 * @brief	Destructor
 */
MbGpio::~MbGpio()
{
	if (kIRQ_MASK) { XIntc_DisableIntr(kIC_BASE, kIRQ_MASK); }
	XGpio_WriteReg(kBASE_ADDR, XGPIO_GIE_OFFSET, ~XGPIO_GIE_GINTR_ENABLE_MASK);
	XGpio_WriteReg(kBASE_ADDR, XGPIO_IER_OFFSET, 0);
}

/**
 * @brief	Write data
 * @param	data			data
 * @return	none
 */
void MbGpio::writeData(uint32_t data)
{
	XGpio_WriteReg(kBASE_ADDR, XGPIO_DATA_OFFSET, data);
}

/**
 * @brief	Read data
 * @return	data
 */
uint32_t MbGpio::readData() const
{
	return XGpio_ReadReg(kBASE_ADDR, XGPIO_DATA_OFFSET);
}

/**
 * @brief	Write the input/output direction
 * @param	direction		[1:output, 0:input]
 * @return	none
 */
void MbGpio::writeDirection(const uint32_t direction)
{
	XGpio_WriteReg(kBASE_ADDR, XGPIO_TRI_OFFSET, ~direction);
}

/**
 * @brief	Read the input/output direction
 * @return	direction [1:output, 0:input]
 */
uint32_t MbGpio::readDirection() const
{
	return ~XGpio_ReadReg(kBASE_ADDR, XGPIO_TRI_OFFSET);
}

/**
 * @brief	Set up interrupt
 * @param	interrupt_bits	[1:enable, 0:disable]
 * @param	callback_func	callback function
 * @param	callback_arg	arguments for the callback function
 * @retval	0				success
 * @retval	!=0				failure
 */
int MbGpio::setupInterrupt(const uint32_t interrupt_bits,
		const CallbackFunc callback_func, void* const callback_arg)
{
	ASSERT_(kIRQ_MASK);
	ASSERT_(callback_func);

	interruptFlags_ = interrupt_bits;
	callbackFunc_ = callback_func;
	callbackArg_ = callback_arg;

	XGpio_WriteReg(kBASE_ADDR, XGPIO_IER_OFFSET, interruptFlags_);
	XGpio_WriteReg(kBASE_ADDR, XGPIO_GIE_OFFSET, XGPIO_GIE_GINTR_ENABLE_MASK);

	XIntc_RegisterHandler(kIC_BASE, kIRQ, (XInterruptHandler)interruptHandler, this);
	XIntc_EnableIntr(kIC_BASE, kIRQ_MASK);

	return 0;
}

/**
 * @brief	Enable multiple interrupts for each input port
 * @param	bitmask			[1:enable, 0:unaffected]
 * @return	none
 */
void MbGpio::enableMultipleInterrupts(const uint32_t bitmask)
{
	ASSERT_(kIRQ_MASK);

	XIntc_DisableIntr(kIC_BASE, kIRQ_MASK);
	interruptFlags_ |= bitmask;
	XGpio_WriteReg(kBASE_ADDR, XGPIO_IER_OFFSET, interruptFlags_);
	XIntc_EnableIntr(kIC_BASE, kIRQ_MASK);
}

/**
 * @brief	Disable multiple interrupts for each input port
 * @param	bitmask			[1:disable, 0:unaffected]
 * @return	none
 */
void MbGpio::disableMultipleInterrupts(const uint32_t bitmask)
{
	ASSERT_(kIRQ_MASK);

	XIntc_DisableIntr(kIC_BASE, kIRQ_MASK);
	interruptFlags_ &= ~bitmask;
	XGpio_WriteReg(kBASE_ADDR, XGPIO_IER_OFFSET, interruptFlags_);
	XIntc_EnableIntr(kIC_BASE, kIRQ_MASK);
}

/**
 * @brief	Enable interrupt
 * @return	none
 */
void MbGpio::enableInterrupt()
{
	ASSERT_(kIRQ_MASK);

	XIntc_EnableIntr(kIC_BASE, kIRQ_MASK);
}

/**
 * @brief	Disable interrupt
 * @return	none
 */
void MbGpio::disableInterrupt()
{
	ASSERT_(kIRQ_MASK);

	XIntc_DisableIntr(kIC_BASE, kIRQ_MASK);
}

/**
 * @brief	Interrupt Handler
 * @param	context			context
 * @return	none
 */
void MbGpio::interruptHandler(void* const context)
{
	MbGpio* const instance = reinterpret_cast<MbGpio*>(context);
	const uint32_t status = XGpio_ReadReg(instance->kBASE_ADDR, XGPIO_ISR_OFFSET);

	instance->callbackFunc_(instance->callbackArg_, status);
	XGpio_WriteReg(instance->kBASE_ADDR, XGPIO_ISR_OFFSET, status);

	XIntc_AckIntr(instance->kIC_BASE, instance->kIRQ_MASK);
}

} /* namespace device */

} /* namespace sdpses */
