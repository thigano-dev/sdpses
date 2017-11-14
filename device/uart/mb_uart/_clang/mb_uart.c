/**
 * @file	mb_uart.c
 * @brief	Xilinx Uart Lite
 * @author	Tsuguyoshi Higano
 * @date	Nov 14, 2017
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

#include "allocator.h"
#include "xuartlite_l.h"
#include "uart_private.h"
#include "mb_uart.h"
#include "fixed_queue8.h"
#include "free_run_counter.h"
#include "lib_debug.h"

/**
 * @struct	MbUart
 * @brief	MbUart struct
 * @extends	Uart
 */
struct MbUart {
	struct Uart uart; /*!< must be the first member for mutual conversion of pointers */

	uint32_t baseAddr;
	uint32_t freq;
	uint32_t icBase;
	uint32_t irq;
	uint32_t irqMask;

	uint32_t errorMask;
	uint32_t lastError;

	unsigned int framePeriodUsec;

	FixedQueue8* txQueue;
	FixedQueue8* rxQueue;

	const FreeRunCounter* freeRunCounter;
};

static int validateSerialParams(const SerialParams* params);

static void clearBuffer(struct MbUart* instance);
static int waitTxFifoReady(const struct MbUart* instance);
static int waitTxFifoEmpty(const struct MbUart* instance);
static void writeToTxFifo(struct MbUart* instance);

static void setupInterrupt(struct MbUart* instance);
static void interruptHandler(void* context);
static void transmitInterrupt(struct MbUart* instance);
static void receiveInterrupt(struct MbUart* instance);

static void assignVirtualFunctions(struct MbUart* instance);

/**
 * @brief	Get the size of MbUart
 * @return	the size of MbUart
 */
size_t MbUart_sizeOf(void)
{
	return sizeof(struct MbUart);
}

/**
 * @brief	Create
 * @param	base_addr		base address
 * @param	ic_base			intc base address
 * @param	irq				irq number
 * @param	uart_params		MbUartParams
 * @return	instance
 */
struct MbUart* MbUart_create(const uint32_t base_addr, const uint32_t ic_base,
		const uint32_t irq, const MbUartParams* const uart_params)
{
	struct MbUart* const instance = Allocator_allocate(sizeof(struct MbUart));
	if (!instance) {
		DEBUG_PRINTF_("Cannot allocate memory\r\n");
		return NULL;
	}

	if (MbUart_ctor(instance, base_addr, ic_base, irq, uart_params)) {
		Allocator_deallocate(instance);
		return NULL;
	}

	return instance;
}

/**
 * @brief	Destroy
 * @param	self			Uart*
 * @return	Uart*
 */
struct Uart* MbUart_destroy(struct Uart* const self)
{
	if (!self) { return NULL; }

	struct MbUart* const instance = (struct MbUart*)self;
	MbUart_dtor(instance);
	Allocator_deallocate(instance);

	return NULL;
}

/**
 * @brief	Constructor
 * @param	instance		instance
 * @param	base_addr		base address
 * @param	ic_base			intc base address
 * @param	irq				irq number
 * @param	uart_params		MbUartParams
 * @retval	0				success
 * @retval	!=0				failure
 */
int MbUart_ctor(struct MbUart* const instance, const uint32_t base_addr,
		const uint32_t ic_base, const uint32_t irq, const MbUartParams* const uart_params)
{
	DEBUG_PRINTF_("<MicroBlaze UART parameters>\r\n");
	DEBUG_PRINTF_("  BASE ADDR     : [H'%08lX]\r\n", base_addr);
	DEBUG_PRINTF_("  IC BASE       : [H'%08lX]\r\n", ic_base);
	DEBUG_PRINTF_("  IRQ           : [%lu]\r\n", irq);
	DEBUG_PRINTF_("  TX BUFF SIZE  : [%u]\r\n", uart_params->txBuffSz);
	DEBUG_PRINTF_("  RX BUFF SIZE  : [%u]\r\n", uart_params->rxBuffSz);
	DEBUG_PRINTF_("\r\n");

	if (Uart_ctor((struct Uart*)instance)) { return 1; }

	assignVirtualFunctions(instance);

	instance->baseAddr			= base_addr;
	instance->icBase			= ic_base;
	instance->irq				= irq;
	instance->irqMask			= (1UL << irq);

	instance->errorMask			= 0;
	instance->lastError			= 0;
	instance->framePeriodUsec	= 0;

	instance->txQueue = NULL;
	instance->rxQueue = NULL;

	if (uart_params->txBuffSz) {
		instance->txQueue = FixedQueue8_create(uart_params->txBuffSz);
		if (!instance->txQueue) { goto TERMINATE; }
	}

	if (uart_params->rxBuffSz) {
		instance->rxQueue = FixedQueue8_create(uart_params->rxBuffSz);
		if (!instance->rxQueue) { goto TERMINATE; }
	}

	instance->freeRunCounter = FreeRunCounter_getInstance();

	const SerialParams serialParams = {
			kSERIAL_BITRATE_DEFAULT,
			kSERIAL_DATABIT_DEFAULT,
			kSERIAL_PARITY_DEFAULT,
			kSERIAL_STOPBIT_DEFAULT,
			kSERIAL_FLOW_CONTROL_DEFAULT,
	};
	if (MbUart_setup((struct Uart*)instance, &serialParams)) { goto TERMINATE; }

	return 0;

TERMINATE:
	if (instance->txQueue) { instance->txQueue = FixedQueue8_destroy(instance->txQueue); }
	if (instance->rxQueue) { instance->rxQueue = FixedQueue8_destroy(instance->rxQueue); }
	return 1;
}

/**
 * @brief	Destructor
 * @param	instance		instance
 * @return	none
 */
void MbUart_dtor(struct MbUart* const instance)
{
	if (!instance) { return; }

	XUartLite_DisableIntr(instance->baseAddr);
	XIntc_DisableIntr(instance->icBase, instance->irqMask);

	if (instance->txQueue) { instance->txQueue = FixedQueue8_destroy(instance->txQueue); }
	if (instance->rxQueue) { instance->rxQueue = FixedQueue8_destroy(instance->rxQueue); }

	Uart_dtor((struct Uart*)instance);
}

/**
 * @brief	Set up
 * @param	self			Uart*
 * @param	params			SerialParams*
 * @retval	0				success
 * @retval	!=0				failure
 */
int MbUart_setup(struct Uart* const self, const SerialParams* const params)
{
	if (validateSerialParams(params)) { return 1; }

	struct MbUart* const instance = (struct MbUart*)self;

	XIntc_DisableIntr(instance->icBase, instance->irqMask);
	instance->framePeriodUsec = SerialParams_calcFramePeriodUsec(params);

	clearBuffer(instance);
	instance->lastError = 0;

	setupInterrupt(instance);
	XIntc_EnableIntr(instance->icBase, instance->irqMask);

	return 0;
}

/**
 * @brief	Validate serial parameters
 * @param	params			SerialParams
 * @retval	0				success
 * @retval	!=0				failure
 */
static int validateSerialParams(const SerialParams* const params)
{
	switch (params->bitrate) {
	case kSERIAL_BITRATE_9600:
	case kSERIAL_BITRATE_19200:
	case kSERIAL_BITRATE_38400:
	case kSERIAL_BITRATE_57600:
	case kSERIAL_BITRATE_115200:
	case kSERIAL_BITRATE_230400:
		break;
	default:
		DEBUG_PRINTF_("error: MicroBlaze UART bitrate parameter [%dbps]\r\n", params->bitrate);
		return 1;
	}

	switch (params->databit) {
	case kSERIAL_DATABIT_5:
	case kSERIAL_DATABIT_6:
	case kSERIAL_DATABIT_7:
	case kSERIAL_DATABIT_8:
		break;
	case kSERIAL_DATABIT_9: /*!< not supported */
	default:
		DEBUG_PRINTF_("error: MicroBlaze UART databit parameter [%dbit]\r\n", params->databit);
		return 1;
	}

	switch (params->parity) {
	case kSERIAL_PARITY_NONE:
	case kSERIAL_PARITY_ODD:
	case kSERIAL_PARITY_EVEN:
		break;
	default:
		DEBUG_PRINTF_("error: MicroBlaze UART parity parameter [%d]\r\n", params->parity);
		return 1;
	}

	switch (params->stopbit) {
	case kSERIAL_STOPBIT_1:
	case kSERIAL_STOPBIT_2:
		break;
	default:
		DEBUG_PRINTF_("error: MicroBlaze UART stopbit parameter [%dbit]\r\n", params->stopbit);
		return 1;
	}

	switch (params->flowControl) {
	case kSERIAL_FLOW_CONTROL_NONE:
		break;
	default:
		DEBUG_PRINTF_("error: MicroBlaze UART flow control parameter [%d]\r\n", params->flowControl);
		return 1;
	}

	return 0;
}

/**
 * @brief	Get a data
 * @param	self			Uart*
 * @param	data			pointer to a data
 * @retval	0				success
 * @retval	!=0				failure
 */
int MbUart_get(struct Uart* const self, uint8_t* const data)
{
	int rc = 1;
	struct MbUart* const instance = (struct MbUart*)self;

	XIntc_DisableIntr(instance->icBase, instance->irqMask);
	if (!FixedQueue8_empty(instance->rxQueue)) {
		*data = FixedQueue8_peek(instance->rxQueue);
		FixedQueue8_pop(instance->rxQueue);
		rc = 0;
	}
	XIntc_EnableIntr(instance->icBase, instance->irqMask);

	return rc;
}

/**
 * @brief	Put a data
 * @param	self			Uart*
 * @param	data			data
 * @retval	0				success
 * @retval	!=0				failure
 */
int MbUart_put(struct Uart* const self, const uint8_t data)
{
	int rc = 1;
	struct MbUart* const instance = (struct MbUart*)self;

	XIntc_DisableIntr(instance->icBase, instance->irqMask);
	if ((XUartLite_GetStatusReg(instance->baseAddr) & XUL_SR_TX_FIFO_FULL) == 0) {
		if (FixedQueue8_empty(instance->txQueue)) {
			XUartLite_SendByte(instance->baseAddr, data);
		} else {
			XUartLite_SendByte(instance->baseAddr, FixedQueue8_peek(instance->txQueue));
			FixedQueue8_pop(instance->txQueue);
			FixedQueue8_push(instance->txQueue, data);
		}
		rc = 0;
	} else if (!FixedQueue8_full(instance->txQueue)) {
		FixedQueue8_push(instance->txQueue, data);
		rc = 0;
	}
	XIntc_EnableIntr(instance->icBase, instance->irqMask);

	return rc;
}

/**
 * @brief	Read data into buffer
 * @param	self			Uart*
 * @param	data_buff		data buffer
 * @param	data_count		number of data
 * @retval	0				success
 * @retval	!=0				failure
 */
int MbUart_read(struct Uart* const self, uint8_t data_buff[], const unsigned int data_count)
{
	int rc = 1;
	struct MbUart* const instance = (struct MbUart*)self;

	XIntc_DisableIntr(instance->icBase, instance->irqMask);
	if (FixedQueue8_size(instance->rxQueue) >= data_count) {
		unsigned int i;
		for (i = 0; i < data_count; i++) {
			data_buff[i] = FixedQueue8_peek(instance->rxQueue);
			FixedQueue8_pop(instance->rxQueue);
		}
		rc = 0;
	}
	XIntc_EnableIntr(instance->icBase, instance->irqMask);

	return rc;
}

/**
 * @brief	Write data buffer
 * @param	self			Uart*
 * @param	data_buff		data buffer
 * @param	data_count		number of data
 * @retval	0				success
 * @retval	!=0				failure
 */
int MbUart_write(struct Uart* const self, const uint8_t data_buff[], const unsigned int data_count)
{
	int rc = 1;
	struct MbUart* const instance = (struct MbUart*)self;

	XIntc_DisableIntr(instance->icBase, instance->irqMask);
	if (FixedQueue8_availableSize(instance->txQueue) >= data_count) {
		unsigned int i;
		for (i = 0; i < data_count; i++) {
			FixedQueue8_push(instance->txQueue, data_buff[i]);
		}
		rc = 0;
	}
	writeToTxFifo(instance);
	XIntc_EnableIntr(instance->icBase, instance->irqMask);

	return rc;
}

/**
 * @brief	Clear receive/transmit buffer and errors
 * @param	self			Uart*
 * @return	none
 */
void MbUart_clear(struct Uart* const self)
{
	struct MbUart* const instance = (struct MbUart*)self;

	XIntc_DisableIntr(instance->icBase, instance->irqMask);
	clearBuffer(instance);
	instance->lastError = 0;
	XIntc_EnableIntr(instance->icBase, instance->irqMask);
}

/**
 * @brief	Clear receive/transmit buffer
 * @param	instance		pointer to the instance
 * @return	none
 */
static void clearBuffer(struct MbUart* const instance)
{
	if (instance->txQueue) { FixedQueue8_clear(instance->txQueue); }
	if (instance->rxQueue) { FixedQueue8_clear(instance->rxQueue); }
}

/**
 * @brief	Flush TX-Buffer
 * @param	self			Uart*
 * @retval	0				success
 * @retval	!=0				failure
 */
int MbUart_flush(struct Uart* const self)
{
	int rc = 1;
	struct MbUart* const instance = (struct MbUart*)self;

	XIntc_DisableIntr(instance->icBase, instance->irqMask);
	while (!FixedQueue8_empty(instance->txQueue)) {
		if (waitTxFifoReady(instance)) { goto TERMINATE; }
		XUartLite_SendByte(instance->baseAddr, FixedQueue8_peek(instance->txQueue));
		FixedQueue8_pop(instance->txQueue);
	}
	if (waitTxFifoEmpty(instance)) { goto TERMINATE; }
	instance->freeRunCounter->waitUsec(instance->framePeriodUsec); /*!< wait for transmit complete */
	rc = 0;

TERMINATE:
	XIntc_EnableIntr(instance->icBase, instance->irqMask);
	return rc;
}

/**
 * @brief	Wait until there is space in TX-FIFO
 * @param	instance		instance
 * @retval	0				success
 * @retval	!=0				failure
 */
static int waitTxFifoReady(const struct MbUart* const instance)
{
	const uint32_t baseCount = instance->freeRunCounter->now();
	const uint32_t timeoutCount = instance->freeRunCounter->convertUsecToCount(instance->framePeriodUsec);

	while (XUartLite_GetStatusReg(instance->baseAddr) & XUL_SR_TX_FIFO_FULL) {
		if (instance->freeRunCounter->timeout(baseCount, timeoutCount)) {
			if ((XUartLite_GetStatusReg(instance->baseAddr) & XUL_SR_TX_FIFO_FULL) == 0) { break; }
			return 1;
		}
	}

	return 0;
}

/**
 * @brief	Wait for TX-FIFO is empty
 * @param	instance		instance
 * @retval	0				success
 * @retval	!=0				failure
 */
static int waitTxFifoEmpty(const struct MbUart* const instance)
{
	const uint32_t baseCount = instance->freeRunCounter->now();
	const uint32_t timeoutCount = instance->freeRunCounter->convertUsecToCount(instance->framePeriodUsec * XUL_FIFO_SIZE);

	while ((XUartLite_GetStatusReg(instance->baseAddr) & XUL_SR_TX_FIFO_EMPTY) == 0) {
		if (instance->freeRunCounter->timeout(baseCount, timeoutCount)) {
			if (XUartLite_GetStatusReg(instance->baseAddr) & XUL_SR_TX_FIFO_EMPTY) { break; }
			return 1;
		}
	}

	return 0;
}

/**
 * @brief	Write to TX-FIFO
 * @param	instance		instance
 * @return	none
 */
static void writeToTxFifo(struct MbUart* const instance)
{
	for (int i = 0; i < XUL_FIFO_SIZE; i++) {
		if (XUartLite_GetStatusReg(instance->baseAddr) & XUL_SR_TX_FIFO_FULL) { break; }
		if (FixedQueue8_empty(instance->txQueue)) { break; }
		XUartLite_SendByte(instance->baseAddr, FixedQueue8_peek(instance->txQueue));
		FixedQueue8_pop(instance->txQueue);
	}
}

/**
 * @brief	Get frame period
 * @param	self			Uart*
 * @return	frame period
 */
unsigned int MbUart_getFramePeriodUsec(const struct Uart* const self)
{
	return ((const struct MbUart*)self)->framePeriodUsec;
}

/**
 * @brief	Overrun error occurred
 * @param	self			Uart*
 * @retval	true			occurred
 * @retval	false			not occurred
 */
bool MbUart_overrunErrorOccurred(const struct Uart* const self)
{
	struct MbUart* const instance = (struct MbUart*)self;

	XIntc_DisableIntr(instance->icBase, instance->irqMask);
	const uint32_t lastError = instance->lastError;
	XIntc_EnableIntr(instance->icBase, instance->irqMask);

	return (lastError & XUL_SR_OVERRUN_ERROR) ? true : false;
}

/**
 * @brief	Framing error occurred
 * @param	self			Uart*
 * @retval	true			occurred
 * @retval	false			not occurred
 */
bool MbUart_framingErrorOccurred(const struct Uart* const self)
{
	struct MbUart* const instance = (struct MbUart*)self;

	XIntc_DisableIntr(instance->icBase, instance->irqMask);
	const uint32_t lastError = instance->lastError;
	XIntc_EnableIntr(instance->icBase, instance->irqMask);

	return (lastError & XUL_SR_FRAMING_ERROR) ? true : false;
}

/**
 * @brief	Parity error occurred
 * @param	self			Uart*
 * @retval	true			occurred
 * @retval	false			not occurred
 */
bool MbUart_parityErrorOccurred(const struct Uart* const self)
{
	struct MbUart* const instance = (struct MbUart*)self;

	XIntc_DisableIntr(instance->icBase, instance->irqMask);
	const uint32_t lastError = instance->lastError;
	XIntc_EnableIntr(instance->icBase, instance->irqMask);

	return (lastError & XUL_SR_PARITY_ERROR) ? true : false;
}

/**
 * @brief	Set up interrupt
 * @param	instance		instance
 * @return	none
 */
static void setupInterrupt(struct MbUart* const instance)
{
	XUartLite_DisableIntr(instance->baseAddr);

	/* make error mask */
	instance->errorMask = (
			XUL_SR_PARITY_ERROR		/*!< parity error */
		|	XUL_SR_FRAMING_ERROR	/*!< framing error */
		|	XUL_SR_OVERRUN_ERROR	/*!< receiver-overrun error */
	);

	XUartLite_SetControlReg(instance->baseAddr, (XUL_CR_FIFO_RX_RESET | XUL_CR_FIFO_TX_RESET));
	XUartLite_EnableIntr(instance->baseAddr);

	XIntc_RegisterHandler(instance->icBase, instance->irq, interruptHandler, instance);
}

/**
 * @brief	Interrupt Handler
 * @param	context			context
 * @return	none
 */
static void interruptHandler(void* const context)
{
	struct MbUart* const instance = (struct MbUart*)context;
	const uint32_t status = XUartLite_GetStatusReg(instance->baseAddr);

	if (status & instance->errorMask) {
		instance->lastError |= (status & instance->errorMask);
		XUartLite_SetControlReg(instance->baseAddr, XUL_CR_FIFO_RX_RESET);
	}

	if (status & XUL_SR_RX_FIFO_VALID_DATA) { receiveInterrupt(instance); }
	if ((status & XUL_SR_TX_FIFO_FULL) == 0) { transmitInterrupt(instance); }

	XIntc_AckIntr(instance->icBase, instance->irqMask);
}

/**
 * @brief	Transmit Interrupt Processing
 * @param	instance		instance
 * @return	none
 */
static void transmitInterrupt(struct MbUart* const instance)
{
	writeToTxFifo(instance);
}

/**
 * @brief	Receive Interrupt Processing
 * @param	instance		instance
 * @return	none
 */
static void receiveInterrupt(struct MbUart* const instance)
{
	for (int i = 0; i < XUL_FIFO_SIZE; i++) {
		if ((XUartLite_GetStatusReg(instance->baseAddr) & XUL_SR_RX_FIFO_VALID_DATA) == 0) { break; }
		if (FixedQueue8_full(instance->rxQueue)) {
			instance->lastError |= XUL_SR_OVERRUN_ERROR;
			XUartLite_RecvByte(instance->baseAddr); /*!< thrown away */
		} else {
			FixedQueue8_push(instance->rxQueue, XUartLite_RecvByte(instance->baseAddr));
		}
	}
}

/**
 * @brief	Assign virtual functions
 * @param	instance		instance
 * @return	none
 */
static void assignVirtualFunctions(struct MbUart* const instance)
{
	instance->uart.destroy					= MbUart_destroy;

	instance->uart.setup					= MbUart_setup;

	instance->uart.get						= MbUart_get;
	instance->uart.put						= MbUart_put;
	instance->uart.read						= MbUart_read;
	instance->uart.write					= MbUart_write;

	instance->uart.clear					= MbUart_clear;
	instance->uart.flush					= MbUart_flush;

	instance->uart.getFramePeriodUsec		= MbUart_getFramePeriodUsec;
	instance->uart.overrunErrorOccurred		= MbUart_overrunErrorOccurred;
	instance->uart.framingErrorOccurred		= MbUart_framingErrorOccurred;
	instance->uart.parityErrorOccurred		= MbUart_parityErrorOccurred;
}
