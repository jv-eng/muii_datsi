// informacion bus PCI: http://wiki.osdev.org/PCI
#include <sys/io.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define CONFIG_DIR 0xCF8
#define CONFIG_DAT 0xCFC

//declaracion de funciones
void buscar_disp(uint32_t dir, int clase, int subclase, int interfaz, int * encontrado);
void buscar_funcion(uint32_t dir, int clase, int subclase, int interfaz, int * encontrado);
void imprimir_info(uint32_t dir, int clase, int subclase, int interfaz);



//imprimir informacion
void imprimir_info(uint32_t dir, int clase, int subclase, int interfaz) {
	uint32_t dir_check, info_device, bar;
	int i;

	//obtenemos la posicion del bus
	dir_check =  dir;
	printf("Bus 0x%x Slot 0x%x Func 0x%x ", (dir_check & 0xFF0000) >> 16, (dir_check & 0xF800) >> 11, (dir_check & 0x700) >> 8);

	//obtener el id del dispositivo y del vendedor
	dir_check =  dir;
	outl (dir_check, CONFIG_DIR);
	info_device = inl(CONFIG_DAT);
	printf("Vendedor 0x%x Producto 0x%x ", info_device & 0x0000FFFF, info_device >> 16);

	//obtenemos informacion de clase, subclase, interfaz
	printf("Clase 0x%x Subclase 0x%x Interfaz 0x%x ", clase, subclase, interfaz);
	
	//informacion de los BAR
	dir_check = dir + 0x10;
	for (i = 0; i < 6; i++) {
		outl (dir_check, CONFIG_DIR);
		bar = inl(CONFIG_DAT);
		if ((bar & 0xFFFF0000) != 0x0) { //MMIO
			printf("BAR%d-Mem 0x%x ", i, bar);
		} else { //PIO
			printf("BAR%d-IO 0x%x ", i, bar);
		}

		dir_check = dir_check + (uint32_t)0x4;
	}
	
	//salto de linea
	printf("\n");
}

//realiza la busqueda de un dispositvo PCI
void buscar_funcion(uint32_t dir, int clase, int subclase, int interfaz, int * encontrado) {
	//variables locales
	uint32_t dir_check, dir_pci, dat, check_pci_pci;
	int i = 0, j = 0;
	uint8_t class, subclass, interface;

	//iniciamos por el primer dispostivo
	dir_check = (uint32_t)(dir + (uint32_t)0xc);
	outl (dir_check, CONFIG_DIR);
	dat = inl(CONFIG_DAT);

	while (i < 32 && !*encontrado) {
		//no analizar dispositivo si no hay nada
		if (dat == 0xFFFFFFFF) {
			dir_check = (uint32_t)(dir_check + (uint32_t)0x800);
			outl (dir_check, CONFIG_DIR);
			dat = inl(CONFIG_DAT);
			i++;
			continue;
		}

		//comprobamos el tipo de bus
		check_pci_pci = (dat & 0x00FF0000) >> 23;

		//comprobar si es el dispositivo
		dir_pci = (dir_check - (uint32_t)0xc) + (uint32_t)0x8;
		outl (dir_pci, CONFIG_DIR);
		dat = inl(CONFIG_DAT);
		class = (dat & 0xFF000000) >> 24;
		subclass = (dat & 0x00FF0000) >> 16;
		interface = (dat & 0x0000FF00) >> 8;

		//si es el dispositivo, fin
		if ((class - clase) == 0 &&  (subclass - subclase) == 0 && (interface - interfaz) == 0) { 
			//dispositivo encontrado
			*encontrado = 1;
			imprimir_info(dir_pci - 0x8, clase, subclase, interfaz);
		} else if (check_pci_pci) { //si no es el dispositivo, comprobar si es un pci to pci
			dir_pci = (dir_check - 0xc) + 0x100;
			outl (dir_pci, CONFIG_DIR);
			dat = inl(CONFIG_DAT);

			j = 0;
			while (j < 8) {
				if (dat != 0xFFFFFFFF) {
					buscar_disp(dir_pci, clase, subclase, interfaz, encontrado);	
				}

				dir_pci = dir_pci + 0x100;
				outl (dir_pci, CONFIG_DIR);
				dat = inl(CONFIG_DAT);
				j++;
			}
		}

		//siguiente iteracion
		dir_check = (uint32_t)(dir_check + (uint32_t)0x800);
		outl (dir_check, CONFIG_DIR);
		dat = inl(CONFIG_DAT);
		i++;
	}
}

//realiza la busqueda de un dispositivo dentro de un PCI-PCI
void buscar_disp(uint32_t dir, int clase, int subclase, int interfaz, int * encontrado) {
	//variables locales
	uint32_t dir_check, dat;
	uint8_t class, subclass, interface;

	//iniciamos por el primer dispostivo
	dir_check = (uint32_t)(dir + (uint32_t)0x8);
	outl (dir_check, CONFIG_DIR);
	dat = inl(CONFIG_DAT);

	//obtener datos
	class = (dat & 0xFF000000) >> 24;
	subclass = (dat & 0x00FF0000) >> 16;
	interface = (dat & 0x0000FF00) >> 8;

	//dispositivo encontrado
	if ((class - clase) == 0 &&  (subclass - subclase) == 0 && (interface - interfaz) == 0) {
		*encontrado = 1;
		imprimir_info(dir_check, clase, subclase, interfaz);
	}

}


int main(int argc, char *argv[]) {

	//comprobar argumentos
	if (argc != 4) {
		printf("Introduzca argumentos.\n");
		return -1;
	}

	//variables locales
	int clase = atoi(argv[1]), subclase = atoi(argv[2]), interfaz = atoi(argv[3]), encontrado = 0;
	
	//iteramos sobre el dominio
	// permiso para acceso a los 2 puertos modo usuario
	if (ioperm(CONFIG_DIR, 8, 1) < 0) { 
        perror("ioperm"); 
		return 1;
    }

	//variables locales
	uint32_t dir_check, dat;
	int i = 0;

	//iniciamos por el primer dispostivo
	dir_check = (uint32_t) 0x80000000;
	
	outl (dir_check, CONFIG_DIR);
	dat = inl(CONFIG_DAT);

	while (!encontrado && i < 256) {
		//solo buscamos en el dominio si hay algo
		if (dat != 0xFFFFFFFF) {
			buscar_funcion(dir_check, clase, subclase, interfaz, &encontrado);
		}
		
		//siguiente dominio
		dir_check = (uint32_t)(dir_check + (uint32_t)0x10000);
		outl (dir_check, CONFIG_DIR);
		dat = inl(CONFIG_DAT);
		i++;
	}

	return 0;
} 
