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

#define PERFORM_VS_HEALTH_CHECK 0
#define PERFORM_VS_SOFT_RESET_ON_TRACK_CHANGE 0 // for vs1003
#define PERFORM_VS_SOFT_RESET_ON_PLAYLIST_CHANGE 0
#define PERFORM_VS_HARD_RESET_ON_PLAYLIST_CHANGE 0

#define LST_BUFFER_SIZE 16000

#define USE_UART_PORT_NUM 3
#define MODE_STOP 1
#define MODE_PLAY 2

#define FILE_OK 1
#define FILE_FAIL 2
#define MAX_PATH_LENGTH 200

#define WAIT_TIME_CHANGE_TRACK 20

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

uint8_t lst_buffer[LST_BUFFER_SIZE];
uint8_t path[MAX_PATH_LENGTH];
char lstName[MAX_PATH_LENGTH];
uint8_t play_mode = MODE_STOP;


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

void get_playlist_file_for(uint32_t lst_id, char* name) {
	strcpy(name, "/index/");
	if (lst_id == 1) {
		strcat(name, "6083044.lst");
	}
	else if (lst_id == 2) {
		strcat(name, "6132551.lst");
	}
	else if (lst_id == 3) {
		strcat(name, "6165473.lst");
	}
	else {
		char temp_str[12];
		math_itoa(lst_id, temp_str);
		strcat(name, temp_str);
		strcat(name, ".lst");
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
		play_mode = MODE_PLAY;
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
		play_mode = MODE_PLAY;
		lcdfront_trackinfo(next_track, lst_get_trackcount());
	}
	else {
		uart_sendStringln(USE_UART_PORT_NUM, "WARNING: track out of bounds ");
	}
}

void command_toggle_play() {
	if (play_mode == MODE_PLAY) {
		play_mode = MODE_STOP;
		uart_sendStringln(USE_UART_PORT_NUM, "stop");
	} else 	if (play_mode == MODE_STOP) {
		play_mode = MODE_PLAY;
		uart_sendStringln(USE_UART_PORT_NUM, "play");
	}
}

void command_toggle_on_off() {
	if (play_mode == MODE_PLAY) {
		play_mode = MODE_STOP;
	}
}

void command_vol_up() {
	// check for underflow
	if (rotary_get_value() >= 5) {
		rotary_set_value(rotary_get_value()-5);
	}
}

void command_vol_down() {
	rotary_set_value(rotary_get_value()+5);
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

int main(void) {

	uint8_t file_status = FILE_FAIL;
	uint8_t decoder_status = VS_STATUS_FAIL;

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
	rotary_set_value(VS_INIT_VOLUME);

	buttons_init();
	relay_init();
	relay_set(0);

	rdm630_init();
	rdm630_reset();

	uint32_t stopwatch_a = msTicks;
	uint32_t interval = 0;

	FIL audioFile;   // Filehandler


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


	/*

	lcdfront_init();
	lcdfront_string(version_get_product_name());
	lcdfront_setcursor(0, 1);
	lcdfront_string(version_get_build_id());
	delay_ms(1000);


    uart_sendStringln(USE_UART_PORT_NUM, "initialising vs1033!");
	vs_init();
	decoder_status = vs_get_status();
    uart_sendString(USE_UART_PORT_NUM, "done: ");
    uart_sendNumber(USE_UART_PORT_NUM, (msTicks - stopwatch_a));
    uart_sendStringln(USE_UART_PORT_NUM, "0ms");

*/

	uint8_t mp3_buf[32];
	uint32_t bytes_read = 0;
	uint32_t stopwatch_b = msTicks;
	interval = stopwatch_b - stopwatch_a;
	uint8_t volume = VS_INIT_VOLUME;

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


	// File zum schreiben im root neu anlegen
	uart_sendStringln(USE_UART_PORT_NUM, "opening psy.mp3 ...");

	if (FR_OK == f_open(&audioFile, "0:/psy.mp3", FA_OPEN_EXISTING | FA_READ)) {
		uart_sendStringln(USE_UART_PORT_NUM, "file successfully opened");
		file_status = FILE_OK;
		interval = msTicks - stopwatch_a;
		uart_sendString(USE_UART_PORT_NUM, "opening took ");
		uart_sendNumber(USE_UART_PORT_NUM, interval);
		uart_sendStringln(USE_UART_PORT_NUM, "0 ms");
		current_track = 1;
	}
	else {
		uart_sendStringln(USE_UART_PORT_NUM, "error opening file!");
	}

	file_exists("0:/media/001. Avicii - Wake Me Up.mp3");
	file_exists("0:/media/001. Avicii - Wake Me Up2.mp3");
	file_exists("/media/Wieso Weshalb Warum/30-Alles über den Zirkus/05 - Wie kommt der Zirkus in die Stadt und wie wird ein Zirkuszelt aufgebaut.mp3");
	file_exists("/media/Wieso Weshalb Warum/30-Alles über den Zirkus/29 - Ich bin ganz Ohr (instrumental).mp3");
	file_exists("/media/Detlev Joecker/Sei gegrüßt lieber Nikolaus [2001]/64 - Was kann in diesen Tagen.mp3	");

	GPIOD->BSRRL = GPIO_Pin_12; // set green
	GPIOD->BSRRH = GPIO_Pin_14; // clear red


	while (1) {


		USBH_Process(&USB_OTG_Core, &USB_Host);


		/*
		if ((play_mode == MODE_PLAY) && (file_status == FILE_OK) && (decoder_status == VS_STATUS_OK) && (IS_DREQ_HIGH)) {
			GPIOD->BSRRL = GPIO_Pin_15;

			f_read(&audioFile, mp3_buf, 32, (UINT*) &bytes_read);

			//f_read(&file, file_read_buffer, FILE_READ_BUFFER_SIZE, &br);

			if (bytes_read > 0) {
				vs_send_32(mp3_buf, bytes_read);
			}
			GPIOD->BSRRH = GPIO_Pin_15;
			// aussume file has end, so play next song
			if (bytes_read == 0 && math_calc_diff(msTicks, last_auto_track_change) > 30) {
				// lock auto track change for 300ms
				last_auto_track_change = msTicks;
				command_next_track();
			}
		}

		buttons_process(msTicks);
		lcdfront_process(msTicks);

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


		if (next_playlist != 0) {
			if (next_playlist == current_playlist) {
				// playlist already loaded
				uart_sendStringln(USE_UART_PORT_NUM, "WARNING: playlist already loaded");
			}
			else {
				FIL lstFile;
				uint8_t i=0;
				for(;i < MAX_PATH_LENGTH; i++) {
					lstName[i] = 0;
				}
				uart_sendString(USE_UART_PORT_NUM, "selected playlist id: ");
				uart_sendNumberln(USE_UART_PORT_NUM, next_playlist);
				get_playlist_file_for(next_playlist, lstName);
				if (strlen(lstName) > 0) {
					uart_sendString(USE_UART_PORT_NUM, "opening lst file: ");
					uart_sendStringln(USE_UART_PORT_NUM, lstName);


					if (f_open(&lstFile, lstName, FA_OPEN_EXISTING | FA_READ) == FR_OK) {
						//uart_sendStringln(USE_UART_PORT_NUM, "file size: ");
						//uart_sendNumberln(USE_UART_PORT_NUM, UB_Fatfs_FileSize(&lstFile));
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
								char fname[MAX_PATH_LENGTH];
								lst_get_item_at(0, fname, MAX_PATH_LENGTH);




								if (f_open(&lstFile, fname, FA_OPEN_EXISTING | FA_READ) == FR_OK) {
									f_close(&lstFile);

									current_playlist = next_playlist;

									// force track change
									current_track = 0;
									next_track = 1;

									play_mode = MODE_PLAY;

									lcdfront_trackinfo(next_track, tcount);
								}
							}
						}
						else {
							uart_sendStringln(USE_UART_PORT_NUM, "ERROR: empty lst file!");
						}
					}
					else {
						// error loading playlist file
						uart_sendStringln(USE_UART_PORT_NUM, "ERROR: invalid playlist id?");
						// handle error
					}
				}
				else {
					uart_sendStringln(USE_UART_PORT_NUM, "ERROR: lstName == 0 ?");
				}
			}
			next_playlist = 0;
		}

		if (next_track != 0 && math_calc_diff(msTicks, last_track_change) > WAIT_TIME_CHANGE_TRACK) {
			if (next_track == current_track) {
				uart_sendStringln(USE_UART_PORT_NUM, "WARNING: track is already playing");
			}
			else {
				char fname[MAX_PATH_LENGTH];
				// -1 due next_track and current_track is 1 based
				lst_get_item_at(next_track - 1, fname, MAX_PATH_LENGTH);
				uart_sendString(USE_UART_PORT_NUM, "loading file: ");
				uart_sendStringln(USE_UART_PORT_NUM, fname);
				FIL tstFile;

				if (f_open(&tstFile, fname, FA_OPEN_EXISTING | FA_READ) == FR_OK) {
					f_close(&tstFile);

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
					// close previous file
					f_close(&audioFile);
					// open new file
					f_open(&audioFile, fname, FA_OPEN_EXISTING | FA_READ);

					current_track = next_track;
				}
			}
			next_track = 0;
		}

		// check vs health every 1 second
		if (PERFORM_VS_HEALTH_CHECK && math_calc_diff(msTicks, last_vs_health_check) > 100) {
			last_vs_health_check = msTicks;
			uart_sendStringln(USE_UART_PORT_NUM, "vs health check");
			vs_health_check();
		}

		// check volume settings every 200ms
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
			next_playlist = rdm630_read_rfid_id();
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
			if (UART3Buffer[0] == '1') {
				next_playlist = 1;
			}
			if (UART3Buffer[0] == '2') {
				next_playlist = 2;
			}
			if (UART3Buffer[0] == '3') {
				next_playlist = 3;
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
		*/
	}
}
