#ifndef __COMANDOS__
#define __COMANDOS__

void lsRemotoServidor (int meuSock, char *destino, msg_t msg);
void lsRemotoCliente (int meuSock, msg_t *mensagem);
void cdRemotoCliente(int meuSock, msg_t *mensagem);
void cdRemotoServidor(msg_t mensagem, int meuSock, char *param);
void getCliente (int meuSock, msg_t *mensagem);
void getServidor (int meuSock, char *destino, msg_t msg);
void putServidor (int meuSock, msg_t *mensagem);
void putCliente (int meuSock, char *origem, msg_t *msg);

#endif