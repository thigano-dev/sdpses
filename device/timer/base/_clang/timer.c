/**
 * @file	timer.c
 * @brief	timer
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

#include "timer_private.h"

static int setupInterrupt(struct Timer* self,
		GenCallbackFunc callback_func, void* callback_arg);
static void enableInterrupt(struct Timer* self);
static void disableInterrupt(struct Timer* self);

static void assignVirtualFunctions(struct Timer* self);

/**
 * @brief	Destroy
 * @param	self			Timer*
 * @return	instance
 */
struct Timer* Timer_destroy(struct Timer* const self)
{
	if (!self) { return NULL; }
	return self->destroy(self);
}

/**
 * @brief	Constructor
 * @param	self			Timer*
 * @retval	0				success
 * @retval	!=0				failure
 */
int Timer_ctor(struct Timer* const self)
{
	assignVirtualFunctions(self);

	return 0;
}

/**
 * @brief	Destructor
 * @param	self			Timer*
 * @return	none
 */
void Timer_dtor(struct Timer* const self)
{
	if (!self) { return; }
}

/**
 * @brief	Set up
 * @param	self			Timer*
 * @param	params			TimerCountParams*
 * @retval	0				success
 * @retval	!=0				failure
 */
int Timer_setup(struct Timer* const self, const TimerCountParams* const params)
{
	return self->setup(self, params);
}

/**
 * @brief	Start counting
 * @param	self			Timer*
 * @return	none
 */
void Timer_start(struct Timer* const self)
{
	self->start(self);
}

/**
 * @brief	Stop counting
 * @param	self			Timer*
 * @return	none
 */
void Timer_stop(struct Timer* const self)
{
	self->stop(self);
}

/**
 * @brief	Read counter value
 * @param	self			Timer*
 * @return	counter value
 */
uint32_t Timer_readCounter(const struct Timer* const self)
{
	return self->readCounter(self);
}

/**
 * @brief	Get frequency
 * @param	self			Timer*
 * @return	frequency
 */
uint32_t Timer_getFrequency(const struct Timer* const self)
{
	return self->getFrequency(self);
}

/**
 * @brief	Set up interrupt
 * @param	self			Timer*
 * @param	callback_func	callback function
 * @param	callback_arg	arguments for the callback function
 * @retval	0				success
 * @retval	!=0				failure
 */
int Timer_setupInterrupt(struct Timer* const self,
		const GenCallbackFunc callback_func, void* const callback_arg)
{
	return self->setupInterrupt(self, callback_func, callback_arg);
}

/**
 * @brief	Enable interrupt
 * @param	self			Timer*
 * @return	none
 */
void Timer_enableInterrupt(struct Timer* const self)
{
	self->enableInterrupt(self);
}

/**
 * @brief	Disable interrupt
 * @param	self			Timer*
 * @return	none
 */
void Timer_disableInterrupt(struct Timer* const self)
{
	self->disableInterrupt(self);
}

/**
 * @brief	Set up interrupt
 * @param	self			Timer*
 * @param	callback_func	callback function
 * @param	callback_arg	arguments for the callback function
 * @retval	0				success
 * @retval	!=0				failure
 */
static int setupInterrupt(struct Timer* const self,
		const GenCallbackFunc callback_func, void* const callback_arg)
{
	return 1;
}

/**
 * @brief	Enable interrupt
 * @param	self			Timer*
 * @return	none
 */

static void enableInterrupt(struct Timer* const self)
{
}

/**
 * @brief	Disable interrupt
 * @param	self			Timer*
 * @return	none
 */
static void disableInterrupt(struct Timer* const self)
{
}

/**
 * @brief	Assign virtual functions
 * @param	self			Timer*
 * @return	none
 */
static void assignVirtualFunctions(struct Timer* const self)
{
	self->setupInterrupt	= setupInterrupt;
	self->enableInterrupt	= enableInterrupt;
	self->disableInterrupt	= disableInterrupt;
}
