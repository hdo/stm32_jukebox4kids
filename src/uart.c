#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "stm32.h"
#include "stm32f4xx_conf.h"
#include "uart.h"
#include "math_utils.h"

volatile uint32_t UART1Status, UART2Status, UART3Status;
volatile uint8_t  UART1Buffer[UART_BUFSIZE], UART2Buffer[UART_BUFSIZE], UART3Buffer[UART_BUFSIZE];
volatile uint16_t UART1Count = 0, UART2Count = 0, UART3Count = 0;


void uart1_init(uint32_t baudrate) {

	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	//enable bus clocks

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	/* This sequence sets up the TX and RX pins
	 * so they work correctly with the USART peripheral
	 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;  // Pins 6 (TX) and 7 (RX) are used
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF; 			// the pins are configured as alternate function so the USART peripheral has access to them
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		// this defines the IO speed and has nothing to do with the baudrate!
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;			// this defines the output type as push pull mode (as opposed to open drain)
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;			// this activates the pullup resistors on the IO pins
	GPIO_Init(GPIOB, &GPIO_InitStructure);					// now all the values are passed to the GPIO_Init() function which sets the GPIO registers

	/* The RX and TX pins are now connected to their AF
	 * so that the USART can take over control of the
	 * pins
	 */
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_USART1); //
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_USART1);

	/* Now the USART_InitStruct is used to define the
	 * properties of USART
	 */
	USART_InitStructure.USART_BaudRate = baudrate;				// the baudrate is set to the value we passed into this init function
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;// we want the data frame size to be 8 bits (standard)
	USART_InitStructure.USART_StopBits = USART_StopBits_1;		// we want 1 stop bit (standard)
	USART_InitStructure.USART_Parity = USART_Parity_No;		// we don't want a parity bit (standard)
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // we don't want flow control (standard)
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx; // we want to enable the transmitter and the receiver

	USART_Init(USART1, &USART_InitStructure);					// again all the properties are passed to the USART_Init function which takes care of all the bit setting

	/* Here the USART1 receive interrupt is enabled
	 * and the interrupt controller is configured
	 * to jump to the USART1_IRQHandler() function
	 * if the USART1 receive interrupt occurs
	 */
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);             // enable the USART1 receive interrupt

	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;          // we want to configure the USART1 interrupts
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  // this sets the priority group of the USART1 interrupts
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;         // this sets the subpriority inside the group
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;	           // the USART1 interrupts are globally enabled
	NVIC_Init(&NVIC_InitStructure);                            // the properties are passed to the NVIC_Init function which takes care of the low level stuff

	//Enable USART
	USART_Cmd(USART1, ENABLE);
}

void uart2_init(uint32_t baudrate) {

	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;


	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);

	USART_InitStructure.USART_BaudRate = baudrate;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;

	USART_Init(USART2, &USART_InitStructure);

	USART_Cmd(USART2, ENABLE);
}


void uart3_init(uint32_t baudrate) {

	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_USART3); //
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_USART3);

	USART_InitStructure.USART_BaudRate = baudrate;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;

	USART_Init(USART3, &USART_InitStructure);

	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	//Enable USART3
	USART_Cmd(USART3, ENABLE);
}


uint8_t uart_init( uint8_t portNum, uint32_t baudrate ) {
	if (portNum == 1) {
		uart1_init(baudrate);
		return 1;
	}
	if (portNum == 2) {
		uart2_init(baudrate);
		return 1;
	}
	if (portNum == 3) {
		uart3_init(baudrate);
		return 1;
	}
	return 0;
}

void uart_send( uint8_t portNum, uint8_t *BufferPtr, uint32_t length ) {
	if (portNum == 1) {
		while (length != 0) {
					// wait until data register is empty
			while( !(USART1->SR & USART_FLAG_TC) );
			USART_SendData(USART1, *BufferPtr++);
		}
	} else	if (portNum == 2) {
		while (length != 0) {
					// wait until data register is empty
			while( !(USART2->SR & USART_FLAG_TC) );
			USART_SendData(USART2, *BufferPtr++);
		}
	} else	if (portNum == 3) {
		while (length != 0) {
					// wait until data register is empty
			while( !(USART3->SR & USART_FLAG_TC) );
			USART_SendData(USART3, *BufferPtr++);
		}
	}
}

void uart_sendByte( uint8_t portNum, uint8_t data) {
	if (portNum == 1) {
		while( !(USART1->SR & USART_FLAG_TC) );
		USART_SendData(USART1, data);
	} else	if (portNum == 2) {
		while( !(USART2->SR & USART_FLAG_TC) );
		USART_SendData(USART2, data);
	} else	if (portNum == 3) {
		while( !(USART3->SR & USART_FLAG_TC) );
		USART_SendData(USART3, data);
	}
}

void uart_sendCRLF( uint8_t portNum) {
	uart_sendByte(portNum, 13);
	uart_sendByte(portNum, 10);
}

void uart_sendString( uint8_t portNum, char* data) {
	while(*data) {
		uart_sendByte(portNum, *data++);
	}
}

void uart_sendStringln( uint8_t portNum, char* data) {
	uart_sendString(portNum, data);
	uart_sendCRLF(portNum);
}

void uart_sendNumber( uint8_t portNum, uint32_t value) {
	char buf[10];
	math_itoa(value, buf);
	uart_sendString(portNum, (char*) buf);
}

void uart_sendNumberln( uint8_t portNum, uint32_t value) {
	uart_sendNumber(portNum, value);
	uart_sendCRLF(portNum);
}

void USART1_IRQHandler() {

	// check if the USART1 receive interrupt flag was set
	if( USART_GetITStatus(USART1, USART_IT_RXNE) ){
		/* Receive Data Available */
		UART1Buffer[UART1Count++] = USART_ReceiveData(USART1);
		if ( UART1Count == UART_BUFSIZE ) {
		  UART1Count = 0;		/* buffer overflow */
		}
	}

}

void USART2_IRQHandler() {

	if( USART_GetITStatus(USART2, USART_IT_RXNE) ){
		UART2Buffer[UART2Count++] = USART_ReceiveData(USART2);
		if ( UART2Count == UART_BUFSIZE ) {
		  UART2Count = 0;
		}
	}

}

void USART3_IRQHandler() {
	if( USART_GetITStatus(USART3, USART_IT_RXNE) ){
		UART3Buffer[UART3Count++] = USART_ReceiveData(USART3);
		if ( UART3Count == UART_BUFSIZE ) {
		  UART3Count = 0;
		}
	}
}
