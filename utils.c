#include <inttypes.h>
#include <stdint.h>
#include "utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include "rawsocket.h"
#include "crc.h"
#include <sys/time.h>
#include <sys/types.h>
#include <sys/statvfs.h>

char interpretaComando (char *comando, char *adicionais){
	char aux[100];
	char *destino = malloc((sizeof(adicionais))*sizeof(char));
	if (comando[0] == 'c'){
		if (comando[1] == 'd'){
			destino = strtok(adicionais, " \n");
			chdir(destino);
			if (getcwd(aux, sizeof(aux)) != NULL)
				printf("Local: %s\n", aux);
			return '0';
		}
		else {
			return CD;
		}
	}
	if ((comando[0] == 'l') || (comando[0] == 'd')){
		if (comando[1] == 's'){
			if (adicionais == NULL)
				system(comando);
			else {
				aux[0] = comando[0];
				aux[1] = comando[1];
				strcat(aux, " ");
				strcat(aux, adicionais);
				system(aux);
			}
			return '0';
		}
		else {
			return LS;
		}
	}
	if ((comando[0] == 'p') || (comando[0] == 'b'))
		return PUT;
	if ((comando[0] == 'g') || (comando[0] == 'a'))
		return GET;
	if (strcasecmp(comando,"quit") == 0)
		return QUIT;
	return '0';
}

void criaMensagem (unsigned char tam, unsigned char seq, unsigned char tipo, char *dados, msg_t *mensagem){
	mensagem->inicio = INICIO;
	if(tam == 128)
		mensagem->tam = tam-1;
	else
		mensagem->tam = tam;
	mensagem->sequencia = seq;
	mensagem->tipo = tipo;

	if (!dados)
		memset(dados, 0, sizeof(mensagem->dados));
	else
		strncpy(mensagem->dados, dados, tam);
	mensagem->crc = 0;
}

void imprimeMensagem (msg_t mensagem){
	printf("Inicio %x\n", mensagem.inicio);
	printf("Tam %d\n", mensagem.tam);
	printf("Seq %d\n", mensagem.sequencia);
	printf("Tipo %x\n", mensagem.tipo);
	printf("Dados:");
	for(int i = 0; i < mensagem.tam; ++i)
		printf("%c", mensagem.dados[i]);
	printf("\nCRC %x\n", mensagem.crc);
}

void msgToBuffer (msg_t *mensagem, unsigned char buffer[]){
	if (mensagem){
		buffer[0] = mensagem->inicio;
		buffer[1] = mensagem->tam;
		buffer[2] = mensagem->sequencia;
		buffer[3] = mensagem->tipo;

		for (int i = 0; i < mensagem->tam+1; ++i)
			buffer[4+i] = mensagem->dados[i];

		buffer[4+mensagem->tam+1] = 0;
		return ;
	}
	else {
		printf("mensagem vazia");
		buffer = NULL;
	}
}

void bufferToMsg (msg_t *mensagem, unsigned char buffer[]){

	mensagem->inicio = buffer[0];
	mensagem->tam = buffer[1];
	mensagem->sequencia = buffer[2];
	mensagem->tipo = buffer[3];
	bzero(mensagem->dados, 128);
	for (int i = 0; i < mensagem->tam+1; ++i)
		mensagem->dados[i] = buffer[4+i];

	mensagem->crc = buffer[4+mensagem->tam+1];
}

void instrucoes(void){
	printf("-------------------------\n\tCOMANDOS\n-------------------------\n");
	printf("ls < -l/ -a/ -la > - ls local\n");
    printf("l < -l/ -a/ -la > - ls remoto\n");
    printf("cd < arquivo >  - cd local\n");
    printf("c < arquivo > - cd remoto\n");
    printf("g < arquivo > - recebe arquivo\n");
    printf("p < arquivo > - envia arquivo\n");
    printf("quit ou q - sai do programa\n\n");
}

void timeout(unsigned char buffer[], int socket){
	fd_set rfds;
	struct timeval tv;
	int retval;
	int i;

	for (i = 0; i < TMAX; ++i) {
		FD_ZERO(&rfds);
		FD_SET(socket, &rfds);

		tv.tv_sec = WAIT;
		tv.tv_usec = 0;

		retval = select(socket + 133, &rfds, NULL, NULL, &tv);
		if (retval <= 0) {
			puts("timeout");
			send(socket, buffer, MAX, 0);
		}
		else if (FD_ISSET(socket, &rfds))
			break;
	}

	if (i == TMAX-1){
		puts("Erro: TIMEOUT MAXIMO");
		exit(0);
	}
}

long tamanhoDisponivel(const char* path){
  struct statvfs stat;
  
  if (statvfs(path, &stat) != 0) {
    // error happens, just quits here
    return -1;
  }

  // the available size is f_bsize * f_bavail
  return stat.f_bsize * stat.f_bavail;
}