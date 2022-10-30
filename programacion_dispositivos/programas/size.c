// Muestra cómo el compilador introduce huecos de relleno en las
// estructuras para asegurar las restricciones de alineamiento de los campos
#include <stdio.h>
#include <stdint.h>

// ocupa 12 bytes en vez de 10 porque se genera un hueco de 2 bytes
// entre los dos primeros campos
struct info_dispo {
        uint16_t registro;
        uint32_t dir;
        uint32_t tam;
};
// con al atributo no estándar packed ocupa justo 10 bytes
struct info_dispo_p {
        uint16_t registro;
        uint32_t dir;
        uint32_t tam;
} __attribute__ ((packed)); 

int main(int argc, char *argv[]) {
	printf("tam %ld tam empaquetada %ld\n",
		sizeof(struct info_dispo), sizeof(struct info_dispo_p));
	return 0;
} 
