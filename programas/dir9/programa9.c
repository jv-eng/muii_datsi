#include <stdio.h>
#include <unistd.h>
#include <signal.h>

#define dir_pag(d) (unsigned long) d & (~(unsigned long)(getpagesize()-1))

void continua(int s) { }
// vector global sin valor inicial
int v[20000];

int main(int argc, char *argv[]) {
	int pid;
	signal(SIGUSR1, continua);
        printf("cuando terminen todos los procesos, acabe el programa con Ctrl-C\n\n");
        printf("%d %lx\n", getpid(), dir_pag(&v[19000]));
	argc=v[19000];
        printf("paso 1: después de acceso de lectura de padre %d; pulse para continuar\n", getpid());
	getchar();
	pid=fork(); // crea hijo
        if (pid==0) {
		pause();
        	++v[19000];
		pause();
	} else {
		printf("paso 2: después de que padre %d crea hijo %d; pulse para continuar padre\n", getpid(), pid);
		getchar();
        	++v[19000];
		printf("paso 3: después de escritura del padre; pulse para continuar hijo\n");
		getchar();
		kill(pid, SIGUSR1);
		usleep(300000);
		printf("paso 4: después de escritura del hijo; pulse para continuar padre\n");
		getchar();
		pid=fork(); // crea segundo hijo
        	if (pid==0) {
			pause();
        		++v[19000];
			pause();
		}
		printf("paso 5: después de que padre %d crea segundo hijo %d; pulse para continuar padre\n", getpid(), pid);
		getchar();
        	++v[19000];
		printf("paso 6: después de segunda escritura del padre; pulse para continuar segundo hijo\n");
		getchar();
		kill(pid, SIGUSR1);
		usleep(300000);
		printf("paso 7: después de escritura del segundo hijo; pulse Control-C para terminar el programa\n");
	}
	pause();
	return 0;
}
