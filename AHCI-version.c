#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

// Dirección base por defecto de registros MMIO de Local-APIC (/proc/iomem)
#define LOCAL_APIC_BASE_ADDRESS 0xfee00000

//estructura de datos donde meter los datos del AHCI
struct lapic_info {
	uint32_t reservado[8];
	uint16_t minor;
	uint16_t major;
	//uint32_t ID;
	//uint32_t relleno[3];
	//uint32_t version; // nº versión: 8 bits de menor peso
};

int main(int argc, char *argv[]) {

	//revisar numero de argumentos
	if (argc != 2) {
		printf("Introduzca un argumento.\n");
		return -1;
	}

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

	printf("numero introducido: %lx\n",num);

	//abrir fichero /dev/mem
	int fd, tam=4096;
	struct lapic_info volatile *lapic; // volatile elimina optimizaciones de compilador
	if ((fd = open("/dev/mem", O_RDONLY|O_DSYNC)) < 0) { // O_DSYNC: accesos no usan cache
		perror("open"); 
		return 1; 
	}

	// obtiene rango dir. lógicas usuario asociadas a dir. físicas dispositivo
	if ((lapic = mmap(NULL, tam, PROT_READ, MAP_SHARED, fd, num)) == MAP_FAILED) {
		perror("mmap"); 
		return 1; 
	}


	printf("Acceso dir física %lx usando %p\n", num, lapic);
	printf("test minor: %d major: %d\n", lapic->minor, lapic->major);
	//printf("Local APIC ID: %d Versión %d\n", lapic->ID, lapic->version&0xFF);

	close(fd); 
	munmap((void *)lapic, tam);
	return 0;
}
