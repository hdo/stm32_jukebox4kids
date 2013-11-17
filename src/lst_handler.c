#include <stdint.h>
#include <stdio.h>
#include "lst_handler.h"

#define PADDING 5

uint8_t* buffer;
uint16_t length;
uint16_t track_count = 0;

void lst_init(uint8_t* buf, uint16_t len) {
	buffer = buf;
	length = len;
	uint16_t i = 0;
	uint16_t last = 0;
	track_count = 0;
	uint8_t* pdata = buf;
	for(i=0; i < len; i++) {
		uint8_t ch = *pdata++;
		if (ch == 0x0A && i > (last+PADDING) && i < (length-PADDING)) {
			track_count++;
			last = i;
		}
	}
	if (last < length-PADDING) {
		track_count++;
	}
}

uint16_t lst_get_trackcount() {
	return track_count;
}

void lst_get_item_at(uint16_t index, char* str, uint8_t max_length) {
	uint16_t i = 0;
	uint16_t last = 0;
	uint16_t current_index = 0;
	uint8_t* pdata = buffer;
	uint16_t pos = 0;
	char* tempdata = str;
	for(i=0; i < max_length; i++) {
		*tempdata++ = 0; // initialising output buffer
	}
	for (i = 0; i < length && current_index < index; i++) {
		char ch = (char) *pdata++;
		if (ch == 0x0A && i > (last + PADDING) && i < (length - PADDING)) {
			current_index++;
			pos = i + 1;
			last = i;
		}
	}
	if (current_index == index) {
		//printf("index: %d\n", index);
		//printf("pos: %d\n", pos);
		uint8_t* pdata = buffer;
		pdata += pos;
		uint16_t i=0;
		uint16_t sentinel = 0;
		for(i = pos; i < length && sentinel < max_length; i++) {
			if (*pdata == 0x0A || *pdata == 0x0D || *pdata == 0x00) {
				break;
			}
			*str++ = (char) *pdata++;
			sentinel++;
		}
	}
}
