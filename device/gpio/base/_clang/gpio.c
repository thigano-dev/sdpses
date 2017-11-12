/**
 * @file	gpio.c
 * @brief	GPIO(General-purpose input/output)
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

#include "gpio_private.h"

static int setupInterrupt(struct Gpio* self, uint32_t interrupt_bits,
		Gpio_CallbackFunc callback_func, void* callback_arg);
static void enableMultipleInterrupts(struct Gpio* self, uint32_t bitmask);
static void disableMultipleInterrupts(struct Gpio* self, uint32_t bitmask);
static void enableInterrupt(struct Gpio* self);
static void disableInterrupt(struct Gpio* self);

static void assignVirtualFunctions(struct Gpio* self);

/**
 * @brief	Destroy
 * @param	self			Gpio*
 * @return	Gpio*
 */
struct Gpio* Gpio_destroy(struct Gpio* const self)
{
	if (!self) { return NULL; }
	return self->destroy(self);
}

/**
 * @brief	Constructor
 * @param	self			Gpio*
 * @retval	0				success
 * @retval	!=0				failure
 */
int Gpio_ctor(struct Gpio* const self)
{
	assignVirtualFunctions(self);

	return 0;
}

/**
 * @brief	Destructor
 * @param	self			Gpio*
 * @return	none
 */
void Gpio_dtor(struct Gpio* const self)
{
	if (!self) { return; }
}

/**
 * @brief	Write data
 * @param	self			Gpio*
 * @param	data			data
 * @return	none
 */
void Gpio_writeData(struct Gpio* const self, const uint32_t data)
{
	self->writeData(self, data);
}

/**
 * @brief	Read data
 * @param	self			Gpio*
 * @return	data
 */
uint32_t Gpio_readData(struct Gpio* const self)
{
	return self->readData(self);
}

/**
 * @brief	Set multiple bits
 * @param	self			Gpio*
 * @param	bitmask			[1:set, 0:unaffected]
 * @return	none
 */
void Gpio_setDataBit(struct Gpio* const self, uint32_t bitmask)
{
	bitmask = (self->readData(self) | bitmask);
	self->writeData(self, bitmask);
}

/**
 * @brief	Clear multiple bits
 * @param	self			Gpio*
 * @param	bitmask			[1:set to zero, 0:unaffected]
 * @return	none
 */
void Gpio_clearDataBit(struct Gpio* const self, uint32_t bitmask)
{
	bitmask = (self->readData(self) & ~bitmask);
	self->writeData(self, bitmask);
}

/**
 * @brief	Write the input/output direction
 * @param	self			Gpio*
 * @param	direction		[1:output, 0:input]
 * @return	none
 */
void Gpio_writeDirection(struct Gpio* const self, const uint32_t direction)
{
	self->writeDirection(self, direction);
}

/**
 * @brief	Read the input/output direction
 * @param	self			Gpio*
 * @return	direction [1:output, 0:input]
 */
uint32_t Gpio_readDirection(struct Gpio* const self)
{
	return self->readDirection(self);
}

/**
 * @brief	Set the output direction
 * @param	self			Gpio*
 * @param	bitmask			[1:output, 0:unaffected]
 * @return	none
 */
void Gpio_setOutputBit(struct Gpio* const self, uint32_t bitmask)
{
	bitmask = (self->readDirection(self) | bitmask);
	self->writeDirection(self, bitmask);
}

/**
 * @brief	Set the input direction
 * @param	self			Gpio*
 * @param	bitmask			[1:input, 0:unaffected]
 * @return	none
 */
void Gpio_setInputBit(struct Gpio* const self, uint32_t bitmask)
{
	bitmask = (self->readDirection(self) & ~bitmask);
	self->writeDirection(self, bitmask);
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
int Gpio_setupInterrupt(struct Gpio* const self, const uint32_t interrupt_bits,
		const Gpio_CallbackFunc callback_func, void* const callback_arg)
{
	return self->setupInterrupt(self, interrupt_bits, callback_func, callback_arg);
}

/**
 * @brief	Enable multiple interrupts for each input port
 * @param	self			Gpio*
 * @param	bitmask			[1:enable, 0:unaffected]
 * @return	none
 */
void Gpio_enableMultipleInterrupts(struct Gpio* const self, const uint32_t bitmask)
{
	self->enableMultipleInterrupts(self, bitmask);
}

/**
 * @brief	Disable multiple interrupts for each input port
 * @param	self			Gpio*
 * @param	bitmask			[1:disable, 0:unaffected]
 * @return	none
 */
void Gpio_disableMultipleInterrupts(struct Gpio* const self, const uint32_t bitmask)
{
	self->disableMultipleInterrupts(self, bitmask);
}

/**
 * @brief	Enable interrupt
 * @param	self			Gpio*
 * @return	none
 */
void Gpio_enableInterrupt(struct Gpio* const self)
{
	self->enableInterrupt(self);
}

/**
 * @brief	Disable interrupt
 * @param	self			Gpio*
 * @return	none
 */
void Gpio_disableInterrupt(struct Gpio* const self)
{
	self->disableInterrupt(self);
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
static int setupInterrupt(struct Gpio* const self, const uint32_t interrupt_bits,
		const Gpio_CallbackFunc callback_func, void* const callback_arg)
{
	return 1;
}

/**
 * @brief	Enable multiple interrupts for each input port
 * @param	self			Gpio*
 * @param	bitmask			[1:enable, 0:unaffected]
 * @return	none
 */
static void enableMultipleInterrupts(struct Gpio* const self, const uint32_t bitmask)
{
}

/**
 * @brief	Disable multiple interrupts for each input port
 * @param	self			Gpio*
 * @param	bitmask			[1:disable, 0:unaffected]
 * @return	none
 */
static void disableMultipleInterrupts(struct Gpio* const self, const uint32_t bitmask)
{
}

/**
 * @brief	Enable interrupt
 * @param	self			Gpio*
 * @return	none
 */
static void enableInterrupt(struct Gpio* const self)
{
}

/**
 * @brief	Disable interrupt
 * @param	self			Gpio*
 * @return	none
 */
static void disableInterrupt(struct Gpio* const self)
{
}

/**
 * @brief	Assign virtual functions
 * @param	self			Gpio*
 * @return	none
 */
static void assignVirtualFunctions(struct Gpio* const self)
{
	self->setupInterrupt				= setupInterrupt;
	self->enableMultipleInterrupts		= enableMultipleInterrupts;
	self->disableMultipleInterrupts		= disableMultipleInterrupts;
	self->enableInterrupt				= enableInterrupt;
	self->disableInterrupt				= disableInterrupt;
}
