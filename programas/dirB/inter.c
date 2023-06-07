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

//ejemplo llamada LD_PRELOAD=./inter.so wget fi.upm.es

int connect(int sd, const struct sockaddr *addr, socklen_t addrlen) {
	// debe escribir en el fichero la información de conexión;
	FILE *log_file = fopen("connect.log", "a");
    if (log_file == NULL) {
        perror("Error al abrir el archivo de registro");
        exit(1);
    }

	// Obtener la dirección IP y el puerto de la conexión
    struct sockaddr_in *sin = (struct sockaddr_in *)addr;
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(sin->sin_addr), client_ip, INET_ADDRSTRLEN);
    int client_port = ntohs(sin->sin_port);

	fprintf(log_file, "Conexión realizada. IP: %s, Puerto: %d\n", client_ip, client_port);

    // Cerrar el archivo de registro
    fclose(log_file);

	
	// buscar con dlsym usando RTLD_NEXT la dirección real de connect
	dlsym(RTLD_NEXT, "connect");
	
	// realizar la llamada al connect real
	int (*original_connect)(int sd, const struct sockaddr *addr, socklen_t addrlen) = NULL;
	original_connect = (int (*)(int, const struct sockaddr *, socklen_t))
    dlsym(RTLD_NEXT, "connect");
    if (original_connect == NULL) {
        perror("Error al obtener el puntero a la función connect");
        exit(1);
    }

    return original_connect(sd, addr, addrlen);;
}

