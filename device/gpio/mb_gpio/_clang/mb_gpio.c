/**
 * @file	mb_gpio.c
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

#include "allocator.h"
#include "xgpio_l.h"
#include "gpio_private.h"
#include "mb_gpio.h"
#include "lib_assert.h"
#include "lib_debug.h"

/**
 * @struct	MbGpio
 * @brief	MbGpio struct
 * @extends	Gpio
 */
struct MbGpio {
	struct Gpio gpio; /*!< must be the first member for mutual conversion of pointers */

	uint32_t baseAddr;
	uint32_t icBase;
	uint32_t irq;
	uint32_t irqMask;

	uint32_t			interruptFlags;
	Gpio_CallbackFunc	callbackFunc;
	void*				callbackArg;
};

static const uint32_t kDIRECTION_OUTPUT		= 0;
static const uint32_t kDIRECTION_INPUT		= 1;

static const uint32_t kPORTINT_DISABLE		= 0;
static const uint32_t kPORTINT_ENABLE		= 1;

static void interruptHandler(void* context);
static void assignVirtualFunctions(struct MbGpio* instance);

/**
 * @brief	Get the size of MbGpio
 * @return	the size of MbGpio
 */
size_t MbGpio_sizeOf(void)
{
	return sizeof(struct MbGpio);
}

/**
 * @brief	Create
 * @param	base_addr		base address
 * @return	instance
 */
struct MbGpio* MbGpio_create(const uint32_t base_addr)
{
	return MbGpio_createWithInterrupt(base_addr, 0, 0);
}

/**
 * @brief	Create(with interrupt)
 * @param	base_addr		base address
 * @param	ic_base			intc base address
 * @param	irq				irq number
 * @return	instance
 */
struct MbGpio* MbGpio_createWithInterrupt(const uint32_t base_addr,
		const uint32_t ic_base, const uint32_t irq)
{
	struct MbGpio* const instance = Allocator_allocate(sizeof(struct MbGpio));
	if (!instance) {
		DEBUG_PRINTF_("Cannot allocate memory\r\n");
		return NULL;
	}

	if (MbGpio_ctorWithInterrupt(instance, base_addr, ic_base, irq)) {
		Allocator_deallocate(instance);
		return NULL;
	}

	return instance;
}

/**
 * @brief	Destroy
 * @param	self			Gpio*
 * @return	Gpio*
 */
struct Gpio* MbGpio_destroy(struct Gpio* const self)
{
	if (!self) { return NULL; }

	struct MbGpio* const instance = (struct MbGpio*)self;
	MbGpio_dtor(instance);
	Allocator_deallocate(instance);

	return NULL;
}

/**
 * @brief	Constructor
 * @param	instance		instance
 * @param	base_addr		base address
 * @retval	0				success
 * @retval	!=0				failure
 */
int MbGpio_ctor(struct MbGpio* const instance, const uint32_t base_addr)
{
	return MbGpio_ctorWithInterrupt(instance, base_addr, 0, 0);
}

/**
 * @brief	Constructor(with interrupt)
 * @param	instance		instance
 * @param	base_addr		base address
 * @param	ic_base			intc base address
 * @param	irq				irq number
 * @retval	0				success
 * @retval	!=0				failure
 */
int MbGpio_ctorWithInterrupt(struct MbGpio* const instance,
		const uint32_t base_addr, const uint32_t ic_base, const uint32_t irq)
{
	DEBUG_PRINTF_("<MicroBlaze GPIO parameters>\r\n");
	DEBUG_PRINTF_("  BASE_ADDR     : [H'%08lX]\r\n", base_addr);
	if (ic_base) {
		DEBUG_PRINTF_("  IC BASE       : [H'%08lX]\r\n", ic_base);
		DEBUG_PRINTF_("  IRQ           : [%u]\r\n", (unsigned int)irq);
	}
	DEBUG_PRINTF_("\r\n");

	if (Gpio_ctor((struct Gpio*)instance)) { return 1; }

	assignVirtualFunctions(instance);

	instance->baseAddr = base_addr;
	instance->icBase = ic_base;
	instance->irq = irq;
	instance->irqMask = ic_base ? (1UL << irq) : 0;

	instance->interruptFlags = 0;
	instance->callbackFunc = NULL;
	instance->callbackArg = NULL;

	XGpio_WriteReg(instance->baseAddr, XGPIO_GIE_OFFSET, ~XGPIO_GIE_GINTR_ENABLE_MASK);

	return 0;
}

/**
 * @brief	Destructor
 * @param	instance		instance
 * @return	none
 */
void MbGpio_dtor(struct MbGpio* const instance)
{
	if (!instance) { return; }

	if (instance->irqMask) {
		XIntc_DisableIntr(instance->icBase, instance->irqMask);
	}
	XGpio_WriteReg(instance->baseAddr, XGPIO_GIE_OFFSET, ~XGPIO_GIE_GINTR_ENABLE_MASK);
	XGpio_WriteReg(instance->baseAddr, XGPIO_IER_OFFSET, 0);

	Gpio_dtor((struct Gpio*)instance);
}

/**
 * @brief	Write data
 * @param	self			Gpio*
 * @param	data			data
 * @return	none
 */
void MbGpio_writeData(struct Gpio* const self, const uint32_t data)
{
	XGpio_WriteReg(((struct MbGpio*)self)->baseAddr, XGPIO_DATA_OFFSET, data);
}

/**
 * @brief	Read data
 * @param	self			Gpio*
 * @return	data
 */
uint32_t MbGpio_readData(struct Gpio* const self)
{
	return XGpio_ReadReg(((struct MbGpio*)self)->baseAddr, XGPIO_DATA_OFFSET);
}

/**
 * @brief	Set multiple bits
 * @param	self			Gpio*
 * @param	bitmask			[1:set, 0:unaffected]
 * @return	none
 */
void MbGpio_setDataBit(struct Gpio* const self, uint32_t bitmask)
{
	Gpio_setDataBit(self, bitmask);
}

/**
 * @brief	Clear multiple bits
 * @param	self			Gpio*
 * @param	bitmask			[1:set to zero, 0:unaffected]
 * @return	none
 */
void MbGpio_clearDataBit(struct Gpio* const self, uint32_t bitmask)
{
	Gpio_clearDataBit(self, bitmask);
}

/**
 * @brief	Write the input/output direction
 * @param	self			Gpio*
 * @param	direction		[1:output, 0:input]
 * @return	none
 */
void MbGpio_writeDirection(struct Gpio* const self, const uint32_t direction)
{
	XGpio_WriteReg(((struct MbGpio*)self)->baseAddr, XGPIO_TRI_OFFSET, ~direction);
}

/**
 * @brief	Read the input/output direction
 * @param	self			Gpio*
 * @return	direction [1:output, 0:input]
 */
uint32_t MbGpio_readDirection(struct Gpio* const self)
{
	return ~XGpio_ReadReg(((struct MbGpio*)self)->baseAddr, XGPIO_TRI_OFFSET);
}

/**
 * @brief	Set the output direction
 * @param	self			Gpio*
 * @param	bitmask			[1:output, 0:unaffected]
 * @return	none
 */
void MbGpio_setOutputBit(struct Gpio* const self, uint32_t bitmask)
{
	Gpio_setOutputBit(self, bitmask);
}

/**
 * @brief	Set the input direction
 * @param	self			Gpio*
 * @param	bitmask			[1:input, 0:unaffected]
 * @return	none
 */
void MbGpio_setInputBit(struct Gpio* const self, uint32_t bitmask)
{
	Gpio_setInputBit(self, bitmask);
}

/**
 * @brief	Set up interrupt
 * @param	self			Gpio*
 * @param	interrupt_bits	[1:enable, 0:disable]
 * @param	callback_func	callback function
 * @param	callback_arg	arguments for the callback function
 * @retval	0				success
 * @retval	!=0				failure
 */
int MbGpio_setupInterrupt(struct Gpio* const self, const uint32_t interrupt_bits,
		const Gpio_CallbackFunc callback_func, void* const callback_arg)
{
	struct MbGpio* const instance = (struct MbGpio*)self;

	ASSERT_(instance->irqMask);
	ASSERT_(callback_func);

	instance->interruptFlags = interrupt_bits;
	instance->callbackFunc = callback_func;
	instance->callbackArg = callback_arg;

	XGpio_WriteReg(instance->baseAddr, XGPIO_IER_OFFSET, instance->interruptFlags);
	XGpio_WriteReg(instance->baseAddr, XGPIO_GIE_OFFSET, XGPIO_GIE_GINTR_ENABLE_MASK);

	XIntc_RegisterHandler(instance->icBase, instance->irq, (XInterruptHandler)interruptHandler, instance);
	XIntc_EnableIntr(instance->icBase, instance->irqMask);

	return 0;
}

/**
 * @brief	Enable multiple interrupts for each input port
 * @param	self			Gpio*
 * @param	bitmask			[1:enable, 0:unaffected]
 * @return	none
 */
void MbGpio_enableMultipleInterrupts(struct Gpio* const self, const uint32_t bitmask)
{
	struct MbGpio* const instance = (struct MbGpio*)self;

	ASSERT_(instance->irqMask);

	XIntc_DisableIntr(instance->icBase, instance->irqMask);
	instance->interruptFlags |= bitmask;
	XGpio_WriteReg(instance->baseAddr, XGPIO_IER_OFFSET, instance->interruptFlags);
	XIntc_EnableIntr(instance->icBase, instance->irqMask);
}

/**
 * @brief	Disable multiple interrupts for each input port
 * @param	self			Gpio*
 * @param	bitmask			[1:disable, 0:unaffected]
 * @return	none
 */
void MbGpio_disableMultipleInterrupts(struct Gpio* const self, const uint32_t bitmask)
{
	struct MbGpio* const instance = (struct MbGpio*)self;

	ASSERT_(instance->irqMask);

	XIntc_DisableIntr(instance->icBase, instance->irqMask);
	instance->interruptFlags &= ~bitmask;
	XGpio_WriteReg(instance->baseAddr, XGPIO_IER_OFFSET, instance->interruptFlags);
	XIntc_EnableIntr(instance->icBase, instance->irqMask);
}

/**
 * @brief	Enable interrupt
 * @param	self			Gpio*
 * @return	none
 */
void MbGpio_enableInterrupt(struct Gpio* const self)
{
	struct MbGpio* const instance = (struct MbGpio*)self;

	ASSERT_(instance->irqMask);

	XIntc_EnableIntr(instance->icBase, instance->irqMask);
}

/**
 * @brief	Disable interrupt
 * @param	self			Gpio*
 * @return	none
 */
void MbGpio_disableInterrupt(struct Gpio* const self)
{
	struct MbGpio* const instance = (struct MbGpio*)self;

	ASSERT_(instance->irqMask);

	XIntc_DisableIntr(instance->icBase, instance->irqMask);
}

/**
 * @brief	Interrupt Handler
 * @param	context			context
 * @return	none
 */
static void interruptHandler(void* const context)
{
	struct MbGpio* const instance = (struct MbGpio*)context;
	const uint32_t status = XGpio_ReadReg(instance->baseAddr, XGPIO_ISR_OFFSET);

	instance->callbackFunc(instance->callbackArg, status);
	XGpio_WriteReg(instance->baseAddr, XGPIO_ISR_OFFSET, status);

	XIntc_AckIntr(instance->icBase, instance->irqMask);
}

/**
 * @brief	Assign virtual functions
 * @param	instance		instance
 * @return	none
 */
static void assignVirtualFunctions(struct MbGpio* const instance)
{
	instance->gpio.destroy						= MbGpio_destroy;

	instance->gpio.writeData					= MbGpio_writeData;
	instance->gpio.readData						= MbGpio_readData;

	instance->gpio.writeDirection				= MbGpio_writeDirection;
	instance->gpio.readDirection				= MbGpio_readDirection;

	instance->gpio.setupInterrupt				= MbGpio_setupInterrupt;
	instance->gpio.enableMultipleInterrupts		= MbGpio_enableMultipleInterrupts;
	instance->gpio.disableMultipleInterrupts	= MbGpio_disableMultipleInterrupts;
	instance->gpio.enableInterrupt				= MbGpio_enableInterrupt;
	instance->gpio.disableInterrupt				= MbGpio_disableInterrupt;
}
