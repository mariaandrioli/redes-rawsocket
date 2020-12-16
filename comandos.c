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
#include "comandos.h"
#include <stdio_ext.h>

void lsRemotoServidor (int meuSock, char *destino, msg_t msg){
/* faz ls e coloca no arquivo ls.txt*/
	FILE *arquivo;
	char *aux = malloc((sizeof(destino)+3)*sizeof(char));	
	msg_t mensagem, recebida;
    unsigned char buffer1[MAX], buffer2[MAX];
    
    bzero(buffer1, MAX); 
    bzero(buffer2, MAX);
	
	uint8_t crc = calc_crc(&msg);
	recebida.crc = msg.crc;
    
    strcpy(aux, "");

    while(crc != recebida.crc){
      	criaMensagem (1,0,NACK,aux,&mensagem);
	    msgToBuffer(&mensagem, (unsigned char *) buffer1);
	    buffer1[5+mensagem.tam] = calc_crc(&mensagem);
		send(meuSock, buffer1, sizeof(buffer1), 0);
	    
	    recv(meuSock, buffer2, sizeof(buffer2), 0);
	    bufferToMsg(&recebida, (unsigned char *) buffer2);
	    crc = calc_crc(&recebida);
	}

	strcpy(aux, "ls ");
	if (destino)
		strcat(aux, destino);
	strcat(aux, " > ls.txt");

	arquivo = fopen("ls.txt", "wb+");
	__fpurge(arquivo);
	int ret = system(aux);

	// system("cat ls.txt");
	strcpy(aux, "");
	if(ret == -1){
        if(errno == EACCES)
            criaMensagem (1,0,ERRO,"2",&mensagem);
	}
    else{
    	criaMensagem(0,0,TELA,aux,&mensagem);  
    }

    msgToBuffer(&mensagem, (unsigned char *) buffer1);
    buffer1[5+mensagem.tam] = calc_crc(&mensagem);
	send(meuSock, buffer1, sizeof(buffer1), 0);
	
	recv(meuSock, buffer2, sizeof(buffer2), 0);
    bufferToMsg(&recebida, (unsigned char *) buffer2);
    
    bzero(buffer1, MAX); 
    bzero(buffer2, MAX);
    
    while(recebida.tipo == NACK){
      	send(meuSock, buffer1, sizeof(buffer1), 0);
        
        recv(meuSock, buffer2, sizeof(buffer2), 0);
        bzero(recebida.dados, 128);
        bufferToMsg(&recebida, (unsigned char *) buffer2);
    }

    bzero(buffer1, MAX); 
    bzero(buffer2, MAX);
    bzero(mensagem.dados,128);
    
	char c, *arq = malloc(1000000);
	int i = 0; 
	
	c = getc(arquivo);
	while (!feof(arquivo)){
		arq[i] = c;
		c = getc(arquivo);
		i++;
	}

	char *s = malloc(i*sizeof(char));
	sprintf(s,"%d",i);
    
    criaMensagem(strlen(s),0,TAM,s,&mensagem); 
    msgToBuffer(&mensagem, (unsigned char *) buffer1);
    buffer1[5+mensagem.tam] = calc_crc(&mensagem);
	send(meuSock, buffer1, sizeof(buffer1), 0);

	
	recv(meuSock, buffer2, sizeof(buffer2), 0);
	bzero(recebida.dados, 128);
    bufferToMsg(&recebida, (unsigned char *) buffer2);
    
    while((recebida.tipo != ACK) && (recebida.tipo != NACK)){
		
		recv(meuSock, buffer2, sizeof(buffer2), 0);
		bzero(recebida.dados, 128);
	    bufferToMsg(&recebida, (unsigned char *) buffer2);
    }

    if(recebida.tipo == NACK){
      	send(meuSock, buffer1, sizeof(buffer1), 0);
        
        recv(meuSock, buffer2, sizeof(buffer2), 0);
        bzero(recebida.dados, 128);
        bufferToMsg(&recebida, (unsigned char *) buffer2);
  	    while(recebida.tipo == NACK){
      		send(meuSock, buffer1, sizeof(buffer1), 0);
			
			recv(meuSock, buffer2, sizeof(buffer2), 0);
			bzero(recebida.dados, 128);
		    bufferToMsg(&recebida, (unsigned char *) buffer2);
		}
    }

	int qtdMsgs = (i + 1)/128;
	int qtdEnvios = qtdMsgs/3;
	int restoMsgs = qtdMsgs % 3;
	int resto = i % 128;
	int seq = 0;

	aux = arq;

	for (int j = 0; j < qtdEnvios; ++j){
		criaMensagem(128, seq, DADOS, aux, &mensagem);
		msgToBuffer(&mensagem, (unsigned char *) buffer1);
	    buffer1[5+mensagem.tam] = calc_crc(&mensagem);
		send(meuSock, buffer1, sizeof(buffer1), 0);
		
		criaMensagem(128, seq+1, DADOS, aux+128, &mensagem);
		msgToBuffer(&mensagem, (unsigned char *) buffer1);
	    buffer1[5+mensagem.tam] = calc_crc(&mensagem);
		send(meuSock, buffer1, sizeof(buffer1), 0);

		criaMensagem(128, seq+2, DADOS, aux+256, &mensagem);
		msgToBuffer(&mensagem, (unsigned char *) buffer1);
	    buffer1[5+mensagem.tam] = calc_crc(&mensagem);
		send(meuSock, buffer1, sizeof(buffer1), 0);

		bzero(buffer2, MAX);
		
		recv(meuSock, buffer2, sizeof(buffer2), 0);
        bzero(recebida.dados, 128);
        bufferToMsg(&recebida, (unsigned char *) buffer2);

        while(recebida.tipo == NACK){

        	criaMensagem(128, seq, DADOS, aux, &mensagem);
			msgToBuffer(&mensagem, (unsigned char *) buffer1);
		    buffer1[5+mensagem.tam] = calc_crc(&mensagem);
		    
			send(meuSock, buffer1, sizeof(buffer1), 0);
			criaMensagem(128, seq+1, DADOS, aux+128, &mensagem);
			msgToBuffer(&mensagem, (unsigned char *) buffer1);
		    buffer1[5+mensagem.tam] = calc_crc(&mensagem);
			send(meuSock, buffer1, sizeof(buffer1), 0);

			criaMensagem(128, seq+2, DADOS, aux+256, &mensagem);
			msgToBuffer(&mensagem, (unsigned char *) buffer1);
		    buffer1[5+mensagem.tam] = calc_crc(&mensagem);
			send(meuSock, buffer1, sizeof(buffer1), 0);

			bzero(buffer2, MAX);
			
			recv(meuSock, buffer2, sizeof(buffer2), 0);
	        bzero(recebida.dados, 128);
	        bufferToMsg(&recebida, (unsigned char *) buffer2);

        }
        seq = (seq+3)%32;
		aux += 384;
	}

	for (int j = 0; j < restoMsgs; ++j){
		bzero(buffer1, MAX);
		criaMensagem(128, seq, DADOS, aux, &mensagem);
		msgToBuffer(&mensagem, (unsigned char *) buffer1);
	    buffer1[132] = calc_crc(&mensagem);
		send(meuSock, buffer1, sizeof(buffer1), 0);
		bzero(buffer2, MAX);
		
		recv(meuSock, buffer2, sizeof(buffer2), 0);
        bzero(recebida.dados, 128);
        bufferToMsg(&recebida, (unsigned char *) buffer2);


        while(recebida.tipo == NACK){

        	criaMensagem(128, seq, DADOS, aux, &mensagem);
			msgToBuffer(&mensagem, (unsigned char *) buffer1);
		    buffer1[5+mensagem.tam] = calc_crc(&mensagem);
			send(meuSock, buffer1, sizeof(buffer1), 0);
			
			bzero(buffer2, MAX);
			
			recv(meuSock, buffer2, sizeof(buffer2), 0);
    	    bzero(recebida.dados, 128);
        	bufferToMsg(&recebida, (unsigned char *) buffer2);
        }

        seq = (seq+1)%32;
		aux += 128;

	}
	
	bzero(buffer1, MAX); 
	bzero(mensagem.dados,128);
	criaMensagem(resto, seq, DADOS, aux, &mensagem);
	msgToBuffer(&mensagem, (unsigned char *) buffer1);
    buffer1[5+mensagem.tam] = calc_crc(&mensagem);
	send(meuSock, buffer1, sizeof(buffer1), 0);

	bzero(buffer2, MAX);
	
	recv(meuSock, buffer2, sizeof(buffer2), 0);
    bzero(recebida.dados, 128);
    bufferToMsg(&recebida, (unsigned char *) buffer2);

    while(recebida.tipo == NACK){
    	criaMensagem(resto, seq, DADOS, aux, &mensagem);
		msgToBuffer(&mensagem, (unsigned char *) buffer1);
	    buffer1[5+mensagem.tam] = calc_crc(&mensagem);
		send(meuSock, buffer1, sizeof(buffer1), 0);
		
		bzero(buffer2, MAX);
		
		recv(meuSock, buffer2, sizeof(buffer2), 0);
	    bzero(recebida.dados, 128);
    	bufferToMsg(&recebida, (unsigned char *) buffer2);
    }

    strcpy(aux,"");
    bzero(buffer1, MAX); 
	bzero(mensagem.dados,128);
    criaMensagem(0,0,FIM,aux, &mensagem);
    msgToBuffer(&mensagem,(unsigned char*) buffer1);
	send(meuSock, buffer1, sizeof(buffer1), 0);
	system("rm -rf ls.txt");

}

void lsRemotoCliente (int meuSock, msg_t *mensagem){
	
	int qtdMsgs = 0;
	int qtdEnvios = 0;
	int restoMsgs = 0;
	msg_t recebida;
	long tam;
	uint8_t crc;
	unsigned char buffer1[MAX], buffer2[MAX];
	bzero(buffer1,MAX);
	bzero(buffer2,MAX);
    bzero(recebida.dados, 128);
    char aux2[1];
    strcpy(aux2,"");
    
    msgToBuffer(mensagem, (unsigned char *) buffer1);
    buffer1[5+mensagem->tam] = calc_crc(mensagem);
    send(meuSock, buffer1, sizeof(buffer1), 0); 
    
    recv(meuSock, buffer2, sizeof(buffer2), 0);
    timeout(buffer2,meuSock);
    bufferToMsg(&recebida, (unsigned char *) buffer2);

    bzero(buffer1, MAX); 
    bzero(buffer2, MAX);
    
    while(recebida.tipo == NACK){
    	send(meuSock, buffer1, sizeof(buffer1), 0);
        recv(meuSock, buffer2, sizeof(buffer2), 0);
        timeout(buffer2,meuSock);
        bzero(recebida.dados, 128);
        bufferToMsg(&recebida, (unsigned char *) buffer2);
    }

    
    recv(meuSock, buffer2, sizeof(buffer2), 0);
    timeout(buffer2,meuSock);
    bufferToMsg(&recebida, (unsigned char *) buffer2);
    
    if((recebida.tipo == ERRO) || (recebida.tipo == TELA)){
        crc = calc_crc(&recebida);
		if(crc == recebida.crc){
            if(recebida.tipo == ERRO){
            	puts("Voce nao tem permissao para acessar esse diretorio.");
            }
    		bzero(mensagem->dados, 128);
			criaMensagem (0,0,ACK,aux2,mensagem);
	    	msgToBuffer(mensagem, (unsigned char *) buffer1);
	    	buffer1[5+mensagem->tam] = calc_crc(mensagem);
	    	send(meuSock, buffer1, sizeof(buffer1), 0);
	   	}
    	else{
    		bzero(mensagem->dados, 128);
    		criaMensagem (0,0,NACK,aux2,mensagem);
	    	msgToBuffer(mensagem, (unsigned char *) buffer1);
	    	buffer1[5+mensagem->tam] = calc_crc(mensagem);
	    	send(meuSock, buffer1, sizeof(buffer1), 0);	
    	}
	}
    
    bzero(buffer1, MAX); 
    bzero(buffer2, MAX);
    bzero(recebida.dados, 128);

    
    recv(meuSock, buffer2, sizeof(buffer2), 0);
    timeout(buffer2,meuSock);
    bufferToMsg(&recebida, (unsigned char *) buffer2);

    while(recebida.tipo != TAM){
	    bzero(buffer2, MAX);
	    recv(meuSock, buffer2, sizeof(buffer2), 0);
	    timeout(buffer2,meuSock);
	    bufferToMsg(&recebida, (unsigned char *) buffer2);
	}
	crc = calc_crc(&recebida);
	while(crc != recebida.crc){
		bzero(mensagem->dados, 128);
		criaMensagem (0,0,NACK,aux2,mensagem);
    	msgToBuffer(mensagem, (unsigned char *) buffer1);
    	buffer1[5+mensagem->tam] = calc_crc(mensagem);
    	send(meuSock, buffer1, sizeof(buffer1), 0);
    	
    	bzero(buffer2, MAX);
	    
	    recv(meuSock, buffer2, sizeof(buffer2), 0);
	    timeout(buffer2,meuSock);
	    bzero(recebida.dados, 128);
	    bufferToMsg(&recebida, (unsigned char *) buffer2);
	    crc = calc_crc(&recebida);
    }
	
	tam = atoi(recebida.dados);
	bzero(mensagem->dados, 128);
	criaMensagem (0,0,ACK,aux2,mensagem);
	msgToBuffer(mensagem, (unsigned char *) buffer1);
	buffer1[5+mensagem->tam] = calc_crc(mensagem);
	send(meuSock, buffer1, sizeof(buffer1), 0);
	qtdMsgs = (tam + 1)/(long)128;
	qtdEnvios = qtdMsgs/3;
	restoMsgs = qtdMsgs % 3;
	int resto = (tam + 1) % 128;

    bzero(buffer1, MAX); 
    bzero(buffer2, MAX);
    bzero(recebida.dados, 128);

	unsigned char buff_m1[MAX], buff_m2[MAX], buff_m3[MAX];
	msg_t msg1, msg2, msg3;
	uint8_t crc_m1, crc_m2, crc_m3;

	for (int j = 0; j < qtdEnvios; ++j){
		bzero(buff_m1,MAX);
		bzero(buff_m2,MAX);
		bzero(buff_m3,MAX);

		recv(meuSock, buff_m1, sizeof(buff_m1), 0);
		timeout(buff_m1,meuSock);
		bufferToMsg(&msg1, (unsigned char *) buff_m1);
		while(msg1.tipo != DADOS){
			bzero(buff_m1,MAX);
			recv(meuSock, buff_m1, sizeof(buff_m1), 0);
			timeout(buff_m1,meuSock);
			bzero(msg1.dados,128);
			bufferToMsg(&msg1, (unsigned char *) buff_m1);
		}
		crc_m1 = calc_crc(&msg1);

		recv(meuSock, buff_m2, sizeof(buff_m2), 0);
		timeout(buff_m2,meuSock);
		bufferToMsg(&msg2, (unsigned char *) buff_m2);
		while(msg2.crc == msg1.crc){
			bzero(buff_m2,MAX);
			recv(meuSock, buff_m2, sizeof(buff_m2), 0);
			timeout(buff_m2,meuSock);
			bzero(msg2.dados,128);
			bufferToMsg(&msg2, (unsigned char *) buff_m2);
		}
		crc_m2 = calc_crc(&msg2);
		
		
		recv(meuSock, buff_m3, sizeof(buff_m3), 0);
		timeout(buff_m3,meuSock);
		bufferToMsg(&msg3, (unsigned char *) buff_m3);
		while(msg3.crc == msg2.crc){
			bzero(buff_m3,MAX);
			recv(meuSock, buff_m3, sizeof(buff_m3), 0);
			timeout(buff_m3,meuSock);
			bzero(msg3.dados,128);
			bufferToMsg(&msg3, (unsigned char *) buff_m3);
		}
		crc_m3 = calc_crc(&msg3);
		
		while((crc_m1 != msg1.crc) || (crc_m2 != msg2.crc) || (crc_m3 != msg3.crc)){
			bzero(mensagem->dados, 128);
    		criaMensagem (0,0,NACK,aux2,mensagem);
	    	bzero(buffer1,MAX);
	    	msgToBuffer(mensagem, (unsigned char *) buffer1);
	    	buffer1[5+mensagem->tam] = calc_crc(mensagem);
	    	send(meuSock, buffer1, sizeof(buffer1), 0);
			
			bzero(buff_m1,MAX);
			recv(meuSock, buff_m1, sizeof(buff_m1), 0);
			timeout(buff_m1,meuSock);
			bufferToMsg(&msg1, (unsigned char *) buff_m1);
			while(msg1.crc == msg3.crc){
				bzero(buff_m1,MAX);
				recv(meuSock, buff_m1, sizeof(buff_m1), 0);
				timeout(buff_m1,meuSock);
				bzero(msg1.dados,128);
				bufferToMsg(&msg1, (unsigned char *) buff_m1);
			}
			crc_m1 = calc_crc(&msg1);
			
			bzero(buff_m2,MAX);
			recv(meuSock, buff_m2, sizeof(buff_m2), 0);
			timeout(buff_m2,meuSock);
			bzero(msg2.dados,128);
			bufferToMsg(&msg2, (unsigned char *) buff_m2);
			while(msg2.crc == msg1.crc){
				bzero(buff_m2,MAX);
				recv(meuSock, buff_m2, sizeof(buff_m2), 0);
				timeout(buff_m2,meuSock);
				bufferToMsg(&msg2, (unsigned char *) buff_m2);
			}
			crc_m2 = calc_crc(&msg2);
			
			bzero(buff_m3,MAX);
			recv(meuSock, buff_m3, sizeof(buff_m3), 0);
			timeout(buff_m3,meuSock);
			bzero(msg3.dados,128);
			bufferToMsg(&msg3, (unsigned char *) buff_m3);
			while(msg3.crc == msg2.crc){
				bzero(buff_m3,MAX);
				recv(meuSock, buff_m3, sizeof(buff_m3), 0);
				timeout(buff_m3,meuSock);
				bzero(msg3.dados,128);
				bufferToMsg(&msg3, (unsigned char *) buff_m3);
			}
			crc_m3 = calc_crc(&msg3);
			}
	
		for (int i = 0; i < 128; ++i)
			printf("%c", msg1.dados[i]);
		for (int i = 0; i < 128; ++i)
			printf("%c", msg2.dados[i]);
		for (int i = 0; i < 128; ++i)
			printf("%c", msg3.dados[i]);
	
		bzero(buffer1,MAX);
		bzero(mensagem->dados, 128);
		criaMensagem (0,0,ACK,aux2,mensagem);
    	msgToBuffer(mensagem, (unsigned char *) buffer1);
    	buffer1[5+mensagem->tam] = calc_crc(mensagem);
    	send(meuSock, buffer1, sizeof(buffer1), 0);
	}

	for (int i = 0; i < restoMsgs; ++i){
		bzero(msg1.dados,128);
		bzero(buffer2, MAX);
		recv(meuSock, buffer2, sizeof(buffer2), 0);
		timeout(buffer2,meuSock);
		bufferToMsg(&msg1, (unsigned char *) buffer2);
		
		while(msg1.tipo != DADOS){
			bzero(buffer2, MAX);
			recv(meuSock, buffer2, sizeof(buffer2), 0);
			timeout(buffer2,meuSock);
			bzero(msg1.dados,128);
			bufferToMsg(&msg1, (unsigned char *) buffer2);
		}
		crc_m1 = calc_crc(&msg1);

		while(crc_m1 != msg1.crc){
			bzero(mensagem->dados, 128);
			bzero(buffer1,MAX);
    		criaMensagem (0,0,NACK,aux2,mensagem);
	    	msgToBuffer(mensagem, (unsigned char *) buffer1);
	    	buffer1[5+mensagem->tam] = calc_crc(mensagem);
	    	send(meuSock, buffer1, sizeof(buffer1), 0);	
			}

			for (int i = 0; i < 128; ++i){
				printf("%c", msg1.dados[i]);
			bzero(buffer1,MAX);
			bzero(mensagem->dados, 128);
    		criaMensagem (0,0,ACK,aux2,mensagem);
	    	msgToBuffer(mensagem, (unsigned char *) buffer1);
	    	buffer1[5+mensagem->tam] = calc_crc(mensagem);
	    	send(meuSock, buffer1, sizeof(buffer1), 0);	
		}
	}
	
	bzero(buffer2,MAX);
	bzero(recebida.dados,128);
	
	recv(meuSock, buffer2, sizeof(buffer2), 0);
	timeout(buffer2,meuSock);
	bufferToMsg(&recebida, (unsigned char *) buffer2);
	
	while(recebida.tam != resto){
		bzero(buffer2,MAX);
		bzero(recebida.dados,128);
		
		recv(meuSock, buffer2, sizeof(buffer2), 0);
		timeout(buffer2,meuSock);
		bufferToMsg(&recebida, (unsigned char *) buffer2);
		
	}
	
	crc_m1 = calc_crc(&recebida);

	while(crc_m1 != recebida.crc){
		bzero(mensagem->dados, 128);
		bzero(buffer1,MAX);
		criaMensagem (0,0,NACK,aux2,mensagem);
    	msgToBuffer(mensagem, (unsigned char *) buffer1);
    	buffer1[5+mensagem->tam] = calc_crc(mensagem);
    	send(meuSock, buffer1, sizeof(buffer1), 0);	
	}
	
	for (int i = 0; i < 128; ++i){
		printf("%c", recebida.dados[i]);
	}
	bzero(mensagem->dados, 128);
	bzero(buffer1,MAX);
	criaMensagem (0,0,ACK,aux2,mensagem);
	msgToBuffer(mensagem, (unsigned char *) buffer1);
	buffer1[5+mensagem->tam] = calc_crc(mensagem);
	send(meuSock, buffer1, sizeof(buffer1), 0);	
	
	bzero(buffer2,MAX);
	bzero(recebida.dados,128);
	
	recv(meuSock, buffer2, sizeof(buffer2), 0);
	timeout(buffer2,meuSock);
	bufferToMsg(&recebida, (unsigned char *) buffer2);
	
	while(recebida.tam != 0){
		bzero(buffer2,MAX);
		bzero(recebida.dados,128);
		
		recv(meuSock, buffer2, sizeof(buffer2), 0);
		timeout(buffer2,meuSock);
		bufferToMsg(&recebida, (unsigned char *) buffer2);
	}

}

void cdRemotoCliente(int meuSock, msg_t *mensagem){
	
	msg_t recebida;
	unsigned char buffer[MAX], buffer2[MAX];
	bzero(buffer,MAX);
	bzero(buffer2,MAX);
    bzero(recebida.dados,128);

    msgToBuffer(mensagem, (unsigned char *) buffer);
    buffer[5+mensagem->tam] = calc_crc(mensagem);
    send(meuSock, buffer, sizeof(buffer), 0); 
    
    recv(meuSock, buffer2, sizeof(buffer2), 0);
    timeout(buffer2,meuSock);
    bufferToMsg(&recebida, (unsigned char *) buffer2);


    while((recebida.tipo != NACK) && (recebida.tipo != OK) && (recebida.tipo != ERRO)){
    	bzero(buffer2,MAX);
    	recv(meuSock, buffer2, sizeof(buffer2), 0);
    	timeout(buffer2,meuSock);
    	bzero(recebida.dados,128);
	    bufferToMsg(&recebida, (unsigned char *) buffer2);
    }

    if(recebida.tipo == NACK){
    	bzero(buffer,MAX);
    	send(meuSock, buffer, sizeof(buffer), 0);
    	bzero(buffer2,MAX);
        recv(meuSock, buffer2, sizeof(buffer2), 0);
        timeout(buffer2,meuSock);
    	bzero(recebida.dados,128);
        bufferToMsg(&recebida, (unsigned char *) buffer2);
        while(recebida.tipo == NACK){
		   	bzero(buffer,MAX);
	    	send(meuSock, buffer, sizeof(buffer), 0);
	    	bzero(buffer2,MAX);
	        recv(meuSock, buffer2, sizeof(buffer2), 0);
	        timeout(buffer2,meuSock);
	    	bzero(recebida.dados,128);
	        bufferToMsg(&recebida, (unsigned char *) buffer2);
        }
    }
    else if(recebida.tipo == ERRO){
        char *erro = recebida.dados;
        if (strcmp(erro,"1") == 0){
            puts("Diretorio nao existe.");
            return;
        }
        if (strcmp(erro,"2") == 0){
            puts("Voce nao tem permissao para acessar esse diretorio");
            return;
        }
    }
    else if(recebida.tipo == OK){
		puts("Mudou de diretorio!");
		return;
	}

}

void cdRemotoServidor(msg_t mensagem, int meuSock, char *param){

    msg_t recebida, enviada;
    unsigned char buffer1[MAX], buffer2[MAX];
    char aux[100], aux2[1];

	uint8_t crc = calc_crc(&mensagem);
	recebida.crc = mensagem.crc;
    
    strcpy(aux2,"");
    
    bzero(buffer1, MAX); 
    bzero(buffer2, MAX); 

    while(crc != recebida.crc){
      	criaMensagem (0,0,NACK,aux2,&enviada);
	    msgToBuffer(&enviada, (unsigned char *) buffer1);
	    buffer1[5+enviada.tam] = calc_crc(&enviada);
		send(meuSock, buffer1, sizeof(buffer1), 0);	    
	    
	    recv(meuSock, buffer2, sizeof(buffer2), 0);
	    bufferToMsg(&recebida, (unsigned char *) buffer2);
	    crc = calc_crc(&recebida);
	}

    int dir = chdir(param);
	getcwd(aux, sizeof(aux));
	printf("Local: %s\n", aux);

    if(dir == -1){
        if(errno == ENOENT){
        	criaMensagem (1,0,ERRO,"1",&enviada);
        }
        if(errno == EACCES){
            criaMensagem (1,0,ERRO,"2",&enviada);
        }
    }
    else{
    	criaMensagem(strlen(aux),0,OK,aux,&enviada);  
    }

	bzero(buffer1,MAX);
   	
   	msgToBuffer(&enviada, (unsigned char *) buffer1);
   	buffer1[5+enviada.tam] = calc_crc(&enviada);
	send(meuSock, buffer1, sizeof(buffer1), 0);
	return;
}

void getServidor (int meuSock, char *destino, msg_t msg) {
	FILE *arquivo;

	char *aux = malloc((sizeof(destino)+3)*sizeof(char));	
	msg_t mensagem, recebida;
    unsigned char buffer1[MAX], buffer2[MAX];
    
    bzero(buffer1, MAX); 
    bzero(buffer2, MAX);
	
	uint8_t crc = calc_crc(&msg);
	
	recebida.crc = msg.crc;
    
    strcpy(aux, "");

    while(crc != recebida.crc){
    	
      	criaMensagem (0,0,NACK,aux,&mensagem);
	    msgToBuffer(&mensagem, (unsigned char *) buffer1);
	    buffer1[5+mensagem.tam] = calc_crc(&mensagem);
		send(meuSock, buffer1, sizeof(buffer1), 0);
	    
	    recv(meuSock, buffer2, sizeof(buffer2), 0);
	    bufferToMsg(&recebida, (unsigned char *) buffer2);
	    crc = calc_crc(&recebida);
	}

	arquivo = fopen(destino, "rb+");
	char c, *arq = malloc(1000000);
	int i = 0; 

	strcpy(aux, "");
	if(!arquivo){
        if(errno == ENOENT){
        	criaMensagem (1,0,ERRO,"1",&mensagem);
        }
        if (errno == EACCES){
        	criaMensagem (1,0,ERRO,"2",&mensagem);
        }
	}
    else{
		c = getc(arquivo);
		while (!feof(arquivo)){
			arq[i] = c;
			c = getc(arquivo);
			i++;
		}
		char *s = malloc(i*sizeof(char));
		sprintf(s,"%d",i);
		
	    criaMensagem(strlen(s),0,TAM,s,&mensagem); 
    }

    msgToBuffer(&mensagem, (unsigned char *) buffer1);
    buffer1[5+mensagem.tam] = calc_crc(&mensagem);
	send(meuSock, buffer1, sizeof(buffer1), 0);


	
	recv(meuSock, buffer2, sizeof(buffer2), 0);
    bufferToMsg(&recebida, (unsigned char *) buffer2);

    while ((recebida.tipo != OK) && (recebida.tipo != ERRO)) {
    	if (recebida.tipo == ERRO) {
    		puts("Não há espaço suficiente para receber o arquivo");
    		return;
    	}
    	else if (recebida.tipo == OK){
			send(meuSock, buffer1, sizeof(buffer1), 0);
		    
		    recv(meuSock, buffer2, sizeof(buffer2), 0);
		    bufferToMsg(&recebida, (unsigned char *) buffer2);
		}
	}
	int qtdMsgs = (i + 1)/128;
	int qtdEnvios = qtdMsgs/3;
	int restoMsgs = qtdMsgs % 3;
	int resto = (i) % 128;
	int seq = 0;

	aux = arq;

	for (int j = 0; j < qtdEnvios; ++j){
		
		criaMensagem(128, seq, DADOS, aux, &mensagem);
		msgToBuffer(&mensagem, (unsigned char *) buffer1);
	    buffer1[5+mensagem.tam] = calc_crc(&mensagem);
		send(meuSock, buffer1, sizeof(buffer1), 0);
		
		criaMensagem(128, seq+1, DADOS, aux+128, &mensagem);
		msgToBuffer(&mensagem, (unsigned char *) buffer1);
	    buffer1[5+mensagem.tam] = calc_crc(&mensagem);
		send(meuSock, buffer1, sizeof(buffer1), 0);

		criaMensagem(128, seq+2, DADOS, aux+256, &mensagem);
		msgToBuffer(&mensagem, (unsigned char *) buffer1);
	    buffer1[5+mensagem.tam] = calc_crc(&mensagem);
		send(meuSock, buffer1, sizeof(buffer1), 0);

		bzero(buffer2, MAX);
		
		recv(meuSock, buffer2, sizeof(buffer2), 0);
        bzero(recebida.dados, 128);
        bufferToMsg(&recebida, (unsigned char *) buffer2);

        while(recebida.tipo == NACK){

        	criaMensagem(128, seq, DADOS, aux, &mensagem);
			msgToBuffer(&mensagem, (unsigned char *) buffer1);
		    buffer1[5+mensagem.tam] = calc_crc(&mensagem);
			send(meuSock, buffer1, sizeof(buffer1), 0);

			criaMensagem(128, seq+1, DADOS, aux+128, &mensagem);
			msgToBuffer(&mensagem, (unsigned char *) buffer1);
		    buffer1[5+mensagem.tam] = calc_crc(&mensagem);
			send(meuSock, buffer1, sizeof(buffer1), 0);

			criaMensagem(128, seq+2, DADOS, aux+256, &mensagem);
			msgToBuffer(&mensagem, (unsigned char *) buffer1);
		    buffer1[5+mensagem.tam] = calc_crc(&mensagem);
			send(meuSock, buffer1, sizeof(buffer1), 0);

			bzero(buffer2, MAX);
			
			recv(meuSock, buffer2, sizeof(buffer2), 0);
	        bzero(recebida.dados, 128);
	        bufferToMsg(&recebida, (unsigned char *) buffer2);

        }
        seq = (seq+3)%32;
		aux += 384;
	}

	for (int j = 0; j < restoMsgs; ++j){
		bzero(buffer1, MAX);
		criaMensagem(128, seq, DADOS, aux, &mensagem);
		msgToBuffer(&mensagem, (unsigned char *) buffer1);
	    buffer1[132] = calc_crc(&mensagem);
		send(meuSock, buffer1, sizeof(buffer1), 0);
		bzero(buffer2, MAX);
		
		recv(meuSock, buffer2, sizeof(buffer2), 0);
        bzero(recebida.dados, 128);
        bufferToMsg(&recebida, (unsigned char *) buffer2);


        while(recebida.tipo == NACK){

        	criaMensagem(128, seq, DADOS, aux, &mensagem);
			msgToBuffer(&mensagem, (unsigned char *) buffer1);
		    buffer1[5+mensagem.tam] = calc_crc(&mensagem);
			send(meuSock, buffer1, sizeof(buffer1), 0);
			
			bzero(buffer2, MAX);
			
			recv(meuSock, buffer2, sizeof(buffer2), 0);
    	    bzero(recebida.dados, 128);
        	bufferToMsg(&recebida, (unsigned char *) buffer2);
        }

        seq = (seq+3)%32;
		aux += 128;

	}
	
	bzero(buffer1, MAX); 
	bzero(mensagem.dados,128);
	criaMensagem(resto, seq, DADOS, aux, &mensagem);
	msgToBuffer(&mensagem, (unsigned char *) buffer1);
    buffer1[5+mensagem.tam] = calc_crc(&mensagem);
	send(meuSock, buffer1, sizeof(buffer1), 0);
	bzero(buffer2, MAX);
	
	recv(meuSock, buffer2, sizeof(buffer2), 0);
    bzero(recebida.dados, 128);
    bufferToMsg(&recebida, (unsigned char *) buffer2);

    while(recebida.tipo == NACK){
    	criaMensagem(resto, seq, DADOS, aux, &mensagem);
		msgToBuffer(&mensagem, (unsigned char *) buffer1);
	    buffer1[5+mensagem.tam] = calc_crc(&mensagem);
		send(meuSock, buffer1, sizeof(buffer1), 0);
		bzero(buffer2, MAX);
		
		recv(meuSock, buffer2, sizeof(buffer2), 0);
	    bzero(recebida.dados, 128);
    	bufferToMsg(&recebida, (unsigned char *) buffer2);
    }

    strcpy(aux,"");
    bzero(buffer1, MAX); 
	bzero(mensagem.dados,128);
    criaMensagem(0,0,FIM,aux, &mensagem);
	
    msgToBuffer(&mensagem,(unsigned char*) buffer1);
	send(meuSock, buffer1, sizeof(buffer1), 0);
	fclose(arquivo);

}

void getCliente (int meuSock, msg_t *mensagem) {
	int qtdMsgs = 0;
	int qtdEnvios = 0;
	int restoMsgs = 0;
	int resto = 0;
	msg_t recebida;
	long tam;
	uint8_t crc;
	unsigned char buffer1[MAX], buffer2[MAX];
	bzero(buffer1,MAX);
	bzero(buffer2,MAX);
    bzero(recebida.dados, 128);
    char aux2[1];
    strcpy(aux2,"");
    FILE *arquivo;
    char dirCorrente[100];
    long tamDisponivel;
    char *nome = malloc(mensagem->tam * sizeof(char));
    strcpy(nome,mensagem->dados);

	if (getcwd(dirCorrente, sizeof(dirCorrente)) != NULL)
		tamDisponivel = tamanhoDisponivel(dirCorrente);
    
    msgToBuffer(mensagem, (unsigned char *) buffer1);
    buffer1[5+mensagem->tam] = calc_crc(mensagem);
    send(meuSock, buffer1, sizeof(buffer1), 0); 

    
    recv(meuSock, buffer2, sizeof(buffer2), 0);
    timeout(buffer2,meuSock);
    bufferToMsg(&recebida, (unsigned char *) buffer2);

    while((recebida.tipo != NACK) && (recebida.tipo != TAM) && (recebida.tipo != ERRO)){
	    
	    recv(meuSock, buffer2, sizeof(buffer2), 0);
	    timeout(buffer2,meuSock);
	    bufferToMsg(&recebida, (unsigned char *) buffer2);
    }

    bzero(buffer1, MAX); 
    bzero(buffer2, MAX);
    if(recebida.tipo == NACK){
	    while(recebida.tipo == NACK){
	    	send(meuSock, buffer1, sizeof(buffer1), 0);
	        recv(meuSock, buffer2, sizeof(buffer2), 0);
	        timeout(buffer2,meuSock);
	        bzero(recebida.dados, 128);
	        bufferToMsg(&recebida, (unsigned char *) buffer2);
	    }    
    }
    else if(recebida.tipo == ERRO){
    	char *erro = recebida.dados;
    	if(strcmp(erro, "1") == 0){
       		puts("Arquivo inexistente.");
       		return;
    	}
       	else if(strcmp(erro, "2") == 0){
       		puts("Voce nao tem permissao para acessar esse arquivo.");
       		return;
       	}
    }
    else if(recebida.tipo == TAM){
    	crc = calc_crc(&recebida);
		if(crc == recebida.crc){
    		tam = atoi(recebida.dados);
			bzero(mensagem->dados, 128);
    		if (tam > tamDisponivel){
    			criaMensagem (1,0,ERRO,"3",mensagem);
    		}
    		else{
				criaMensagem (0,0,OK,aux2,mensagem);
    		}
	    	msgToBuffer(mensagem, (unsigned char *) buffer1);
	    	buffer1[5+mensagem->tam] = calc_crc(mensagem);
	    	send(meuSock, buffer1, sizeof(buffer1), 0);
	    	
			qtdMsgs = (tam + 1)/(long)128;
			qtdEnvios = qtdMsgs/3;
			restoMsgs = qtdMsgs % 3;
			resto = tam % 128;
    	}
    	else{
    		bzero(mensagem->dados, 128);
    		criaMensagem (0,0,NACK,aux2,mensagem);
	    	msgToBuffer(mensagem, (unsigned char *) buffer1);
	    	buffer1[5+mensagem->tam] = calc_crc(mensagem);
	    	send(meuSock, buffer1, sizeof(buffer1), 0);	
	    }    	
	}
	
	bzero(buffer2,MAX);
    bzero(recebida.dados, 128);
    bzero(buffer1,MAX);

    arquivo = fopen(nome, "wb+");

    if (!arquivo){
    	puts("Arquivo nao pode ser criado");
    	exit(0);
    }

	unsigned char buff_m1[MAX], buff_m2[MAX], buff_m3[MAX];
	msg_t msg1, msg2, msg3;
	uint8_t crc_m1, crc_m2, crc_m3;

	for (int j = 0; j < qtdEnvios; ++j){
		
		
		recv(meuSock, buff_m1, sizeof(buff_m1), 0);
		timeout(buff_m1,meuSock);
		bufferToMsg(&msg1, (unsigned char *) buff_m1);
		while(msg1.tipo != DADOS){
			
			recv(meuSock, buff_m1, sizeof(buff_m1), 0);
			timeout(buff_m1,meuSock);
			bufferToMsg(&msg1, (unsigned char *) buff_m1);
		}
		crc_m1 = calc_crc(&msg1);
		
		recv(meuSock, buff_m2, sizeof(buff_m2), 0);
		timeout(buff_m2,meuSock);
		bufferToMsg(&msg2, (unsigned char *) buff_m2);
		while(msg2.crc == msg1.crc){
			
			recv(meuSock, buff_m2, sizeof(buff_m2), 0);
			timeout(buff_m2,meuSock);
			bufferToMsg(&msg2, (unsigned char *) buff_m2);
		}
		crc_m2 = calc_crc(&msg2);
		
		recv(meuSock, buff_m3, sizeof(buff_m3), 0);
		timeout(buff_m3,meuSock);
		bufferToMsg(&msg3, (unsigned char *) buff_m3);
		while(msg3.crc == msg2.crc){
			
			recv(meuSock, buff_m3, sizeof(buff_m3), 0);
			timeout(buff_m3,meuSock);
			bufferToMsg(&msg3, (unsigned char *) buff_m3);
		}
		crc_m3 = calc_crc(&msg3);
		
		while((crc_m1 != msg1.crc) || (crc_m2 != msg2.crc) || (crc_m3 != msg3.crc)){
			bzero(mensagem->dados, 128);
    		criaMensagem (0,0,NACK,aux2,mensagem);
	    	msgToBuffer(mensagem, (unsigned char *) buffer1);
	    	buffer1[5+mensagem->tam] = calc_crc(mensagem);
	    	send(meuSock, buffer1, sizeof(buffer1), 0);	

			
			recv(meuSock, buff_m1, sizeof(buff_m1), 0);
			timeout(buff_m1,meuSock);
			bufferToMsg(&msg1, (unsigned char *) buff_m1);
			while(msg1.crc == msg3.crc){
				
				recv(meuSock, buff_m1, sizeof(buff_m1), 0);
				timeout(buff_m1,meuSock);
				bufferToMsg(&msg1, (unsigned char *) buff_m1);
			}
			crc_m1 = calc_crc(&msg1);
			
			recv(meuSock, buff_m2, sizeof(buff_m2), 0);
			timeout(buff_m2,meuSock);
			bufferToMsg(&msg2, (unsigned char *) buff_m2);
			while(msg2.crc == msg1.crc){
				
				recv(meuSock, buff_m2, sizeof(buff_m2), 0);
				timeout(buff_m2,meuSock);
				bufferToMsg(&msg2, (unsigned char *) buff_m2);
			}
			crc_m2 = calc_crc(&msg2);
			
			
			recv(meuSock, buff_m3, sizeof(buff_m3), 0);
			timeout(buff_m3,meuSock);
			bufferToMsg(&msg3, (unsigned char *) buff_m3);
			while(msg3.crc == msg2.crc){
				
				recv(meuSock, buff_m3, sizeof(buff_m3), 0);
				timeout(buff_m3,meuSock);
				bufferToMsg(&msg3, (unsigned char *) buff_m3);
			}
			crc_m3 = calc_crc(&msg3);
			}
	
		fwrite(msg1.dados, sizeof(char), 128, arquivo);
		fwrite(msg2.dados, sizeof(char), 128, arquivo);
		fwrite(msg3.dados, sizeof(char), 128, arquivo);
		
		bzero(buffer1,MAX);
		bzero(mensagem->dados, 128);
		criaMensagem (0,0,ACK,aux2,mensagem);
    	msgToBuffer(mensagem, (unsigned char *) buffer1);
    	buffer1[5+mensagem->tam] = calc_crc(mensagem);
    	send(meuSock, buffer1, sizeof(buffer1), 0);
	}

	for (int i = 0; i < restoMsgs; ++i){
		bzero(buffer2, MAX);
		
		recv(meuSock, buffer2, sizeof(buffer2), 0);
		timeout(buffer2,meuSock);
		bufferToMsg(&msg1, (unsigned char *) buffer2);
		
		while(msg1.tipo != DADOS){
			bzero(buffer2, MAX);
			bzero(msg1.dados,128);
			recv(meuSock, buffer2, sizeof(buffer2), 0);
			timeout(buffer2,meuSock);
			bufferToMsg(&msg1, (unsigned char *) buffer2);
		}
		crc_m1 = calc_crc(&msg1);

		while(crc_m1 != msg1.crc){
			bzero(mensagem->dados, 128);
    		criaMensagem (0,0,NACK,aux2,mensagem);
	    	msgToBuffer(mensagem, (unsigned char *) buffer1);
	    	buffer1[5+mensagem->tam] = calc_crc(mensagem);
	    	send(meuSock, buffer1, sizeof(buffer1), 0);
	    	bzero(buffer2, MAX);
			bzero(msg1.dados,128);
			recv(meuSock, buffer2, sizeof(buffer2), 0);
			timeout(buffer2,meuSock);
			bufferToMsg(&msg1, (unsigned char *) buffer2);
			crc_m1 = calc_crc(&msg1);
		}

		fwrite(msg1.dados, sizeof(char), 128, arquivo);

		bzero(buffer1,MAX);
		bzero(mensagem->dados, 128);
		criaMensagem (0,0,ACK,aux2,mensagem);
    	msgToBuffer(mensagem, (unsigned char *) buffer1);
    	buffer1[5+mensagem->tam] = calc_crc(mensagem);
    	send(meuSock, buffer1, sizeof(buffer1), 0);	
	}

	bzero(buffer2,MAX);
	bzero(recebida.dados,128);
	
	recv(meuSock, buffer2, sizeof(buffer2), 0);
	timeout(buffer2,meuSock);
	bufferToMsg(&recebida, (unsigned char *) buffer2);
	
	while(recebida.tam != resto){
		bzero(buffer2,MAX);
		bzero(recebida.dados,128);
		recv(meuSock, buffer2, sizeof(buffer2), 0);
		timeout(buffer2,meuSock);
		bufferToMsg(&recebida, (unsigned char *) buffer2);
	}
	
	
	crc_m1 = calc_crc(&recebida);
	
	while(crc_m1 != recebida.crc){
		
		bzero(mensagem->dados, 128);
		criaMensagem (0,0,NACK,aux2,mensagem);
    	msgToBuffer(mensagem, (unsigned char *) buffer1);
    	buffer1[5+mensagem->tam] = calc_crc(mensagem);
    	send(meuSock, buffer1, sizeof(buffer1), 0);	
    	bzero(recebida.dados,128);
    	bzero(buffer2,MAX);
    	
    	recv(meuSock, buffer2, sizeof(buffer2), 0);
    	timeout(buffer2,meuSock);
		bufferToMsg(&recebida, (unsigned char *) buffer2);
		crc_m1 = calc_crc(&recebida);
	}

	fwrite(recebida.dados, sizeof(char), resto, arquivo);

	bzero(mensagem->dados, 128);
	bzero(buffer1,MAX);
	criaMensagem (0,0,ACK,aux2,mensagem);
	msgToBuffer(mensagem, (unsigned char *) buffer1);
	buffer1[5+mensagem->tam] = calc_crc(mensagem);
	send(meuSock, buffer1, sizeof(buffer1), 0);	
	
	bzero(buffer2,MAX);
	bzero(recebida.dados,128);
	
	recv(meuSock, buffer2, sizeof(buffer2), 0);
	timeout(buffer2,meuSock);
	bufferToMsg(&recebida, (unsigned char *) buffer2);
	
	while(recebida.tam != 0){
		bzero(buffer2,MAX);
		bzero(recebida.dados,128);
		
		recv(meuSock, buffer2, sizeof(buffer2), 0);
		timeout(buffer2,meuSock);
		bufferToMsg(&recebida, (unsigned char *) buffer2);
	} 
    	
    fclose(arquivo);

    bzero(buffer1, MAX); 
    bzero(buffer2, MAX);
    bzero(recebida.dados, 128);

    return;
}

void putCliente (int meuSock, char *origem, msg_t *msg) {
	
	FILE *arquivo;
	char *aux = malloc((sizeof(origem)+3)*sizeof(char));	
	msg_t mensagem, recebida;
    unsigned char buffer1[MAX], buffer2[MAX];
    strcpy(aux, "");
   
    bzero(buffer1, MAX); 
    bzero(buffer2, MAX);    

	arquivo = fopen(origem, "rb+");
	char c, *arq = malloc(1000000);
	int i = 0; 

	strcpy(aux, "");
	if(!arquivo){
        if(errno == ENOENT){
        	puts("Arquivo nao existe.");
        	return;
        }
        if (errno == EACCES){
        	puts("Voce nao tem permissao para acessar esse arquivo.");
        	return;
        }
	}

    msgToBuffer(msg, (unsigned char *) buffer1);
    buffer1[5+msg->tam] = calc_crc(msg);
    send(meuSock, buffer1, sizeof(buffer1), 0); 

    recv(meuSock, buffer2, sizeof(buffer2), 0);
    timeout(buffer2,meuSock);
	bufferToMsg(&recebida, (unsigned char *) buffer2);
    while((recebida.tipo != OK) && (recebida.tipo != NACK)){
    	recv(meuSock, buffer2, sizeof(buffer2), 0);
    	timeout(buffer2,meuSock);
		bufferToMsg(&recebida, (unsigned char *) buffer2);
    }

    if(recebida.tipo == NACK){
	    while(recebida.tipo == NACK){
	    	send(meuSock, buffer1, sizeof(buffer1), 0);
	        bzero(recebida.dados, 128);
	        bufferToMsg(&recebida, (unsigned char *) buffer2);
	    }    	
    }
  
	c = getc(arquivo);
	while (!feof(arquivo)){
		arq[i] = c;
		c = getc(arquivo);
		i++;
	}
	char *s = malloc(i*sizeof(char));
	sprintf(s,"%d",i);

    criaMensagem(strlen(s),0,TAM,s,&mensagem); 
    msgToBuffer(&mensagem, (unsigned char *) buffer1);
    buffer1[5+mensagem.tam] = calc_crc(&mensagem);
    send(meuSock, buffer1, sizeof(buffer1), 0);

	recv(meuSock, buffer2, sizeof(buffer2), 0);
	timeout(buffer2,meuSock);
    bufferToMsg(&recebida, (unsigned char *) buffer2);
    
    while(recebida.tipo == NACK){
    	send(meuSock, buffer1, sizeof(buffer1), 0);
	    recv(meuSock, buffer2, sizeof(buffer2), 0);
	    timeout(buffer2,meuSock);
	    bufferToMsg(&recebida, (unsigned char *) buffer2);
    }
    
    while ((recebida.tipo != OK) && (recebida.tipo != ERRO)){
		
		recv(meuSock, buffer2, sizeof(buffer2), 0);
		timeout(buffer2,meuSock);
	    bufferToMsg(&recebida, (unsigned char *) buffer2);
    	
    	if (recebida.tipo == ERRO) {
    		puts("Não há espaço suficiente para receber o arquivo.");
    		return;
    	}
    }

    bzero(buffer1, MAX); 
    bzero(buffer2, MAX);

	int qtdMsgs = (i + 1)/128;
	int qtdEnvios = qtdMsgs/3;
	int restoMsgs = qtdMsgs % 3;
	int resto = (i) % 128;
	int seq = 0;

	aux = arq;

	for (int j = 0; j < qtdEnvios; ++j){
		criaMensagem(128, seq, DADOS, aux, &mensagem);
		msgToBuffer(&mensagem, (unsigned char *) buffer1);
	    buffer1[5+mensagem.tam] = calc_crc(&mensagem);
		send(meuSock, buffer1, sizeof(buffer1), 0);
		
		criaMensagem(128, seq+1, DADOS, aux+128, &mensagem);
		msgToBuffer(&mensagem, (unsigned char *) buffer1);
	    buffer1[5+mensagem.tam] = calc_crc(&mensagem);
		send(meuSock, buffer1, sizeof(buffer1), 0);	
		
		criaMensagem(128, seq+2, DADOS, aux+256, &mensagem);
		msgToBuffer(&mensagem, (unsigned char *) buffer1);
	    buffer1[5+mensagem.tam] = calc_crc(&mensagem);
		send(meuSock, buffer1, sizeof(buffer1), 0);

		bzero(buffer2, MAX);
		recv(meuSock, buffer2, sizeof(buffer2), 0);
		timeout(buffer2,meuSock);
        bzero(recebida.dados, 128);
        bufferToMsg(&recebida, (unsigned char *) buffer2);

        while(recebida.tipo == NACK){

        	criaMensagem(128, seq, DADOS, aux, &mensagem);
			msgToBuffer(&mensagem, (unsigned char *) buffer1);
		    buffer1[5+mensagem.tam] = calc_crc(&mensagem);
			send(meuSock, buffer1, sizeof(buffer1), 0);
			
			criaMensagem(128, seq+1, DADOS, aux+128, &mensagem);
			msgToBuffer(&mensagem, (unsigned char *) buffer1);
		    buffer1[5+mensagem.tam] = calc_crc(&mensagem);
			send(meuSock, buffer1, sizeof(buffer1), 0);

			criaMensagem(128, seq+2, DADOS, aux+256, &mensagem);
			msgToBuffer(&mensagem, (unsigned char *) buffer1);
		    buffer1[5+mensagem.tam] = calc_crc(&mensagem);
			send(meuSock, buffer1, sizeof(buffer1), 0);

			bzero(buffer2, MAX);
			recv(meuSock, buffer2, sizeof(buffer2), 0);
			timeout(buffer2,meuSock);
	        bzero(recebida.dados, 128);
	        bufferToMsg(&recebida, (unsigned char *) buffer2);
        }
        seq = (seq+3)%32;
		aux += 384;
	}

	for (int j = 0; j < restoMsgs; ++j){
		bzero(buffer1, MAX);
		criaMensagem(128, seq, DADOS, aux, &mensagem);
		msgToBuffer(&mensagem, (unsigned char *) buffer1);
	    buffer1[132] = calc_crc(&mensagem);
		send(meuSock, buffer1, sizeof(buffer1), 0);
		bzero(buffer2, MAX);
		recv(meuSock, buffer2, sizeof(buffer2), 0);
		timeout(buffer2,meuSock);
        bzero(recebida.dados, 128);
        bufferToMsg(&recebida, (unsigned char *) buffer2);


        while(recebida.tipo == NACK){

        	criaMensagem(128, seq, DADOS, aux, &mensagem);
			msgToBuffer(&mensagem, (unsigned char *) buffer1);
		    buffer1[5+mensagem.tam] = calc_crc(&mensagem);
			send(meuSock, buffer1, sizeof(buffer1), 0);
			
			bzero(buffer2, MAX);
			recv(meuSock, buffer2, sizeof(buffer2), 0);
			timeout(buffer2,meuSock);
    	    bzero(recebida.dados, 128);
        	bufferToMsg(&recebida, (unsigned char *) buffer2);
        }

        seq = (seq+3)%32;
		aux += 128;
	}
	
	bzero(buffer1, MAX); 
	bzero(mensagem.dados,128);
	
	criaMensagem(resto, seq, DADOS, aux, &mensagem);
	msgToBuffer(&mensagem, (unsigned char *) buffer1);
    buffer1[5+mensagem.tam] = calc_crc(&mensagem);
	send(meuSock, buffer1, sizeof(buffer1), 0);
	bzero(buffer2, MAX);
	
	recv(meuSock, buffer2, sizeof(buffer2), 0);
	timeout(buffer2,meuSock);
    bzero(recebida.dados, 128);
    bufferToMsg(&recebida, (unsigned char *) buffer2);

    while(recebida.tipo == NACK){
    	criaMensagem(resto, seq, DADOS, aux, &mensagem);
		msgToBuffer(&mensagem, (unsigned char *) buffer1);
	    buffer1[5+mensagem.tam] = calc_crc(&mensagem);
		send(meuSock, buffer1, sizeof(buffer1), 0);
		bzero(buffer2, MAX);
		recv(meuSock, buffer2, sizeof(buffer2), 0);
		timeout(buffer2,meuSock);
	    bzero(recebida.dados, 128);
    	bufferToMsg(&recebida, (unsigned char *) buffer2);
    }

    strcpy(aux,"");
    bzero(buffer1, MAX); 
	bzero(mensagem.dados,128);
    criaMensagem(0,0,FIM,aux, &mensagem);
    msgToBuffer(&mensagem,(unsigned char*) buffer1);
	send(meuSock, buffer1, sizeof(buffer1), 0);

	fclose(arquivo);
}

void putServidor (int meuSock, msg_t *mensagem) {
	
	int qtdMsgs = 0;
	int qtdEnvios = 0;
	int restoMsgs = 0;
	int resto = 0;
	msg_t recebida;
	long tam;
	uint8_t crc;
	unsigned char buffer1[MAX], buffer2[MAX];
	bzero(buffer1,MAX);
	bzero(buffer2,MAX);
    bzero(recebida.dados, 128);
    char aux2[1];
    strcpy(aux2,"");
    FILE *arquivo;
    char dirCorrente[100];
    long tamDisponivel;
    char *nome = malloc(mensagem->tam * sizeof(char));
    strcpy(nome,mensagem->dados);

	if (getcwd(dirCorrente, sizeof(dirCorrente)) != NULL)
		tamDisponivel = tamanhoDisponivel(dirCorrente);
    
   	crc = calc_crc(mensagem);
	recebida.crc = mensagem->crc;
    
    strcpy(aux2, "");

    while(crc != recebida.crc){
      	criaMensagem (0,0,NACK,aux2,mensagem);
	    msgToBuffer(mensagem, (unsigned char *) buffer1);
	    buffer1[5+mensagem->tam] = calc_crc(mensagem);
		send(meuSock, buffer1, sizeof(buffer1), 0);
	    recv(meuSock, buffer2, sizeof(buffer2), 0);
	    bufferToMsg(&recebida, (unsigned char *) buffer2);
	    crc = calc_crc(&recebida);
	}
	
	criaMensagem(0,0,OK,aux2,mensagem);
	msgToBuffer(mensagem, (unsigned char *) buffer1);
	buffer1[5+mensagem->tam] = calc_crc(mensagem);
	send(meuSock, buffer1, sizeof(buffer1), 0);
	
	recv(meuSock, buffer2, sizeof(buffer2), 0);
	bufferToMsg(&recebida, (unsigned char *) buffer2);
	
	while(recebida.tipo != TAM){
		recv(meuSock, buffer2, sizeof(buffer2), 0);
		bufferToMsg(&recebida, (unsigned char *) buffer2);
	}

	crc = calc_crc(&recebida);
	if(crc == recebida.crc){
		tam = atoi(recebida.dados);
		bzero(mensagem->dados, 128);
		if (tam > tamDisponivel){
			criaMensagem (1,0,ERRO,"3",mensagem);
		}
		else{
			criaMensagem (0,0,OK,aux2,mensagem);
		}
    	msgToBuffer(mensagem, (unsigned char *) buffer1);
    	buffer1[5+mensagem->tam] = calc_crc(mensagem);
    	send(meuSock, buffer1, sizeof(buffer1), 0);
		qtdMsgs = (tam + 1)/(long)128;
		qtdEnvios = qtdMsgs/3;
		restoMsgs = qtdMsgs % 3;
		resto = tam % 128;
	}
	else{
		bzero(mensagem->dados, 128);
		criaMensagem (0,0,NACK,aux2,mensagem);
    	msgToBuffer(mensagem, (unsigned char *) buffer1);
    	buffer1[5+mensagem->tam] = calc_crc(mensagem);
    	send(meuSock, buffer1, sizeof(buffer1), 0);	
    }

	bzero(buffer2,MAX);
    bzero(recebida.dados, 128);
    bzero(buffer1,MAX);

    arquivo = fopen(nome, "wb+");
	unsigned char buff_m1[MAX], buff_m2[MAX], buff_m3[MAX];
	msg_t msg1, msg2, msg3;
	uint8_t crc_m1, crc_m2, crc_m3;

	for (int j = 0; j < qtdEnvios; ++j){
		
		recv(meuSock, buff_m1, sizeof(buff_m1), 0);
		bufferToMsg(&msg1, (unsigned char *) buff_m1);
		while(msg1.tipo != DADOS){
			recv(meuSock, buff_m1, sizeof(buff_m1), 0);
			bufferToMsg(&msg1, (unsigned char *) buff_m1);
		}
		crc_m1 = calc_crc(&msg1);
		
		recv(meuSock, buff_m2, sizeof(buff_m2), 0);
		bufferToMsg(&msg2, (unsigned char *) buff_m2);
		while(msg2.crc == msg1.crc){
			recv(meuSock, buff_m2, sizeof(buff_m2), 0);
			bufferToMsg(&msg2, (unsigned char *) buff_m2);
		}
		crc_m2 = calc_crc(&msg2);
		
		recv(meuSock, buff_m3, sizeof(buff_m3), 0);
		bufferToMsg(&msg3, (unsigned char *) buff_m3);
		while(msg3.crc == msg2.crc){
			recv(meuSock, buff_m3, sizeof(buff_m3), 0);
			bufferToMsg(&msg3, (unsigned char *) buff_m3);
		}
		crc_m3 = calc_crc(&msg3);
		
		while((crc_m1 != msg1.crc) || (crc_m2 != msg2.crc) || (crc_m3 != msg3.crc)){
			bzero(mensagem->dados, 128);
    		criaMensagem (0,0,NACK,aux2,mensagem);
	    	msgToBuffer(mensagem, (unsigned char *) buffer1);
	    	buffer1[5+mensagem->tam] = calc_crc(mensagem);
	    	send(meuSock, buffer1, sizeof(buffer1), 0);	

			recv(meuSock, buff_m1, sizeof(buff_m1), 0);
			bufferToMsg(&msg1, (unsigned char *) buff_m1);
			while(msg1.crc == msg3.crc){
				recv(meuSock, buff_m1, sizeof(buff_m1), 0);
				bufferToMsg(&msg1, (unsigned char *) buff_m1);
			}
			crc_m1 = calc_crc(&msg1);
			
			recv(meuSock, buff_m2, sizeof(buff_m2), 0);
			bufferToMsg(&msg2, (unsigned char *) buff_m2);
			while(msg2.crc == msg1.crc){
				recv(meuSock, buff_m2, sizeof(buff_m2), 0);
				bufferToMsg(&msg2, (unsigned char *) buff_m2);
			}
			crc_m2 = calc_crc(&msg2);
			
			recv(meuSock, buff_m3, sizeof(buff_m3), 0);
			bufferToMsg(&msg3, (unsigned char *) buff_m3);
			while(msg3.crc == msg2.crc){
				recv(meuSock, buff_m3, sizeof(buff_m3), 0);
				bufferToMsg(&msg3, (unsigned char *) buff_m3);
			}
			crc_m3 = calc_crc(&msg3);
			}
	
		fwrite(msg1.dados, sizeof(char), 128, arquivo);
		fwrite(msg2.dados, sizeof(char), 128, arquivo);
		fwrite(msg3.dados, sizeof(char), 128, arquivo);
		
		bzero(buffer1,MAX);
		bzero(mensagem->dados, 128);
		criaMensagem (0,0,ACK,aux2,mensagem);
    	msgToBuffer(mensagem, (unsigned char *) buffer1);
    	buffer1[5+mensagem->tam] = calc_crc(mensagem);
    	send(meuSock, buffer1, sizeof(buffer1), 0);
	}

	for (int i = 0; i < restoMsgs; ++i){
		bzero(buffer2, MAX);
		recv(meuSock, buffer2, sizeof(buffer2), 0);
		bufferToMsg(&msg1, (unsigned char *) buffer2);
		
		while(msg1.tipo != DADOS){
			bzero(buffer2, MAX);
			bzero(msg1.dados,128);
			recv(meuSock, buffer2, sizeof(buffer2), 0);
			bufferToMsg(&msg1, (unsigned char *) buffer2);
		}
		crc_m1 = calc_crc(&msg1);

		while(crc_m1 != msg1.crc){
			bzero(mensagem->dados, 128);
    		criaMensagem (0,0,NACK,aux2,mensagem);
	    	msgToBuffer(mensagem, (unsigned char *) buffer1);
	    	buffer1[5+mensagem->tam] = calc_crc(mensagem);
	    	send(meuSock, buffer1, sizeof(buffer1), 0);
	    	bzero(buffer2,MAX);
	    	recv(meuSock, buffer2, sizeof(buffer2), 0);
			bufferToMsg(&msg1, (unsigned char *) buffer2);
			crc_m1 = calc_crc(&msg1);

		}

		fwrite(msg1.dados, sizeof(char), 128, arquivo);

		bzero(buffer1,MAX);
		bzero(mensagem->dados, 128);
		criaMensagem (0,0,ACK,aux2,mensagem);
    	msgToBuffer(mensagem, (unsigned char *) buffer1);
    	buffer1[5+mensagem->tam] = calc_crc(mensagem);
    	send(meuSock, buffer1, sizeof(buffer1), 0);	
	}	
	
	bzero(buffer2,MAX);
	bzero(recebida.dados,128);
	recv(meuSock, buffer2, sizeof(buffer2), 0);
	bufferToMsg(&recebida, (unsigned char *) buffer2);
	
	while(recebida.tam != resto){
		bzero(buffer2,MAX);
		bzero(recebida.dados,128);
		recv(meuSock, buffer2, sizeof(buffer2), 0);
		bufferToMsg(&recebida, (unsigned char *) buffer2);
	}
	
	crc_m1 = calc_crc(&recebida);
	while(crc_m1 != recebida.crc){
		bzero(mensagem->dados, 128);
		criaMensagem (0,0,NACK,aux2,mensagem);
    	msgToBuffer(mensagem, (unsigned char *) buffer1);
    	buffer1[5+mensagem->tam] = calc_crc(mensagem);
    	send(meuSock, buffer1, sizeof(buffer1), 0);	
    	bzero(recebida.dados,128);
    	bzero(buffer2,MAX);
    	recv(meuSock, buffer2, sizeof(buffer2), 0);
		bufferToMsg(&recebida, (unsigned char *) buffer2);
		crc_m1 = calc_crc(&recebida);
	}

	fwrite(recebida.dados, sizeof(char), resto, arquivo);

	bzero(mensagem->dados, 128);
	bzero(buffer1,MAX);
	criaMensagem (0,0,ACK,aux2,mensagem);
	msgToBuffer(mensagem, (unsigned char *) buffer1);
	buffer1[5+mensagem->tam] = calc_crc(mensagem);
	send(meuSock, buffer1, sizeof(buffer1), 0);	
	
	bzero(buffer2,MAX);
	bzero(recebida.dados,128);
	recv(meuSock, buffer2, sizeof(buffer2), 0);
	bufferToMsg(&recebida, (unsigned char *) buffer2);
	
	while(recebida.tam != 0){
		bzero(buffer2,MAX);
		bzero(recebida.dados,128);
		recv(meuSock, buffer2, sizeof(buffer2), 0);
		bufferToMsg(&recebida, (unsigned char *) buffer2);
	} 
    	
    fclose(arquivo);

    bzero(buffer1, MAX); 
    bzero(buffer2, MAX);
    bzero(recebida.dados, 128);

    return;
}