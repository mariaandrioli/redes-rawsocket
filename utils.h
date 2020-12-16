#ifndef __UTILS__
#define __UTILS__
#include <stdint.h>

typedef struct msg_t { // struct que define as informações da header e seus respectivos tamanhos
	char inicio;
	unsigned char tam:7;
	unsigned char sequencia:5;
	unsigned char tipo:4;
	char dados[128];
	uint8_t crc;
} msg_t;

#define NACK 0
#define ACK 1
#define OK 2
#define DADOS 3
#define QUIT 4
#define TAM 5
#define TELA 7
#define GET 0xA
#define PUT 0xB
#define CD 0xC
#define LS 0xD
#define ERRO 0xE
#define FIM 0xF
#define MAX 133
#define INICIO 0x7E

#define WAIT 5
#define TMAX 12

char interpretaComando (char *comando, char *adicionais);
void criaMensagem (unsigned char tam, unsigned char seq, unsigned char tipo, char *dados, msg_t *mensagem);
void imprimeMensagem(msg_t mensagem);
void msgToBuffer (msg_t *mensagem, unsigned char buffer[]);
void bufferToMsg (msg_t *mensagem, unsigned char buffer[]);
void instrucoes(void);
void timeout(unsigned char buffer[], int socket);
long tamanhoDisponivel(const char* path);

#endif