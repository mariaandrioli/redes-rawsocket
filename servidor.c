#include "utils.h"
#include "crc.h"
#include "rawsocket.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "comandos.h"

void servidor(char *port){
	char buffer[MAX]; 
	msg_t recebida;
    int meuSock = ConexaoRawSocket(port);
    char *comando = malloc(2*sizeof(char));
    char *param = malloc(30*sizeof(char));
    crcTable();

	while (1){
		bzero(buffer, MAX); 
        bzero(recebida.dados, 128);
        // le a mensagem recebida e coloca no buffer
        recv(meuSock, buffer, sizeof(buffer), 0); 
        bufferToMsg(&recebida, (unsigned char *) buffer);
        // imprimeMensagem(recebida);
        strcpy(comando, "");
        strcpy(param, "");
        sprintf(comando, "%x", recebida.tipo);
        sprintf(param, "%s", recebida.dados);

        // printf("%x\n", recebida.tipo);
		switch (interpretaComando(comando, param)){
            case CD:
                cdRemotoServidor(recebida,meuSock,param);
                break;
            case LS:
            	lsRemotoServidor(meuSock, param, recebida);
                break;
            case PUT:
                putServidor(meuSock, &recebida);
                break;
            case GET:
            	getServidor(meuSock, param, recebida);
                break;
            default:
                break;
        }
    }
}
