/**
 * @file	nios_timer.c
 * @brief	Altera Avalon Timer
 * @author	Tsuguyoshi Higano
 * @date	Mar 25, 2018
 *
 * @par Project
 * Software Development Platform for Small-scale Embedded Systems (SDPSES)
 *
 * @copyright (c) Tsuguyoshi Higano, 2017-2018
 *
 * @par License
 * Released under the MIT license@n
 * http://opensource.org/licenses/mit-license.php
 */

#include <sys/alt_irq.h>

#include "allocator.h"

#include "altera_avalon_timer_regs.h"
#include "timer_private.h"
#include "nios_timer.h"
#include "lib_assert.h"
#include "lib_debug.h"

/**
 * @struct	NiosTimer
 * @brief	NiosTimer struct
 * @extends	Timer
 */
struct NiosTimer {
	struct Timer timer; /*!< must be the first member for mutual conversion of pointers */

	uint32_t baseAddr;
	uint32_t freq;
	uint32_t icId;
	uint32_t irq;

	uint16_t		controlFlags;
	GenCallbackFunc	callbackFunc;
	void*			callbackArg;
};

static const uint32_t kINVALID_VALUE = 0xFFFFFFFFUL;

static void interruptServiceRoutine(void* isr_context);
static void assignVirtualFunctions(struct NiosTimer* instance);

/**
 * @brief	Get the size of NiosTimer
 * @return	the size of NiosTimer
 */
size_t NiosTimer_sizeOf(void)
{
	return sizeof(struct NiosTimer);
}

/**
 * @brief	Create
 * @param	base_addr		base address
 * @param	freq			frequency(Hz)
 * @return	instance
 */
struct NiosTimer* NiosTimer_create(const uint32_t base_addr, const uint32_t freq)
{
	return NiosTimer_createWithInterrupt(base_addr, freq, kINVALID_VALUE, kINVALID_VALUE);
}

/**
 * @brief	Create(with interrupt)
 * @param	base_addr		base address
 * @param	freq			frequency(Hz)
 * @param	ic_id			intc id
 * @param	irq				irq number
 * @return	instance
 */
struct NiosTimer* NiosTimer_createWithInterrupt(const uint32_t base_addr,
		const uint32_t freq, const uint32_t ic_id, const uint32_t irq)
{
	struct NiosTimer* const instance = Allocator_allocate(sizeof(struct NiosTimer));
	if (!instance) {
		DEBUG_PRINTF_("Cannot allocate memory\r\n");
		return NULL;
	}

	if (NiosTimer_ctorWithInterrupt(instance, base_addr, freq, ic_id, irq)) {
		Allocator_deallocate(instance);
		return NULL;
	}

	return instance;
}

/**
 * @brief	Destructor
 * @param	self			Timer*
 * @return	Timer*
 */
struct Timer* NiosTimer_destroy(struct Timer* const self)
{
	if (!self) { return NULL; }

	struct NiosTimer* const instance = (struct NiosTimer*)self;
	NiosTimer_dtor(instance);
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
int NiosTimer_ctor(struct NiosTimer* const instance, const uint32_t base_addr, const uint32_t freq)
{
	return NiosTimer_ctorWithInterrupt(instance, base_addr, freq, kINVALID_VALUE, kINVALID_VALUE);
}

/**
 * @brief	Constructor(with interrupt)
 * @param	instance		instance
 * @param	base_addr		base address
 * @param	freq			frequency(Hz)
 * @param	ic_id			intc id
 * @param	irq				irq number
 * @retval	0				success
 * @retval	!=0				failure
 */
int NiosTimer_ctorWithInterrupt(struct NiosTimer* const instance,
		const uint32_t base_addr, const uint32_t freq, const uint32_t ic_id, const uint32_t irq)
{
	DEBUG_PRINTF_("<NiosII Timer parameters>\r\n");
	DEBUG_PRINTF_("  BASE ADDR     : [H'%08lX]\r\n", base_addr);
	DEBUG_PRINTF_("  FREQ          : [%lu.", (freq / 1000000UL));
	DEBUG_PRINTF_("%luMHz]\r\n", (freq % 1000000UL));
	if (ic_id != kINVALID_VALUE) {
		DEBUG_PRINTF_("  IC ID         : [");
		if (ic_id == 0UL) {
			DEBUG_PRINTF_("IIC: in NiosII Core]\r\n");
		} else {
			DEBUG_PRINTF_("EIC: H'%08lX]\r\n", ic_id);
		}
		DEBUG_PRINTF_("  IRQ           : [%d]\r\n", (int)irq);
	}
	DEBUG_PRINTF_("\r\n");

	if (Timer_ctor((struct Timer*)instance)) { return 1; }

	assignVirtualFunctions(instance);

	instance->baseAddr = base_addr;
	instance->freq = freq;
	instance->icId = ic_id;
	instance->irq = irq;

	instance->controlFlags = 0;
	instance->callbackFunc = NULL;
	instance->callbackArg = NULL;

	const TimerCountParams params = {
		kTIMER_COUNT_METHOD_DEFAULT,
		kTIMER_RELOAD_DEFAULT,
		kLOAD_COUNT_VALUE_DEFAULT
	};
	if (NiosTimer_setup((struct Timer*)instance, &params)) { return 1; }

	return 0;
}

/**
 * @brief	Destructor
 * @param	instance		instance
 * @return	none
 */
void NiosTimer_dtor(struct NiosTimer* const instance)
{
	if (!instance) { return; }

	if (instance->icId != kINVALID_VALUE) {
		alt_ic_irq_disable(instance->icId, instance->irq);
	}
	IOWR_ALTERA_AVALON_TIMER_CONTROL(instance->baseAddr, ALTERA_AVALON_TIMER_CONTROL_STOP_MSK);
	IOWR_ALTERA_AVALON_TIMER_PERIODL(instance->baseAddr, 0);
	IOWR_ALTERA_AVALON_TIMER_PERIODH(instance->baseAddr, 0);
	IOWR_ALTERA_AVALON_TIMER_STATUS(instance->baseAddr, 0);

	Timer_dtor((struct Timer*)instance);
}

/**
 * @brief	Set up
 * @param	self			Timer*
 * @param	params			TimerCountParams*
 * @retval	0				success
 * @retval	!=0				failure
 */
int NiosTimer_setup(struct Timer* const self, const TimerCountParams* const params)
{
	if (params->method == kTIMER_COUNT_METHOD_UP) { return 1; }

	NiosTimer_disableInterrupt(self);
	struct NiosTimer* const instance = (struct NiosTimer*)self;

	IOWR_ALTERA_AVALON_TIMER_CONTROL(instance->baseAddr, ALTERA_AVALON_TIMER_CONTROL_STOP_MSK);
	IOWR_ALTERA_AVALON_TIMER_PERIODL(instance->baseAddr, (uint16_t)params->loadCountValue);
	IOWR_ALTERA_AVALON_TIMER_PERIODH(instance->baseAddr, (uint16_t)(params->loadCountValue >> 16));

	instance->controlFlags = (params->reload == kTIMER_RELOAD_DISABLE) ? 0 : ALTERA_AVALON_TIMER_CONTROL_CONT_MSK;

	return 0;
}

/**
 * @brief	Start counting
 * @param	self			Timer*
 * @return	none
 */
void NiosTimer_start(struct Timer* const self)
{
	struct NiosTimer* const instance = (struct NiosTimer*)self;
	IOWR_ALTERA_AVALON_TIMER_CONTROL(instance->baseAddr, (instance->controlFlags | ALTERA_AVALON_TIMER_CONTROL_START_MSK));
}

/**
 * @brief	Stop counting
 * @param	self			Timer*
 * @return	none
 */
void NiosTimer_stop(struct Timer* const self)
{
	struct NiosTimer* const instance = (struct NiosTimer*)self;
	IOWR_ALTERA_AVALON_TIMER_CONTROL(instance->baseAddr, (instance->controlFlags | ALTERA_AVALON_TIMER_CONTROL_STOP_MSK));
}

/**
 * @brief	Read counter value
 * @param	self			Timer*
 * @return	counter value
 */
uint32_t NiosTimer_readCounter(const struct Timer* const self)
{
	struct NiosTimer* const instance = (struct NiosTimer*)self;

	const alt_irq_context context = alt_irq_disable_all();
	IOWR_ALTERA_AVALON_TIMER_SNAPL(instance->baseAddr, 0);
	const uint32_t counter = ((IORD_ALTERA_AVALON_TIMER_SNAPH(instance->baseAddr) << 16) | IORD_ALTERA_AVALON_TIMER_SNAPL(instance->baseAddr));
	alt_irq_enable_all(context);

	return counter;
}

/**
 * @brief	Get frequency
 * @param	self			Timer*
 * @return	frequency
 */
uint32_t NiosTimer_getFrequency(const struct Timer* const self)
{
	return ((const struct NiosTimer*)self)->freq;
}

/**
 * @brief	Set up interrupt
 * @param	self			Timer*
 * @param	callback_func	callback function
 * @param	callback_arg	arguments for the callback function
 * @retval	0				success
 * @retval	!=0				failure
 */
int NiosTimer_setupInterrupt(struct Timer* const self,
		const GenCallbackFunc callback_func, void* const callback_arg)
{
	struct NiosTimer* const instance = (struct NiosTimer*)self;

	ASSERT_(instance->icId != kINVALID_VALUE);
	ASSERT_(callback_func);

	alt_ic_irq_disable(instance->icId, instance->irq);
	instance->controlFlags |= ALTERA_AVALON_TIMER_CONTROL_ITO_MSK;
	instance->callbackFunc = callback_func;
	instance->callbackArg = callback_arg;

	IOWR_ALTERA_AVALON_TIMER_STATUS(instance->baseAddr, 0);
	alt_ic_isr_register(instance->icId, instance->irq, interruptServiceRoutine, instance, (void*)0);
	alt_ic_irq_enable(instance->icId, instance->irq);

	return 0;
}

/**
 * @brief	Enable interrupt
 * @param	self			Timer*
 * @return	none
 */
void NiosTimer_enableInterrupt(struct Timer* const self)
{
	struct NiosTimer* const instance = (struct NiosTimer*)self;

	ASSERT_(instance->icId != kINVALID_VALUE);
	ASSERT_(instance->controlFlags & ALTERA_AVALON_TIMER_CONTROL_ITO_MSK);

	alt_ic_irq_enable(instance->icId, instance->irq);
}

/**
 * @brief	Disable interrupt
 * @param	self			Timer*
 * @return	none
 */
void NiosTimer_disableInterrupt(struct Timer* const self)
{
	struct NiosTimer* const instance = (struct NiosTimer*)self;

	ASSERT_(instance->icId != kINVALID_VALUE);
	ASSERT_(instance->controlFlags & ALTERA_AVALON_TIMER_CONTROL_ITO_MSK);

	alt_ic_irq_disable(instance->icId, instance->irq);
}

/**
 * @brief	Interrupt Service Routine
 * @param	isr_context		ISR context
 * @return	none
 */
static void interruptServiceRoutine(void* const isr_context)
{
	struct NiosTimer* const instance = (struct NiosTimer*)isr_context;
	IOWR_ALTERA_AVALON_TIMER_STATUS(instance->baseAddr, 0);
	instance->callbackFunc(instance->callbackArg);
}

/**
 * @brief	Assign virtual functions
 * @param	instance		instance
 * @return	none
 */
static void assignVirtualFunctions(struct NiosTimer* const instance)
{
	instance->timer.destroy				= NiosTimer_destroy;
	instance->timer.setup				= NiosTimer_setup;

	instance->timer.start				= NiosTimer_start;
	instance->timer.stop				= NiosTimer_stop;

	instance->timer.readCounter			= NiosTimer_readCounter;
	instance->timer.getFrequency		= NiosTimer_getFrequency;

	instance->timer.setupInterrupt		= NiosTimer_setupInterrupt;
	instance->timer.enableInterrupt		= NiosTimer_enableInterrupt;
	instance->timer.disableInterrupt	= NiosTimer_disableInterrupt;
}
