/**
 * @file	nios_uart.c
 * @brief	Altera Avalon UART
 * @author	Tsuguyoshi Higano
 * @date	Dec 06, 2018
 *
 * @par Project
 * Software Development Platform for Small-scale Embedded Systems (SDPSES)
 *
 * @copyright (c) Tsuguyoshi Higano, 2017-2018
 *
 * @par License
 * Released under the MIT license@n
 * http://opensource.org/licenses/mit-license.php
 */

#include <sys/alt_irq.h>

#include "allocator.h"
#include "altera_avalon_uart_regs.h"
#include "uart_private.h"
#include "nios_uart.h"
#include "fixed_queue8.h"
#include "free_run_counter.h"
#include "lib_debug.h"

/**
 * @struct	NiosUart
 * @brief	NiosUart struct
 * @extends	Uart
 */
struct NiosUart {
	struct Uart uart; /*!< must be the first member for mutual conversion of pointers */

	uint32_t baseAddr;
	uint32_t freq;
	uint32_t icId;
	uint32_t irq;

	uint16_t interruptFlags;
	uint16_t errorMask;
	uint16_t lastError;

	unsigned int framePeriodUsec;

	FixedQueue8* txQueue;
	FixedQueue8* rxQueue;

	const FreeRunCounter* freeRunCounter;
};

static int validateSerialParams(const SerialParams* params);

static void clearBuffer(struct NiosUart* instance);
static int waitStatusReady(const struct NiosUart* instance, uint16_t status);

static int setupInterrupt(struct NiosUart* instance);
static void interruptServiceRoutine(void* isr_context);
static void transmitInterrupt(struct NiosUart* instance);
static void receiveInterrupt(struct NiosUart* instance);

static void assignVirtualFunctions(struct NiosUart* instance);

/**
 * @brief	Get the size of NiosUart
 * @return	the size of NiosUart
 */
size_t NiosUart_sizeOf(void)
{
	return sizeof(struct NiosUart);
}

/**
 * @brief	Create
 * @param	base_addr		base address
 * @param	freq			frequency(Hz)
 * @param	ic_id			intc id
 * @param	irq				irq number
 * @param	uart_params		NiosUartParams*
 * @return	instance
 */
struct NiosUart* NiosUart_create(const uint32_t base_addr, const uint32_t freq,
		const uint32_t ic_id, const uint32_t irq, const NiosUartParams* const uart_params)
{
	struct NiosUart* const instance = Allocator_allocate(sizeof(struct NiosUart));
	if (!instance) {
		DEBUG_PRINTF_("Cannot allocate memory\r\n");
		return NULL;
	}

	if (NiosUart_ctor(instance, base_addr, freq, ic_id, irq, uart_params)) {
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
struct Uart* NiosUart_destroy(struct Uart* const self)
{
	if (!self) { return NULL; }

	struct NiosUart* const instance = (struct NiosUart*)self;
	NiosUart_dtor(instance);
	Allocator_deallocate(instance);

	return NULL;
}

/**
 * @brief	Constructor
 * @param	instance		instance
 * @param	base_addr		base address
 * @param	freq			frequency(Hz)
 * @param	ic_id			intc id
 * @param	irq				irq number
 * @param	uart_params		NiosUartParams*
 * @retval	0				success
 * @retval	!=0				failure
 */
int NiosUart_ctor(struct NiosUart* const instance, const uint32_t base_addr,
		const uint32_t freq, const uint32_t ic_id, const uint32_t irq, const NiosUartParams* const uart_params)
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
	DEBUG_PRINTF_("  IRQ           : [%d]\r\n", (int)irq);
	DEBUG_PRINTF_("  TX BUFF SIZE  : [%u]\r\n", uart_params->txBuffSz);
	DEBUG_PRINTF_("  RX BUFF SIZE  : [%u]\r\n", uart_params->rxBuffSz);
	DEBUG_PRINTF_("\r\n");

	if (Uart_ctor((struct Uart*)instance)) { return 1; }

	assignVirtualFunctions(instance);

	instance->baseAddr			= base_addr;
	instance->freq				= freq;
	instance->icId				= ic_id;
	instance->irq				= irq;

	instance->interruptFlags	= 0;
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

	instance->freeRunCounter	= FreeRunCounter_getInstance();

	const SerialParams serialParams = {
			kSERIAL_BITRATE_DEFAULT,
			kSERIAL_DATABIT_DEFAULT,
			kSERIAL_PARITY_DEFAULT,
			kSERIAL_STOPBIT_DEFAULT,
			kSERIAL_FLOW_CONTROL_DEFAULT,
	};
	if (NiosUart_setup((struct Uart*)instance, &serialParams)) { goto TERMINATE; }

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
void NiosUart_dtor(struct NiosUart* const instance)
{
	if (!instance) { return; }

	alt_ic_irq_disable(instance->icId, instance->irq);
	IOWR_ALTERA_AVALON_UART_DIVISOR(instance->baseAddr, 0);
	IOWR_ALTERA_AVALON_UART_CONTROL(instance->baseAddr, 0);
	IOWR_ALTERA_AVALON_UART_STATUS(instance->baseAddr, 0);

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
int NiosUart_setup(struct Uart* const self, const SerialParams* const params)
{
	if (validateSerialParams(params)) { return 1; }

	struct NiosUart* const instance = (struct NiosUart*)self;

	alt_ic_irq_disable(instance->icId, instance->irq);
	instance->framePeriodUsec = SerialParams_calcFramePeriodUsec(params);

	/* set bitrate */
	const uint16_t divisor = (uint16_t)(((double)instance->freq / (double)params->bitrate) + 0.5F);
	IOWR_ALTERA_AVALON_UART_DIVISOR(instance->baseAddr, divisor);

	clearBuffer(instance);
	instance->lastError = 0;

	if (setupInterrupt(instance)) { return 1; }
	alt_ic_irq_enable(instance->icId, instance->irq);

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
		break;
	default:
		DEBUG_PRINTF_("error: NiosII UART bitrate parameter [%dbps]\r\n", params->bitrate);
		return 1;
	}

	switch (params->databit) {
	case kSERIAL_DATABIT_7:
	case kSERIAL_DATABIT_8:
		break;
	case kSERIAL_DATABIT_9: /*!< not supported */
	default:
		DEBUG_PRINTF_("error: NiosII UART databit parameter [%dbit]\r\n", params->databit);
		return 1;
	}

	switch (params->parity) {
	case kSERIAL_PARITY_NONE:
	case kSERIAL_PARITY_ODD:
	case kSERIAL_PARITY_EVEN:
		break;
	default:
		DEBUG_PRINTF_("error: NiosII UART parity parameter [%d]\r\n", params->parity);
		return 1;
	}

	switch (params->stopbit) {
	case kSERIAL_STOPBIT_1:
	case kSERIAL_STOPBIT_2:
		break;
	default:
		DEBUG_PRINTF_("error: NiosII UART stopbit parameter [%dbit]\r\n", params->stopbit);
		return 1;
	}

	switch (params->flowControl) {
	case kSERIAL_FLOW_CONTROL_NONE:
		break;
	default:
		DEBUG_PRINTF_("error: NiosII UART flow control parameter [%d]\r\n", params->flowControl);
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
int NiosUart_get(struct Uart* const self, uint8_t* const data)
{
	int rc = 1;
	struct NiosUart* const instance = (struct NiosUart*)self;

	alt_ic_irq_disable(instance->icId, instance->irq);
	if (!FixedQueue8_empty(instance->rxQueue)) {
		*data = FixedQueue8_front(instance->rxQueue);
		FixedQueue8_pop(instance->rxQueue);
		rc = 0;
	}
	alt_ic_irq_enable(instance->icId, instance->irq);

	return rc;
}

/**
 * @brief	Put a data
 * @param	self			Uart*
 * @param	data			data
 * @retval	0				success
 * @retval	!=0				failure
 */
int NiosUart_put(struct Uart* const self, const uint8_t data)
{
	int rc = 1;
	struct NiosUart* const instance = (struct NiosUart*)self;

	alt_ic_irq_disable(instance->icId, instance->irq);
	if (IORD_ALTERA_AVALON_UART_STATUS(instance->baseAddr) & ALTERA_AVALON_UART_STATUS_TRDY_MSK) {
		if (FixedQueue8_empty(instance->txQueue)) {
			IOWR_ALTERA_AVALON_UART_TXDATA(instance->baseAddr, data);
		} else {
			IOWR_ALTERA_AVALON_UART_TXDATA(instance->baseAddr, FixedQueue8_front(instance->txQueue));
			FixedQueue8_pop(instance->txQueue);
			FixedQueue8_push(instance->txQueue, data);
		}
		rc = 0;
	} else if (!FixedQueue8_full(instance->txQueue)) {
		FixedQueue8_push(instance->txQueue, data);
		rc = 0;
	}
	instance->interruptFlags |= ALTERA_AVALON_UART_CONTROL_TRDY_MSK;
	IOWR_ALTERA_AVALON_UART_CONTROL(instance->baseAddr, instance->interruptFlags);
	alt_ic_irq_enable(instance->icId, instance->irq);

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
int NiosUart_read(struct Uart* const self, uint8_t data_buff[], const unsigned int data_count)
{
	int rc = 1;
	struct NiosUart* const instance = (struct NiosUart*)self;

	alt_ic_irq_disable(instance->icId, instance->irq);
	if (FixedQueue8_size(instance->rxQueue) >= data_count) {
		for (unsigned int i = 0; i < data_count; i++) {
			data_buff[i] = FixedQueue8_front(instance->rxQueue);
			FixedQueue8_pop(instance->rxQueue);
		}
		rc = 0;
	}
	alt_ic_irq_enable(instance->icId, instance->irq);

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
int NiosUart_write(struct Uart* const self, const uint8_t data_buff[], const unsigned int data_count)
{
	int rc = 1;
	struct NiosUart* const instance = (struct NiosUart*)self;

	alt_ic_irq_disable(instance->icId, instance->irq);
	if (FixedQueue8_availableSize(instance->txQueue) >= data_count) {
		for (unsigned int i = 0; i < data_count; i++) {
			FixedQueue8_push(instance->txQueue, data_buff[i]);
		}
		rc = 0;
	}
	instance->interruptFlags |= ALTERA_AVALON_UART_CONTROL_TRDY_MSK;
	IOWR_ALTERA_AVALON_UART_CONTROL(instance->baseAddr, instance->interruptFlags);
	alt_ic_irq_enable(instance->icId, instance->irq);

	return rc;
}

/**
 * @brief	Clear receive/transmit buffer and errors
 * @param	self			Uart*
 * @return	none
 */
void NiosUart_clear(struct Uart* const self)
{
	struct NiosUart* const instance = (struct NiosUart*)self;

	alt_ic_irq_disable(instance->icId, instance->irq);
	clearBuffer(instance);
	instance->lastError = 0;
	alt_ic_irq_enable(instance->icId, instance->irq);
}

/**
 * @brief	Clear receive/transmit buffer
 * @param	instance		pointer to the instance
 * @return	none
 */
static void clearBuffer(struct NiosUart* const instance)
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
int NiosUart_flush(struct Uart* const self)
{
	int rc = 1;
	struct NiosUart* const instance = (struct NiosUart*)self;

	alt_ic_irq_disable(instance->icId, instance->irq);
	while (!FixedQueue8_empty(instance->txQueue)) {
		if (waitStatusReady(instance, ALTERA_AVALON_UART_STATUS_TRDY_MSK)) { goto TERMINATE; }
		IOWR_ALTERA_AVALON_UART_TXDATA(instance->baseAddr, FixedQueue8_front(instance->txQueue));
		FixedQueue8_pop(instance->txQueue);
	}
	if (waitStatusReady(instance, ALTERA_AVALON_UART_STATUS_TRDY_MSK)) { goto TERMINATE; }
	if (waitStatusReady(instance, ALTERA_AVALON_UART_STATUS_TMT_MSK)) { goto TERMINATE; }

	instance->interruptFlags &= ~ALTERA_AVALON_UART_CONTROL_TRDY_MSK;
	IOWR_ALTERA_AVALON_UART_CONTROL(instance->baseAddr, instance->interruptFlags);
	rc = 0;

TERMINATE:
	alt_ic_irq_enable(instance->icId, instance->irq);
	return rc;
}

/**
 * @brief	Wait for status is ready
 * @param	instance		instance
 * @param	status			status
 * @retval	0				success
 * @retval	!=0				failure
 */
static int waitStatusReady(const struct NiosUart* const instance, const uint16_t status)
{
	const uint32_t baseCount = instance->freeRunCounter->now();
	const uint32_t timeoutCount = instance->freeRunCounter->convertUsecToCount(instance->framePeriodUsec);

	while ((IORD_ALTERA_AVALON_UART_STATUS(instance->baseAddr) & status) != status) {
		if (instance->freeRunCounter->timeout(baseCount, timeoutCount)) {
			if ((IORD_ALTERA_AVALON_UART_STATUS(instance->baseAddr) & status) == status) { break; }
			return 1;
		}
	}

	return 0;
}

/**
 * @brief	Get frame period
 * @param	self			Uart*
 * @return	frame period
 */
unsigned int NiosUart_getFramePeriodUsec(const struct Uart* const self)
{
	return ((const struct NiosUart*)self)->framePeriodUsec;
}

/**
 * @brief	Overrun error occurred
 * @param	self			Uart*
 * @retval	true			occurred
 * @retval	false			not occurred
 */
bool NiosUart_overrunErrorOccurred(const struct Uart* const self)
{
	struct NiosUart* const instance = (struct NiosUart*)self;

	alt_ic_irq_disable(instance->icId, instance->irq);
	const uint16_t lastError = instance->lastError;
	alt_ic_irq_enable(instance->icId, instance->irq);

	return (lastError & ALTERA_AVALON_UART_STATUS_ROE_MSK) ? true : false;
}

/**
 * @brief	Framing error occurred
 * @param	self			Uart*
 * @retval	true			occurred
 * @retval	false			not occurred
 */
bool NiosUart_framingErrorOccurred(const struct Uart* const self)
{
	struct NiosUart* const instance = (struct NiosUart*)self;

	alt_ic_irq_disable(instance->icId, instance->irq);
	const uint16_t lastError = instance->lastError;
	alt_ic_irq_enable(instance->icId, instance->irq);

	return (lastError & ALTERA_AVALON_UART_STATUS_FE_MSK) ? true : false;
}

/**
 * @brief	Parity error occurred
 * @param	self			Uart*
 * @retval	true			occurred
 * @retval	false			not occurred
 */
bool NiosUart_parityErrorOccurred(const struct Uart* const self)
{
	struct NiosUart* const instance = (struct NiosUart*)self;

	alt_ic_irq_disable(instance->icId, instance->irq);
	const uint16_t lastError = instance->lastError;
	alt_ic_irq_enable(instance->icId, instance->irq);

	return (lastError & ALTERA_AVALON_UART_STATUS_PE_MSK) ? true : false;
}

/**
 * @brief	Set up interrupt
 * @param	instance		instance
 * @retval	0				success
 * @retval	!=0				failure
 */
static int setupInterrupt(struct NiosUart* const instance)
{
	IOWR_ALTERA_AVALON_UART_CONTROL(instance->baseAddr, 0);

	/* make interrupt flags */
	instance->interruptFlags = (
			ALTERA_AVALON_UART_CONTROL_PE_MSK		/*!< parity error */
		|	ALTERA_AVALON_UART_CONTROL_FE_MSK		/*!< framing error */
		|	ALTERA_AVALON_UART_CONTROL_ROE_MSK		/*!< receiver-overrun error */
		|	ALTERA_AVALON_UART_CONTROL_RRDY_MSK		/*!< read-ready */
	);

	/* make error mask */
	instance->errorMask = (
			ALTERA_AVALON_UART_STATUS_PE_MSK		/*!< parity error */
		|	ALTERA_AVALON_UART_STATUS_FE_MSK		/*!< framing error */
		|	ALTERA_AVALON_UART_STATUS_ROE_MSK		/*!< receiver-overrun error */
	);

	if (alt_ic_isr_register(instance->icId, instance->irq, interruptServiceRoutine, instance, (void*)0)) { return 1; }
	alt_ic_irq_disable(instance->icId, instance->irq);

	IOWR_ALTERA_AVALON_UART_CONTROL(instance->baseAddr, instance->interruptFlags);
	IOWR_ALTERA_AVALON_UART_STATUS(instance->baseAddr, 0);

	return 0;
}

/**
 * @brief	Interrupt Service Routine
 * @param	isr_context		ISR context
 * @return	none
 */
static void interruptServiceRoutine(void* const isr_context)
{
	struct NiosUart* const instance = (struct NiosUart*)isr_context;
	const uint16_t status = IORD_ALTERA_AVALON_UART_STATUS(instance->baseAddr);

	if (status & instance->errorMask) {
		instance->lastError |= (status & instance->errorMask);
		IOWR_ALTERA_AVALON_UART_STATUS(instance->baseAddr, 0);
	}

	if (status & ALTERA_AVALON_UART_STATUS_RRDY_MSK) { receiveInterrupt(instance); }
	if (status & ALTERA_AVALON_UART_STATUS_TRDY_MSK) { transmitInterrupt(instance); }
}

/**
 * @brief	Transmit Interrupt Processing
 * @param	instance		instance
 * @return	none
 */
static void transmitInterrupt(struct NiosUart* const instance)
{
	if (FixedQueue8_empty(instance->txQueue)) {
		instance->interruptFlags &= ~ALTERA_AVALON_UART_CONTROL_TRDY_MSK;
		IOWR_ALTERA_AVALON_UART_CONTROL(instance->baseAddr, instance->interruptFlags);
	} else {
		IOWR_ALTERA_AVALON_UART_TXDATA(instance->baseAddr, FixedQueue8_front(instance->txQueue));
		FixedQueue8_pop(instance->txQueue);
	}
}

/**
 * @brief	Receive Interrupt Processing
 * @param	instance		instance
 * @return	none
 */
static void receiveInterrupt(struct NiosUart* const instance)
{
	if (FixedQueue8_full(instance->rxQueue)) {
		instance->lastError |= ALTERA_AVALON_UART_STATUS_ROE_MSK;
		IORD_ALTERA_AVALON_UART_RXDATA(instance->baseAddr); /*!< thrown away */
	} else {
		FixedQueue8_push(instance->rxQueue, IORD_ALTERA_AVALON_UART_RXDATA(instance->baseAddr));
	}
}

/**
 * @brief	Assign virtual functions
 * @param	instance		instance
 * @return	none
 */
static void assignVirtualFunctions(struct NiosUart* const instance)
{
	instance->uart.destroy					= NiosUart_destroy;

	instance->uart.setup					= NiosUart_setup;

	instance->uart.get						= NiosUart_get;
	instance->uart.put						= NiosUart_put;
	instance->uart.read						= NiosUart_read;
	instance->uart.write					= NiosUart_write;

	instance->uart.clear					= NiosUart_clear;
	instance->uart.flush					= NiosUart_flush;

	instance->uart.getFramePeriodUsec		= NiosUart_getFramePeriodUsec;
	instance->uart.overrunErrorOccurred		= NiosUart_overrunErrorOccurred;
	instance->uart.framingErrorOccurred		= NiosUart_framingErrorOccurred;
	instance->uart.parityErrorOccurred		= NiosUart_parityErrorOccurred;
}
