/**
 * @file	mb_uart.cpp
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

#include "xintc_l.h"

#include "xuartlite_l.h"
#include "mb_uart.h"
#include "free_run_counter.h"
#include "lib_debug.h"

namespace sdpses {

namespace device {

/**
 * @brief	Constructor
 * @param	base_addr		base address
 * @param	ic_base			intc base address
 * @param	irq				irq number
 * @param	params			Params
 */
MbUart::MbUart(const uint32_t base_addr, const uint32_t ic_base,
		const uint32_t irq, const Params& params)
	: kBASE_ADDR(base_addr)
	, kIC_BASE(ic_base)
	, kIRQ(irq)
	, kIRQ_MASK(1UL << irq)
	, errorMask_(0)
	, lastError_(0)
	, framePeriodUsec_(0)
	, txQueue_(params.kTX_BUFF_SZ)
	, rxQueue_(params.kRX_BUFF_SZ)
	, freeRunCounter_(FreeRunCounter::getInstance())
{
	DEBUG_PRINTF_("<MicroBlaze UART parameters>\r\n");
	DEBUG_PRINTF_("  BASE ADDR     : [H'%08lX]\r\n", base_addr);
	DEBUG_PRINTF_("  IC BASE       : [H'%08lX]\r\n", ic_base);
	DEBUG_PRINTF_("  IRQ           : [%lu]\r\n", irq);
	DEBUG_PRINTF_("  TX BUFF SIZE  : [%u]\r\n", params.kTX_BUFF_SZ);
	DEBUG_PRINTF_("  RX BUFF SIZE  : [%u]\r\n", params.kRX_BUFF_SZ);
	DEBUG_PRINTF_("\r\n");

	setup(SerialParams());
}

/**
 * @brief	Destructor
 */
MbUart::~MbUart()
{
	XUartLite_DisableIntr(kBASE_ADDR);
	XIntc_DisableIntr(kIC_BASE, kIRQ_MASK);
}

/**
 * @brief	Set up
 * @param	params			SerialParams
 * @retval	0				success
 * @retval	!=0				failure
 */
int MbUart::setup(const SerialParams& params)
{
	if (validateSerialParams(params)) { return 1; }

	XIntc_DisableIntr(kIC_BASE, kIRQ_MASK);
	framePeriodUsec_ = params.calcFramePeriodUsec();

	clearBuffer();
	lastError_ = 0;

	setupInterrupt();
	XIntc_EnableIntr(kIC_BASE, kIRQ_MASK);

	return 0;
}

/**
 * @brief	Validate serial parameters
 * @param	params			SerialParams
 * @retval	0				success
 * @retval	!=0				failure
 */
int MbUart::validateSerialParams(const SerialParams& params) const
{
	switch (params.bitrate_) {
	case SerialParams::kBITRATE_9600:
	case SerialParams::kBITRATE_19200:
	case SerialParams::kBITRATE_38400:
	case SerialParams::kBITRATE_57600:
	case SerialParams::kBITRATE_115200:
	case SerialParams::kBITRATE_230400:
		break;
	default:
		DEBUG_PRINTF_("error: MicroBlaze UART bitrate parameter [%dbps]\r\n", params.bitrate_);
		return 1;
	}

	switch (params.databit_) {
	case SerialParams::kDATABIT_5:
	case SerialParams::kDATABIT_6:
	case SerialParams::kDATABIT_7:
	case SerialParams::kDATABIT_8:
		break;
	case SerialParams::kDATABIT_9: /*!< not supported */
	default:
		DEBUG_PRINTF_("error: MicroBlaze UART databit parameter [%dbit]\r\n", params.databit_);
		return 1;
	}

	switch (params.parity_) {
	case SerialParams::kPARITY_NONE:
	case SerialParams::kPARITY_ODD:
	case SerialParams::kPARITY_EVEN:
		break;
	default:
		DEBUG_PRINTF_("error: MicroBlaze UART parity parameter [%d]\r\n", params.parity_);
		return 1;
	}

	switch (params.stopbit_) {
	case SerialParams::kSTOPBIT_1:
	case SerialParams::kSTOPBIT_2:
		break;
	default:
		DEBUG_PRINTF_("error: MicroBlaze UART stopbit parameter [%dbit]\r\n", params.stopbit_);
		return 1;
	}

	switch (params.flowControl_) {
	case SerialParams::kFLOW_CONTROL_NONE:
		break;
	default:
		DEBUG_PRINTF_("error: MicroBlaze UART flow control parameter [%d]\r\n", params.flowControl_);
		return 1;
	}

	return 0;
}

/**
 * @brief	Get a data
 * @param	data			pointer to a data
 * @retval	0				success
 * @retval	!=0				failure
 */
int MbUart::get(uint8_t* const data)
{
	int rc = 1;

	XIntc_DisableIntr(kIC_BASE, kIRQ_MASK);
	if (!rxQueue_.empty()) {
		*data = rxQueue_.peek();
		rxQueue_.pop();
		rc = 0;
	}
	XIntc_EnableIntr(kIC_BASE, kIRQ_MASK);

	return rc;
}

/**
 * @brief	Put a data
 * @param	data			data
 * @retval	0				success
 * @retval	!=0				failure
 */
int MbUart::put(const uint8_t data)
{
	int rc = 1;

	XIntc_DisableIntr(kIC_BASE, kIRQ_MASK);
	if ((XUartLite_GetStatusReg(kBASE_ADDR) & XUL_SR_TX_FIFO_FULL) == 0) {
		if (txQueue_.empty()) {
			XUartLite_SendByte(kBASE_ADDR, data);
		} else {
			XUartLite_SendByte(kBASE_ADDR, txQueue_.peek());
			txQueue_.pop();
			txQueue_.push(data);
		}
		rc = 0;
	} else if (!txQueue_.full()) {
		txQueue_.push(data);
		rc = 0;
	}
	XIntc_EnableIntr(kIC_BASE, kIRQ_MASK);

	return rc;
}

/**
 * @brief	Read data into buffer
 * @param	data_buff		data buffer
 * @param	data_count		number of data
 * @retval	0				success
 * @retval	!=0				failure
 */
int MbUart::read(uint8_t data_buff[], const unsigned int data_count)
{
	int rc = 1;

	XIntc_DisableIntr(kIC_BASE, kIRQ_MASK);
	if (rxQueue_.size() >= data_count) {
		for (unsigned int i = 0; i < data_count; i++) {
			data_buff[i] = rxQueue_.peek();
			rxQueue_.pop();
		}
		rc = 0;
	}
	XIntc_EnableIntr(kIC_BASE, kIRQ_MASK);

	return rc;
}

/**
 * @brief	Write data buffer
 * @param	data_buff		data buffer
 * @param	data_count		number of data
 * @retval	0				success
 * @retval	!=0				failure
 */
int MbUart::write(const uint8_t data_buff[], const unsigned int data_count)
{
	int rc = 1;

	XIntc_DisableIntr(kIC_BASE, kIRQ_MASK);
	if (txQueue_.availableSize() >= data_count) {
		for (unsigned int i = 0; i < data_count; i++) {
			txQueue_.push(data_buff[i]);
		}
		rc = 0;
	}
	writeToTxFifo();
	XIntc_EnableIntr(kIC_BASE, kIRQ_MASK);

	return rc;
}

/**
 * @brief	Clear receive/transmit buffer and errors
 * @return	none
 */
void MbUart::clear()
{
	XIntc_DisableIntr(kIC_BASE, kIRQ_MASK);
	clearBuffer();
	lastError_ = 0;
	XIntc_EnableIntr(kIC_BASE, kIRQ_MASK);
}

/**
 * @brief	Clear receive/transmit buffer
 * @return	none
 */
void MbUart::clearBuffer()
{
	txQueue_.clear();
	rxQueue_.clear();
}

/**
 * @brief	Flush TX-Buffer
 * @retval	0				success
 * @retval	!=0				failure
 */
int MbUart::flush()
{
	int rc = 1;

	XIntc_DisableIntr(kIC_BASE, kIRQ_MASK);
	while (!txQueue_.empty()) {
		if (waitTxFifoReady()) { goto TERMINATE; }
		XUartLite_SendByte(kBASE_ADDR, txQueue_.peek());
		txQueue_.pop();
	}
	if (waitTxFifoEmpty()) { goto TERMINATE; }
	freeRunCounter_.waitUsec(framePeriodUsec_);
	rc = 0;

TERMINATE:
	XIntc_EnableIntr(kIC_BASE, kIRQ_MASK);
	return rc;
}

/**
 * @brief	Wait until there is space in TX-FIFO
 * @retval	0				success
 * @retval	!=0				failure
 */
int MbUart::waitTxFifoReady() const
{
	const uint32_t baseCount = freeRunCounter_.now();
	const uint32_t timeoutCount = freeRunCounter_.convertUsecToCount(framePeriodUsec_);

	while (XUartLite_GetStatusReg(kBASE_ADDR) & XUL_SR_TX_FIFO_FULL) {
		if (freeRunCounter_.timeout(baseCount, timeoutCount)) {
			if ((XUartLite_GetStatusReg(kBASE_ADDR) & XUL_SR_TX_FIFO_FULL) == 0) { break; }
			return 1;
		}
	}

	return 0;
}

/**
 * @brief	Wait for TX-FIFO is empty
 * @retval	0				success
 * @retval	!=0				failure
 */
int MbUart::waitTxFifoEmpty() const
{
	const uint32_t baseCount = freeRunCounter_.now();
	const uint32_t timeoutCount = freeRunCounter_.convertUsecToCount(framePeriodUsec_ * XUL_FIFO_SIZE);

	while ((XUartLite_GetStatusReg(kBASE_ADDR) & XUL_SR_TX_FIFO_EMPTY) == 0) {
		if (freeRunCounter_.timeout(baseCount, timeoutCount)) {
			if (XUartLite_GetStatusReg(kBASE_ADDR) & XUL_SR_TX_FIFO_EMPTY) { break; }
			return 1;
		}
	}

	return 0;
}

/**
 * @brief	Write to TX-FIFO
 * @return	none
 */
void MbUart::writeToTxFifo()
{
	for (int i = 0; i < XUL_FIFO_SIZE; i++) {
		if (XUartLite_GetStatusReg(kBASE_ADDR) & XUL_SR_TX_FIFO_FULL) { break; }
		if (txQueue_.empty()) { break; }
		XUartLite_SendByte(kBASE_ADDR, txQueue_.peek());
		txQueue_.pop();
	}
}

/**
 * @brief	Get frame period
 * @return	frame period
 */
unsigned int MbUart::getFramePeriodUsec() const
{
	return framePeriodUsec_;
}

/**
 * @brief	Overrun error occurred
 * @retval	true			occurred
 * @retval	false			not occurred
 */
bool MbUart::overrunErrorOccurred() const
{
	XIntc_DisableIntr(kIC_BASE, kIRQ_MASK);
	const uint32_t lastError = lastError_;
	XIntc_EnableIntr(kIC_BASE, kIRQ_MASK);
	return (lastError & XUL_SR_OVERRUN_ERROR) ? true : false;
}

/**
 * @brief	Framing error occurred
 * @retval	true			occurred
 * @retval	false			not occurred
 */
bool MbUart::framingErrorOccurred() const
{
	XIntc_DisableIntr(kIC_BASE, kIRQ_MASK);
	const uint32_t lastError = lastError_;
	XIntc_EnableIntr(kIC_BASE, kIRQ_MASK);
	return (lastError & XUL_SR_FRAMING_ERROR) ? true : false;
}

/**
 * @brief	Parity error occurred
 * @retval	true			occurred
 * @retval	false			not occurred
 */
bool MbUart::parityErrorOccurred() const
{
	XIntc_DisableIntr(kIC_BASE, kIRQ_MASK);
	const uint32_t lastError = lastError_;
	XIntc_EnableIntr(kIC_BASE, kIRQ_MASK);
	return (lastError & XUL_SR_PARITY_ERROR) ? true : false;
}

/**
 * @brief	Set up interrupt
 * @return	none
 */
void MbUart::setupInterrupt()
{
	XUartLite_DisableIntr(kBASE_ADDR);

	/* make error mask */
	errorMask_ = (
			XUL_SR_PARITY_ERROR		/*!< parity error */
		|	XUL_SR_FRAMING_ERROR	/*!< framing error */
		|	XUL_SR_OVERRUN_ERROR	/*!< receiver-overrun error */
	);

	XUartLite_SetControlReg(kBASE_ADDR, (XUL_CR_FIFO_RX_RESET | XUL_CR_FIFO_TX_RESET));
	XUartLite_EnableIntr(kBASE_ADDR);

	XIntc_RegisterHandler(kIC_BASE, kIRQ, interruptHandler, this);
}

/**
 * @brief	Interrupt Handler
 * @param	context			context
 * @return	none
 */
void MbUart::interruptHandler(void* const context)
{
	MbUart* const instance = reinterpret_cast<MbUart*>(context);
	const uint32_t status = XUartLite_GetStatusReg(instance->kBASE_ADDR);

	if (status & instance->errorMask_) {
		instance->lastError_ |= (status & instance->errorMask_);
		XUartLite_SetControlReg(instance->kBASE_ADDR, XUL_CR_FIFO_RX_RESET);
		XUartLite_EnableIntr(instance->kBASE_ADDR);
		return;
	}

	if (status & XUL_SR_RX_FIFO_VALID_DATA) { instance->receiveInterrupt(); }
	if ((status & XUL_SR_TX_FIFO_FULL) == 0) { instance->transmitInterrupt(); }

	XIntc_AckIntr(instance->kIC_BASE, instance->kIRQ_MASK);
}

/**
 * @brief	Transmit Interrupt Processing
 * @return	none
 */
void MbUart::transmitInterrupt()
{
	writeToTxFifo();
}

/**
 * @brief	Receive Interrupt Processing
 * @return	none
 */
void MbUart::receiveInterrupt()
{
	for (int i = 0; i < XUL_FIFO_SIZE; i++) {
		if ((XUartLite_GetStatusReg(kBASE_ADDR) & XUL_SR_RX_FIFO_VALID_DATA) == 0) { break; }
		if (rxQueue_.full()) {
			lastError_ |= XUL_SR_OVERRUN_ERROR;
			XUartLite_RecvByte(kBASE_ADDR); /*!< thrown away */
		} else {
			rxQueue_.push(XUartLite_RecvByte(kBASE_ADDR));
		}
	}
}

} /* namespace device */

} /* namespace sdpses */
