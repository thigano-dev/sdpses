/**
 * @file	serial_params.h
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

/*! @note The include guard is not required. */

typedef enum {
	kSERIAL_BITRATE_110    = 110,
	kSERIAL_BITRATE_300    = 300,
	kSERIAL_BITRATE_600    = 600,
	kSERIAL_BITRATE_1200   = 1200,
	kSERIAL_BITRATE_2400   = 2400,
	kSERIAL_BITRATE_4800   = 4800,

	kSERIAL_BITRATE_9600   = 9600,
	kSERIAL_BITRATE_14400  = 14400,
	kSERIAL_BITRATE_19200  = 19200,
	kSERIAL_BITRATE_38400  = 38400,
	kSERIAL_BITRATE_57600  = 57600,
	kSERIAL_BITRATE_115200 = 115200,

	kSERIAL_BITRATE_230400 = 230400,
	kSERIAL_BITRATE_460800 = 460800,
	kSERIAL_BITRATE_921600 = 921600,

	kSERIAL_BITRATE_DEFAULT = kSERIAL_BITRATE_115200
} SerialBitrate;

typedef enum {
	kSERIAL_DATABIT_5 = 5,
	kSERIAL_DATABIT_6 = 6,
	kSERIAL_DATABIT_7 = 7,
	kSERIAL_DATABIT_8 = 8,
	kSERIAL_DATABIT_9 = 9,

	kSERIAL_DATABIT_DEFAULT = kSERIAL_DATABIT_8
} SerialDatabit;

typedef enum {
	kSERIAL_PARITY_NONE = 0,
	kSERIAL_PARITY_ODD,
	kSERIAL_PARITY_EVEN,

	kSERIAL_PARITY_DEFAULT = kSERIAL_PARITY_NONE
} SerialParity;

typedef enum {
	kSERIAL_STOPBIT_1 = 1,
	kSERIAL_STOPBIT_2 = 2,

	kSERIAL_STOPBIT_DEFAULT = kSERIAL_STOPBIT_1
} SerialStopbit;

typedef enum {
	kSERIAL_FLOW_CONTROL_NONE = 0,
	kSERIAL_FLOW_CONTROL_HARDWARE,
	kSERIAL_FLOW_CONTROL_XON_XOFF,

	kSERIAL_FLOW_CONTROL_DEFAULT = kSERIAL_FLOW_CONTROL_NONE
} SerialFlowControl;

typedef struct {
	SerialBitrate     bitrate;
	SerialDatabit     databit;
	SerialParity      parity;
	SerialStopbit     stopbit;
	SerialFlowControl flowControl;
} SerialParams;

unsigned int SerialParams_calcFramePeriodUsec(const SerialParams* params);
