// Modifica el valor que contiene una determinada dirección virtual de un cierto
// proceso recibidos ambos como parámetros, escribiendo los bytes indicados

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
int main(int argc, char *argv[]) {
	int f;
	uint64_t dirv;
	unsigned char val;
	char fich[32];
	if (argc<4) {
		fprintf(stderr, "uso: %s PID dir_logica(hexa) bytes...\n",
				argv[0]);
		return 1;
	}
	if (geteuid()) {
		fprintf(stderr, "Debe ejecutarse como superusuario\n");
		return 1;
	}
	sprintf(fich, "/proc/%s/mem", argv[1]);
	if ((f=open(fich, O_RDWR))<0) {
		perror("error abriendo mem");
		return 1;
	}
	dirv = strtol(argv[2], NULL, 16);
	lseek(f, dirv, SEEK_SET);
	printf("%lx:", dirv);
	for (int i=3; i<argc; i++) {
		val = strtol(argv[i], NULL, 16);
		if (write(f, &val, sizeof(val))!=sizeof(val)) {
			perror("error accediendo a mem");
			return 1;
		}
		printf(" %x", val);
	}
	printf("\n");
	return 0;
}
