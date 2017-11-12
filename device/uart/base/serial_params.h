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

namespace sdpses {

namespace device {

/**
 * @struct	SerialParams
 * @brief	SerialParams struct
 * @note	Don't inherit from this struct.
 */
struct SerialParams {
	enum Bitrate {
		kBITRATE_110    = 110,
		kBITRATE_300    = 300,
		kBITRATE_600    = 600,
		kBITRATE_1200   = 1200,
		kBITRATE_2400   = 2400,
		kBITRATE_4800   = 4800,

		kBITRATE_9600   = 9600,
		kBITRATE_14400  = 14400,
		kBITRATE_19200  = 19200,
		kBITRATE_38400  = 38400,
		kBITRATE_57600  = 57600,
		kBITRATE_115200 = 115200,

		kBITRATE_230400 = 230400,
		kBITRATE_460800 = 460800,
		kBITRATE_921600 = 921600,

		kBITRATE_DEFAULT = kBITRATE_115200
	};

	enum Databit {
		kDATABIT_5 = 5,
		kDATABIT_6 = 6,
		kDATABIT_7 = 7,
		kDATABIT_8 = 8,
		kDATABIT_9 = 9,

		kDATABIT_DEFAULT = kDATABIT_8
	};

	enum Parity {
		kPARITY_NONE = 0,
		kPARITY_ODD,
		kPARITY_EVEN,

		kPARITY_DEFAULT = kPARITY_NONE
	};

	enum Stopbit {
		kSTOPBIT_1 = 1,
		kSTOPBIT_2 = 2,

		kSTOPBIT_DEFAULT = kSTOPBIT_1
	};

	enum FlowControl {
		kFLOW_CONTROL_NONE = 0,
		kFLOW_CONTROL_HARDWARE,
		kFLOW_CONTROL_XON_XOFF,

		kFLOW_CONTROL_DEFAULT = kFLOW_CONTROL_NONE
	};

	explicit SerialParams(const Bitrate bitrate = kBITRATE_DEFAULT,
						  const Databit databit = kDATABIT_DEFAULT,
						  const Parity parity = kPARITY_DEFAULT,
						  const Stopbit stopbit = kSTOPBIT_DEFAULT,
						  const FlowControl flow_control = kFLOW_CONTROL_DEFAULT)
		: bitrate_(bitrate)
		, databit_(databit)
		, parity_(parity)
		, stopbit_(stopbit)
		, flowControl_(flow_control) {}
	~SerialParams() {}

	/**
	 * @brief	Calculate frame period [microseconds]
	 * @return	frame period
	 */
	unsigned int calcFramePeriodUsec() const {
		const unsigned int startbit = 1;
		const unsigned int paritybit = (parity_ == kPARITY_NONE) ? 0 : 1;
		const unsigned int bitsPerFrame = (startbit + databit_ + paritybit + stopbit_);
		return (((1000000U * bitsPerFrame) + (bitrate_ - 1)) / bitrate_);
	}

	Bitrate     bitrate_;
	Databit     databit_;
	Parity      parity_;
	Stopbit     stopbit_;
	FlowControl flowControl_;
};

} /* namespace device */

} /* namespace sdpses */
