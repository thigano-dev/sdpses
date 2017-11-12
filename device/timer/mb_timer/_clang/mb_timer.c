/**
 * @file	mb_timer.c
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

#include "allocator.h"

#include "xintc_l.h"
#include "xtmrctr_l.h"
#include "timer_private.h"
#include "mb_timer.h"
#include "lib_assert.h"
#include "lib_debug.h"

/**
 * @struct	MbTimer
 * @brief	MbTimer struct
 * @extends	Timer
 */
struct MbTimer {
	struct Timer timer; /*!< must be the first member for mutual conversion of pointers */

	uint32_t baseAddr;
	uint32_t freq;
	uint32_t icBase;
	uint32_t irq;
	uint32_t irqMask;

	uint32_t		interruptFlags;
	GenCallbackFunc	callbackFunc;
	void*			callbackArg;
};

static const uint32_t kTMR_NUM0 = 0;
static const uint32_t kTMR_NUM1 = 1;

static void interruptHandler(void* context);
static void assignVirtualFunctions(struct MbTimer* instance);

/**
 * @brief	Get the size of MbTimer
 * @return	the size of MbTimer
 */
size_t MbTimer_sizeOf(void)
{
	return sizeof(struct MbTimer);
}

/**
 * @brief	Create
 * @param	base_addr		base address
 * @param	freq			frequency(Hz)
 * @return	instance
 */
struct MbTimer* MbTimer_create(const uint32_t base_addr, const uint32_t freq)
{
	return MbTimer_createWithInterrupt(base_addr, freq, 0, 0);
}

/**
 * @brief	Create(with interrupt)
 * @param	base_addr		base address
 * @param	freq			frequency(Hz)
 * @param	ic_base			intc base address
 * @param	irq				irq number
 * @return	instance
 */
struct MbTimer* MbTimer_createWithInterrupt(const uint32_t base_addr,
		const uint32_t freq, const uint32_t ic_base, const uint32_t irq)
{
	struct MbTimer* const instance = Allocator_allocate(sizeof(struct MbTimer));
	if (!instance) {
		DEBUG_PRINTF_("Cannot allocate memory\r\n");
		return NULL;
	}

	if (MbTimer_ctorWithInterrupt(instance, base_addr, freq, ic_base, irq)) {
		Allocator_deallocate(instance);
		return NULL;
	}

	return instance;
}

/**
 * @brief	Destroy
 * @param	self			Timer*
 * @return	Timer*
 */
struct Timer* MbTimer_destroy(struct Timer* const self)
{
	if (!self) { return NULL; }

	struct MbTimer* const instance = (struct MbTimer*)self;
	MbTimer_dtor(instance);
	Allocator_deallocate(instance);

	return NULL;
}

/**
 * @brief	Constructor
 * @param	instance		instance
 * @param	base_addr		base address
 * @param	freq			frequency(Hz)
 * @retval	0				success
 * @retval	!=0				failure
 */
int MbTimer_ctor(struct MbTimer* const instance,
		const uint32_t base_addr, const uint32_t freq)
{
	return MbTimer_ctorWithInterrupt(instance, base_addr, freq, 0, 0);
}

/**
 * @brief	Constructor(with interrupt)
 * @param	instance		instance
 * @param	base_addr		base address
 * @param	freq			frequency(Hz)
 * @param	ic_base			intc base address
 * @param	irq				irq number
 * @retval	0				success
 * @retval	!=0				failure
 */
int MbTimer_ctorWithInterrupt(struct MbTimer* const instance, const uint32_t base_addr,
		const uint32_t freq, const uint32_t ic_base, const uint32_t irq)
{
	DEBUG_PRINTF_("<MicroBlaze Timer parameters>\r\n");
	DEBUG_PRINTF_("  BASE_ADDR     : [H'%08lX]\r\n", base_addr);
	DEBUG_PRINTF_("  FREQ          : [%lu", (freq / 1000000UL));
	DEBUG_PRINTF_(".%luMHz]\r\n", (freq % 1000000UL));
	if (ic_base) {
		DEBUG_PRINTF_("  IC BASE       : [H'%08lX]\r\n", ic_base);
		DEBUG_PRINTF_("  IRQ           : [%lu]\r\n", irq);
	}
	DEBUG_PRINTF_("\r\n");

	if (Timer_ctor((struct Timer*)instance)) { return 1; }

	assignVirtualFunctions(instance);

	instance->baseAddr = base_addr;
	instance->freq = freq;
	instance->icBase = ic_base;
	instance->irq = irq;
	instance->irqMask = ic_base ? (1UL << irq) : 0;

	instance->interruptFlags = 0;
	instance->callbackFunc = NULL;
	instance->callbackArg = NULL;

	const TimerCountParams params = {
		kTIMER_COUNT_METHOD_DEFAULT,
		kTIMER_RELOAD_DEFAULT,
		kLOAD_COUNT_VALUE_DEFAULT
	};
	if (MbTimer_setup((struct Timer*)instance, &params)) { return 1; }

	return 0;
}

/**
 * @brief	Destructor
 * @param	instance		instance
 * @return	none
 */
void MbTimer_dtor(struct MbTimer* const instance)
{
	if (!instance) { return; }

	if (instance->irqMask) { XIntc_DisableIntr(instance->icBase, instance->irqMask); }
	XTmrCtr_DisableIntr(instance->baseAddr, kTMR_NUM0);
	XTmrCtr_SetControlStatusReg(instance->baseAddr, kTMR_NUM0, 0);

	Timer_dtor((struct Timer*)instance);
}

/**
 * @brief	Set up
 * @param	self			Timer*
 * @param	params			TimerCountParams
 * @retval	0				success
 * @retval	!=0				failure
 */
int MbTimer_setup(struct Timer* const self, const TimerCountParams* const params)
{
	struct MbTimer* const instance = (struct MbTimer*)self;

	XTmrCtr_DisableIntr(instance->baseAddr, kTMR_NUM0);
	XTmrCtr_SetControlStatusReg(instance->baseAddr, kTMR_NUM0, 0);
	XTmrCtr_SetLoadReg(instance->baseAddr, kTMR_NUM0, params->loadCountValue);

	uint32_t csr = 0;
	csr |= (params->reload == kTIMER_RELOAD_DISABLE) ?  0 : XTC_CSR_AUTO_RELOAD_MASK;
	csr |= (params->method == kTIMER_COUNT_METHOD_UP) ? 0 : XTC_CSR_DOWN_COUNT_MASK;
	XTmrCtr_SetControlStatusReg(instance->baseAddr, kTMR_NUM0, csr);

	return 0;
}

/**
 * @brief	Start counting
 * @param	self			Timer*
 * @return	none
 */
void MbTimer_start(struct Timer* const self)
{
	struct MbTimer* const instance = (struct MbTimer*)self;

	uint32_t csr = XTmrCtr_GetControlStatusReg(instance->baseAddr, kTMR_NUM0);
	csr |= XTC_CSR_ENABLE_TMR_MASK;
	csr |= instance->interruptFlags;
	XTmrCtr_SetControlStatusReg(instance->baseAddr, kTMR_NUM0, csr);
}

/**
 * @brief	Stop counting
 * @param	self			Timer*
 * @return	none
 */
void MbTimer_stop(struct Timer* const self)
{
	struct MbTimer* const instance = (struct MbTimer*)self;

	uint32_t csr = XTmrCtr_GetControlStatusReg(instance->baseAddr, kTMR_NUM0);
	csr &= ~XTC_CSR_ENABLE_TMR_MASK;
	csr &= ~XTC_CSR_ENABLE_INT_MASK;
	XTmrCtr_SetControlStatusReg(instance->baseAddr, kTMR_NUM0, csr);
}

/**
 * @brief	Read counter value
 * @param	self			Timer*
 * @return	counter value
 */
uint32_t MbTimer_readCounter(const struct Timer* const self)
{
	return XTmrCtr_GetTimerCounterReg(((const struct MbTimer*)self)->baseAddr, kTMR_NUM0);
}

/**
 * @brief	Get frequency
 * @param	self			Timer*
 * @return	frequency
 */
uint32_t MbTimer_getFrequency(const struct Timer* const self)
{
	return ((const struct MbTimer*)self)->freq;
}

/**
 * @brief	Set up interrupt
 * @param	self			Timer*
 * @param	callback_func	callback function
 * @param	callback_arg	arguments for the callback function
 * @retval	0				success
 * @retval	!=0				failure
 */
int MbTimer_setupInterrupt(struct Timer* const self,
		const GenCallbackFunc callback_func, void* const callback_arg)
{
	struct MbTimer* const instance = (struct MbTimer*)self;

	ASSERT_(instance->irqMask);
	ASSERT_(callback_func);

	XIntc_DisableIntr(instance->icBase, instance->irqMask);
	instance->interruptFlags = XTC_CSR_ENABLE_INT_MASK;
	instance->callbackFunc = callback_func;
	instance->callbackArg = callback_arg;

	XIntc_RegisterHandler(instance->icBase, instance->irq, interruptHandler, instance);
	XIntc_EnableIntr(instance->icBase, instance->irqMask);

	return 0;
}

/**
 * @brief	Enable interrupt
 * @param	self			Timer*
 * @return	none
 */
void MbTimer_enableInterrupt(struct Timer* const self)
{
	struct MbTimer* const instance = (struct MbTimer*)self;

	ASSERT_(instance->irqMask);
	ASSERT_(instance->interruptFlags & XTC_CSR_ENABLE_INT_MASK);

	XIntc_EnableIntr(instance->icBase, instance->irqMask);
}

/**
 * @brief	Disable interrupt
 * @param	self			Timer*
 * @return	none
 */
void MbTimer_disableInterrupt(struct Timer* const self)
{
	struct MbTimer* const instance = (struct MbTimer*)self;

	ASSERT_(instance->irqMask);
	ASSERT_(instance->interruptFlags & XTC_CSR_ENABLE_INT_MASK);

	XIntc_DisableIntr(instance->icBase, instance->irqMask);
}

/**
 * @brief	Interrupt Handler
 * @param	context			context
 * @return	none
 */
static void interruptHandler(void* const context)
{
	struct MbTimer* const instance = (struct MbTimer*)context;
	const uint32_t csr = XTmrCtr_GetControlStatusReg(instance->baseAddr, kTMR_NUM0);
	if (csr & XTC_CSR_INT_OCCURED_MASK) {
		XTmrCtr_SetControlStatusReg(instance->baseAddr, kTMR_NUM0, csr);
	}
	instance->callbackFunc(instance->callbackArg);

	XIntc_AckIntr(instance->icBase, instance->irqMask);
}

/**
 * @brief	Assign virtual functions
 * @param	instance		instance
 * @return	none
 */
static void assignVirtualFunctions(struct MbTimer* const instance)
{
	instance->timer.destroy				= MbTimer_destroy;
	instance->timer.setup				= MbTimer_setup;

	instance->timer.start				= MbTimer_start;
	instance->timer.stop				= MbTimer_stop;

	instance->timer.readCounter			= MbTimer_readCounter;
	instance->timer.getFrequency		= MbTimer_getFrequency;

	instance->timer.setupInterrupt		= MbTimer_setupInterrupt;
	instance->timer.enableInterrupt		= MbTimer_enableInterrupt;
	instance->timer.disableInterrupt	= MbTimer_disableInterrupt;
}
