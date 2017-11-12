/**
 * @file	nios_uart.cpp
 * @brief	Altera Avalon UART
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

#include <sys/alt_irq.h>

#include "altera_avalon_uart_regs.h"
#include "nios_uart.h"
#include "free_run_counter.h"
#include "lib_debug.h"

namespace sdpses {

namespace device {

/**
 * @brief	Constructor
 * @param	base_addr		base address
 * @param	freq			frequency(Hz)
 * @param	ic_id			intc id
 * @param	irq				irq number
 * @param	params			Params
 */
NiosUart::NiosUart(const uint32_t base_addr, const uint32_t freq,
		const uint32_t ic_id, const uint32_t irq, const Params& params)
	: kBASE_ADDR(base_addr)
	, kFREQ(freq)
	, kIC_ID(ic_id)
	, kIRQ(irq)
	, interruptFlags_(0)
	, errorMask_(0)
	, lastError_(0)
	, framePeriodUsec_(0)
	, txQueue_(params.kTX_BUFF_SZ)
	, rxQueue_(params.kRX_BUFF_SZ)
	, freeRunCounter_(FreeRunCounter::getInstance())
{
	DEBUG_PRINTF_("<NiosII UART parameters>\r\n");
	DEBUG_PRINTF_("  BASE ADDR     : [H'%08lX]\r\n", base_addr);
	DEBUG_PRINTF_("  FREQ          : [%lu", (freq / 1000000UL));
	DEBUG_PRINTF_(".%luMHz]\r\n", (freq % 1000000UL));
	DEBUG_PRINTF_("  IC ID         : [");
	if (ic_id == 0UL) {
		DEBUG_PRINTF_("IIC: in NiosII Core]\r\n");
	} else if (ic_id == 0xFFFFFFFFUL) {
		DEBUG_PRINTF_("none]\r\n");
	} else {
		DEBUG_PRINTF_("EIC: H'%08lX]\r\n", ic_id);
	}
	DEBUG_PRINTF_("  IRQ           : [%d]\r\n", static_cast<int>(irq));
	DEBUG_PRINTF_("  TX BUFF SIZE  : [%u]\r\n", params.kTX_BUFF_SZ);
	DEBUG_PRINTF_("  RX BUFF SIZE  : [%u]\r\n", params.kRX_BUFF_SZ);
	DEBUG_PRINTF_("\r\n");

	setup(SerialParams());
}

/**
 * @brief	Destructor
 */
NiosUart::~NiosUart()
{
	alt_ic_irq_disable(kIC_ID, kIRQ);
	alt_ic_isr_register(kIC_ID, kIRQ, NULL, (void*)0, (void*)0);
	IOWR_ALTERA_AVALON_UART_DIVISOR(kBASE_ADDR, 0);
	IOWR_ALTERA_AVALON_UART_CONTROL(kBASE_ADDR, 0);
	IOWR_ALTERA_AVALON_UART_STATUS(kBASE_ADDR, 0);
}

/**
 * @brief	Set up
 * @param	params			SerialParams
 * @retval	0				success
 * @retval	!=0				failure
 */
int NiosUart::setup(const SerialParams& params)
{
	if (validateSerialParams(params)) { return 1; }

	alt_ic_irq_disable(kIC_ID, kIRQ);
	framePeriodUsec_ = params.calcFramePeriodUsec();

	/* set bitrate */
	const uint16_t divisor = static_cast<uint16_t>((static_cast<double>(kFREQ) / static_cast<double>(params.bitrate_)) + 0.5F);
	IOWR_ALTERA_AVALON_UART_DIVISOR(kBASE_ADDR, divisor);

	clearBuffer();
	lastError_ = 0;

	setupInterrupt();
	alt_ic_irq_enable(kIC_ID, kIRQ);

	return 0;
}

/**
 * @brief	Validate serial parameters
 * @param	params			SerialParams
 * @retval	0				success
 * @retval	!=0				failure
 */
int NiosUart::validateSerialParams(const SerialParams& params) const
{
	switch (params.bitrate_) {
	case SerialParams::kBITRATE_9600:
	case SerialParams::kBITRATE_19200:
	case SerialParams::kBITRATE_38400:
	case SerialParams::kBITRATE_57600:
	case SerialParams::kBITRATE_115200:
		break;
	default:
		DEBUG_PRINTF_("error: NiosII UART bitrate parameter [%dbps]\r\n", params.bitrate_);
		return 1;
	}

	switch (params.databit_) {
	case SerialParams::kDATABIT_7:
	case SerialParams::kDATABIT_8:
		break;
	case SerialParams::kDATABIT_9: /*!< not supported */
	default:
		DEBUG_PRINTF_("error: NiosII UART databit parameter [%dbit]\r\n", params.databit_);
		return 1;
	}

	switch (params.parity_) {
	case SerialParams::kPARITY_NONE:
	case SerialParams::kPARITY_ODD:
	case SerialParams::kPARITY_EVEN:
		break;
	default:
		DEBUG_PRINTF_("error: NiosII UART parity parameter [%d]\r\n", params.parity_);
		return 1;
	}

	switch (params.stopbit_) {
	case SerialParams::kSTOPBIT_1:
	case SerialParams::kSTOPBIT_2:
		break;
	default:
		DEBUG_PRINTF_("error: NiosII UART stopbit parameter [%dbit]\r\n", params.stopbit_);
		return 1;
	}

	switch (params.flowControl_) {
	case SerialParams::kFLOW_CONTROL_NONE:
		break;
	default:
		DEBUG_PRINTF_("error: NiosII UART flow control parameter [%d]\r\n", params.flowControl_);
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
int NiosUart::get(uint8_t* const data)
{
	int rc = 1;

	alt_ic_irq_disable(kIC_ID, kIRQ);
	if (!rxQueue_.empty()) {
		*data = rxQueue_.peek();
		rxQueue_.pop();
		rc = 0;
	}
	alt_ic_irq_enable(kIC_ID, kIRQ);

	return rc;
}

/**
 * @brief	Put a data
 * @param	data			data
 * @retval	0				success
 * @retval	!=0				failure
 */
int NiosUart::put(const uint8_t data)
{
	int rc = 1;

	alt_ic_irq_disable(kIC_ID, kIRQ);
	if (IORD_ALTERA_AVALON_UART_STATUS(kBASE_ADDR) & ALTERA_AVALON_UART_STATUS_TRDY_MSK) {
		if (txQueue_.empty()) {
			IOWR_ALTERA_AVALON_UART_TXDATA(kBASE_ADDR, data);
		} else {
			IOWR_ALTERA_AVALON_UART_TXDATA(kBASE_ADDR, txQueue_.peek());
			txQueue_.pop();
			txQueue_.push(data);
		}
		rc = 0;
	} else if (!txQueue_.full()) {
		txQueue_.push(data);
		rc = 0;
	}
	interruptFlags_ |= ALTERA_AVALON_UART_CONTROL_TRDY_MSK;
	IOWR_ALTERA_AVALON_UART_CONTROL(kBASE_ADDR, interruptFlags_);
	alt_ic_irq_enable(kIC_ID, kIRQ);

	return rc;
}

/**
 * @brief	Read data into buffer
 * @param	data_buff		data buffer
 * @param	data_count		number of data
 * @retval	0				success
 * @retval	!=0				failure
 */
int NiosUart::read(uint8_t data_buff[], const unsigned int data_count)
{
	int rc = 1;

	alt_ic_irq_disable(kIC_ID, kIRQ);
	if (rxQueue_.size() >= data_count) {
		for (unsigned int i = 0; i < data_count; i++) {
			data_buff[i] = rxQueue_.peek();
			rxQueue_.pop();
		}
		rc = 0;
	}
	alt_ic_irq_enable(kIC_ID, kIRQ);

	return rc;
}

/**
 * @brief	Write data buffer
 * @param	data_buff		data buffer
 * @param	data_count		number of data
 * @retval	0				success
 * @retval	!=0				failure
 */
int NiosUart::write(const uint8_t data_buff[], const unsigned int data_count)
{
	int rc = 1;

	alt_ic_irq_disable(kIC_ID, kIRQ);
	if (txQueue_.availableSize() >= data_count) {
		for (unsigned int i = 0; i < data_count; i++) {
			txQueue_.push(data_buff[i]);
		}
		rc = 0;
	}
	interruptFlags_ |= ALTERA_AVALON_UART_CONTROL_TRDY_MSK;
	IOWR_ALTERA_AVALON_UART_CONTROL(kBASE_ADDR, interruptFlags_);
	alt_ic_irq_enable(kIC_ID, kIRQ);

	return rc;
}

/**
 * @brief	Clear receive/transmit buffer and errors
 * @return	none
 */
void NiosUart::clear()
{
	alt_ic_irq_disable(kIC_ID, kIRQ);
	clearBuffer();
	lastError_ = 0;
	alt_ic_irq_enable(kIC_ID, kIRQ);
}

/**
 * @brief	Clear receive/transmit buffer
 * @return	none
 */
void NiosUart::clearBuffer()
{
	txQueue_.clear();
	rxQueue_.clear();
}

/**
 * @brief	Flush TX-Buffer
 * @retval	0				success
 * @retval	!=0				failure
 */
int NiosUart::flush()
{
	int rc = 1;

	alt_ic_irq_disable(kIC_ID, kIRQ);
	while (!txQueue_.empty()) {
		if (waitStatusReady(ALTERA_AVALON_UART_STATUS_TRDY_MSK)) { goto TERMINATE; }
		IOWR_ALTERA_AVALON_UART_TXDATA(kBASE_ADDR, txQueue_.peek());
		txQueue_.pop();
	}
	if (waitStatusReady(ALTERA_AVALON_UART_STATUS_TRDY_MSK)) { goto TERMINATE; }
	if (waitStatusReady(ALTERA_AVALON_UART_STATUS_TMT_MSK)) { goto TERMINATE; }

	interruptFlags_ &= ~ALTERA_AVALON_UART_CONTROL_TRDY_MSK;
	IOWR_ALTERA_AVALON_UART_CONTROL(kBASE_ADDR, interruptFlags_);
	rc = 0;

TERMINATE:
	alt_ic_irq_enable(kIC_ID, kIRQ);
	return rc;
}

/**
 * @brief	Wait for status is ready
 * @param	status			status
 * @retval	0				success
 * @retval	!=0				failure
 */
int NiosUart::waitStatusReady(const uint16_t status) const
{
	const uint32_t baseCount = freeRunCounter_.now();
	const uint32_t timeoutCount = freeRunCounter_.convertUsecToCount(framePeriodUsec_);

	while ((IORD_ALTERA_AVALON_UART_STATUS(kBASE_ADDR) & status) != status) {
		if (freeRunCounter_.timeout(baseCount, timeoutCount)) {
			if ((IORD_ALTERA_AVALON_UART_STATUS(kBASE_ADDR) & status) == status) { break; }
			return 1;
		}
	}

	return 0;
}

/**
 * @brief	Get frame period
 * @return	frame period
 */
unsigned int NiosUart::getFramePeriodUsec() const
{
	return framePeriodUsec_;
}

/**
 * @brief	Overrun error occurred
 * @retval	true			occurred
 * @retval	false			not occurred
 */
bool NiosUart::overrunErrorOccurred() const
{
	alt_ic_irq_disable(kIC_ID, kIRQ);
	const uint16_t lastError = lastError_;
	alt_ic_irq_enable(kIC_ID, kIRQ);
	return (lastError & ALTERA_AVALON_UART_STATUS_ROE_MSK) ? true : false;
}

/**
 * @brief	Framing error occurred
 * @retval	true			occurred
 * @retval	false			not occurred
 */
bool NiosUart::framingErrorOccurred() const
{
	alt_ic_irq_disable(kIC_ID, kIRQ);
	const uint16_t lastError = lastError_;
	alt_ic_irq_enable(kIC_ID, kIRQ);
	return (lastError & ALTERA_AVALON_UART_STATUS_FE_MSK) ? true : false;
}

/**
 * @brief	Parity error occurred
 * @retval	true			occurred
 * @retval	false			not occurred
 */
bool NiosUart::parityErrorOccurred() const
{
	alt_ic_irq_disable(kIC_ID, kIRQ);
	const uint16_t lastError = lastError_;
	alt_ic_irq_enable(kIC_ID, kIRQ);
	return (lastError & ALTERA_AVALON_UART_STATUS_PE_MSK) ? true : false;
}

/**
 * @brief	Set up interrupt
 * @retval	0				success
 * @retval	!=0				failure
 */
int NiosUart::setupInterrupt()
{
	IOWR_ALTERA_AVALON_UART_CONTROL(kBASE_ADDR, 0);

	/* make interrupt flags */
	interruptFlags_ = (
			ALTERA_AVALON_UART_CONTROL_PE_MSK		/*!< parity error */
		|	ALTERA_AVALON_UART_CONTROL_FE_MSK		/*!< framing error */
		|	ALTERA_AVALON_UART_CONTROL_ROE_MSK		/*!< receiver-overrun error */
		|	ALTERA_AVALON_UART_CONTROL_RRDY_MSK		/*!< read-ready */
	);

	/* make error mask */
	errorMask_ = (
			ALTERA_AVALON_UART_STATUS_PE_MSK		/*!< parity error */
		|	ALTERA_AVALON_UART_STATUS_FE_MSK		/*!< framing error */
		|	ALTERA_AVALON_UART_STATUS_ROE_MSK		/*!< receiver-overrun error */
	);

	if (alt_ic_isr_register(kIC_ID, kIRQ, interruptServiceRoutine, this, (void*)0)) { return 1; }
	alt_ic_irq_disable(kIC_ID, kIRQ);

	IOWR_ALTERA_AVALON_UART_CONTROL(kBASE_ADDR, interruptFlags_);
	IOWR_ALTERA_AVALON_UART_STATUS(kBASE_ADDR, 0);

	return 0;
}

/**
 * @brief	Interrupt Service Routine
 * @param	isr_context		ISR context
 * @return	none
 */
void NiosUart::interruptServiceRoutine(void* const isr_context)
{
	NiosUart* const instance = reinterpret_cast<NiosUart*>(isr_context);
	const uint16_t status = IORD_ALTERA_AVALON_UART_STATUS(instance->kBASE_ADDR);

	if (status & instance->errorMask_) {
		instance->lastError_ |= (status & instance->errorMask_);
		IOWR_ALTERA_AVALON_UART_STATUS(instance->kBASE_ADDR, 0);
	}

	if (status & ALTERA_AVALON_UART_STATUS_RRDY_MSK) { instance->receiveInterrupt(); }
	if (status & ALTERA_AVALON_UART_STATUS_TRDY_MSK) { instance->transmitInterrupt(); }
}

/**
 * @brief	Transmit Interrupt Processing
 * @return	none
 */
void NiosUart::transmitInterrupt()
{
	if (txQueue_.empty()) {
		interruptFlags_ &= ~ALTERA_AVALON_UART_CONTROL_TRDY_MSK;
		IOWR_ALTERA_AVALON_UART_CONTROL(kBASE_ADDR, interruptFlags_);
	} else {
		IOWR_ALTERA_AVALON_UART_TXDATA(kBASE_ADDR, txQueue_.peek());
		txQueue_.pop();
	}
}

/**
 * @brief	Receive Interrupt Processing
 * @return	none
 */
void NiosUart::receiveInterrupt()
{
	if (rxQueue_.full()) {
		lastError_ |= ALTERA_AVALON_UART_STATUS_ROE_MSK;
		IORD_ALTERA_AVALON_UART_RXDATA(kBASE_ADDR); /*!< thrown away */
	} else {
		rxQueue_.push(IORD_ALTERA_AVALON_UART_RXDATA(kBASE_ADDR));
	}
}

} /* namespace device */

} /* namespace sdpses */
