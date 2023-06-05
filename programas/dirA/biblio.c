#include <stdio.h>

void otra_funcion(void) {
	printf("soy otra funcion\n");
}
void funcion(void) {
	printf("soy funcion\n");
	printf("dir. funcion y otra_funcion %p %p\n", funcion, otra_funcion);
}
