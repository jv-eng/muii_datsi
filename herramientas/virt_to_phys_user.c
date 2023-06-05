// Obtiene las direcciones físicas asociadas a las direcciones virtuales de un proceso
// recibidas como argumentos

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int en_region(int pid, uint64_t dirv){
	char fich[32];
        static FILE *f;

	if (f==NULL) {
        	sprintf(fich, "/proc/%d/maps", pid);
        	if ((f=fopen(fich, "r"))<0) {
                	perror("error abriendo maps");
                	return 1;
        	}
        }
	else rewind(f);
        char *linea=NULL;
        size_t n=0;
        uint64_t inicio, fin;
        while (getline(&linea, &n, f)>0) {
        	sscanf(linea, "%lx-%lx", &inicio, &fin);
                if ((dirv>=inicio) && (dirv<fin)) 
			return 1; // está en una región
        }
	return 0;
}

int main(int argc, char *argv[]) {
	char fich[32];
	int f, tpag, pid;
	uint64_t dirv, dirf, pag, desp, pte;
	if (argc<3) {
		fprintf(stderr, "uso: %s PID dir_virtual(hexa)...\n", argv[0]);
		return 1;
	}
	if (geteuid()) {
		fprintf(stderr, "Debe ejecutarse como superusuario\n");
		return 1;
	}
	sprintf(fich, "/proc/%s/pagemap", argv[1]);
	pid=atoi(argv[1]);
	if ((f=open(fich, O_RDONLY))<0) {
		perror("error abriendo pagemap");
		return 1;
	}
	tpag=getpagesize();
	for (int i=2; i<argc; i++) {
		dirv = strtol(argv[i], NULL, 16);
		if (!en_region(pid, dirv)) {
			printf("dirección inválida\n");
			continue;
		}
		pag = dirv/tpag;
		desp = dirv%tpag;
		lseek(f, pag*sizeof(pte), SEEK_SET);
		if (read(f, &pte, sizeof(pte))!=sizeof(pte)) {
			perror("error accediendo a pagemap");
			return 1;
		}
		if (pte&((unsigned long)1<<63)) {
			dirf= pte*tpag+desp;
			printf("%lx\n", dirf);
		} else if (pte&((unsigned long)1<<62)) {
			printf("no residente: está en swap\n");
		} else 
			printf("no residente: está en soporte original (fichero o anónima)\n");
	}
	return 0;
}
