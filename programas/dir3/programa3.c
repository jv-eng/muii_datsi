#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <sched.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/mman.h>

int global_noini;		/* Variable global escalar sin valor inicial */
int global_ini=666;		/* Variable global escalar con valor inicial */
int vec_global_noini[4000];	/* Variable global vector sin valor inicial */

/* Variable global vector con valor inicial */
int vec_global_ini[]={31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

const long double constante= 3.141592653589793238L; /* constante númerica */
char *string="ABCDEFG";	/* literal */

// auxiliares para poder imprimir las variables de los threads
void *aux1, *aux2;

void *thread(void *arg_thread) {
	int local_thread;
	aux1=&arg_thread;
	aux2=&local_thread;
	pause();
	return NULL;
}
int main(int argc, char **argv) {   /* argc es un parámetro de una función */
	int local;		/* Variable local escalar */
	int vec_local[2500];	/* Variable local de tipo vector */
	static int local_estatica; /* Variable local estática */
	int *vec_dinamico = malloc(250*sizeof(int)); /* Variable dinámica */

	int fd=open("Makefile", O_RDWR);
	void *map_file=mmap(NULL, 108, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

	pthread_t thid;
	pthread_create(&thid, NULL, thread, NULL);

	sleep(1);

	printf("\nproceso %d:\n\tmain %p\n\tprintf %p\n\tconstante %p\n\tstring %p\n\tglobal_ini %p\n\tvec_global_ini(%p,%p)\n\tglobal_noini %p\n\tvec_global_noini(%p,%p)\n\targc %p\n\tlocal %p\n\tvec_local(%p,%p)\n\tlocal_estatica %p\n\tvec_dinamico(%p,%p)\n\targ_thread %p\n\tlocal_thread %p\n\tmap_file %p\n",
		getpid(), main, printf, &constante, string, &global_ini, 
		vec_global_ini, &vec_global_ini[sizeof(vec_global_ini)/sizeof(int)],
		&global_noini, vec_global_noini, &vec_global_noini[sizeof(vec_global_noini)/sizeof(int)],
		&argc, &local, vec_local, &vec_local[sizeof(vec_local)/sizeof(int)],
		&local_estatica,
		vec_dinamico, &vec_dinamico[250], aux1, aux2, map_file);
	printf("\nPara cortar y pegar en virt_to_region:");
	printf("\n%d %p %p %p %p %p %p %p %p %p %p %p %p %p %p %p %p %p %p %p %p\n",
		getpid(), main, printf, &constante, string, &global_ini, 
		vec_global_ini, &vec_global_ini[sizeof(vec_global_ini)/sizeof(int)],
		&global_noini, vec_global_noini, &vec_global_noini[sizeof(vec_global_noini)/sizeof(int)],
		&argc, &local, vec_local, &vec_local[sizeof(vec_local)/sizeof(int)],
		&local_estatica, vec_dinamico, &vec_dinamico[250], aux1, aux2,
	       	 map_file);

	printf("\nPulsa para acabar ");
	getchar();
	return 0;
}
