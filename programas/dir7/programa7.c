#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#define dir_pag(d) (unsigned long) d & (~(unsigned long)(getpagesize()-1))

// vector global sin valor inicial
int v[20000];

int main(int argc, char *argv[]) {
	char *p=malloc(20000);

        printf("paso 1: antes de accesos de lectura; pulse para continuar\n");
        printf("%d %lx %lx %lx %lx\n", getpid(), dir_pag(&v[19000]), dir_pag(&v[10000]), dir_pag(&p[10000]), dir_pag(&p[19000]));
	getchar();
	argc=v[19000]; argc=v[10000]; argc=p[19000]; argc=p[10000]; // accesos de lectura
        printf("paso 2: después de accesos de lectura; pulse para terminar\n");
	getchar();
	return 0;
}
