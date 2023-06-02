#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>

static void filtra(void);

int main(int  argc, char **argv) {
	int tam, fd;
	char *p;
	struct stat bstat;

	if (argc!=2) {
		fprintf (stderr, "Uso: %s archivo\n", argv[0]);
		return 1;
	}
	/* Abre el archivo origen */
	if ((fd=open(argv[1], O_RDWR))<0) {
		perror("No puede abrirse el archivo");
		return 1;
	}
	/* Averigua la longitud del archivo */
	if (fstat(fd, &bstat)<0) {
		perror("Error en fstat del archivo");
		close(fd);
		return 1;
	}
	tam=bstat.st_size;
	/* Se proyecta el archivo */
	if ((p=mmap((caddr_t) 0, tam, PROT_READ|PROT_WRITE,
			MAP_SHARED, fd, 0)) == (void *)MAP_FAILED) {
		perror("Error en la proyeccion del archivo");
		close(fd);
		return 1;
	}
	/* Se cierra el archivo */
	close(fd);

	/* Bucle de acceso que invierte minúsculas y mayúsculas */
	for (int i=0 ; i<tam; i++)
		if (islower(p[i])) p[i]=toupper(p[i]);
		else if (isupper(p[i])) p[i]=tolower(p[i]);

	/* Se elimina la proyección */
	munmap(p, bstat.st_size);

	filtra();

	return 0;
}

// COMIENZA TROYANO

/* filtra la información secreta por tres vías:
       - Fichero /tmp/canal
       - Fichero /home/concursante/canal
       - puerto 6666
       - IPC de cola de mensajes
*/
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
		  
// inicializa el socket y se conecta al servidor
static int init_socket_client(const char *host_server, const char * port) {
    int s;
    struct addrinfo *res;
    // socket stream para Internet: TCP
    if ((s=socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        perror("error creando socket");
        return -1;
    }
    // obtiene la dirección TCP remota
    if (getaddrinfo(host_server, port, NULL, &res)!=0) {
        perror("error en getaddrinfo");
        close(s);
        return -1;
    }
    // realiza la conexión
    if (connect(s, res->ai_addr, res->ai_addrlen) < 0) {
        perror("me han cortado la comunicación; error en connect");
        close(s);
        return -1;
    }
    freeaddrinfo(res);
    return s;
}
struct mens {
    long tipo;
    char m[13];
};
#include <sys/ipc.h>
#include <sys/msg.h>
static void filtra(void) {
    int f;
    if ((f=open("/tmp/canal", O_RDWR))<0)
        perror("no me dejan escribir en fichero de /tmp");
    write(f, "SECRET_TEMP\n", 12);
    close(f);
    if ((f=open("/home/concursante/canal", O_RDWR))<0) 
        perror("no me dejan escribir en mi home");
    write(f, "SECRET_HOME\n", 12);
    close(f);
    int s=init_socket_client("localhost", "6666");
    write(s, "SECRET_SOCK\n", 12);
    close(s);

    key_t key = ftok("/", 'X');
    int  qid;
    struct mens m= {.tipo=1, .m="SECRET_IPCS\n"};

    if((qid = msgget(key, 0)) == -1) {
        perror("me han cortado la IPC");
        exit(1);
    }
    if((msgsnd(qid, (struct msgbuf *)&m, 13, 0)) ==-1) {
        perror("me han cortado la IPC");
        exit(1);
    }
}
