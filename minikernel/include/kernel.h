/*
 *  minikernel/include/kernel.h
 *
 *  Minikernel. Versi�n 1.0
 *
 *  Fernando P�rez Costoya
 *
 */

/*
 *
 * Fichero de cabecera que contiene definiciones usadas por kernel.c
 *
 *      SE DEBE MODIFICAR PARA INCLUIR NUEVA FUNCIONALIDAD
 *
 */

#ifndef _KERNEL_H
#define _KERNEL_H

//mutex
#define NO_RECURSIVO 0
#define RECURSIVO 1

#include "const.h"
#include "HAL.h"
#include "llamsis.h"

/*
 *
 * Definicion del tipo que corresponde con el BCP.
 * Se va a modificar al incluir la funcionalidad pedida.
 *
 */
typedef struct BCP_t *BCPptr;

typedef struct BCP_t {
        int id;				/* ident. del proceso */
        int estado;			/* TERMINADO|LISTO|EJECUCION|BLOQUEADO*/
        contexto_t contexto_regs;	/* copia de regs. de UCP */
        void * pila;			/* dir. inicial de la pila */
	BCPptr siguiente;		/* puntero a otro BCP */
	void *info_mem;			/* descriptor del mapa de memoria */

	//añadido
	int nseg_dormir; //numero de segundos a dormir
	int n_sec_s; //numero de ticks de reloj en modo sistema
	int n_sec_u; //numero de ticks de reloj en modo usuario
	int n_mutex; //numero de mutex
} BCP;

/*
 *
 * Definicion del tipo que corresponde con la cabecera de una lista
 * de BCPs. Este tipo se puede usar para diversas listas (procesos listos,
 * procesos bloqueados en sem�foro, etc.).
 *
 */

typedef struct{
	BCP *primero;
	BCP *ultimo;
} lista_BCPs;


/*
 * Variable global que identifica el proceso actual
 */

BCP * p_proc_actual=NULL;

/*
 * Variable global que representa la tabla de procesos
 */

BCP tabla_procs[MAX_PROC];

/*
 * Variable global que representa la cola de procesos listos
 */
lista_BCPs lista_listos= {NULL, NULL};
lista_BCPs lista_esperando = {NULL, NULL};

/*
 *
 * Definici�n del tipo que corresponde con una entrada en la tabla de
 * llamadas al sistema.
 *
 */
typedef struct{
	int (*fservicio)();
} servicio;


/*
 * Prototipos de las rutinas que realizan cada llamada al sistema
 */
int sis_crear_proceso();
int sis_terminar_proceso();
int sis_escribir();

/********************************************Funcionalidades añadidas**********************************************/
////////funciones auxiliares
void cambiar_proceso(lista_BCPs * new_list);

////////funcionalidades
int obtener_id_pr();
int dormir();
int tiempos_proceso();
int leer_caracter();

//mutex
int crear_mutex();
int abrir_mutex();
int lock();
int unlock();
int cerrar_mutex();

////////estructuras
struct tiempos_ejec {
    int usuario;
    int sistema;
};

struct mutex {
	char * nombre;
	int tipo;
	int proc[MAX_PROC];
	int procBlq[MAX_PROC];	
};
/******************************************************************************************************************/

/*
 * Variable global que contiene las rutinas que realizan cada llamada
 */
servicio tabla_servicios[NSERVICIOS]={	{sis_crear_proceso},
										{sis_terminar_proceso},
										{sis_escribir},
										{obtener_id_pr},
										{dormir},
										{tiempos_proceso},
										{crear_mutex},
										{abrir_mutex},
										{lock},
										{unlock},
										{cerrar_mutex},
										{leer_caracter}};

#endif /* _KERNEL_H */