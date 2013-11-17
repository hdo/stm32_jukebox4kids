#include <stdint.h>
#include "stm32.h"
#include "vs1033.h"
#include "delay.h"
#include "uart.h"

#define UART_NUM 3

uint8_t vs_selected_volume = VS_INIT_VOLUME;
uint8_t vs_status = VS_STATUS_FAIL;

void init_SPI1(void){

	GPIO_InitTypeDef GPIO_InitStruct;
	SPI_InitTypeDef SPI_InitStruct;

	// enable clock for used IO pins
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	/* configure pins used by SPI1
	 * PA5 = SCK
	 * PA6 = MISO
	 * PA7 = MOSI
	 */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_6 | GPIO_Pin_5;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStruct);

	// connect SPI1 pins to SPI alternate function
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_SPI1);

	// enable clock for used IO pins
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	// enable peripheral clock
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

	/* configure SPI1 in Mode 0
	 * CPOL = 0 --> clock is low when idle
	 * CPHA = 0 --> data is sampled at the first edge
	 */
	SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex; // set to full duplex mode, seperate MOSI and MISO lines
	SPI_InitStruct.SPI_Mode = SPI_Mode_Master;     // transmit in master mode, NSS pin has to be always high
	SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b; // one packet of data is 8 bits wide
	SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;        // clock is low when idle
	SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;      // data sampled at first edge
	SPI_InitStruct.SPI_NSS = SPI_NSS_Soft | SPI_NSSInternalSoft_Set; // set the NSS management to internal and pull internal NSS high
	SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;// data is transmitted MSB first
	SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32; // SPI frequency is APB2 frequency / 32
	SPI_Init(SPI1, &SPI_InitStruct);

	SPI_Cmd(SPI1, ENABLE); // enable SPI1

	// configure chip select, reset, dreq
	GPIO_InitStruct.GPIO_Pin = RESET_PIN | XDCS_PIN | XCS_PIN;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStruct);

	GPIOA->BSRRL |= RESET_PIN | XDCS_PIN | XCS_PIN; // set high

	 /* Configure input */
	GPIO_InitStruct.GPIO_Pin =  DREQ_PIN;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStruct);

	XDCS_HIGH;
	XCS_HIGH;
}

uint8_t SPI_SendByte(uint8_t data){

	SPI1->DR = data; // write data to be transmitted to the SPI data register
	while( !(SPI1->SR & SPI_I2S_FLAG_TXE) ); // wait until transmit complete
	while( !(SPI1->SR & SPI_I2S_FLAG_RXNE) ); // wait until receive complete
	while( SPI1->SR & SPI_I2S_FLAG_BSY ); // wait until SPI is not busy anymore
	return SPI1->DR; // return received data from SPI data register
}



uint8_t vs_get_status(void) {
	return vs_status;
}

void vs_sci_write(uint8_t address, uint16_t data) {

    WAIT_BUSY;

    XCS_LOW;

    SPI_SendByte(VS1002_WRITE);
	SPI_SendByte(address);

	SPI_SendByte(data >> 8);
	SPI_SendByte(data);

    XCS_HIGH;

    WAIT_BUSY;
}

void vs_send_32(uint8_t *pData, uint8_t len) {
	  XDCS_LOW;
	  uint8_t counter = 0;
		  for(;counter < len; counter++) {
			 SPI_SendByte(*pData++);
		  }
	  XDCS_HIGH;
}

void vs_send_32_endbyte(uint8_t endByte) {
	  XDCS_LOW;
	  uint8_t counter = 0;
	  for(;counter < 32; counter++) {
		 SPI_SendByte(endByte);
	  }
	  XDCS_HIGH;
}

uint16_t vs_sci_read(uint8_t address) {

	WAIT_BUSY;

    XCS_LOW;

    uint16_t retData = 0;

    SPI_SendByte(VS1002_READ);
    SPI_SendByte(address);

    retData = SPI_SendByte(0) << 8;
    retData |= SPI_SendByte(0);

    XCS_HIGH;

    WAIT_BUSY;

    return retData;
}

/*
  Read 16-bit value from addr.
*/
uint16_t vs_read_mem16(uint16_t addr) {
	vs_sci_write(SCI_WRAMADDR, addr);
	return vs_sci_read(SCI_WRAM);
}

void vs_reset(uint8_t type) {
	vs_status = VS_STATUS_FAIL;
    if (type == HARD_RESET) {
    	RESET_LOW;

    	delay_ms(2);
        RESET_HIGH;
    	delay_ms(2);

        WAIT_BUSY;
    }
    else  {
    	vs_sci_write(0x00, VS_INIT_MODE_SOFT_RESET);
    	delay_ms(2);
    	WAIT_BUSY;
    }
}

void vs_start_sinetest(uint8_t pitch)
{
    XDCS_LOW;
    // 0x53, 0xEF, 0x6E, 126, 0, 0, 0, 0
    SPI_SendByte(0x53);
    SPI_SendByte(0xEF);
    SPI_SendByte(0x6E);
    SPI_SendByte(pitch);
    SPI_SendByte(0);
    SPI_SendByte(0);
    SPI_SendByte(0);
    SPI_SendByte(0);
    XDCS_HIGH;
}

void vs_stop_sinetest(void) {
    XDCS_LOW;
    SPI_SendByte(0x45);
    SPI_SendByte(0x78);
    SPI_SendByte(0x69);
    SPI_SendByte(0x74);
    SPI_SendByte(0);
    SPI_SendByte(0);
    SPI_SendByte(0);
    SPI_SendByte(0);
    XDCS_HIGH;
}

void vs_set_volume(uint16_t volume) {
    vs_sci_write(0x0b, volume);
}

void vs_set_simple_volume(uint8_t volume) {
	if (volume > 150) {
		volume = 150;
	}

	vs_selected_volume = 150 - volume;

	uint16_t v = vs_selected_volume | (vs_selected_volume << 8);
    vs_sci_write(0x0b, v);
}

void vs_init(void) {
	init_SPI1();

	uart_sendStringln(UART_NUM, "resetting vs1033");
	vs_reset(HARD_RESET);
	uart_sendStringln(UART_NUM, "done");

	uart_sendString(UART_NUM, "read 0x00: ");
	uart_sendNumberln(UART_NUM, vs_sci_read(0x00));

	vs_sci_write(0x00, VS_INIT_MODE);

	uart_sendString(UART_NUM, "read 0x00: ");
	uart_sendNumberln(UART_NUM, vs_sci_read(0x00));

	uart_sendString(UART_NUM, "read 0x03: ");
	uart_sendNumberln(UART_NUM, vs_sci_read(0x03));
	uart_sendStringln(UART_NUM, "write 0x03, value: 0x9000 ");

	vs_sci_write(0x03, VS_INIT_FREQ);

	uart_sendString(UART_NUM, "read 0x03: ");
	uart_sendNumberln(UART_NUM, vs_sci_read(0x03));
	if (vs_sci_read(0x03) == VS_INIT_FREQ) {
		vs_status = VS_STATUS_OK;
	}

	uart_sendStringln(UART_NUM, "set initial volume ...");
	vs_set_simple_volume(vs_selected_volume);

}

void vs_health_check(void) {
	if (vs_sci_read(0x03) != VS_INIT_FREQ || vs_sci_read(0x00) != VS_INIT_MODE) {
		uart_sendStringln(UART_NUM, "vs failed! (resetting ...)");
		// perform hard reset
		vs_reset(HARD_RESET);
		vs_sci_write(0x00, VS_INIT_MODE);
		vs_sci_write(0x03, VS_INIT_FREQ);
		vs_set_simple_volume(vs_selected_volume);
		vs_status = VS_STATUS_FAIL;
		if (vs_sci_read(0x03) == VS_INIT_FREQ) {
			vs_status = VS_STATUS_OK;
		}
	}
}


