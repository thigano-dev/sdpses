/**
 * @file	nios_timer.cpp
 * @brief	Altera Avalon Timer
 * @author	Tsuguyoshi Higano
 * @date	Nov 17, 2017
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

#include "altera_avalon_timer_regs.h"
#include "nios_timer.h"
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
 * @param	freq			frequency(Hz)
 * @param	ic_id			intc id
 * @param	irq				irq number
 */
NiosTimer::NiosTimer(const uint32_t base_addr,
		const uint32_t freq, const uint32_t ic_id, const uint32_t irq)
	: kBASE_ADDR(base_addr)
	, kFREQ(freq)
	, kIC_ID(ic_id)
	, kIRQ(irq)
	, controlFlags_(0)
	, callbackFunc_(0)
	, callbackArg_(0)
{
	DEBUG_PRINTF_("<NiosII Timer parameters>\r\n");
	DEBUG_PRINTF_("  BASE_ADDR     : [H'%08lX]\r\n", base_addr);
	DEBUG_PRINTF_("  FREQ          : [%lu", (freq / 1000000UL));
	DEBUG_PRINTF_(".%luMHz]\r\n", (freq % 1000000UL));
	DEBUG_PRINTF_("  IC ID         : [");
	if (ic_id == 0UL) {
		DEBUG_PRINTF_("IIC: in NiosII Core]\r\n");
	} else if (ic_id == 0xFFFFFFFFUL) {
		DEBUG_PRINTF_("none]\r\n");
	} else {
		DEBUG_PRINTF_("EIC: H'%08lX]\r\n", ic_id);
	}
	DEBUG_PRINTF_("  IRQ           : [%d]\r\n", static_cast<int>(irq));
	DEBUG_PRINTF_("\r\n");

	setup(CountParams());
}

/**
 * @brief	Constructor
 * @param	base_addr		base address
 * @param	freq			frequency(Hz)
 */
NiosTimer::NiosTimer(const uint32_t base_addr, const uint32_t freq)
	: kBASE_ADDR(base_addr)
	, kFREQ(freq)
	, kIC_ID(kINVALID_VALUE)
	, kIRQ(kINVALID_VALUE)
	, controlFlags_(0)
	, callbackFunc_(0)
	, callbackArg_(0)
{
	DEBUG_PRINTF_("<NiosII Timer parameters>\r\n");
	DEBUG_PRINTF_("  BASE_ADDR     : [H'%08lX]\r\n", base_addr);
	DEBUG_PRINTF_("  FREQ          : [%lu", (freq / 1000000UL));
	DEBUG_PRINTF_(".%luMHz]\r\n", (freq % 1000000UL));
	DEBUG_PRINTF_("\r\n");

	setup(CountParams());
}

/**
 * @brief	Destructor
 */
NiosTimer::~NiosTimer()
{
	if (kIC_ID != kINVALID_VALUE) { alt_ic_irq_disable(kIC_ID, kIRQ); }
	IOWR_ALTERA_AVALON_TIMER_CONTROL(kBASE_ADDR, ALTERA_AVALON_TIMER_CONTROL_STOP_MSK);
	IOWR_ALTERA_AVALON_TIMER_PERIODL(kBASE_ADDR, 0);
	IOWR_ALTERA_AVALON_TIMER_PERIODH(kBASE_ADDR, 0);
	IOWR_ALTERA_AVALON_TIMER_STATUS(kBASE_ADDR, 0);
}

/**
 * @brief	Set up
 * @param	params			CountParams
 * @retval	0				success
 * @retval	!=0				failure
 */
int NiosTimer::setup(const CountParams& params)
{
	if (params.method_ == kCOUNT_METHOD_UP) { return 1; }

	if (kIC_ID != kINVALID_VALUE) { alt_ic_irq_disable(kIC_ID, kIRQ); }
	IOWR_ALTERA_AVALON_TIMER_CONTROL(kBASE_ADDR, ALTERA_AVALON_TIMER_CONTROL_STOP_MSK);
	IOWR_ALTERA_AVALON_TIMER_PERIODL(kBASE_ADDR, static_cast<uint16_t>(params.loadCountValue_));
	IOWR_ALTERA_AVALON_TIMER_PERIODH(kBASE_ADDR, static_cast<uint16_t>(params.loadCountValue_ >> 16));

	controlFlags_ = (params.reload_ == kRELOAD_DISABLE) ? 0 : ALTERA_AVALON_TIMER_CONTROL_CONT_MSK;

	return 0;
}

/**
 * @brief	Start counting
 * @return	none
 */
void NiosTimer::start()
{
	IOWR_ALTERA_AVALON_TIMER_CONTROL(kBASE_ADDR, (controlFlags_ | ALTERA_AVALON_TIMER_CONTROL_START_MSK));
}

/**
 * @brief	Stop counting
 * @return	none
 */
void NiosTimer::stop()
{
	IOWR_ALTERA_AVALON_TIMER_CONTROL(kBASE_ADDR, (controlFlags_ | ALTERA_AVALON_TIMER_CONTROL_STOP_MSK));
}

/**
 * @brief	Read counter value
 * @return	counter value
 */
uint32_t NiosTimer::readCounter() const
{
	const alt_irq_context context = alt_irq_disable_all();
	IOWR_ALTERA_AVALON_TIMER_SNAPL(kBASE_ADDR, 0);
	const uint32_t counter = ((IORD_ALTERA_AVALON_TIMER_SNAPH(kBASE_ADDR) << 16) | IORD_ALTERA_AVALON_TIMER_SNAPL(kBASE_ADDR));
	alt_irq_enable_all(context);

	return counter;
}

/**
 * @brief	Get frequency
 * @return	frequency
 */
uint32_t NiosTimer::getFrequency() const
{
	return kFREQ;
}

/**
 * @brief	Set up interrupt
 * @param	callback_func	callback function
 * @param	callback_arg	arguments for the callback function
 * @retval	0				success
 * @retval	!=0				failure
 */
int NiosTimer::setupInterrupt(const GenCallbackFunc callback_func, void* const callback_arg)
{
	ASSERT_(kIC_ID != kINVALID_VALUE);
	ASSERT_(callback_func);

	alt_ic_irq_disable(kIC_ID, kIRQ);
	controlFlags_ |= ALTERA_AVALON_TIMER_CONTROL_ITO_MSK;
	callbackFunc_ = callback_func;
	callbackArg_ = callback_arg;

	IOWR_ALTERA_AVALON_TIMER_STATUS(kBASE_ADDR, 0);
	alt_ic_isr_register(kIC_ID, kIRQ, interruptServiceRoutine, this, (void*)0);
	alt_ic_irq_enable(kIC_ID, kIRQ);

	return 0;
}

/**
 * @brief	Enable interrupt
 * @return	none
 */
void NiosTimer::enableInterrupt()
{
	ASSERT_(kIC_ID != kINVALID_VALUE);
	ASSERT_(controlFlags_ & ALTERA_AVALON_TIMER_CONTROL_ITO_MSK);

	alt_ic_irq_enable(kIC_ID, kIRQ);
}

/**
 * @brief	Disable interrupt
 * @return	none
 */
void NiosTimer::disableInterrupt()
{
	ASSERT_(kIC_ID != kINVALID_VALUE);
	ASSERT_(controlFlags_ & ALTERA_AVALON_TIMER_CONTROL_ITO_MSK);

	alt_ic_irq_disable(kIC_ID, kIRQ);
}

/**
 * @brief	Interrupt Service Routine
 * @param	isr_context		ISR context
 * @return	none
 */
void NiosTimer::interruptServiceRoutine(void* const isr_context)
{
	NiosTimer* const instance = reinterpret_cast<NiosTimer*>(isr_context);
	IOWR_ALTERA_AVALON_TIMER_STATUS(instance->kBASE_ADDR, 0);
	instance->callbackFunc_(instance->callbackArg_);
}

} /* namespace device */

} /* namespace sdpses */
