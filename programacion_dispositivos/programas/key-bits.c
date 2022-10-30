/*
 * Ejemplo de PIO: acceso a registro del teclado PS/2.
 * Muestra la última tecla pulsada o liberada.
 */
#include <sys/io.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include "bits.h"

#define RDAT_PS2 0x60  // registro de datos de PS/2 

int main() {
	// Habilita (segundo 1) solo 1 puerto (primer 1)
	if (ioperm(RDAT_PS2, 1, 1) < 0) {
       		perror("ioperm"); return 1;
   	}
	while(1){
		// podría usar inserto de ensamblador, pero mejor usar inb
		//__asm__  ("inb %1, %0" : "=a" (codigo) : "Nd" (RDAT_PS2));
		// lee el código de la tecla
		uint8_t codigo = inb(RDAT_PS2);

		// bit de mayor peso distingue si pulsada o liberada
		printf("Código de tecla %d (%s)\n", bit_fija_a_cero(codigo, 7), 
			       	bit_es_igual_uno(codigo, 7) ? "liberada" : "pulsada");
		sleep(3);
	}
	return 0;
} 
