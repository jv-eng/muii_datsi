// obtiene fabricante y modelo dispo. PCI bus 0, slot 0 y función 0; http://wiki.osdev.org/PCI
#include <sys/io.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define CONFIG_DIR 0xCF8
/* en CONFIG_DIR se especifica dir. a leer/escribir: bus|slot|func|registro con el formato:
|    31    |(30-24) |(23-16)|(15-11)  |(10-8)     |(7-2)      |(1-0)| 
|Enable Bit|Reserved|BusNum |DeviceNum|FunctionNum|RegisterNum| 0 0 |    */

#define CONFIG_DAT 0xCFC
// Si se lee de CONFIG_DAT se obtiene contenido de registro especificado en CONFIG_DIR
// Si se escribe en CONFIG_DAT se modifica contenido de registro especificado en CONFIG_DIR

void buscar_dispositivo(int clase, int subclase, int interfaz);


void buscar_dispositivo(int clase, int subclse, int interfaz) {
	// permiso para acceso a los 2 puertos modo usuario
	if (ioperm(CONFIG_DIR, 8, 1) < 0) { 
        perror("ioperm"); 
		//return 1;
    }

	//variables locales
	uint32_t dir, dir_check, dat;

	//iniciamos por el primer dispostivo
	dir = (uint32_t) 0x80000000;
	dir_check = (uint32_t)(0x80000000);// | (uint32_t)0x8);
	outl (dir_check, CONFIG_DIR);
	dat = inl(CONFIG_DAT);
	//printf("%x\n\n", dat >> 24);

	while (dat != 0xFFFFFFFF) {
		
		
		printf("%x\n",dir_check);
		printf("%x\n\n", dat >> 16);
		//siguiente iteracion
		dir_check = (uint32_t)(dir_check + (uint32_t)0x800);
		outl (dir_check, CONFIG_DIR);
		dat = inl(CONFIG_DAT);
		
	}
    

	if (dat == 0xFFFFFFFF) {
		fprintf(stderr, "no existe ese dispositivo\n"); 
		//return 1; 
	}

	//vend = dat & 0x0000FFFF; 
	//prod = dat >> 16; // extrae vendedor y producto
}

/*comandos
sudo ./programacion_dispositivos/programas/getPCI-b0-s0-f0 1 6 1
sudo lspci -nns 00:00.0
sudo lspci -nn
*/
int main(int argc, char *argv[]) {

	//comprobar argumentos
	if (argc != 4) {
		printf("Introduzca argumentos.\n");
		return -1;
	}

	//variables locales
	int vend, prod, clase = atoi(argv[1]), subclase = atoi(argv[2]), interfaz = atoi(argv[3]);
	buscar_dispositivo(clase, subclase, interfaz);

	//imprimir datos
	//printf("Clase: %d | Subclase %d | Interfaz %d\n", clase, subclase, interfaz);
	//printf("Bus 0 | Slot 0 | Función 0 | Fabricante %x | Modelo %x\n",
	//	vend, prod);
	//printf("BAR0-IO 0 | BAR1-IO 0 | BAR2-IO 0 | BAR3-IO 0 | BAR4-IO 0 | BAR5-IO 0\n");//bucle

	return 0;
} 
