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
#include "string.h"

////////////////////////MUTEX//////////////////////////////
typedef struct {
	char * nombre; //nombre del mutex
	int tipo; //tipo del mutex: recursivo o no recursivo
	int proc[MAX_PROC]; //lista de procesos con el mutex
	int proc_blq; //id del proceso que ha bloqueado el mutex
	int blq_lock; //numero de veces que se ha hecho un lock sobre el mutex; depende de tipo 
} mutex;

mutex arr_mutex[NUM_MUT];
int num_mutex = 0;
//////////////////////////////////////////////////////////

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
	mutex * mutex_proc[NUM_MUT_PROC]; //array de mutex
	int blq_mutex; //bloqueado por mutex, espera de crear
	int mutex_lock; //desc del mutex sobre el que se ha hecho lock
	int blq_lock; //bloqueado por mutex, espera para lock
				  //en el caso de no poder hacer lock, guarda
	int rodaja; //tiempo posible en ejecucion
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
lista_BCPs lista_listos= {NULL, NULL}; //procesos listos para ejecutar
lista_BCPs lista_esperando = {NULL, NULL}; //procesos dormidos
lista_BCPs lista_espera_mutex = {NULL, NULL}; //procesos espearndo para crear mutex
lista_BCPs lista_espera_lock = {NULL, NULL}; //procesos esperando para hacer lock
lista_BCPs lista_espera_int_terminal = {NULL, NULL}; //procesos esperando por caracter

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
int nombre_mutex_unico(char * nombre);

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

int acceso_mapa_user = 0;
int id_proc_int_sw = -1;
int num_car_buff = 0;
char buff_teclado[TAM_BUF_TERM];

////////estructuras
struct tiempos_ejec {
    int usuario;
    int sistema;
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