/**
 * @file	mb_timer.cpp
 * @brief	Xilinx timer/counter
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
#include "xtmrctr_l.h"
#include "mb_timer.h"
#include "lib_assert.h"
#include "lib_debug.h"

namespace sdpses {

namespace device {

const uint32_t MbTimer::kTMR_NUM0 = 0;
const uint32_t MbTimer::kTMR_NUM1 = 1;

/**
 * @brief	Constructor
 * @param	base_addr		base address
 * @param	freq			frequency(Hz)
 * @param	ic_base			intc base address
 * @param	irq				irq number
 */
MbTimer::MbTimer(const uint32_t base_addr, const uint32_t freq,
		const uint32_t ic_base, const uint32_t irq)
	: kBASE_ADDR(base_addr)
	, kFREQ(freq)
	, kIC_BASE(ic_base)
	, kIRQ(irq)
	, kIRQ_MASK(1UL << irq)
	, interruptFlags_(0)
	, callbackFunc_(0)
	, callbackArg_(0)
{
	DEBUG_PRINTF_("<MicroBlaze Timer parameters>\r\n");
	DEBUG_PRINTF_("  BASE_ADDR     : [H'%08lX]\r\n", base_addr);
	DEBUG_PRINTF_("  FREQ          : [%lu", (freq / 1000000UL));
	DEBUG_PRINTF_(".%luMHz]\r\n", (freq % 1000000UL));
	DEBUG_PRINTF_("  IC BASE       : [H'%08lX]\r\n", ic_base);
	DEBUG_PRINTF_("  IRQ           : [%lu]\r\n", irq);
	DEBUG_PRINTF_("\r\n");

	setup(CountParams());
}

/**
 * @brief	Constructor
 * @param	base_addr		base address
 * @param	freq			frequency(Hz)
 */
MbTimer::MbTimer(const uint32_t base_addr, const uint32_t freq)
	: kBASE_ADDR(base_addr)
	, kFREQ(freq)
	, kIC_BASE(0)
	, kIRQ(0)
	, kIRQ_MASK(0)
	, interruptFlags_(0)
	, callbackFunc_(0)
	, callbackArg_(0)
{
	DEBUG_PRINTF_("<MicroBlaze Timer parameters>\r\n");
	DEBUG_PRINTF_("  BASE_ADDR     : [H'%08lX]\r\n", base_addr);
	DEBUG_PRINTF_("  FREQ          : [%lu", (freq / 1000000UL));
	DEBUG_PRINTF_(".%luMHz]\r\n", (freq % 1000000UL));
	DEBUG_PRINTF_("\r\n");

	setup(CountParams());
}

/**
 * @brief	Destructor
 */
MbTimer::~MbTimer()
{
	if (kIRQ_MASK) { XIntc_DisableIntr(kIC_BASE, kIRQ_MASK); }
	XTmrCtr_DisableIntr(kBASE_ADDR, kTMR_NUM0);
	XTmrCtr_SetControlStatusReg(kBASE_ADDR, kTMR_NUM0, 0);
}

/**
 * @brief	Set up
 * @param	params			CountParams
 * @retval	0				success
 * @retval	!=0				failure
 */
int MbTimer::setup(const CountParams& params)
{
	XTmrCtr_DisableIntr(kBASE_ADDR, kTMR_NUM0);
	XTmrCtr_SetControlStatusReg(kBASE_ADDR, kTMR_NUM0, 0);
	XTmrCtr_SetLoadReg(kBASE_ADDR, kTMR_NUM0, params.loadCountValue_);

	uint32_t csr = 0;
	csr |= (params.reload_ == kRELOAD_DISABLE) ? 0 : XTC_CSR_AUTO_RELOAD_MASK;
	csr |= (params.method_ == kCOUNT_METHOD_UP) ? 0 : XTC_CSR_DOWN_COUNT_MASK;
	XTmrCtr_SetControlStatusReg(kBASE_ADDR, kTMR_NUM0, csr);

	return 0;
}

/**
 * @brief	Start counting
 * @return	none
 */
void MbTimer::start()
{
	uint32_t csr = XTmrCtr_GetControlStatusReg(kBASE_ADDR, kTMR_NUM0);
	csr |= XTC_CSR_ENABLE_TMR_MASK;
	csr |= interruptFlags_;
	XTmrCtr_SetControlStatusReg(kBASE_ADDR, kTMR_NUM0, csr);
}

/**
 * @brief	Stop counting
 * @return	none
 */
void MbTimer::stop()
{
	uint32_t csr = XTmrCtr_GetControlStatusReg(kBASE_ADDR, kTMR_NUM0);
	csr &= ~XTC_CSR_ENABLE_TMR_MASK;
	csr &= ~XTC_CSR_ENABLE_INT_MASK;
	XTmrCtr_SetControlStatusReg(kBASE_ADDR, kTMR_NUM0, csr);
}

/**
 * @brief	Read counter value
 * @return	counter value
 */
uint32_t MbTimer::readCounter() const
{
	return XTmrCtr_GetTimerCounterReg(kBASE_ADDR, kTMR_NUM0);
}

/**
 * @brief	Get frequency
 * @return	frequency
 */
uint32_t MbTimer::getFrequency() const
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
int MbTimer::setupInterrupt(const GenCallbackFunc callback_func, void* const callback_arg)
{
	ASSERT_(kIRQ_MASK);
	ASSERT_(callback_func);

	XIntc_DisableIntr(kIC_BASE, kIRQ_MASK);
	interruptFlags_ = XTC_CSR_ENABLE_INT_MASK;
	callbackFunc_ = callback_func;
	callbackArg_ = callback_arg;

	XIntc_RegisterHandler(kIC_BASE, kIRQ, interruptHandler, this);
	XIntc_EnableIntr(kIC_BASE, kIRQ_MASK);

	return 0;
}

/**
 * @brief	Enable interrupt
 * @return	none
 */
void MbTimer::enableInterrupt()
{
	ASSERT_(kIRQ_MASK);
	ASSERT_(interruptFlags_ & XTC_CSR_ENABLE_INT_MASK);

	XIntc_EnableIntr(kIC_BASE, kIRQ_MASK);
}

/**
 * @brief	Disable interrupt
 * @return	none
 */
void MbTimer::disableInterrupt()
{
	ASSERT_(kIRQ_MASK);
	ASSERT_(interruptFlags_ & XTC_CSR_ENABLE_INT_MASK);

	XIntc_DisableIntr(kIC_BASE, kIRQ_MASK);
}

/**
 * @brief	Interrupt Handler
 * @param	context			context
 * @return	none
 */
void MbTimer::interruptHandler(void* const context)
{
	MbTimer* const instance = reinterpret_cast<MbTimer*>(context);
	const uint32_t csr = XTmrCtr_GetControlStatusReg(instance->kBASE_ADDR, kTMR_NUM0);
	if (csr & XTC_CSR_INT_OCCURED_MASK) {
		XTmrCtr_SetControlStatusReg(instance->kBASE_ADDR, kTMR_NUM0, csr);
	}
	instance->callbackFunc_(instance->callbackArg_);

	XIntc_AckIntr(instance->kIC_BASE, instance->kIRQ_MASK);
}

} /* namespace device */

} /* namespace sdpses */
