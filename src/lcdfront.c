#include <stdint.h>
#include "lcd.h"
#include "lcdfront.h"
#include "math_utils.h"

/**
 * LCD source based on: http://www.mikrocontroller.net/articles/AVR-GCC-Tutorial/LCD-Ansteuerung
 * Custom characters based on: http://www.instructables.com/id/Music-Playing-Alarm-Clock/step9/Big-Number-Clock-on-LCD/
 *
 */

#define LAST_DISPLAY_TEXT 0
#define LAST_DISPLAY_TIME 1
#define LAST_DISPLAY_TRACKINFO 2
#define LAST_DISPLAY_VOLUME 3

#define DISPLAY_TIME_OUT 200


/**********************
 * CUSTOM CHARACTERS
 */

// addresses for custom chars
#define upperlineaddr 0
#define ulc upperlineaddr
#define lowerlineaddr 1
#define llc lowerlineaddr
#define bothlineaddr 2
#define blc bothlineaddr
#define largecolonleftaddr 3
#define largecolonrightaddr 4
#define xxc 0xFF
#define ssc 0x20
#define upcharaddr 5
#define downcharaddr 6

// below are custom LCD characters

const uint8_t upperlinechar[8] = {
	0b00011111,
	0b00011111,
	0b00011111,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000
};

const uint8_t lowerlinechar[8] = {
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00011111,
	0b00011111,
	0b00011111
};

const uint8_t bothlinechar[8] = {
	0b00011111,
	0b00011111,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00011111,
	0b00011111
};

const uint8_t largecolonleftchar[8] = {
	0b00000000,
	0b00000000,
	0b00000001,
	0b00000011,
	0b00000011,
	0b00000001,
	0b00000000,
	0b00000000
};

const uint8_t largecolonrightchar[8] = {
	0b00000000,
	0b00000000,
	0b00010000,
	0b00011000,
	0b00011000,
	0b00010000,
	0b00000000,
	0b00000000
};

const uint8_t upchar[8] = {
	0b00000000,
	0b00000100,
	0b00001110,
	0b00010101,
	0b00000100,
	0b00000100,
	0b00000100,
	0b00000000
};

const uint8_t downchar[8] = {
	0b00000000,
	0b00000100,
	0b00000100,
	0b00000100,
	0b00010101,
	0b00001110,
	0b00000100,
	0b00000000
};

/*
const uint8_t largecolonleftchar[8] = {
	0b00000000,
	0b00000000,
	0b00000001,
	0b00000011,
	0b00000011,
	0b00000001,
	0b00000000,
	0b00000000
};

const uint8_t largecolonrightchar[8] = {
	0b00000000,
	0b00000000,
	0b00010000,
	0b00011000,
	0b00011000,
	0b00010000,
	0b00000000,
	0b00000000
};
*/


const uint8_t numchar[10][6] = {
	{
		xxc, ulc, xxc,
		xxc, llc, xxc,
	},
	{
		ulc, xxc, ssc,
		llc, xxc, llc,
	},
	{
		ulc, blc, xxc,
		xxc, llc, llc,
	},
	{
		ulc, blc, xxc,
		llc, llc, xxc,
	},
	{
		xxc, llc, xxc,
		ssc, ssc, xxc,
	},
	{
		xxc, blc, ulc,
		llc, llc, xxc,
	},
	{
		xxc, blc, ulc,
		xxc, llc, xxc,
	},
	{
		ulc, ulc, xxc,
		ssc, ssc, xxc,
	},
	{
		xxc, blc, xxc,
		xxc, llc, xxc,
	},
	{
		xxc, blc, xxc,
		llc, llc, xxc,
	}
};

uint8_t last_display = LAST_DISPLAY_TEXT;
uint32_t lcdfront_msticks = 0;
uint32_t last_display_msticks = 0;
uint16_t vol_current = 0;
uint16_t vol_max = 0;
uint16_t track_current = 0;
uint16_t track_max = 0;
uint32_t last_blink_action = 0;
uint8_t flag_blink_mode = 0;
uint8_t flag_current_display_mode = 1;


void lcdfront_setCustomChar(uint8_t address, uint8_t * dataArray) {
	// send custom character definition
	uint8_t addr = address * 8;
	lcd_command(0b01000000 + addr);
	uint8_t i;
	for(i = 0; i < 8; i++)	{
		lcd_data(dataArray[i]);
	}
}

void lcdfront_setTimeCustomCharacters() {

}

void lcdfront_setTrackinfoCustomCharacters() {

}

void lcdfront_setAllCustomCharcaters() {
    // set all custom fonts
    uint8_t temp[8];
    uint8_t i;

	for(i = 0; i < 8; i++) 	{
		temp[i] = upperlinechar[i];
	}
	lcdfront_setCustomChar(upperlineaddr, temp);

	for(i = 0; i < 8; i++) 	{
		temp[i] = lowerlinechar[i];
	}
	lcdfront_setCustomChar(lowerlineaddr, temp);

	for(i = 0; i < 8; i++)	{
		temp[i] = bothlinechar[i];
	}
	lcdfront_setCustomChar(bothlineaddr, temp);

	for(i = 0; i < 8; i++)	{
		temp[i] = largecolonleftchar[i];
	}
	lcdfront_setCustomChar(largecolonleftaddr, temp);

	for(i = 0; i < 8; i++)	{
		temp[i] = largecolonrightchar[i];
	}

	lcdfront_setCustomChar(largecolonrightaddr, temp);

	for(i = 0; i < 8; i++)	{
		temp[i] = upchar[i];
	}
	lcdfront_setCustomChar(upcharaddr, temp);

	for(i = 0; i < 8; i++)	{
		temp[i] = downchar[i];
	}
	lcdfront_setCustomChar(downcharaddr, temp);
}


void lcdfront_bigdigit(uint8_t index, uint8_t d) {
	if (d > 9) {
		d = 9;
	}
	lcd_command(0b10000000 + index);
	lcd_data(numchar[d][0]);
	lcd_data(numchar[d][1]);
	lcd_data(numchar[d][2]);
	lcd_command(0b11000000 + index);
	lcd_data(numchar[d][3]);
	lcd_data(numchar[d][4]);
	lcd_data(numchar[d][5]);
}

void lcdfront_bigwhitespace(uint8_t index) {
	lcd_command(0b10000000 + index);
	lcd_string("   ");
	lcd_command(0b11000000 + index);
	lcd_string("   ");
}

void lcdfront_time(uint8_t hour, uint8_t minute, uint8_t ampm) {
	last_display = LAST_DISPLAY_TIME;
	last_display_msticks = lcdfront_msticks;
    uint8_t a[4];
    uint8_t isPm = 0;

	// handle 24/12 hour modes
	if (hour == 0)
	{
		hour = 24;
	}

    if (hour >= 12)
    {
        isPm = 1;
    }

	if (ampm != 0)
	{
		hour %= 12;
		if(hour == 0)
		{
			hour = 12;
		}
	}

	// get individual digits
	a[0] = hour / 10;
	a[1] = hour % 10;
	a[2] = minute / 10;
	a[3] = minute % 10;

	// print characters
	uint8_t i;
	for(i = 0; i < 4; i++)	{
		// handle spacing
		uint8_t j;
		if(i > 1)
		{
			j = 1;
		}
		else
		{
			j = 0;
		}

		uint8_t d = a[i];

		uint8_t e = (i * 4) + j;

		lcd_command(0b10000000 + e);
		lcd_data(numchar[d][0]);
		lcd_data(numchar[d][1]);
		lcd_data(numchar[d][2]);
		lcd_command(0b11000000 + e);
		lcd_data(numchar[d][3]);
		lcd_data(numchar[d][4]);
		lcd_data(numchar[d][5]);
	}

	// print colon and spacing
	if (ampm == 0)
	{
        lcd_command(0b10000000 + 4 + 3);
        lcd_data(largecolonleftaddr);
        lcd_command(0b10000000 + 8);
        lcd_data(largecolonrightaddr);

        lcd_command(0b11000000 + 4 + 3);
        lcd_data(largecolonleftaddr);
        lcd_command(0b11000000 + 8);
        lcd_data(largecolonrightaddr);
	}
	else if ((hour >= 0 && hour <= 11) || (hour == 24 && isPm == 0)) 	{
        lcd_command(0b10000000 + 4 + 3);
        lcd_data('A');
        lcd_command(0b10000000 + 8);
        lcd_data('M');

        lcd_command(0b11000000 + 4 + 3);
        lcd_data(largecolonleftaddr);
        lcd_command(0b11000000 + 8);
        lcd_data(largecolonrightaddr);
	}
	else if (isPm != 0)
	{
        lcd_command(0b10000000 + 4 + 3);
        lcd_data(largecolonleftaddr);
        lcd_command(0b10000000 + 8);
        lcd_data(largecolonrightaddr);

        lcd_command(0b11000000 + 4 + 3);
        lcd_data('P');
        lcd_command(0b11000000 + 8);
        lcd_data('M');
	}

	lcd_command(0b10000000 + 3);
	lcd_data(' ');
	lcd_command(0b11000000 + 3);
	lcd_data(' ');
	lcd_command(0b10000000 + 12);
	lcd_data(' ');
	lcd_command(0b11000000 + 12);
	lcd_data(' ');
}

void lcdfront_current_max_value(uint16_t current_value, uint16_t max_value, char* title) {
    if (current_value > 999) {
    	current_value = 999;
    }

    if (max_value > 999) {
    	max_value = 299;
    }

	// get individual digits
    uint8_t a[3];
	a[0] = current_value / 100;
	a[1] = (current_value % 100) / 10;
	a[2] = (current_value % 100) % 10;

	if (a[0] == 0) {
		lcdfront_bigwhitespace(0);
	}
	else {
		lcdfront_bigdigit(0, a[0]);
	}
	if (a[0] == 0 && a[1] == 0) {
		lcdfront_bigwhitespace(4);
	}
	else {
		lcdfront_bigdigit(4, a[1]);
	}
	lcdfront_bigdigit(8, a[2]);

	// print colon and spacing
	lcd_command(0b10000000 + 4 + 3);
	lcd_data(' ');
	lcd_command(0b11000000 + 4 + 3);
	lcd_data(' ');

	lcd_command(0b10000000 + 3);
	lcd_data(' ');
	lcd_command(0b11000000 + 3);
	lcd_data(' ');
	lcd_command(0b10000000 + 12);
	lcd_data(' ');
	lcd_command(0b11000000 + 12);
	lcd_data(' ');

	lcd_command(0b10000000 + 11);
	lcd_data(' ');
	lcd_data(' ');
	lcd_string(title);

	lcd_command(0b11000000 + 11);
	lcd_data(' ');
	lcd_data(' ');


	// get individual digits
	a[0] = max_value / 100;
	a[1] = (max_value % 100) / 10;
	a[2] = (max_value % 100) % 10;

	if (a[0] == 0) {
		lcd_data(' ');
	}
	else {
		lcd_data('0' + a[0]);
	}
	if (a[0] == 0 && a[1] == 0) {
		lcd_data(' ');
	}
	else {
		lcd_data('0' + a[1]);
	}
	lcd_data('0' + a[2]);

}

void lcdfront_trackinfo(uint8_t current_track, uint8_t max_track) {
	last_display = LAST_DISPLAY_TRACKINFO;
	last_display_msticks = lcdfront_msticks;
	track_current = current_track;
	track_max = max_track;
	lcdfront_current_max_value(track_current, track_max, "TRK");
}


void lcdfront_volume(uint8_t current_volume, uint8_t max_volume)  {
	last_display = LAST_DISPLAY_VOLUME;
	last_display_msticks = lcdfront_msticks;
	vol_current = current_volume;
	vol_max = max_volume;
	lcdfront_current_max_value(vol_current, vol_max, "VOL");
}

void lcdfront_init() {
	lcd_init();
	lcd_clear();
	lcdfront_setAllCustomCharcaters();
	lcdfront_setcursor(0, 0);
}

void lcdfront_clear() {
	lcd_clear();
}

void lcdfront_setcursor(uint8_t x, uint8_t y) {
	lcd_setcursor(x, y);
}

void lcdfront_string(const char *data) {
	lcd_string(data);
}

void lcdfront_process(uint32_t ms_ticks) {
	lcdfront_msticks = ms_ticks;

	if (last_display != LAST_DISPLAY_TRACKINFO && math_calc_diff(ms_ticks, last_display_msticks) > DISPLAY_TIME_OUT) {
		lcdfront_trackinfo(track_current, track_max);
	}

	if (flag_blink_mode && math_calc_diff(ms_ticks, last_blink_action) > 50) {
		if (flag_current_display_mode) {
			lcdfront_set_display(0);
		}
		else {
			lcdfront_set_display(1);
		}
		last_blink_action = ms_ticks;
	}

}

void lcdfront_set_blink(uint8_t flag) {
	if (flag) {
		flag_blink_mode = 1;
	}
	else {
		flag_blink_mode = 0;
		lcdfront_set_display(1);
	}
}

void lcdfront_set_display(uint8_t flag) {
	if (flag) {
		lcd_display_on();
		flag_current_display_mode = 1;
	}
	else {
		lcd_display_off();
		flag_current_display_mode = 0;
	}
}
