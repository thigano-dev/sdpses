/**
 * @file	lib_callback.h
 * @brief	general callback function
 * @author	Tsuguyoshi Higano
 * @date	Nov 12, 2017
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

#ifndef SDPSES_LIBUTL_LIB_CALLBACK_H_INCLUDED_
#define SDPSES_LIBUTL_LIB_CALLBACK_H_INCLUDED_

/**
 * @brief	General Callback Function
 * @param	callback_arg	argument of Callback Function
 * @return	none
 */
typedef void (*GenCallbackFunc)(void* callback_arg);

#endif /* SDPSES_LIBUTL_LIB_CALLBACK_H_INCLUDED_ */
