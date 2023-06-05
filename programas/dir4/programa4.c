#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>

int main(int argc, char *argv[]) {
        int fd=open("Makefile", O_RDWR);
        char *p=mmap(NULL, getpagesize(), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	close(fd);
        printf("%d %p\n", getpid(), p);

        printf("antes de acceder; pulse para continuar\n");
        getchar();
	argc=*p; // acceso de lectura
        printf("después de acceso de lectura; pulse para continuar\n");
        getchar();
        *p =' '; // acceso de escritura
        printf("después de escritura; pulse para terminar\n");
        getchar();
	munmap(p, getpagesize());
	return 0;
}
