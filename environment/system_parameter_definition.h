/**
 * @file	system_parameter_definition.h
 * @brief	system parameter definition
 */

#ifndef SDPSES_ENVIRONMENT_SYSTEM_PARAMETER_DEFINITION_H_INCLUDED_
#define SDPSES_ENVIRONMENT_SYSTEM_PARAMETER_DEFINITION_H_INCLUDED_

/*--- ALTERA Nios II ---------------------------------------------------------*/
#if defined(__NIOS2__)
#include "replacement_aparam.h"

/*--- XILINX MicroBlaze ------------------------------------------------------*/
#elif defined(__MICROBLAZE__)
#include "replacement_xparam.h"

/*--- Simulation Environment -------------------------------------------------*/
#else

#endif

#endif /* SDPSES_ENVIRONMENT_SYSTEM_PARAMETER_DEFINITION_H_INCLUDED_ */
