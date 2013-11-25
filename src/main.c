#include <stdio.h>
#include <string.h>
#include "stm32.h"
#include "stm32f4xx_conf.h"
#include "main.h"
#include "misc.h"
#include "uart.h"
#include "delay.h"
#include "vs1033.h"
#include "lst_handler.h"
#include "math_utils.h"
#include "rotary.h"
#include "buttons.h"
#include "version.h"
#include "rdm630.h"
#include "lcdfront.h"
#include "relay.h"
#include "ff.h"
#include "audio.h"

#define PERFORM_VS_HEALTH_CHECK 0
#define PERFORM_VS_SOFT_RESET_ON_TRACK_CHANGE 0 // for vs1003
#define PERFORM_VS_SOFT_RESET_ON_PLAYLIST_CHANGE 0
#define PERFORM_VS_HARD_RESET_ON_PLAYLIST_CHANGE 0

#define LST_BUFFER_SIZE 16000

#define USE_UART_PORT_NUM 3
#define MAX_PATH_LENGTH 200

#define WAIT_TIME_CHANGE_TRACK 25

extern volatile uint32_t UART1Count, UART2Count, UART3Count;
extern volatile uint8_t UART1Buffer[UART_BUFSIZE], UART2Buffer[UART_BUFSIZE], UART3Buffer[UART_BUFSIZE];
volatile uint32_t msTicks; // counter for 10ms SysTicks


// USB BEGIN
USB_OTG_CORE_HANDLE		USB_OTG_Core;
USBH_HOST				USB_Host;
volatile int			enum_done = 0;
// USB END



uint32_t current_playlist, next_playlist = 0;
uint16_t current_track, next_track = 0; // 1 based
uint32_t last_track_change = 0;
uint32_t last_auto_track_change = 0;
uint32_t last_check_volume = 0;
uint32_t last_vs_health_check = 0;

uint8_t path[MAX_PATH_LENGTH];
char current_playlist_name[MAX_PATH_LENGTH];
uint8_t lst_buffer[LST_BUFFER_SIZE];



//  SysTick_Handler - just increment SysTick counter
void SysTick_Handler(void) {
  msTicks++;
}

// systick_delay - creates a delay of the appropriate number of Systicks (happens every 10 ms)
__INLINE static void systick_delay (uint32_t delayTicks) {
  uint32_t currentTicks;

  currentTicks = msTicks;	// read current tick counter
  // Now loop until required number of ticks passes.
  while ((msTicks - currentTicks) < delayTicks);
}

void Delay(__IO uint32_t nCount) {
	while (nCount--) {
	}
}

void command_next_track() {
	if (next_track == 0) {
		next_track = current_track;
	}
	if (next_track < lst_get_trackcount()) {
		next_track++;
		uart_sendString(USE_UART_PORT_NUM, "track selected: ");
		uart_sendNumberln(USE_UART_PORT_NUM, next_track);
		last_track_change = msTicks;
		lcdfront_trackinfo(next_track, lst_get_trackcount());

	}
	else {
		uart_sendStringln(USE_UART_PORT_NUM, "WARNING: track out of bounds ");
	}
}

void command_prev_track() {
	if (next_track == 0) {
		next_track = current_track;
	}
	if (next_track > 1) {
		next_track--;
		uart_sendString(USE_UART_PORT_NUM, "track selected: ");
		uart_sendNumberln(USE_UART_PORT_NUM, next_track);
		last_track_change = msTicks;
		lcdfront_trackinfo(next_track, lst_get_trackcount());
	}
	else {
		uart_sendStringln(USE_UART_PORT_NUM, "WARNING: track out of bounds ");
	}
}

void command_toggle_play() {
	if (audio_get_status() == AUDIO_STATUS_PLAYING) {
		audio_pause();
		uart_sendStringln(USE_UART_PORT_NUM, "pause");
		lcdfront_set_blink(1);
	} else 	if (audio_get_status() == AUDIO_STATUS_PAUSED) {
		audio_resume();
		uart_sendStringln(USE_UART_PORT_NUM, "play");
		lcdfront_set_blink(0);
	} else if (audio_get_status() == AUDIO_STATUS_STOPPED) {
		lcdfront_set_blink(0);
		if (current_track == 0 && lst_get_trackcount() > 0) {
			uart_sendStringln(USE_UART_PORT_NUM, "start from beginning");
			next_track = 1;
			lcdfront_trackinfo(next_track, lst_get_trackcount());
		}
	}
}

void command_toggle_on_off() {
}

void command_vol_up() {
	rotary_set_value(rotary_get_value()+5);
}

void command_vol_down() {
	// check for underflow
	if (rotary_get_value() >= 5) {
		rotary_set_value(rotary_get_value()-5);
	}
}

uint8_t file_exists(char* name) {
	uart_sendString(USE_UART_PORT_NUM, "file_exists: ");
	uart_sendStringln(USE_UART_PORT_NUM, name);
	FIL testFile;
	if (f_open(&testFile, name, FA_OPEN_EXISTING | FA_READ) == FR_OK) {
		f_close(&testFile);
		uart_sendStringln(USE_UART_PORT_NUM, "ok");
		return 1;
	}
	uart_sendStringln(USE_UART_PORT_NUM, "failed");
	return 0;
}

void load_playlist(char* lstName) {
	if (strlen(lstName) > 0) {

		// if already loaded
		if (strcmp(lstName, current_playlist_name) == 0) {
			uart_sendString(USE_UART_PORT_NUM, "playlist already loaded!");
			return;
		}

		uart_sendString(USE_UART_PORT_NUM, "opening lst file: ");
		uart_sendStringln(USE_UART_PORT_NUM, lstName);

		FIL lstFile;
		if (f_open(&lstFile, lstName, FA_OPEN_EXISTING | FA_READ) == FR_OK) {
			uart_sendStringln(USE_UART_PORT_NUM, "lst file successfully opened");
			uint32_t current_read = 0;
			uint16_t total_read = 0;
			do {
				uint8_t *bpointer = lst_buffer + total_read;

				f_read(&lstFile, bpointer, 512, (UINT*) &current_read);
				total_read += current_read;
			} while(current_read > 0);

			f_close(&lstFile);

			uart_sendString(USE_UART_PORT_NUM, "bytes read: ");
			uart_sendNumberln(USE_UART_PORT_NUM, total_read);
			if (total_read > 0) {
				lst_init(lst_buffer, total_read);
				uint16_t tcount = lst_get_trackcount();
				uart_sendString(USE_UART_PORT_NUM, "Track count: ");
				uart_sendNumberln(USE_UART_PORT_NUM, tcount);
				if (tcount > 0) {

					// set current playlist name
					strcpy(current_playlist_name, lstName);

					char fname[MAX_PATH_LENGTH];
					lst_get_item_at(0, fname, MAX_PATH_LENGTH);
					if (file_exists(fname)) {
						current_playlist = next_playlist;

						current_track = 1;
						next_track = 0;

						if (PERFORM_VS_SOFT_RESET_ON_PLAYLIST_CHANGE) {
							vs_reset(SOFT_RESET);
						}
						audio_stop();
						audio_play_file(fname);

						lcdfront_trackinfo(current_track, tcount);
					}
				}
			}
			else {
				uart_sendStringln(USE_UART_PORT_NUM, "ERROR: empty lst file!");
			}
		}
		else {
			// error loading playlist file
			uart_sendString(USE_UART_PORT_NUM, "ERROR: file not found!");
		}
	}
	else {
		uart_sendStringln(USE_UART_PORT_NUM, "ERROR: lstName == 0 ?");
	}
}

void load_playlist_for_rfid(uint32_t value) {
	char name[MAX_PATH_LENGTH];
	strcpy(name, "/index/rfid/");
	char temp_str[12];
	math_itoa(value, temp_str);
	strcat(name, temp_str);
	strcat(name, ".lst");
	load_playlist(name);
}

void load_playlist_for_albumid(uint32_t value) {
	char name[MAX_PATH_LENGTH];
	strcpy(name, "/index/albumid/");
	char temp_str[12];
	math_itoa(value, temp_str);
	strcat(name, temp_str);
	strcat(name, ".lst");
	load_playlist(name);
}

void load_playlist_for_uartid(uint32_t value) {
	char name[MAX_PATH_LENGTH];
	strcpy(name, "/index/uartid/");
	char temp_str[12];
	math_itoa(value, temp_str);
	strcat(name, temp_str);
	strcat(name, ".lst");
	load_playlist(name);
}


int main(void) {

	SystemInit(); // Quarz Einstellungen aktivieren
	SystemCoreClockUpdate();

	// SysTick config
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);

	// Setup SysTick Timer to interrupt at 10 msec intervals
	if (SysTick_Config(SystemCoreClock / 100)) {
		while (1)
			;  // Capture error
	}

	delay_init();

	/* GPIOD Periph clock enable */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;

	/* Configure PD12, 13, 14 and PD15 in output pushpull mode */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14
			| GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	GPIOD->BSRRL = GPIO_Pin_14; // set red
	GPIOD->BSRRH = GPIO_Pin_12; // clear green

	rotary_init();
	rotary_set_boundary(0, 150);
	rotary_set_value(audio_get_volume());

	buttons_init();
	relay_init();

	rdm630_init();
	rdm630_reset();

	uint32_t stopwatch_a = msTicks;
	uint32_t interval = 0;

	uart_init(USE_UART_PORT_NUM, 115200);
	uart_init(1, 9600);

	uart_sendStringln(USE_UART_PORT_NUM, "********************************************");
	uart_sendStringln(USE_UART_PORT_NUM, version_get_product_name());
	uart_sendString(USE_UART_PORT_NUM, "BUILD: ");
	uart_sendStringln(USE_UART_PORT_NUM, version_get_build_id());

	uart_sendString(USE_UART_PORT_NUM, "SystemCoreCLock: ");
	uart_sendNumberln(USE_UART_PORT_NUM, SystemCoreClock);
	uart_sendString(USE_UART_PORT_NUM, "HSE_VALUE: ");
	uart_sendNumberln(USE_UART_PORT_NUM, HSE_VALUE);
	uart_sendStringln(USE_UART_PORT_NUM, "********************************************");

	uart_sendStringln(USE_UART_PORT_NUM, "initializing usb host ...");

	// Initialize USB Host Library
	USBH_Init(&USB_OTG_Core, USB_OTG_FS_CORE_ID, &USB_Host, &USBH_MSC_cb, &USR_Callbacks);
	uart_sendStringln(USE_UART_PORT_NUM, "done ...");

	lcdfront_init();
	lcdfront_string(version_get_product_name());
	lcdfront_setcursor(0, 1);
	lcdfront_string(version_get_build_id());

    uart_sendStringln(USE_UART_PORT_NUM, "initialising audio ...");
    audio_init();
    uart_sendString(USE_UART_PORT_NUM, "done: ");
    uart_sendNumber(USE_UART_PORT_NUM, (msTicks - stopwatch_a));
    uart_sendStringln(USE_UART_PORT_NUM, "0ms");


	uint32_t stopwatch_b = msTicks;
	interval = stopwatch_b - stopwatch_a;
	uint8_t volume = audio_get_volume();

	uart_sendString(USE_UART_PORT_NUM, "initialisation took ");
	uart_sendNumber(USE_UART_PORT_NUM, interval);
	uart_sendStringln(USE_UART_PORT_NUM, "0 ms");
	uart_sendStringln(USE_UART_PORT_NUM, "ready ...");

	uart_sendStringln(USE_UART_PORT_NUM, "waiting for usb ...");
	// waiting for usb
	// enum is set in usbh_usr.c
	// USBH_USR_DeviceDisconnected
	// USBH_USR_MSC_Application
	while(enum_done < 2) {
		USBH_Process(&USB_OTG_Core, &USB_Host);
	}


	interval = msTicks - stopwatch_a;
	uart_sendString(USE_UART_PORT_NUM, "mounting usb took ");
	uart_sendNumber(USE_UART_PORT_NUM, interval);
	uart_sendStringln(USE_UART_PORT_NUM, "0 ms");

    uart_sendStringln(USE_UART_PORT_NUM, "activating amp ...");

	relay_set(0);
	relay_reset(1);

	load_playlist("/index/boot.lst");
	audio_pause();
	lcdfront_trackinfo(current_track, lst_get_trackcount());

	GPIOD->BSRRL = GPIO_Pin_12; // set green
	GPIOD->BSRRH = GPIO_Pin_14; // clear red

	while (1) {

		/* the following line causes f_read to read 0 bytes */
		//USBH_Process(&USB_OTG_Core, &USB_Host);

		audio_process(msTicks);
		buttons_process(msTicks);
		lcdfront_process(msTicks);

		uint8_t audio_status = audio_get_status();

		// auto next track check (check only every 500ms)
		if (audio_status == AUDIO_STATUS_FINISHED && math_calc_diff(msTicks, last_auto_track_change) > 50) {
			if (current_track < lst_get_trackcount()) {
				last_auto_track_change = msTicks;
				command_next_track();
			}
			else {
				// reached end of playlist
				audio_stop();
				current_track = 0;
				next_track = 0;
				lcdfront_trackinfo(current_track, lst_get_trackcount());
			}
		}

		if (buttons_triggered(BUTTON_ON_OFF)) {
			uart_sendStringln(USE_UART_PORT_NUM, "on/off");
		}

		if (buttons_triggered(BUTTON_PLAY_PAUSE)) {
			uart_sendStringln(USE_UART_PORT_NUM, "play/pause");
			command_toggle_play();
		}

		if (buttons_triggered(BUTTON_PREV)) {
			uart_sendStringln(USE_UART_PORT_NUM, "prev track");
			command_prev_track();
		}

		if (buttons_triggered(BUTTON_NEXT)) {
			uart_sendStringln(USE_UART_PORT_NUM, "next track");
			command_next_track();
		}

		if (buttons_triggered(BUTTON_VOLUME)) {
			uart_sendStringln(USE_UART_PORT_NUM, "volume button");
		}

		if (next_track != 0 && next_track != current_track && math_calc_diff(msTicks, last_track_change) > WAIT_TIME_CHANGE_TRACK) {
			char fname[MAX_PATH_LENGTH];
			// -1 due next_track and current_track is 1 based
			lst_get_item_at(next_track - 1, fname, MAX_PATH_LENGTH);
			uart_sendString(USE_UART_PORT_NUM, "loading file: ");
			uart_sendStringln(USE_UART_PORT_NUM, fname);

			if (file_exists(fname)) {
				audio_stop();
				audio_play_file(fname);
				current_track = next_track;
			}
			next_track = 0;
		}

		// check vs health every 1 second
		if (PERFORM_VS_HEALTH_CHECK && math_calc_diff(msTicks, last_vs_health_check) > 100) {
			last_vs_health_check = msTicks;
			uart_sendStringln(USE_UART_PORT_NUM, "vs health check");
			vs_health_check();
		}

		// check volume settings every x0ms
		if (math_calc_diff(msTicks, last_check_volume) > 10) {
			last_check_volume = msTicks;
			if (volume != rotary_get_value()) {
				volume = rotary_get_value();
				uart_sendString(USE_UART_PORT_NUM, "set volume: ");
				uart_sendNumberln(USE_UART_PORT_NUM, volume);
				vs_set_simple_volume(volume);
				lcdfront_volume(volume, 150);
			}
		}

		if (rdm630_data_available()) {
			uart_sendNumberln(USE_UART_PORT_NUM, rdm630_read_rfid_id());
			load_playlist_for_rfid(rdm630_read_rfid_id());
			rdm630_reset();
		}

		if (UART3Count > 0) {
			if (UART3Buffer[0] == 'p') {
				command_toggle_play();
			}
			if (UART3Buffer[0] == '+') {
				command_vol_up();
			}
			if (UART3Buffer[0] == '-') {
				command_vol_down();
			}
			if (UART3Buffer[0] >= '1' && UART3Buffer[0] <= '9') {
				load_playlist_for_uartid(UART3Buffer[0]);
			}
			if (UART3Buffer[0] == 'n') {
				// next
				command_next_track();
			}
			if (UART3Buffer[0] == 'b') {
				// previous
				command_prev_track();
			}
			uart_sendByte(USE_UART_PORT_NUM, '[');
			uart_sendByte(USE_UART_PORT_NUM, UART3Buffer[0]);
			uart_sendByte(USE_UART_PORT_NUM, ']');
			uart_sendCRLF(USE_UART_PORT_NUM);
			UART3Count = 0;
		}

		if (UART1Count > 0) {
			uint8_t i = 0;
			for(; i < UART1Count; i++) {
				rdm630_process_serial_data(UART1Buffer[i]);
			}
			UART1Count = 0;
		}
	}
}
