#include <unistd.h>
#include <stdio.h>

#define dir_pag(d) (unsigned long) d & (~(unsigned long)(getpagesize()-1))

// vector global con valor inicial
int v[20000]={[19000]=0X33};

int main(int argc, char *argv[]) {
        printf("%d %lx\n", getpid(), dir_pag(&v[19000]));
        printf("antes de acceder; pulsa para continuar\n");
        getchar();
	argc=v[19000];
        printf("después de acceso de lectura; pulse para continuar\n");
        getchar();
        ++v[19000];
        printf("después de escritura; pulse para terminar\n");
        getchar();
	return 0;
}
