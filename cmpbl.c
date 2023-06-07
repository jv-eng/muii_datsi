#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
int main(int argc, char *argv[]) {
	if (argc!=4) {
		fprintf(stderr, "uso: %s disp1 disp2 tam_bloque\n", argv[0]);
		return 1;
	}
	int tam_bloque = atoi(argv[3]);
	unsigned char buf1[tam_bloque];
	unsigned char buf2[tam_bloque];

	int f1=open(argv[1], O_RDONLY);
	int f2=open(argv[2], O_RDONLY);
	if ((f1<0) || (f2<0)) {
		fprintf(stderr, "uso: %s disp1 disp2\n", argv[0]);
		return 1;
	}
	int cont, i, n;
	for (cont=0; (n=read(f1, buf1, tam_bloque))>0; cont++) {
		if (read(f2, buf2, n)<n) {
			printf("el segundo dispositivo es más pequeño\n");
			break;
		}
		for (i=0; i<n; i++)
			if (buf1[i]!=buf2[i]) break;
		if (i<n)
			printf("bloque %d difiere a partir de byte %d: (%x) (%x)\n",
				cont, i, buf1[i], buf2[i]);
	}
	close(f1);
	close(f2);
}
