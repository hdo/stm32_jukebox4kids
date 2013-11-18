#include <stdint.h>
#include "audio.h"
#include "vs1033.h"
#include "ff.h"
#include "stm32.h"
#include "uart.h"
#include "delay.h"

#define USE_UART_PORT_NUM 3
#define PERFORM_VS_SOFT_RESET_ON_TRACK_CHANGE 0

uint8_t audio_status = AUDIO_STATUS_ERROR;
FIL audioFile;
uint8_t mp3_buf[32];
uint32_t bytes_read = 0;
uint8_t audio_volume = VS_INIT_VOLUME;


void audio_init() {
	vs_init();
	audio_status = AUDIO_STATUS_STOPPED;
}

void audio_reset() {
	vs_reset(SOFT_RESET);
}

void audio_process(uint32_t msticks) {
	if ((audio_status == AUDIO_STATUS_PLAYING) && (IS_DREQ_HIGH)) {
		GPIOD->BSRRL = GPIO_Pin_15;

		f_read(&audioFile, mp3_buf, 32, &bytes_read);

		if (bytes_read > 0) {
			vs_send_32(mp3_buf, bytes_read);
		}
		else {
			audio_status = AUDIO_STATUS_FINISHED;
		}
		GPIOD->BSRRH = GPIO_Pin_15;
	}
}

void audio_stop() {
	if (audio_status == AUDIO_STATUS_PLAYING) {
		// send SM_CANCEL
	    uint16_t oldMode = vs_sci_read(SCI_MODE);
	    vs_sci_write(SCI_MODE, oldMode | SM_CANCEL);

	    delay_us(100);

	    oldMode = vs_sci_read(SCI_MODE);
	    if (!(oldMode & SM_CANCEL)) {
			uart_sendString(USE_UART_PORT_NUM, "SM_CANCEL error!");
	    }

	    uint16_t endFillByte = vs_read_mem16(PAR_END_FILL_BYTE);

		uint8_t i = 0;
		for(;i < 65; i++) {
			if (IS_DREQ_HIGH) {
				vs_send_32_endbyte(endFillByte);
			}
		}
		if (PERFORM_VS_SOFT_RESET_ON_TRACK_CHANGE) {
			vs_reset(SOFT_RESET);
		}
	}
	audio_status = AUDIO_STATUS_STOPPED;
}

void audio_pause() {
	if (audio_status == AUDIO_STATUS_PLAYING) {
		audio_status = AUDIO_STATUS_PAUSED;
	}
}

void audio_resume() {
	if (audio_status == AUDIO_STATUS_PAUSED) {
		audio_status = AUDIO_STATUS_PLAYING;
	}
}

void audio_play_file(char* fname) {
	if (f_open(&audioFile, fname, FA_OPEN_EXISTING | FA_READ) == FR_OK) {
		audio_status = AUDIO_STATUS_PLAYING;
	}
}

uint8_t audio_get_status() {
	return audio_status;
}

void audio_set_volume(uint8_t vol) {

}

uint8_t audio_get_volume() {
	return audio_volume;
}
