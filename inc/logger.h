#include <stdint.h>

#ifndef __LOGGER_H
#define __LOGGER_H

#define LOGGER_BUFFER_SIZE 255

void logger_logString(char* data);
void logger_logStringln(char* data);
void logger_logNumber(uint32_t value);
void logger_logNumberln(uint32_t value);
void logger_logCRLF();
void logger_logByte(uint8_t data);
void logger_setEnabled(uint8_t enabled);
uint8_t logger_read();
uint8_t logger_dataAvailable();


#endif /* end __LOGGER_H */
/*****************************************************************************
**                            End Of File
******************************************************************************/
