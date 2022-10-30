// Ej. GPIO Raspberry Pi3: arg nº de pin, lo configura como salida, lo pone a 1 y luego a 0.
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#define PI3_PERIF  0x3F000000 // Dirección física de los periféricos en Raspberry Pi 3
#define GPIO_OFF  0x200000 // Desplazamiento de los registros vinculados con GPIO
#define PI3_GPIO_BASE  (PI3_PERIF + GPIO_OFF) // Dirección física de GPIO en Raspberry Pi 3

int main(int argc, char *argv[]) {
	if (argc!=2) { fprintf(stderr, "uso: %s PIN\n", argv[0]); return 1; }
        int fd, tam=4096, pin=atoi(argv[1]);
        uint32_t volatile *gpio;
	uint32_t off_selec=0; // registros de selección de función
	uint32_t off_set=7; // registros de set
	uint32_t off_clear=10; // registros de clear

        if ((fd = open("/dev/mem", O_RDWR|O_DSYNC))<0) {
                perror("open"); return 1; }
        // obtiene rango d. lógicas de usuario asociadas a dir. físicas de dispo,
        if ((gpio = mmap(NULL, tam, PROT_READ|PROT_WRITE, MAP_SHARED, fd, PI3_GPIO_BASE))==MAP_FAILED) {
                perror("mmap"); return 1; }

	// Configura el pin como de salida.
	gpio[off_selec+pin/10] = (gpio[off_selec+pin/10] & ~(7<<(off_selec+pin%10)*3)) | (1 << (off_selec+pin%10)*3);
	
	gpio[off_set+pin/32] = 1 << pin%32; // activa el pin
	
	gpio[off_clear+pin/32] = 1 << pin%32; // desactiva el pin
	
        munmap((void *)gpio, tam); close(fd);
	return 0;
} 
