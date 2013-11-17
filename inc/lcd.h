#include <stdint.h>

#ifndef __LCD_H
#define __LCD_H


void lcd_init();
void lcd_reset();
void lcd_clear();
void lcd_command(uint8_t data);
void lcd_data(uint8_t data);
void lcd_setcursor(uint8_t x, uint8_t y);
void lcd_string(const char *data);

#endif /* end __LCD_H */
