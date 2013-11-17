#include <stdlib.h>
#include "logger.h"
#include "queue.h"

uint8_t logger_enabled = 1;
uint8_t logger_buffer_data[LOGGER_BUFFER_SIZE];
ringbuffer_t logger_rbuffer = {.buffer=logger_buffer_data, .head=0, .tail=0, .count=0, .size=LOGGER_BUFFER_SIZE};


/**
 * expected zero terminated string
 */
void logger_logString(char* data) {
	while(!queue_isFull(&logger_rbuffer) && *data) {
		logger_logByte(*data++);
	}
}

void logger_logStringln(char* data) {
	logger_logString(data);
	logger_logCRLF();
}
void logger_logNumber(uint32_t value) {
	char buf[10];
	sprintf(buf, "%d", value);
	// not supported by stm32 itoa(value, buf, 10);
	logger_logString((char*) buf);
}

void logger_logNumberln(uint32_t value) {
	logger_logNumber(value);
	logger_logCRLF();
}

void logger_logCRLF() {
	logger_logByte(13);
	logger_logByte(10);
}

void logger_logByte(uint8_t data) {
	if (logger_enabled) {
		queue_put(&logger_rbuffer, data);
	}
}

void logger_setEnabled(uint8_t enabled) {
	logger_enabled = enabled;
}

uint8_t logger_dataAvailable() {
	return queue_dataAvailable(&logger_rbuffer);
}

uint8_t logger_read() {
	return queue_read(&logger_rbuffer);
}

