#define _GNU_SOURCE
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

static int fd; // descriptor del fichero de log

// se ejecuta al iniciarse la biblioteca;
// crea el fichero donde se almacenará en formato texto las conexiones
// (IP y puerto) realizadas por el programa
__attribute__((constructor)) void inicio(void){
	fd=creat("connect.log", 0600);
}

int connect(int sd, const struct sockaddr *addr, socklen_t addrlen) {
	// debe escribir en el fichero la información de conexión;
	
	// buscar con dlsym usando RTLD_NEXT la dirección real de connect
	
	// realizar la llamada al connect real
        return 0;
}
