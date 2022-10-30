/* 
 * Ejemplo de PIO: acceso a registro del teclado PS/2 usando bitfields.
 * Muestra la última tecla pulsada o liberada.
 */
// USA BITFIELDS; CUIDADO: no portables; dependen del compilador y procesador
#include <sys/io.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#define RDAT_PS2 0x60  // registro de datos de PS/2 
typedef union reg {
	struct {
		uint8_t codigo_tecla:7;
		uint8_t liberada:1;
	};
	uint8_t valor;
} reg_t;
int main() {
	// Habilita (segundo 1) solo 1 puerto (primer 1)
	if (ioperm(RDAT_PS2, 1, 1) < 0) {
       		perror("ioperm"); return 1;
   	}
	while(1){
		// lee el código de la tecla
		reg_t r;
		r.valor = inb(RDAT_PS2);
		printf("Código de tecla %d (%s)\n", r.codigo_tecla,
			       	r.liberada ? "liberada" : "pulsada");
		sleep(3);
	}
	return 0;
} 
