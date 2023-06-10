#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <arpa/inet.h>

static int crea_socket(int puerto) {
	int s;
	struct sockaddr_in dir;
	int opcion=1;

	if ((s=socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		perror("error creando socket");
		return -1;
	}
        if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opcion, sizeof(opcion))<0){
                perror("error en setsockopt");
                return -1;
        }
	dir.sin_addr.s_addr=INADDR_ANY;
	dir.sin_port=htons(puerto);
	dir.sin_family=PF_INET;
	if (bind(s, (struct sockaddr *)&dir, sizeof(dir)) < 0) {
		perror("error en bind");
		close(s);
		return -1;
	}
	return s;
}

int main(int argc, char *argv[]) {
	int s, s_conec;
	struct sockaddr_in dir_cliente;
	unsigned int tam_dir;

	if ((s=crea_socket(666))<0) {
		printf("No he podido reservar un puerto privilegiado\n");
		return -1;
	}

	printf("He podido reservar un puerto privilegiado\n");

	if (listen(s, 5) < 0) {
		perror("error en listen");
		close(s);
		return 1;
	}
	tam_dir=sizeof(dir_cliente);
	while(1) {
		if ((s_conec=accept(s, (struct sockaddr *)&dir_cliente, &tam_dir))<0){
			perror("error en accept");
			close(s);
			return 1;
		}
		printf("se ha conectado: %s\n", inet_ntoa(dir_cliente.sin_addr));
		close(s_conec);
	}
	close(s);
	return 0;
}
