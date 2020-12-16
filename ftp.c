#include <strings.h>
#include <unistd.h>
#include "rawsocket.h"
#include "utils.h"
#include "cliente.h"
#include "servidor.h"
#include <stdio.h>
#include <stdlib.h>


int main(int argc, char **argv) {

/* Le a linha de comando e valores necessarios */
	if (strcasecmp(argv[1],"servidor") == 0) 
		servidor(argv[2]);
	else if (strcasecmp(argv[1],"cliente") == 0){
		instrucoes();
		cliente(argv[2]);
	}
	else {
		printf("Identificação cliente/servidor\n");
		return (-1);
	}

	return(0);
}
