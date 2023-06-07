#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
int main(int argc, char *argv[]) {
	if (argc!=4) {
		fprintf(stderr, "uso: %s disp_org disp_back tam_bloque\n", argv[0]);
		return 1;
	}
	int tam_bloque = atoi(argv[3]);
	unsigned long buf[tam_bloque/sizeof(long)];
	int f1=open(argv[1], O_RDONLY);
	int f2=creat(argv[2], 0666);
	if ((f1<0) || (f2<0)) {
		fprintf(stderr, "uso: %s disp_org disp_back tam_bloque\n", argv[0]);
		return 1;
	}
	int cont, i, n, leido, resto;
	for (cont=0; ((leido=read(f1, buf, tam_bloque))>0); cont++) {
		if ((resto=leido%sizeof(long)))
			memset((char *)(buf)+leido, 0, sizeof(long)-resto);
		n=(leido+sizeof(long)-1)/sizeof(long);
		for (i=0; i<n; i++)
			if (buf[i]) break;
		if (i<n) {
			lseek(f2, cont*tam_bloque, SEEK_SET);
			write(f2, buf, leido);
		}
	}
	ftruncate(f2, cont*tam_bloque);
	close(f1);
	close(f2);
}
