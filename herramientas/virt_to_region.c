// Obtiene la información de la región asociada a cada dirección virtual de un proceso especificada
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
int main(int argc, char *argv[]) {
        char fich[32];
	FILE *f;
        if (argc<3) {
                fprintf(stderr, "uso: %s PID dir_virtual(hexa)...\n", argv[0]);
                return 1;
        }
        sprintf(fich, "/proc/%s/maps", argv[1]);
        if ((f=fopen(fich, "r"))<0) {
                perror("error abriendo maps");
                return 1;
        }
        char *linea=NULL;
        char * inicio, *fin, *dirv;

	for (int i=2; i<argc; i++) {
		rewind(f);
		dirv = (char *)strtol(argv[i], NULL, 16);
		int cont=0;
		size_t n=0;
        	while (getline(&linea, &n, f)>0) {
                	sscanf(linea, "%p-%p", &inicio, &fin);
			if ((dirv>=inicio) && (dirv<fin)) {
				printf("dir %p pertenece a región %d:\n%s\n",
					dirv, cont, linea);
				break;
			}
			++cont;
		}
	}
	fclose(f);
	return 0;
}






