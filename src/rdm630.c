#include <stdint.h>
#include "rdm630.h"
#include "math_utils.h"

uint32_t rdm630_last_received_msticks = 0;

uint8_t rdm630_buffer[RDM630_BUFFER_SIZE];
uint8_t rdm630_buffer_count = 0;
uint8_t rdm630_status = RDM630_STATUS_DISABLED;
uint32_t rdm630_rfid = 0;


void rdm630_clear() {
    uint8_t i;
    for(i = 0; i < RDM630_BUFFER_SIZE; i++) {
    	rdm630_buffer[i] = 0;
    }
    rdm630_buffer_count = 0;
}


void rdm630_init() {
	rdm630_clear();
}

void rdm630_enable() {
	rdm630_clear();
	rdm630_status = RDM630_STATUS_WAIT_FOR_START;
}

void rdm630_disable() {
	rdm630_status = RDM630_STATUS_DISABLED;
}

void rdm630_reset() {
	rdm630_clear();
	rdm630_status = RDM630_STATUS_WAIT_FOR_START;
}

uint8_t rdm630_data_available() {
	return rdm630_status == RDM630_STATUS_FINISHED;
}

uint8_t char_to_hex(uint8_t cdata) {
	switch(cdata) {
	case '0' : return 0x00;
	case '1' : return 0x01;
	case '2' : return 0x02;
	case '3' : return 0x03;
	case '4' : return 0x04;
	case '5' : return 0x05;
	case '6' : return 0x06;
	case '7' : return 0x07;
	case '8' : return 0x08;
	case '9' : return 0x09;
	case 'A' : return 0x0A;
	case 'B' : return 0x0B;
	case 'C' : return 0x0C;
	case 'D' : return 0x0D;
	case 'E' : return 0x0E;
	case 'F' : return 0x0F;
	case 'a' : return 0x0A;
	case 'b' : return 0x0B;
	case 'c' : return 0x0C;
	case 'd' : return 0x0D;
	case 'e' : return 0x0E;
	case 'f' : return 0x0F;
	}
	return 0x00;
}

uint32_t rdm630_read_rfid_id() {
	return rdm630_rfid;
}

void rdm630_process(uint32_t msticks) {
}

void rdm630_process_serial_data(uint8_t data) {
	if (rdm630_status == RDM630_STATUS_WAIT_FOR_START) {
		if (data == RDM630_TRIGGER_START) {
			rdm630_status = RDM630_STATUS_WAIT_FOR_STOP;
		}
	}
	else if (rdm630_status == RDM630_STATUS_WAIT_FOR_STOP) {
		if (data == RDM630_TRIGGER_STOP) {
			// parse data here
			uint8_t i;
			rdm630_rfid = 0;
			for(i = 2; i < 10; i++) {
				uint8_t hdata = char_to_hex(rdm630_buffer[i]);
				rdm630_rfid |= hdata;
				if (i < 9) {
					rdm630_rfid = rdm630_rfid << 4;
				}
			}
			if (rdm630_rfid == 0) {
				// invalid rfid id
				rdm630_reset();
			}
			else {
				rdm630_status = RDM630_STATUS_FINISHED;
			}
		}
		else {
			if (rdm630_buffer_count < RDM630_BUFFER_SIZE) {
				rdm630_buffer[rdm630_buffer_count++] = data;
			}
		}
	}
}
