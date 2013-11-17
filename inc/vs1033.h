#include <stdint.h>

#ifndef __VS1033_H
#define __VS1033_H

#define VS_FAILED_READ 0xFFFF
#define VS_STATUS_OK 1
#define VS_STATUS_FAIL 2
#define VS_END_FILL_BYTE_COUNT 2052

// VS INIT
#define VS_INIT_MODE 0x0800 // SD_NEW
#define VS_INIT_MODE_SOFT_RESET 0x0804 // SD_NEW
//#define VS_INIT_FREQ 0x9000 //
#define VS_INIT_FREQ 0x9800 //
#define VS_INIT_VOLUME 0x30

// VS1002 commands
#define VS1002_READ     0x03
#define VS1002_WRITE    0x02
#define DREQ_PIN  GPIO_Pin_1
#define RESET_PIN GPIO_Pin_2
#define XCS_PIN   GPIO_Pin_3
#define XDCS_PIN  GPIO_Pin_4
#define HARD_RESET 2
#define SOFT_RESET 1

#define WAIT_BUSY   while(!(GPIOA->IDR & DREQ_PIN))
#define IS_DREQ_HIGH GPIOA->IDR & DREQ_PIN
#define XCS_HIGH    GPIOA->BSRRL = XCS_PIN
#define XCS_LOW     GPIOA->BSRRH = XCS_PIN
#define XDCS_HIGH   GPIOA->BSRRL = XDCS_PIN
#define XDCS_LOW    GPIOA->BSRRH = XDCS_PIN
#define RESET_HIGH  GPIOA->BSRRL = RESET_PIN
#define RESET_LOW   GPIOA->BSRRH = RESET_PIN

/* SCI registers */

#define SCI_MODE        0x00
#define SCI_STATUS      0x01
#define SCI_BASS        0x02
#define SCI_CLOCKF      0x03
#define SCI_DECODE_TIME 0x04
#define SCI_AUDATA      0x05
#define SCI_WRAM        0x06
#define SCI_WRAMADDR    0x07
#define SCI_HDAT0       0x08 /* VS1063, VS1053, VS1033, VS1003, VS1011 */
#define SCI_IN0         0x08 /* VS1103 */
#define SCI_HDAT1       0x09 /* VS1063, VS1053, VS1033, VS1003, VS1011 */
#define SCI_IN1         0x09 /* VS1103 */
#define SCI_AIADDR      0x0A
#define SCI_VOL         0x0B
#define SCI_AICTRL0     0x0C /* VS1063, VS1053, VS1033, VS1003, VS1011 */
#define SCI_MIXERVOL    0x0C /* VS1103 */
#define SCI_AICTRL1     0x0D /* VS1063, VS1053, VS1033, VS1003, VS1011 */
#define SCI_ADPCMRECCTL 0x0D /* VS1103 */
#define SCI_AICTRL2     0x0E
#define SCI_AICTRL3     0x0F

#define SM_DIFF           (1<< 0)
#define SM_LAYER12        (1<< 1) /* VS1063, VS1053, VS1033, VS1011 */
#define SM_RECORD_PATH    (1<< 1) /* VS1103 */
#define SM_RESET          (1<< 2)
#define SM_CANCEL         (1<< 3) /* VS1063, VS1053 */
#define SM_OUTOFWAV       (1<< 3) /* VS1033, VS1003, VS1011 */
#define SM_OUTOFMIDI      (1<< 3) /* VS1103 */
#define SM_EARSPEAKER_LO  (1<< 4) /* VS1053, VS1033 */
#define SM_PDOWN          (1<< 4) /* VS1003, VS1103 */
#define SM_TESTS          (1<< 5)
#define SM_STREAM         (1<< 6) /* VS1053, VS1033, VS1003, VS1011 */
#define SM_ICONF          (1<< 6) /* VS1103 */
#define SM_EARSPEAKER_HI  (1<< 7) /* VS1053, VS1033 */
#define SM_DACT           (1<< 8)
#define SM_SDIORD         (1<< 9)
#define SM_SDISHARE       (1<<10)
#define SM_SDINEW         (1<<11)
#define SM_ENCODE         (1<<12) /* VS1063 */
#define SM_ADPCM          (1<<12) /* VS1053, VS1033, VS1003 */
#define SM_EARSPEAKER1103 (1<<12) /* VS1103 */
#define SM_ADPCM_HP       (1<<13) /* VS1033, VS1003 */
#define SM_LINE1          (1<<14) /* VS1063, VS1053 */
#define SM_LINE_IN        (1<<14) /* VS1033, VS1003, VS1103 */
#define SM_CLK_RANGE      (1<<15) /* VS1063, VS1053, VS1033 */
#define SM_ADPCM_1103     (1<<15) /* VS1103 */

#define PAR_END_FILL_BYTE 0x1e06 /* VS1063, VS1053 */


void vs_io_init(void);
void vs_init(void);
uint8_t vs_get_status(void);
void vs_sci_write(uint8_t address, uint16_t data);
void vs_send_32(uint8_t *pData, uint8_t len);
void vs_send_32_endbyte(uint8_t endByte);
uint16_t vs_sci_read(uint8_t address);
uint16_t vs_read_mem16(uint16_t addr);
void vs_start_sinetest(uint8_t pitch);
void vs_stop_sinetest(void);
void vs_reset(uint8_t type);
void vs_set_volume(uint16_t volume);
void vs_set_simple_volume(uint8_t volume); // 0-150 (150 is loudest)
void vs_health_check(void);

#endif /* end __VS1033_H */
