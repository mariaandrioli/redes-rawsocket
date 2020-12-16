#ifndef __CRC__
#define __CRC__
#include <stdint.h>

uint8_t calc_crc(msg_t *msg);

void crcTable(void);

#endif
