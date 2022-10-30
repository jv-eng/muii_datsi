/*
 * Ejemplo de uso del ensamblador en línea de GCC.
 * Imprime el nombre del fabricante del procesador
 * obtenido usando la intrucción CPUID
 */

#include <sys/uio.h>

int main(int argc, char *argv[]) {
	int op=0; // tipo de operación
	char Nombre1[4];
	char Nombre2[4];
	char Nombre3[4];

	/* Instrucción CPUID:
	 * 	- entrada EAX: tipo de información solicitada
	 * 	- salida EAX,EBX,ECX,EDX: información retornada
	 *
	 * Ejemplo de uso:
	 * 	- entrada EAX = 0
	 * 	- salida EBX, EDX, ECX: nombre del fabricante
	 */
	asm ("cpuid"
		// salida: EBX a Nombre1
		// salida: ECX a Nombre3
		// salida: EDX a Nombre2
    		: "=b" (Nombre1), "=c" (Nombre3), "=d" (Nombre2)
		// entrada: EAX tipo de información solicitada
		: "a" (op) );

	// lo imprime "de golpe" usando writev
	struct iovec iov[3] = { {.iov_base=Nombre1, .iov_len=4},
	    {.iov_base=Nombre2, .iov_len=4}, {.iov_base=Nombre3, .iov_len=4}};

	writev(1, iov, 3);
	return 0;
}
