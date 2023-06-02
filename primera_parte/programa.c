#include <sys/mman.h>
#include <sys/wait.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

unsigned long *contador;

/* crea una región compartida basada en un fichero
   para almacenar los contadores de todos los procesos */
void *crear_zona_compartida(int nprocs) {
    int f;
    void *p;
    if ((f=open("/tmp/F", O_CREAT|O_RDWR, 0600))<0) {
        perror("open");
        return NULL;
    }
    if (ftruncate(f, nprocs*sizeof(unsigned long))<0) {
        perror("ftruncate");
        return NULL;
    }
    if ((p = mmap(NULL, nprocs*sizeof(unsigned long),
      PROT_READ|PROT_WRITE, MAP_SHARED, f, 0)) == MAP_FAILED){
        perror("mmap");
        return NULL;
    }
    return p;
}
// calcula e imprime estimación cuota UCP de cada proceso
void imprimir(int nprocs, int proceso) {
    unsigned long total = 0;
    int i;
    printf("Primero en terminar %d ->", proceso);
    for (i=0; i<nprocs; i++)
        total += contador[i];
    for (i=0; i<nprocs; i++)
        printf(" proceso %d: %.2f%%;", i,
            100.0*contador[i]/total);
    printf("\n");
}
//todos los procesos terminan cuando uno alcanza el valor máximo
int fin(int nprocs, unsigned long n_iter) {
    int i;
    
    for (i=0; i<nprocs; i++)
        if (contador[i]>=n_iter) return 1;
    return 0;
}
/* bucle de trabajo de los procesos */
void funcion(int nprocs, int proceso, unsigned long n_iter) {
    // ejecutan hasta que uno llegue al valor máximo
    for (; !fin(nprocs, n_iter); contador[proceso]++);

    // primero en terminar imprime cuota de UCP de todos
    if (contador[proceso]==n_iter)
        imprimir(nprocs, proceso);
}
int main(int argc, char *argv[]) {
    int i, n_proc_total, n_proc_prog, id_primero, prio;
    unsigned long num_iter;
    if (argc<6) {
        fprintf(stderr, "Uso: %s num_iter n_proc_total n_proc_prog id_primero prio_proc... \n", argv[0]);
        return 1;
    }
    num_iter = atol(argv[1]);
    n_proc_total = atoi(argv[2]);
    n_proc_prog = atoi(argv[3]);
    id_primero = atoi(argv[4]);

    if ((contador=crear_zona_compartida(n_proc_total)) == NULL)
        return 1;

    // escribe su PID: lo usaremos con cgroups
    fprintf(stderr, "%d", getpid());
    /* crea los procesos hijos con la prioridad especificada */
    for (i=1; i<n_proc_prog; i++) {
        if (fork() == 0) {
            /* hijo cambia su prioridad */
            prio = atoi(argv[5+i]);
            errno = 0;
            // nice puede devolver -1 sin haber error,
            // hay que usar errno para controlarlo
            if (nice(prio), errno != 0) {
                perror("Error cambiando prioridad");
                return 1;
            }
            funcion(n_proc_total, id_primero + i, num_iter);
            exit(0);
        }
    }
    /* padre cambia su prioridad */
    prio = atoi(argv[5]);
    errno = 0;
    if (nice(prio), errno != 0) {
        perror("Error cambiando prioridad");
        return 1;
    }
    funcion(n_proc_total, id_primero, num_iter);

    /* espera finalización de los hijos */
    for (i=1; i<n_proc_prog; i++)
        wait(NULL);
    munmap(contador, n_proc_total*sizeof(unsigned long));
    unlink("/tmp/F");
    return 0;
}
