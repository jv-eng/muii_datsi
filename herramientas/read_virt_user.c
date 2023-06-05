// Imprime el valor que contiene una determinada dirección virtual de un cierto
// proceso recibidos ambos como parámetros, así como cuántos bytes se leen

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
int main(int argc, char *argv[]) {
	int f, tam=1;
	uint64_t dirv;
	unsigned char val;
	char fich[32];
	if ((argc!=3) && (argc!=4)) {
		fprintf(stderr, "uso: %s PID dir_logica(hexa) [nºbytes]\n",
				argv[0]);
		return 1;
	}
	if (geteuid()) {
		fprintf(stderr, "Debe ejecutarse como superusuario\n");
		return 1;
	}
	sprintf(fich, "/proc/%s/mem", argv[1]);
	if ((f=open(fich, O_RDONLY))<0) {
		perror("error abriendo mem");
		return 1;
	}
	dirv = strtol(argv[2], NULL, 16);
	if (argc==4) tam=atoi(argv[3]);
	lseek(f, dirv, SEEK_SET);
	printf("%lx:", dirv);
	for (int i=0; i<tam; i++) {
		if (read(f, &val, sizeof(val))!=sizeof(val)) {
			perror("error accediendo a mem");
			return 1;
		}
		printf(" %02x", val);
	}
	printf("\n");
	return 0;
}
