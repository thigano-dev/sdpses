/**
 * @file	mb_uart.h
 * @brief	Xilinx Uart Lite
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

#ifndef SDPSES_DEVICE_MB_UART_H_INCLUDED_
#define SDPSES_DEVICE_MB_UART_H_INCLUDED_

#include "uart.h"
#include "fixed_queue.h"

namespace sdpses {

namespace device {

class FreeRunCounter;

/**
 * @class	MbUart
 * @brief	MbUart class
 * @note	Don't inherit from this class.
 */
class MbUart : public Uart {

public:
	struct Params {
		explicit Params(const unsigned int tx_buff_sz = 64,
						const unsigned int rx_buff_sz = 64)
			: kTX_BUFF_SZ(tx_buff_sz)
			, kRX_BUFF_SZ(rx_buff_sz) {}
		~Params() {}

		const unsigned int kTX_BUFF_SZ;
		const unsigned int kRX_BUFF_SZ;
	};

	MbUart(uint32_t base_addr, uint32_t ic_base, uint32_t irq, const Params& params);
	~MbUart();

	int setup(const SerialParams& params);

	int get(uint8_t* data);
	int put(uint8_t data);
	int read(uint8_t data_buff[], unsigned int data_count);
	int write(const uint8_t data_buff[], unsigned int data_count);

	void clear();
	int flush();

	unsigned int getFramePeriodUsec() const;

	bool overrunErrorOccurred() const;
	bool framingErrorOccurred() const;
	bool parityErrorOccurred() const;

private:
	MbUart();
	MbUart(const MbUart&);
	MbUart& operator=(const MbUart&);

	const uint32_t kBASE_ADDR;
	const uint32_t kIC_BASE;
	const uint32_t kIRQ;
	const uint32_t kIRQ_MASK;

	uint32_t errorMask_;
	uint32_t lastError_;

	unsigned int framePeriodUsec_;

	container::FixedQueue<uint8_t> txQueue_;
	container::FixedQueue<uint8_t> rxQueue_;

	const FreeRunCounter& freeRunCounter_;

	int validateSerialParams(const SerialParams& params) const;

	void clearBuffer();
	int waitTxFifoReady() const;
	int waitTxFifoEmpty() const;
	void writeToTxFifo();

	void setupInterrupt();
	static void interruptHandler(void* context);
	void transmitInterrupt();
	void receiveInterrupt();
};

} /* namespace device */

} /* namespace sdpses */

#endif /* SDPSES_DEVICE_MB_UART_H_INCLUDED_ */
