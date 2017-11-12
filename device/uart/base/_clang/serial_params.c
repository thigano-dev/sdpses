/**
 * @file	serial_params.c
 * @brief	serial parameters
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

#include "serial_params.h"

/**
 * @brief	Calculate frame period [microseconds]
 * @param	self			SerialParams*
 * @return	frame period
 */
unsigned int SerialParams_calcFramePeriodUsec(const SerialParams* const self)
{
	const unsigned int startbit = 1;
	const unsigned int paritybit = (self->parity == kSERIAL_PARITY_NONE) ? 0 : 1;
	const unsigned int bitsPerFrame = (startbit + self->databit + paritybit + self->stopbit);
	return (((1000000U * bitsPerFrame) + (self->bitrate - 1)) / self->bitrate);
}
