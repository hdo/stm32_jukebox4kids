#include <stdint.h>

#ifndef __LCDFRONT_H
#define __LCDFRONT_H

void lcdfront_init();
void lcdfront_clear();
void lcdfront_setcursor(uint8_t x, uint8_t y);
void lcdfront_string(const char *data);
void lcdfront_time(uint8_t hour, uint8_t minute, uint8_t ampm);
void lcdfront_trackinfo(uint8_t current_track, uint8_t max_track);
void lcdfront_volume(uint8_t current_volume, uint8_t max_volume);
void lcdfront_process(uint32_t ms_ticks);
void lcdfront_set_blink(uint8_t flag);
void lcdfront_set_display(uint8_t flag);

#endif /* end __LCDFRONT_H */
