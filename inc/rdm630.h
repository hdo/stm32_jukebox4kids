#ifndef __RDM630_H
#define __RDM630_H

#define RDM630_BUFFER_SIZE 16

#define RDM630_STATUS_DISABLED 0
#define RDM630_STATUS_WAIT_FOR_START 1
#define RDM630_STATUS_WAIT_FOR_STOP 2
#define RDM630_STATUS_FINISHED 3

#define RDM630_TRIGGER_START 0x02
#define RDM630_TRIGGER_STOP 0x03

void rdm630_init();
void rdm630_enable();
void rdm630_disable();
void rdm630_reset();
uint8_t rdm630_data_available();
uint32_t rdm630_read_rfid_id();
void rdm630_process(uint32_t msticks);
void rdm630_process_serial_data(uint8_t data);

#endif /* end __RDM630_H */
/*****************************************************************************
**                            End Of File
******************************************************************************/
