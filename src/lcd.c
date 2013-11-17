#include <stdint.h>
#include <string.h>
#include "stm32.h"
#include "lcd.h"
#include "delay.h"
#include "uart.h"

/**
 * LCD source based on: http://www.mikrocontroller.net/articles/AVR-GCC-Tutorial/LCD-Ansteuerung
 *
 */

#define LCD_PORT GPIOB
#define LCD_PORT_RCC RCC_AHB1Periph_GPIOB
#define LCD_PIN_RS GPIO_Pin_10
#define LCD_PIN_E  GPIO_Pin_11
#define LCD_PIN_D4 GPIO_Pin_12
#define LCD_PIN_D5 GPIO_Pin_13
#define LCD_PIN_D6 GPIO_Pin_14
#define LCD_PIN_D7 GPIO_Pin_15

#define LCD_E_LOW LCD_PORT->BSRRH = LCD_PIN_E
#define LCD_E_HIGH LCD_PORT->BSRRL = LCD_PIN_E
#define LCD_RS_LOW LCD_PORT->BSRRH = LCD_PIN_RS
#define LCD_RS_HIGH LCD_PORT->BSRRL = LCD_PIN_RS

////////////////////////////////////////////////////////////////////////////////
// LCD Ausführungszeiten (MS=Millisekunden, US=Mikrosekunden)

#define LCD_BOOTUP_MS           15
#define LCD_ENABLE_US           20
#define LCD_WRITEDATA_US        50
#define LCD_COMMAND_US          50

#define LCD_SOFT_RESET_MS1      5
#define LCD_SOFT_RESET_MS2      1
#define LCD_SOFT_RESET_MS3      1
#define LCD_SET_4BITMODE_MS     5

#define LCD_CLEAR_DISPLAY_MS    2
#define LCD_CURSOR_HOME_MS      2

////////////////////////////////////////////////////////////////////////////////
// Zeilendefinitionen des verwendeten LCD
// Die Einträge hier sollten für ein LCD mit einer Zeilenlänge von 16 Zeichen passen
// Bei anderen Zeilenlängen müssen diese Einträge angepasst werden

#define LCD_DDADR_LINE1         0x00
#define LCD_DDADR_LINE2         0x40
#define LCD_DDADR_LINE3         0x10
#define LCD_DDADR_LINE4         0x50

////////////////////////////////////////////////////////////////////////////////
// LCD Befehle und Argumente.
// Zur Verwendung in lcd_command

// Clear Display -------------- 0b00000001
#define LCD_CLEAR_DISPLAY       0x01

// Cursor Home ---------------- 0b0000001x
#define LCD_CURSOR_HOME         0x02

// Set Entry Mode ------------- 0b000001xx
#define LCD_SET_ENTRY           0x04

#define LCD_ENTRY_DECREASE      0x00
#define LCD_ENTRY_INCREASE      0x02
#define LCD_ENTRY_NOSHIFT       0x00
#define LCD_ENTRY_SHIFT         0x01

// Set Display ---------------- 0b00001xxx
#define LCD_SET_DISPLAY         0x08

#define LCD_DISPLAY_OFF         0x00
#define LCD_DISPLAY_ON          0x04
#define LCD_CURSOR_OFF          0x00
#define LCD_CURSOR_ON           0x02
#define LCD_BLINKING_OFF        0x00
#define LCD_BLINKING_ON         0x01

// Set Shift ------------------ 0b0001xxxx
#define LCD_SET_SHIFT           0x10

#define LCD_CURSOR_MOVE         0x00
#define LCD_DISPLAY_SHIFT       0x08
#define LCD_SHIFT_LEFT          0x00
#define LCD_SHIFT_RIGHT         0x04

// Set Function --------------- 0b001xxxxx
#define LCD_SET_FUNCTION        0x20

#define LCD_FUNCTION_4BIT       0x00
#define LCD_FUNCTION_8BIT       0x10
#define LCD_FUNCTION_1LINE      0x00
#define LCD_FUNCTION_2LINE      0x08
#define LCD_FUNCTION_5X7        0x00
#define LCD_FUNCTION_5X10       0x04

#define LCD_SOFT_RESET          0x30

// Set CG RAM Address --------- 0b01xxxxxx  (Character Generator RAM)
#define LCD_SET_CGADR           0x40

#define LCD_GC_CHAR0            0
#define LCD_GC_CHAR1            1
#define LCD_GC_CHAR2            2
#define LCD_GC_CHAR3            3
#define LCD_GC_CHAR4            4
#define LCD_GC_CHAR5            5
#define LCD_GC_CHAR6            6
#define LCD_GC_CHAR7            7

// Set DD RAM Address --------- 0b1xxxxxxx  (Display Data RAM)
#define LCD_SET_DDADR           0x80



void lcd_enable() {
	LCD_E_HIGH;
	delay_us(LCD_ENABLE_US);
	LCD_E_LOW;
}

/*
 * write the high nibble to lcd
 */
void lcd_out(uint8_t data) {
	uint16_t set_data = 0x0000;
	uint16_t clear_data = 0x0000;

	//LCD_E_LOW;

	// look for the high nibble

	if (data & 0x10) {
		set_data |= LCD_PIN_D4;
	}
	else {
		clear_data |= LCD_PIN_D4;
	}
	if (data & 0x20) {
		set_data |= LCD_PIN_D5;
	}
	else {
		clear_data |= LCD_PIN_D5;
	}
	if (data & 0x40) {
		set_data |= LCD_PIN_D6;
	}
	else {
		clear_data |= LCD_PIN_D6;
	}
	if (data & 0x80) {
		set_data |= LCD_PIN_D7;
	}
	else {
		clear_data |= LCD_PIN_D7;
	}
	LCD_PORT->BSRRL = set_data;
	LCD_PORT->BSRRH = clear_data;

	lcd_enable();
}

void lcd_command(uint8_t data) {
   	LCD_RS_LOW;
	lcd_out(data); // high nibble first
	lcd_out(data << 4); // high nibble first
	LCD_RS_HIGH;
	delay_us(LCD_COMMAND_US);
}

void lcd_data( uint8_t data ) {
   	LCD_RS_HIGH;
    lcd_out(data);
    lcd_out(data << 4);
	delay_us(LCD_WRITEDATA_US);
}


void lcd_reset() {
	delay_ms(LCD_BOOTUP_MS);

    // Soft-Reset muss 3mal hintereinander gesendet werden zur Initialisierung
	lcd_out(LCD_SOFT_RESET);
	delay_ms(LCD_SOFT_RESET_MS1);
	lcd_enable();
	delay_ms(LCD_SOFT_RESET_MS2);
	lcd_enable();
	delay_ms(LCD_SOFT_RESET_MS3);

	// 4-bit Modus aktivieren
	lcd_out(LCD_SET_FUNCTION | LCD_FUNCTION_4BIT);
	delay_ms(LCD_SET_4BITMODE_MS);

	// 4-bit Modus / 2 Zeilen / 5x7
	lcd_command(LCD_SET_FUNCTION | LCD_FUNCTION_4BIT | LCD_FUNCTION_2LINE | LCD_FUNCTION_5X7);

	// Display ein / Cursor aus / Blinken aus
	lcd_command(LCD_SET_DISPLAY | LCD_DISPLAY_ON | LCD_CURSOR_OFF | LCD_BLINKING_OFF);

	// Cursor inkrement / kein Scrollen
	lcd_command(LCD_SET_ENTRY | LCD_ENTRY_INCREASE | LCD_ENTRY_NOSHIFT);

	lcd_clear();

}

void lcd_clear() {
    lcd_command( LCD_CLEAR_DISPLAY );
    delay_ms(LCD_CLEAR_DISPLAY_MS);
}


void lcd_home() {
    lcd_command( LCD_CURSOR_HOME );
    delay_ms(LCD_CURSOR_HOME_MS);
}

void lcd_setcursor(uint8_t x, uint8_t y) {
    uint8_t data;

    switch (y) {
        case 0:    // 1. Zeile
            data = LCD_SET_DDADR + LCD_DDADR_LINE1 + x;
            break;

        case 1:    // 2. Zeile
            data = LCD_SET_DDADR + LCD_DDADR_LINE2 + x;
            break;

        case 2:    // 3. Zeile
            data = LCD_SET_DDADR + LCD_DDADR_LINE3 + x;
            break;

        case 3:    // 4. Zeile
            data = LCD_SET_DDADR + LCD_DDADR_LINE4 + x;
            break;

        default:
            return; // für den Fall einer falschen Zeile
    }

    lcd_command(data);
}

void lcd_init() {
	/* GPIOD Periph clock enable */
	RCC_AHB1PeriphClockCmd(LCD_PORT_RCC, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;

	/* Configure PD12, 13, 14 and PD15 in output pushpull mode */
	GPIO_InitStructure.GPIO_Pin = LCD_PIN_E | LCD_PIN_RS | LCD_PIN_D4 | LCD_PIN_D5 | LCD_PIN_D6 | LCD_PIN_D7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(LCD_PORT, &GPIO_InitStructure);

	// set all low
	LCD_PORT->BSRRH = LCD_PIN_E | LCD_PIN_RS | LCD_PIN_D4 | LCD_PIN_D5 | LCD_PIN_D6 | LCD_PIN_D7;

	lcd_reset();
}


void lcd_string(const char *data) {
    while( *data != '\0' )
        lcd_data( *data++ );
}
