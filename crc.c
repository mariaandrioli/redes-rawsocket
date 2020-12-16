#include <stdint.h>
#include <strings.h>
#include "utils.h"
#define POLINOMIO 0xD5
#define LARGURA  (8 * sizeof(unsigned char))
#define TOPBIT (1 << (LARGURA - 1))

unsigned char crcT[256];

uint8_t calc_crc(msg_t *msg){
  
  uint8_t dados;
  uint8_t crc = 0;
  unsigned char buffer[MAX];
  bzero(buffer,MAX);
  msgToBuffer(msg,(unsigned char*)buffer);
  
  for (int i = 1; i < MAX-1; i++) {
    dados = buffer[i]^(crc >> (LARGURA - 8));
    crc = crcT[dados]^(crc <<8);
  }

  return(crc);
}

void crcTable(void){
  
  uint8_t crc;
  
  for (int i = 0; i < 256; i++) {
    crc = i << (LARGURA -8);
    for (uint8_t j = 8; 0 < j; j--) {
      if (crc & TOPBIT) {
        crc = (crc << 1) ^ POLINOMIO;
      }
      else{
        crc = (crc << 1);
      }
    }
    crcT[i] = crc;
  }
}
