#include <stdint.h>

#ifndef __LST_HANDLER_H
#define __LST_HANDLER_H

void lst_init(uint8_t* buffer, uint16_t length);
uint16_t lst_get_trackcount();
void lst_get_item_at(uint16_t index, char* str, uint8_t max_length);

#endif /* end __LST_HANDLER_H */
