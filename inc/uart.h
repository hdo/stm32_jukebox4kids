#include <stdint.h>

#ifndef __UART_H
#define __UART_H

#define UART_BUFSIZE		0x40

uint8_t uart_init( uint8_t portNum, uint32_t baudrate );
void uart_send( uint8_t portNum, uint8_t *BufferPtr, uint32_t length );
void uart_sendByte( uint8_t portNum, uint8_t data);
void uart_sendString( uint8_t portNum, char* data);
void uart_sendStringln( uint8_t portNum, char* data);
void uart_sendNumber( uint8_t portNum, uint32_t value);
void uart_sendNumberln( uint8_t portNum, uint32_t value);
void uart_sendCRLF( uint8_t portNum);

void USART1_IRQHandler();
void USART2_IRQHandler();
void USART3_IRQHandler();


#endif /* end __UART_H */
