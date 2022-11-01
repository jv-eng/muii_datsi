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


/*comandos
sudo ./programacion_dispositivos/programas/getPCI-b0-s0-f0 1 6 1
sudo lspci -nns 00:00.0
*/
int main(int argc, char *argv[]) {

	//comprobar argumentos
	if (argc != 4) {
		printf("Introduzca argumentos.\n");
		return -1;
	}

	//variables locales
	uint32_t dir, dat; //poner aqui las variables para leer cosas ¿struct?
	int vend, prod, clase = atoi(argv[1]), subclase = atoi(argv[2]), interfaz = atoi(argv[3]);

	// permiso para acceso a los 2 puertos modo usuario
	if (ioperm(CONFIG_DIR, 8, 1) < 0) { 
        perror("ioperm"); 
		return 1;
    }


	// al ser bus 0, slot 0, función 0 y registro 0 basta con especificar el "enable" bit
	dir = (uint32_t) 0x80000900;

    outl (dir, CONFIG_DIR);
	dat = inl(CONFIG_DAT); //hacemos la lectura del registro offset 0, hay mas que leer

	if (dat == 0xFFFFFFFF) {
		fprintf(stderr, "no existe ese dispositivo\n"); 
		return 1; 
	}

	vend = dat & 0x0000FFFF; 
	prod = dat >> 16; // extrae vendedor y producto

	//imprimir datos
	printf("Clase: %d | Subclase %d | Interfaz %d\n", clase, subclase, interfaz);
	printf("Bus 0 | Slot 0 | Función 0 | Fabricante %x | Modelo %x\n",
		vend, prod);
	printf("BAR0-IO 0 | BAR1-IO 0 | BAR2-IO 0 | BAR3-IO 0 | BAR4-IO 0 | BAR5-IO 0\n");//bucle

	return 0;
} 
