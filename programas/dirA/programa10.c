#include <unistd.h>
#include <stdio.h>
extern void funcion(void);

int main(int argc, char *argv[]) {
	printf("PID: %d\n", getpid());
	printf("antes de llamar a 'funcion'; compruebe entrada GOT; pulse para continuar\n");
	getchar();
	printf("después de llamar a 'funcion'; compruebe entrada GOT y modifíquela para que apunte a 'otra_funcion'; pulse para continuar\n");
	funcion();
	getchar();
	printf("debería haber llamado a 'otra_funcion'; pulse para terminar\n");
	funcion();
	getchar();
	return 0;
}
