/**
 * @file	nios_gpio.cpp
 * @brief	Altera Avalon PIO
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

#include <sys/alt_irq.h>

#include "altera_avalon_pio_regs.h"
#include "nios_gpio.h"
#include "lib_assert.h"
#include "lib_debug.h"

namespace sdpses {

namespace device {

namespace {
const uint32_t kINVALID_VALUE = 0xFFFFFFFFUL;
} /* namespace */

/**
 * @brief	Constructor
 * @param	base_addr		base address
 * @param	ic_id			intc id
 * @param	irq				irq number
 * @param	int_trg			interrupt trigger
 */
NiosGpio::NiosGpio(const uint32_t base_addr, const uint32_t ic_id,
		const uint32_t irq, const InterruptTrigger int_trg)
	: kBASE_ADDR(base_addr)
	, kIC_ID(ic_id)
	, kIRQ(irq)
	, kINT_TRG(int_trg)
	, interruptFlags_(0)
	, callbackFunc_(0)
	, callbackArg_(0)
{
	DEBUG_PRINTF_("<NiosII GPIO parameters>\r\n");
	DEBUG_PRINTF_("  BASE_ADDR     : [H'%08lX]\r\n", base_addr);
	DEBUG_PRINTF_("  IC ID         : [");
	if (ic_id == 0UL) {
		DEBUG_PRINTF_("IIC: in NiosII Core]\r\n");
	} else if (ic_id == 0xFFFFFFFFUL) {
		DEBUG_PRINTF_("none]\r\n");
	} else {
		DEBUG_PRINTF_("EIC: H'%08lX]\r\n", ic_id);
	}
	DEBUG_PRINTF_("  IRQ           : [%d]\r\n", static_cast<int>(irq));
	DEBUG_PRINTF_("  INT TRIGGER   : [");
	if (int_trg == kTRG_LEVEL) {
		DEBUG_PRINTF_("LEVEL SENSITIVE]\r\n");
	} else if (int_trg == kTRG_EDGE) {
		DEBUG_PRINTF_("EDGE SENSITIVE]\r\n");
	} else {
		DEBUG_PRINTF_("UNKNOWN]\r\n");
	}
	DEBUG_PRINTF_("\r\n");

	alt_ic_irq_disable(kIC_ID, kIRQ);
	IOWR_ALTERA_AVALON_PIO_IRQ_MASK(kBASE_ADDR, interruptFlags_);
	IOWR_ALTERA_AVALON_PIO_EDGE_CAP(kBASE_ADDR, 0);
}

/**
 * @brief	Constructor
 * @param	base_addr		base address
 */
NiosGpio::NiosGpio(const uint32_t base_addr)
	: kBASE_ADDR(base_addr)
	, kIC_ID(kINVALID_VALUE)
	, kIRQ(kINVALID_VALUE)
	, kINT_TRG(kINSENSITIVE)
	, interruptFlags_(0)
	, callbackFunc_(0)
	, callbackArg_(0)
{
	DEBUG_PRINTF_("<NiosII GPIO parameters>\r\n");
	DEBUG_PRINTF_("  BASE_ADDR     : [H'%08lX]\r\n", base_addr);
	DEBUG_PRINTF_("\r\n");

	IOWR_ALTERA_AVALON_PIO_IRQ_MASK(kBASE_ADDR, interruptFlags_);
	IOWR_ALTERA_AVALON_PIO_EDGE_CAP(kBASE_ADDR, 0);
}

/**
 * @brief	Destructor
 */
NiosGpio::~NiosGpio()
{
	if (kIC_ID != kINVALID_VALUE) { alt_ic_irq_disable(kIC_ID, kIRQ); }
	IOWR_ALTERA_AVALON_PIO_IRQ_MASK(kBASE_ADDR, 0);
}

/**
 * @brief	Write data
 * @param	data			data
 * @return	none
 */
void NiosGpio::writeData(const uint32_t data)
{
	IOWR_ALTERA_AVALON_PIO_DATA(kBASE_ADDR, data);
}

/**
 * @brief	Read data
 * @return	data
 */
uint32_t NiosGpio::readData() const
{
	return IORD_ALTERA_AVALON_PIO_DATA(kBASE_ADDR);
}

/**
 * @brief	Write the input/output direction
 * @param	direction		[1:output, 0:input]
 * @return	none
 */
void NiosGpio::writeDirection(const uint32_t direction)
{
	IOWR_ALTERA_AVALON_PIO_DIRECTION(kBASE_ADDR, direction);
}

/**
 * @brief	Read the input/output direction
 * @return	direction [1:output, 0:input]
 */
uint32_t NiosGpio::readDirection() const
{
	return IORD_ALTERA_AVALON_PIO_DIRECTION(kBASE_ADDR);
}

/**
 * @brief	Set up interrupt
 * @param	interrupt_bits	[1:enable, 0:disable]
 * @param	callback_func	callback function
 * @param	callback_arg	arguments for the callback function
 * @retval	0				success
 * @retval	!=0				failure
 */
int NiosGpio::setupInterrupt(const uint32_t interrupt_bits,
		const CallbackFunc callback_func, void* const callback_arg)
{
	ASSERT_(kIC_ID != kINVALID_VALUE);
	ASSERT_(callback_func);

	interruptFlags_ = interrupt_bits;
	callbackFunc_ = callback_func;
	callbackArg_ = callback_arg;
	IOWR_ALTERA_AVALON_PIO_IRQ_MASK(kBASE_ADDR, interruptFlags_);

	alt_ic_isr_register(kIC_ID, kIRQ, interruptServiceRoutine, this, (void*)0);
	alt_ic_irq_enable(kIC_ID, kIRQ);

	return 0;
}

/**
 * @brief	Enable multiple interrupts for each input port
 * @param	bitmask			[1:enable, 0:unaffected]
 * @return	none
 */
void NiosGpio::enableMultipleInterrupts(const uint32_t bitmask)
{
	ASSERT_(kIC_ID != kINVALID_VALUE);

	alt_ic_irq_disable(kIC_ID, kIRQ);
	interruptFlags_ |= bitmask;
	IOWR_ALTERA_AVALON_PIO_IRQ_MASK(kBASE_ADDR, interruptFlags_);
	alt_ic_irq_enable(kIC_ID, kIRQ);
}

/**
 * @brief	Disable multiple interrupts for each input port
 * @param	bitmask			[1:disable, 0:unaffected]
 * @return	none
 */
void NiosGpio::disableMultipleInterrupts(const uint32_t bitmask)
{
	ASSERT_(kIC_ID != kINVALID_VALUE);

	alt_ic_irq_disable(kIC_ID, kIRQ);
	interruptFlags_ &= ~bitmask;
	IOWR_ALTERA_AVALON_PIO_IRQ_MASK(kBASE_ADDR, interruptFlags_);
	alt_ic_irq_enable(kIC_ID, kIRQ);
}

/**
 * @brief	Enable interrupt
 * @return	none
 */
void NiosGpio::enableInterrupt()
{
	ASSERT_(kIC_ID != kINVALID_VALUE);

	alt_ic_irq_enable(kIC_ID, kIRQ);
}

/**
 * @brief	Disable interrupt
 * @return	none
 */
void NiosGpio::disableInterrupt()
{
	ASSERT_(kIC_ID != kINVALID_VALUE);

	alt_ic_irq_disable(kIC_ID, kIRQ);
}

/**
 * @brief	Interrupt Service Routine
 * @param	isr_context		ISR context
 * @return	none
 */
void NiosGpio::interruptServiceRoutine(void* const isr_context)
{
	NiosGpio* const niosGpio = reinterpret_cast<NiosGpio*>(isr_context);
	uint32_t status = 0;

	if (kINT_TRG == kTRG_LEVEL) {
		status = IORD_ALTERA_AVALON_PIO_DATA(niosGpio->kBASE_ADDR);
	} else {
		status = IORD_ALTERA_AVALON_PIO_EDGE_CAP(niosGpio->kBASE_ADDR);
		IOWR_ALTERA_AVALON_PIO_EDGE_CAP(niosGpio->kBASE_ADDR, status);
	}

	niosGpio->callbackFunc_(niosGpio->callbackArg_, (status & niosGpio->interruptFlags_));
}

} /* namespace device */

} /* namespace sdpses */
