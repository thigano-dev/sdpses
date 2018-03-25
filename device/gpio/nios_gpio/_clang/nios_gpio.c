/**
 * @file	nios_gpio.c
 * @brief	Altera Avalon PIO
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
#include "altera_avalon_pio_regs.h"
#include "gpio_private.h"
#include "nios_gpio.h"
#include "lib_assert.h"
#include "lib_debug.h"

/**
 * @struct	NiosGpio
 * @brief	NiosGpio struct
 * @extends	Gpio
 */
struct NiosGpio {
	struct Gpio gpio; /*!< must be the first member for mutual conversion of pointers */

	uint32_t baseAddr;
	uint32_t icId;
	uint32_t irq;
	NiosGpio_InterruptTrigger intTrg;

	uint32_t			interruptFlags;
	Gpio_CallbackFunc	callbackFunc;
	void*				callbackArg;
};

static const uint32_t kINVALID_VALUE = 0xFFFFFFFFUL;

static void interruptServiceRoutine(void* isr_context);
static void assignVirtualFunctions(struct NiosGpio* instance);

/**
 * @brief	Get the size of NiosGpio
 * @return	the size of NiosGpio
 */
size_t NiosGpio_sizeOf(void)
{
	return sizeof(struct NiosGpio);
}

/**
 * @brief	Create
 * @param	base_addr		base address
 * @return	instance
 */
struct NiosGpio* NiosGpio_create(const uint32_t base_addr)
{
	return NiosGpio_createWithInterrupt(base_addr, kINVALID_VALUE, kINVALID_VALUE);
}

/**
 * @brief	Create(with interrupt)
 * @param	base_addr		base address
 * @param	ic_id			intc id
 * @param	irq				irq number
 * @param	int_trg			NiosGpio_InterruptTrigger
 * @return	instance
 */
struct NiosGpio* NiosGpio_createWithInterrupt(const uint32_t base_addr,
		const uint32_t ic_id, const uint32_t irq, const NiosGpio_InterruptTrigger int_trg)
{
	struct NiosGpio* const instance = Allocator_allocate(sizeof(struct NiosGpio));
	if (!instance) {
		DEBUG_PRINTF_("Cannot allocate memory\r\n");
		return NULL;
	}

	if (NiosGpio_ctorWithInterrupt(instance, base_addr, ic_id, irq, int_trg)) {
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
struct Gpio* NiosGpio_destroy(struct Gpio* const self)
{
	if (!gpio) { return NULL; }

	struct NiosGpio* const instance = (struct NiosGpio*)self;
	NiosGpio_dtor(instance);
	Allocator_deallocate(instance);

	return NULL;
}

/**
 * @brief	Constructor
 * @param	instance		instance
 * @param	base_addr		base address
 * @return	instance
 */
int NiosGpio_ctor(struct NiosGpio* const instance, const uint32_t base_addr)
{
	return NiosGpio_ctorWithInterrupt(instance, base_addr, kINVALID_VALUE, kINVALID_VALUE, kNIOS_GPIO_LEVEL_SENSITIVE);
}

/**
 * @brief	Constructor(with interrupt)
 * @param	instance		instance
 * @param	base_addr		base address
 * @param	ic_id			intc id
 * @param	irq				irq number
 * @param	int_trg			NiosGpio_InterruptTrigger
 * @retval	0				success
 * @retval	!=0				failure
 */
int NiosGpio_ctorWithInterrupt(struct NiosGpio* const instance, const uint32_t base_addr,
		const uint32_t ic_id, const uint32_t irq, const NiosGpio_InterruptTrigger int_trg)
{
	DEBUG_PRINTF_("<NiosII GPIO parameters>\r\n");
	DEBUG_PRINTF_("  BASE ADDR     : [H'%08lX]\r\n", base_addr);
	if (ic_id != kINVALID_VALUE) {
		DEBUG_PRINTF_("  IC ID         : [");
		if (ic_id == 0UL) {
			DEBUG_PRINTF_("IIC: in NiosII Core]\r\n");
		} else {
			DEBUG_PRINTF_("EIC: H'%08lX]\r\n", ic_id);
		}
		DEBUG_PRINTF_("  IRQ           : [%d]\r\n", (int)irq);
		DEBUG_PRINTF_("  INT TRIGGER   : [");
		if (int_trg == kNIOS_GPIO_TRG_LEVEL) {
			DEBUG_PRINTF_("LEVEL SENSITIVE]\r\n");
		} else if (int_trg == kNIOS_GPIO_TRG_EDGE) {
			DEBUG_PRINTF_("EDGE SENSITIVE]\r\n");
		} else {
			DEBUG_PRINTF_("UNKNOWN]\r\n");
		}
	}
	DEBUG_PRINTF_("\r\n");

	if (Gpio_ctor((struct Gpio*)instance)) { return 1; }

	assignVirtualFunctions(instance);

	instance->baseAddr = base_addr;
	instance->icId = ic_id;
	instance->irq = irq;
	instance->intTrg = int_trg;

	instance->interruptFlags = 0;
	instance->callbackFunc = NULL;
	instance->callbackArg = NULL;

	IOWR_ALTERA_AVALON_PIO_IRQ_MASK(instance->baseAddr, 0);
	IOWR_ALTERA_AVALON_PIO_EDGE_CAP(instance->baseAddr, 0);

	return 0;
}

/**
 * @brief	Destructor
 * @param	instance		instance
 * @return	none
 */
void NiosGpio_dtor(struct NiosGpio* const instance)
{
	if (!instance) { return; }

	if (instance->icId != kINVALID_VALUE) {
		alt_ic_irq_disable(instance->icId, instance->irq);
	}
	IOWR_ALTERA_AVALON_PIO_IRQ_MASK(instance->baseAddr, 0);

	Gpio_dtor((struct Gpio*)instance);
}

/**
 * @brief	Write data
 * @param	self			Gpio*
 * @param	data			data
 * @return	none
 */
void NiosGpio_writeData(struct Gpio* const self, const uint32_t data)
{
	IOWR_ALTERA_AVALON_PIO_DATA(((struct NiosGpio*)gpio)->baseAddr, data);
}

/**
 * @brief	Read data
 * @param	self			Gpio*
 * @return	data
 */
uint32_t NiosGpio_readData(struct Gpio* const self)
{
	return IORD_ALTERA_AVALON_PIO_DATA(((struct NiosGpio*)gpio)->baseAddr);
}

/**
 * @brief	Set multiple bits
 * @param	self			Gpio*
 * @param	bitmask			[1:set, 0:unaffected]
 * @return	none
 */
void NiosGpio_setDataBit(struct Gpio* const self, uint32_t bitmask)
{
	Gpio_setDataBit(self, bitmask);
}

/**
 * @brief	Clear multiple bits
 * @param	self			Gpio*
 * @param	bitmask			[1:set to zero, 0:unaffected]
 * @return	none
 */
void NiosGpio_clearDataBit(struct Gpio* const self, uint32_t bitmask)
{
	Gpio_clearDataBit(self, bitmask);
}

/**
 * @brief	Write the input/output direction
 * @param	self			Gpio*
 * @param	direction		[1:output, 0:input]
 * @return	none
 */
void NiosGpio_writeDirection(struct Gpio* const self, const uint32_t direction)
{
	IOWR_ALTERA_AVALON_PIO_DIRECTION(((struct NiosGpio*)gpio)->baseAddr, direction);
}

/**
 * @brief	Read the input/output direction
 * @param	self			Gpio*
 * @return	direction [1:output, 0:input]
 */
uint32_t NiosGpio_readDirection(struct Gpio* const self)
{
	return IORD_ALTERA_AVALON_PIO_DIRECTION(((struct NiosGpio*)gpio)->baseAddr);
}

/**
 * @brief	Set the output direction
 * @param	self			Gpio*
 * @param	bitmask			[1:output, 0:unaffected]
 * @return	none
 */
void NiosGpio_setOutputBit(struct Gpio* const self, uint32_t bitmask)
{
	Gpio_setOutputBit(self, bitmask);
}

/**
 * @brief	Set the input direction
 * @param	self			Gpio*
 * @param	bitmask			[1:input, 0:unaffected]
 * @return	none
 */
void NiosGpio_setInputBit(struct Gpio* const self, uint32_t bitmask)
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
int NiosGpio_setupInterrupt(struct Gpio* const self, const uint32_t interrupt_bits,
		const Gpio_CallbackFunc callback_func, void* const callback_arg)
{
	struct NiosGpio* const instance = (struct NiosGpio*)self;

	ASSERT_(instance->icId != kINVALID_VALUE);
	ASSERT_(callback_func);

	instance->interruptFlags = interrupt_bits;
	instance->callbackFunc = callback_func;
	instance->callbackArg = callback_arg;
	IOWR_ALTERA_AVALON_PIO_IRQ_MASK(instance->baseAddr, instance->interruptFlags);

	alt_ic_isr_register(instance->icId, instance->irq, interruptServiceRoutine, instance, (void*)0);
	alt_ic_irq_enable(instance->icId, instance->irq);

	return 0;
}

/**
 * @brief	Enable multiple interrupts for each input port
 * @param	self			Gpio*
 * @param	bitmask			[1:enable, 0:unaffected]
 * @return	none
 */
void NiosGpio_enableMultipleInterrupts(struct Gpio* const self, const uint32_t bitmask)
{
	struct NiosGpio* const instance = (struct NiosGpio*)self;

	ASSERT_(instance->icId != kINVALID_VALUE);

	alt_ic_irq_disable(instance->icId, instance->irq);
	instance->interruptFlags |= bitmask;
	IOWR_ALTERA_AVALON_PIO_IRQ_MASK(instance->baseAddr, instance->interruptFlags);
	alt_ic_irq_enable(instance->icId, instance->irq);
}

/**
 * @brief	Disable multiple interrupts for each input port
 * @param	self			Gpio*
 * @param	bitmask			[1:disable, 0:unaffected]
 * @return	none
 */
void NiosGpio_disableMultipleInterrupts(struct Gpio* const self, const uint32_t bitmask)
{
	struct NiosGpio* const instance = (struct NiosGpio*)self;

	ASSERT_(instance->icId != kINVALID_VALUE);

	alt_ic_irq_disable(instance->icId, instance->irq);
	instance->interruptFlags &= ~bitmask;
	IOWR_ALTERA_AVALON_PIO_IRQ_MASK(instance->baseAddr, instance->interruptFlags);
	alt_ic_irq_enable(instance->icId, instance->irq);
}

/**
 * @brief	Enable interrupt
 * @param	self			Gpio*
 * @return	none
 */
void NiosGpio_enableInterrupt(struct Gpio* const self)
{
	struct NiosGpio* const instance = (struct NiosGpio*)self;

	ASSERT_(instance->icId != kINVALID_VALUE);

	alt_ic_irq_enable(instance->icId, instance->irq);
}

/**
 * @brief	Disable interrupt
 * @param	self			Gpio*
 * @return	none
 */
void NiosGpio_disableInterrupt(struct Gpio* const self)
{
	struct NiosGpio* const instance = (struct NiosGpio*)self;

	ASSERT_(instance->icId != kINVALID_VALUE);

	alt_ic_irq_disable(instance->icId, instance->irq);
}

/**
 * @brief	Interrupt Service Routine
 * @param	isr_context		ISR context
 * @return	none
 */
static void interruptServiceRoutine(void* const isr_context)
{
	struct NiosGpio* const instance = (struct NiosGpio*)isr_context;
	uint32_t status;

	if (instance->intTrg == kNIOS_GPIO_TRG_LEVEL) {
		status = IORD_ALTERA_AVALON_PIO_DATA(instance->baseAddr);
	} else {
		status = IORD_ALTERA_AVALON_PIO_EDGE_CAP(instance->baseAddr);
		IOWR_ALTERA_AVALON_PIO_EDGE_CAP(niosGpio->kBASE_ADDR, status);
	}

	instance->callbackFunc(instance->callbackArg, (status & instance->interruptFlags));
}

/**
 * @brief	Assign virtual functions
 * @param	instance		instance
 * @return	none
 */
static void assignVirtualFunctions(struct NiosGpio* const instance)
{
	instance->gpio.destroy						= NiosGpio_destroy;

	instance->gpio.writeData					= NiosGpio_writeData;
	instance->gpio.readData						= NiosGpio_readData;

	instance->gpio.writeDirection				= NiosGpio_writeDirection;
	instance->gpio.readDirection				= NiosGpio_readDirection;

	instance->gpio.setupInterrupt				= NiosGpio_setupInterrupt;
	instance->gpio.enableMultipleInterrupts		= NiosGpio_enableMultipleInterrupts;
	instance->gpio.disableMultipleInterrupts		= NiosGpio_disableMultipleInterrupts;
	instance->gpio.enableInterrupt				= NiosGpio_enableInterrupt;
	instance->gpio.disableInterrupt				= NiosGpio_disableInterrupt;
}
