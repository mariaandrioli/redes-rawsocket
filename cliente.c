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

void cliente(char *port){
    msg_t mensagem;
	int meuSock = ConexaoRawSocket(port);
	char buffer[MAX]; 
    int n; 
    char *comando = malloc(2*sizeof(char));
    char *param = malloc(30*sizeof(char));
    int quit = 0;
    crcTable();

    do {
        bzero(buffer, MAX);
        bzero(mensagem.dados, 128);
        printf("> "); 
        n = 0; 
        // coloca o que esta sendo digitado no buffer
        while ((buffer[n++] = getchar()) != '\n'); 
        strncpy(comando, (char*)buffer, 2);
        //strcpy(param,"");
        if (buffer[2]){
        	bzero(param,30);
            for (int i = 2; buffer[i] != '\0'; i++){
                    param[i-2] = buffer[i];
            }

            *(param+strlen(param)-1) = '\0';
        }
        *(buffer+strlen((char*)(buffer))-1) = '\0';
        
        if (strncmp((char*)buffer, "q", 1) == 0){
            quit = 1;
        }
        switch (interpretaComando(comando, param)){
            case CD:
                criaMensagem(strlen(param), 0, CD, param, &mensagem);
                cdRemotoCliente(meuSock, &mensagem);
                //imprimeMensagem(mensagem);
                //strcpy(param,"");
                break;
            case LS:
                criaMensagem(sizeof(param), 0, LS, param, &mensagem);
                lsRemotoCliente(meuSock, &mensagem);
                break;
            case PUT:
            	// puts("put cliente");
                criaMensagem(strlen(param), 0, PUT, param, &mensagem);   
                putCliente (meuSock, param, &mensagem);
                break;
            case GET:
                criaMensagem(strlen(param), 0, GET, param, &mensagem);
                getCliente(meuSock, &mensagem);
                break;
            default:
                break;
        }
        bzero(param, sizeof(param)); 
    } while (!quit);

    close(meuSock);
}