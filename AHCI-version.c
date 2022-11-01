#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

//estructura de datos donde meter los datos del AHCI
struct lapic_info {
	uint32_t reservado[4];
	uint16_t minor;
	uint16_t major;
};

int main(int argc, char *argv[]) {

	//revisar numero de argumentos
	if (argc != 2) {
		printf("Introduzca un argumento.\n");
		return -1;
	}

	//variables locales
	int fd, tam=4096, major = 0, minor = 0;
	struct lapic_info volatile *lapic;
	unsigned long num = 0;
	
	//tratar valor introducido
	if (argv[1][0] == '0' && argv[1][1] == 'x') {
		//hexadecimal
		num = strtoul(argv[1],NULL,16);
	} else if (argv[1][0] == '0' && argv[1][1] == '0') {
		//octal
		num = strtoul(argv[1],NULL,8);
	} else {
		//decimal
		num = strtoul(argv[1],NULL,0);
	}

	//abrir fichero /dev/mem
	if ((fd = open("/dev/mem", O_RDONLY|O_DSYNC)) < 0) {
		perror("open"); 
		return 1; 
	}

	// obtiene rango dir. lógicas usuario asociadas a dir. físicas dispositivo
	if ((lapic = mmap(NULL, tam, PROT_READ, MAP_SHARED, fd, num)) == MAP_FAILED) {
		perror("mmap"); 
		return 1; 
	}

	//obtener datos relevantes
	major = lapic->major;
	minor = 0;

	//filtrar minor
	switch (lapic->minor) {
		case 0x905: minor = 95; break;
		case 0x100: minor = 1; break;
		case 0x200: minor = 2; break;
		case 0x300: minor = 3; break;
		case 0x301: minor = 3.1; break;
	}


	//printf("Acceso dir física %lx usando %p\n", num, lapic);
	printf("Versión %d.%d\n", major, minor);

	close(fd); 
	munmap((void *)lapic, tam);
	return 0;
}
