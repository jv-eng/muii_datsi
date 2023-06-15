/*
 *  kernel/kernel.c
 *
 *  Minikernel. Versi�n 1.0
 *
 *  Fernando P�rez Costoya
 *
 */

/*
 *
 * Fichero que contiene la funcionalidad del sistema operativo
 *
 */

#include "kernel.h"	/* Contiene defs. usadas por este modulo */

/*
 *
 * Funciones relacionadas con la tabla de procesos:
 *	iniciar_tabla_proc buscar_BCP_libre
 *
 */

/*
 * Funci�n que inicia la tabla de procesos
 */
static void iniciar_tabla_proc(){
	int i;

	for (i=0; i<MAX_PROC; i++)
		tabla_procs[i].estado=NO_USADA;
}

/*
 * Funci�n que busca una entrada libre en la tabla de procesos
 */
static int buscar_BCP_libre(){
	int i;

	for (i=0; i<MAX_PROC; i++)
		if (tabla_procs[i].estado==NO_USADA)
			return i;
	return -1;
}

/*
 *
 * Funciones que facilitan el manejo de las listas de BCPs
 *	insertar_ultimo eliminar_primero eliminar_elem
 *
 * NOTA: PRIMERO SE DEBE LLAMAR A eliminar Y LUEGO A insertar
 */

/*
 * Inserta un BCP al final de la lista.
 */
static void insertar_ultimo(lista_BCPs *lista, BCP * proc){
	if (lista->primero==NULL)
		lista->primero= proc;
	else
		lista->ultimo->siguiente=proc;
	lista->ultimo= proc;
	proc->siguiente=NULL;
}

/*
 * Elimina el primer BCP de la lista.
 */
static void eliminar_primero(lista_BCPs *lista){

	if (lista->ultimo==lista->primero)
		lista->ultimo=NULL;
	lista->primero=lista->primero->siguiente;
}

/*
 * Elimina un determinado BCP de la lista.
 */
static void eliminar_elem(lista_BCPs *lista, BCP * proc){
	BCP *paux=lista->primero;

	if (paux==proc)
		eliminar_primero(lista);
	else {
		for ( ; ((paux) && (paux->siguiente!=proc));
			paux=paux->siguiente);
		if (paux) {
			if (lista->ultimo==paux->siguiente)
				lista->ultimo=paux;
			paux->siguiente=paux->siguiente->siguiente;
		}
	}
}

/*
 *
 * Funciones relacionadas con la planificacion
 *	espera_int planificador
 */

/*
 * Espera a que se produzca una interrupcion
 */
static void espera_int(){
	int nivel;

	//printk("-> NO HAY LISTOS. ESPERA INT\n");

	/* Baja al m�nimo el nivel de interrupci�n mientras espera */
	nivel=fijar_nivel_int(NIVEL_1);
	halt();
	fijar_nivel_int(nivel);
}

/*
 * Funci�n de planificacion que implementa un algoritmo FIFO.
 */
static BCP * planificador(){
	while (lista_listos.primero==NULL)
		espera_int();		/* No hay nada que hacer */
	lista_listos.primero->rodaja = TICKS_POR_RODAJA;
	return lista_listos.primero;
}

////////////////Funciones auxiliares///////////////////////////////////////////////////////////////

//cambiar proceso en ejecucion
void cambiar_proceso(lista_BCPs * new_list) {
	//obtener proceso actual
	BCP * actual = p_proc_actual;
	int nivel_int = 0;
	//mover proceso de lista
	nivel_int = fijar_nivel_int(NIVEL_3);
	actual->estado = BLOQUEADO;
	eliminar_elem(&lista_listos,actual);
	insertar_ultimo(new_list,actual);
	//replanificar
	p_proc_actual = planificador();
	fijar_nivel_int(nivel_int);
	//cambio de contexto
	cambio_contexto(&(actual->contexto_regs),&(p_proc_actual->contexto_regs)); 
}

//devuelve la posicion del mutex o -1
int nombre_mutex_unico(char * nombre) { 
	int i, res = -1;
	for (i = 0; res == -1 && i < NUM_MUT; i++) {
		if (arr_mutex[i].nombre != NULL && strcmp(arr_mutex[i].nombre,nombre) == 0) {
			res = i;
		}
	}
	return res; 
}

//cierra y elimina los mutex del proceso
//flag == 0 -> liberar proc
//flag == 1 -> cerrar mutex
void eliminar_mutex(BCP * proc_actual, int desc, int flag) {
	BCP * proc;
	int i, j, m_uso = 1, res = -1;
	mutex * m;

	//comprobar si existe el mutex
	if (proc_actual->mutex_proc[desc] != NULL && proc_actual->mutex_proc[desc]->nombre != NULL) {
		m_uso = 1;
		//obtener mutex
		m = proc_actual->mutex_proc[desc]; 
		//comprobar si el mutex esta en uso
		for (j = 0; m_uso == 1 && j < MAX_PROC; j++) {
			if (j != proc_actual->id && m->proc[j] == 1) {
				m_uso = 0;
			}
		}
		//si no esta en uso, borrarlo
		if (m_uso == 1) { 
			//buscar posicion del mutex y borrarlo
			res = -1;
			for (j = 0; res == -1 && j < NUM_MUT; j++) {
				if (arr_mutex[j].nombre != NULL && strcmp(arr_mutex[j].nombre,m->nombre) == 0) {
					res = 0; 
					arr_mutex[j].nombre = NULL;
					arr_mutex[j].proc[proc_actual->id] = 0;
					num_mutex--;
					if (flag == 0) {
						proc_actual->n_mutex--;
						proc_actual->mutex_proc[desc] = NULL;
					}
				}
			}
			if (flag == 1) {
				p_proc_actual->n_mutex--;
				p_proc_actual->mutex_proc[desc] = NULL;
			}
		} else { //si esta en uso, borrar del proceso
			m->proc[proc_actual->id] = 0;
			proc_actual->n_mutex--;
			proc_actual->mutex_proc[desc] = NULL;
		}

		//si ha hecho lock, quitarlo
		if (m->proc_blq == proc_actual->id) {
			m->proc_blq = -1;
			m->blq_lock = 0;
		}

		//comprobar si hay algun proceso dormido esperando por crear un mutex
		proc = lista_espera_mutex.primero;
		if (proc && proc->blq_mutex) {
			res = fijar_nivel_int(NIVEL_3);
			proc->estado = LISTO;
			proc->blq_mutex = 0;
			eliminar_elem(&lista_espera_mutex,proc);
			insertar_ultimo(&lista_listos,proc);
			fijar_nivel_int(res);
		}

		proc = lista_espera_lock.primero; res = 1;
		while (proc && res) {
			i = fijar_nivel_int(NIVEL_3);
			if (proc->mutex_proc[proc->blq_lock] == m) {
				res = 0;
				proc->estado = LISTO;
				eliminar_elem(&lista_espera_lock, proc);
				insertar_ultimo(&lista_listos, proc);
			}
			//siguiente
			proc = lista_espera_lock.primero;
			fijar_nivel_int(i);
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////

/*
 *
 * Funcion auxiliar que termina proceso actual liberando sus recursos.
 * Usada por llamada terminar_proceso y por rutinas que tratan excepciones
 *
 */
static void liberar_proceso(){
	BCP * p_proc_anterior;
	int i;

	for (i = 0; i < NUM_MUT_PROC; i++) { 
		eliminar_mutex(p_proc_actual, i, 0);
	}

	//liberar campos proceso
	liberar_imagen(p_proc_actual->info_mem); /* liberar mapa */

	int var_int = fijar_nivel_int(NIVEL_3);
	p_proc_actual->estado=TERMINADO;
	eliminar_primero(&lista_listos); /* proc. fuera de listos */

	/* Realizar cambio de contexto */
	p_proc_anterior=p_proc_actual;
	p_proc_actual=planificador();
	printk("-> C.CONTEXTO POR FIN: de %d a %d\n", p_proc_anterior->id, p_proc_actual->id);
	fijar_nivel_int(var_int);

	liberar_pila(p_proc_anterior->pila);
	cambio_contexto(NULL, &(p_proc_actual->contexto_regs));
        return; /* no deber�a llegar aqui */
}

/*
 *
 * Funciones relacionadas con el tratamiento de interrupciones
 *	excepciones: exc_arit exc_mem
 *	interrupciones de reloj: int_reloj
 *	interrupciones del terminal: int_terminal
 *	llamadas al sistemas: llam_sis
 *	interrupciones SW: int_sw
 *
 */

/*
 * Tratamiento de excepciones aritmeticas
 */
static void exc_arit(){

	if (!viene_de_modo_usuario())
		panico("excepcion aritmetica cuando estaba dentro del kernel");


	printk("-> EXCEPCION ARITMETICA EN PROC %d\n", p_proc_actual->id);
	liberar_proceso();

        return; /* no deber�a llegar aqui */
}

/*
 * Tratamiento de excepciones en el acceso a memoria
 */
static void exc_mem(){

	printf("Excepcion de memoria\n");
	if (!acceso_mapa_user)
		if (!viene_de_modo_usuario())
			panico("excepcion");

	printk("-> EXCEPCION DE MEMORIA EN PROC %d\n", p_proc_actual->id);
	printk("Liberando recursos del proceso\n");
	liberar_proceso();

    return; /* no deber�a llegar aqui */
}

/*
 * Tratamiento de interrupciones de terminal
 */
static void int_terminal(){
	//variables locales
	char car;
	BCP * proc;
	int nivel_int;

	//leer de terminal
	car = leer_puerto(DIR_TERMINAL);
	printk("-> TRATANDO INT. DE TERMINAL %c\n", car);

	//si el buffer esta lleno, ignoramos lectura
	if (num_car_buff >= TAM_BUF_TERM) {
		printf("No se pueden leer caracteres, el buffer esta lleno\n");
		return;
	}

	//guardar en buffer
	nivel_int = fijar_nivel_int(NIVEL_2);
	buff_teclado[num_car_buff] = car;
	num_car_buff++;
	fijar_nivel_int(nivel_int);

	//despertar procesos dormidos si hay caracteres en el buffer
	proc = lista_espera_int_terminal.primero;
	if (proc) {
		nivel_int = fijar_nivel_int(NIVEL_3);
		proc->estado = LISTO;
		eliminar_elem(&lista_espera_int_terminal, proc);
		insertar_ultimo(&lista_listos, proc);
		fijar_nivel_int(nivel_int);
	}

    return;
}

//tiempo de ejecucion
int n_ticks_ejec = 0;
/*
 * Tratamiento de interrupciones de reloj
 */
//int n_int = 0;
static void int_reloj(){

	//printk("-> TRATANDO INT. DE RELOJ\n");
	int nivel_int = fijar_nivel_int(NIVEL_3);
	//obtener primer elemento
	BCP * actual = lista_listos.primero, * tmp;	
	fijar_nivel_int(nivel_int);

	//actualizr tiempo de uso de cpu por proceso y rodajas de cpu
	n_ticks_ejec++;
	if (actual) {
		if (viene_de_modo_usuario()) actual->n_sec_u++; else actual->n_sec_s++;
		if (actual->rodaja > 1) {
			actual->rodaja--;
		} else {
			id_proc_int_sw = actual->id;
			actual->rodaja--;
			activar_int_SW();
		}
	}

	//revisar el resto de procesos esperando
	actual = lista_esperando.primero;
	nivel_int = fijar_nivel_int(NIVEL_3);
	while (actual) {
		tmp = actual->siguiente;
		//decrementar
		actual->nseg_dormir--;
		//si hemos llegado a 0, liberamos
		if (actual->nseg_dormir <= 0) {
			actual->estado = LISTO;
			eliminar_elem(&lista_esperando, actual);
			insertar_ultimo(&lista_listos,actual);
		}
		actual = tmp;
	}
	fijar_nivel_int(nivel_int);
	return;
}

/*
 * Tratamiento de llamadas al sistema
 */
static void tratar_llamsis(){
	int nserv, res;

	nserv=leer_registro(0);
	if (nserv<NSERVICIOS)
		res=(tabla_servicios[nserv].fservicio)();
	else
		res=-1;		/* servicio no existente */
	escribir_registro(0,res);
	return;
}

/*
 * Tratamiento de interrupciuones software
 */
static void int_sw(){

	printk("-> TRATANDO INT. SW\n");
	int tmp = p_proc_actual->id;
	BCP * proc = p_proc_actual;
	//si el id no coincide, terminamos
	if (id_proc_int_sw != p_proc_actual->id) {
		return;
	}

	int nivel_int = fijar_nivel_int(NIVEL_3);
	
	eliminar_elem(&lista_listos,p_proc_actual);
	insertar_ultimo(&lista_listos,p_proc_actual);
	p_proc_actual = planificador();
	printf("Cambio de %d a %d\n",tmp,p_proc_actual->id);

	fijar_nivel_int(nivel_int); 
	
	//cambio de contexto
	cambio_contexto(&(proc->contexto_regs),&(p_proc_actual->contexto_regs)); 

	return;
}

/*
 *
 * Funcion auxiliar que crea un proceso reservando sus recursos.
 * Usada por llamada crear_proceso.
 *
 */
static int crear_tarea(char *prog){
	void * imagen, *pc_inicial;
	int error=0;
	int proc;
	BCP *p_proc;

	proc=buscar_BCP_libre();
	if (proc==-1)
		return -1;	/* no hay entrada libre */

	/* A rellenar el BCP ... */
	p_proc=&(tabla_procs[proc]);

	/* crea la imagen de memoria leyendo ejecutable */
	imagen=crear_imagen(prog, &pc_inicial);
	if (imagen)
	{
		p_proc->info_mem=imagen;
		p_proc->pila=crear_pila(TAM_PILA);
		fijar_contexto_ini(p_proc->info_mem, p_proc->pila, TAM_PILA,
			pc_inicial,
			&(p_proc->contexto_regs));
		p_proc->id=proc;
		p_proc->estado=LISTO;
		p_proc->rodaja = TICKS_POR_RODAJA;

		//inicializar nuevos campos
		p_proc->nseg_dormir = 0;
		p_proc->mutex_lock = -1;
		int nivel_int = fijar_nivel_int(NIVEL_3);
		
		/* lo inserta al final de cola de listos */
		insertar_ultimo(&lista_listos, p_proc);
		fijar_nivel_int(nivel_int);
		error= 0;
	}
	else
		error= -1; /* fallo al crear imagen */

	return error;
}

/*
 *
 * Rutinas que llevan a cabo las llamadas al sistema
 *	sis_crear_proceso sis_escribir
 *
 */

/*
 * Tratamiento de llamada al sistema crear_proceso. Llama a la
 * funcion auxiliar crear_tarea sis_terminar_proceso
 */
int sis_crear_proceso(){
	char *prog;
	int res;
	printk("-> PROC %d: CREAR PROCESO\n", p_proc_actual->id);
	prog=(char *)leer_registro(1);
	res=crear_tarea(prog);
	return res;
}

/*
 * Tratamiento de llamada al sistema escribir. Llama simplemente a la
 * funcion de apoyo escribir_ker
 */
int sis_escribir()
{
	char *texto;
	unsigned int longi;

	texto=(char *)leer_registro(1);
	longi=(unsigned int)leer_registro(2);

	escribir_ker(texto, longi);
	return 0;
}

/*
 * Tratamiento de llamada al sistema terminar_proceso. Llama a la
 * funcion auxiliar liberar_proceso
 */
int sis_terminar_proceso(){

	printk("-> FIN PROCESO %d\n", p_proc_actual->id);

	liberar_proceso();

        return 0; /* no deber�a llegar aqui */
}

//////////////////////////////////////////////FUNCIONALIDADES AÑADIDAS//////////////////////////////////////////////////////

////////////////Funcionalidades
//obtener id de un proceso
int obtener_id_pr() {
	return p_proc_actual->id;
}

//dormir un proceso
int dormir() {
	//leer nº de segundos
	unsigned int nseg = (unsigned int)leer_registro(1);
	//modificar BCP
	p_proc_actual->nseg_dormir = nseg * TICK; 
	//cambiar de proceso
	cambiar_proceso(&lista_esperando);
	return 0;
}

//leer un caracter
int leer_caracter() {
	//variables locales
	int nivel = fijar_nivel_int(NIVEL_2), i;
	char res;

	//si no hay caracteres, a dormir
	while (num_car_buff == 0) {
		cambiar_proceso(&lista_espera_int_terminal); 
	}

	//obtener resultado y mover caracteres
	res = buff_teclado[0];
	num_car_buff--;
	for (i = 0; i < num_car_buff; i++) {
		buff_teclado[i] = buff_teclado[i+1];
	}

	fijar_nivel_int(nivel);
	return res;
}

//ver numero de interrupciones de reloj
int tiempos_proceso() {
	int nivel_int = 0;
	//obtener dato pasado como arg
	struct tiempos_ejec * t_ejec = (struct tiempos_ejec *)leer_registro(1);
	//si pasan algo, tratarlo
	nivel_int = fijar_nivel_int(NIVEL_3);
	if (t_ejec) {
		acceso_mapa_user++;
		t_ejec->usuario = p_proc_actual->n_sec_u;
		t_ejec->sistema = p_proc_actual->n_sec_s;
		acceso_mapa_user--;
	}
	fijar_nivel_int(nivel_int);
	return n_ticks_ejec;
}

/////////////////////////mutex
int crear_mutex() { 
	//obtener argumentos
	char *nombre = (char *)leer_registro(1);
	int tipo = (int)leer_registro(2);
	int i = 0, desc = -1, pos_m = -1;
	
	//comprobar long del nombre
	if (strlen(nombre) > MAX_NOM_MUT){
		return -1;
	}	

	//si no hay descriptores libres en el proceso error
	if (p_proc_actual->n_mutex == NUM_MUT_PROC) {
		return -2;
	}

	//comprobar nombre unico del mutex
	if (nombre_mutex_unico(nombre) != -1) {
		return -3;
	}

	//comprobar n mutex del sistema
	while (num_mutex == NUM_MUT) {
		//dormir hasta que no haya procesos durmiendo	
		p_proc_actual->blq_mutex = 1;
		if (id_proc_int_sw == p_proc_actual->id) id_proc_int_sw = -1;
		cambiar_proceso(&lista_espera_mutex); 
		if (nombre_mutex_unico(nombre) != -1) {
			return -3;
		}
	}

	//proceso no espera por mutex
	p_proc_actual->blq_mutex = 0;

	//buscar sitio para el nuevo mutex
	for (i = 0; pos_m == -1 && i < NUM_MUT; i++) {
		if (arr_mutex[i].nombre == NULL) {
			arr_mutex[i].nombre = strdup(nombre);
			arr_mutex[i].tipo = tipo;
			arr_mutex[i].proc[p_proc_actual->id] = 1;
			arr_mutex[i].proc_blq = -1;
			num_mutex++; //incrementar numero de mutex en el sistema
			pos_m = i;
		}
	}

	//desc libre para el mutex
	for (i = 0; desc < 0 && i < NUM_MUT_PROC; i++) {
		if (p_proc_actual->mutex_proc[i] == NULL) {
			p_proc_actual->mutex_proc[i] = &(arr_mutex[pos_m]);
			p_proc_actual->n_mutex++;
			desc = i;
		}
	}
	return desc;
}

int abrir_mutex() {
	//variables locales
	int desc = -1, index = -1, i;
	char * nombre = (char *)leer_registro(1);

	//comprobar n mutex proc
	if (p_proc_actual->n_mutex == NUM_MUT_PROC) {
		return -1;
	}

	//buscar mutex, comprobar que existe
	index = nombre_mutex_unico(nombre);
	if (index == -1) {
		return -2;
	}

	//obtener mutex y guardarlo en el BCP
	for (i = 0; desc == -1 && i < NUM_MUT_PROC; i++) {
		if (p_proc_actual->mutex_proc[i] == NULL || p_proc_actual->mutex_proc[i]->nombre == NULL) {
			p_proc_actual->mutex_proc[i] = &(arr_mutex[index]);
			p_proc_actual->n_mutex++;
			desc = i;
		}
	}
	//actualizar lista del mutex
	arr_mutex[index].proc[p_proc_actual->id] = 1;

	return desc;
}

int cerrar_mutex() {
	//variables locales
	int desc = (unsigned int) leer_registro(1);

	//comprobar que el proceso tiene el mutex abierto
	if (p_proc_actual->mutex_proc[desc] == NULL) {
		return -1;
	}

	eliminar_mutex(p_proc_actual, desc, 1);

	return 0;
}

int lock() {
	//variables locales
	int desc = (unsigned int) leer_registro(1);
	mutex * m;

	//comprobar que existe el mutex
	if (desc < 0 || desc >= NUM_MUT_PROC || p_proc_actual->mutex_proc[desc] == NULL) {
		return -1;
	}

	//obtener mutex
	m = p_proc_actual->mutex_proc[desc];

	//comprobar tipo de mutex
	//si no es recursivo y ya lo ha bloqueado, error
	if (m->proc_blq == p_proc_actual->id && m->tipo == NO_RECURSIVO) {
		return -2;
	}

	//si esta bloqueado, esperar
	while (m->proc_blq != -1 && m->proc_blq != p_proc_actual->id) {
		p_proc_actual->blq_lock = desc;
		if (id_proc_int_sw == p_proc_actual->id) id_proc_int_sw = -1;
		cambiar_proceso(&lista_espera_lock);
		//comprobar que sigue existiendo mutex
		if (m == NULL) {
			return -3;
		} 
	}

	//hacer el lock
	m->proc_blq = p_proc_actual->id;
	m->blq_lock++;
	//p_proc_actual->mutex_lock = desc;
	p_proc_actual->blq_lock = -1;

	return 0;
}

int unlock() {
	//variables locales
	int desc = (unsigned int) leer_registro(1), i, flag = 1;
	mutex * m;
	BCP * proc;

	//comprobar que existe el mutex
	if (desc < 0 || desc > NUM_MUT_PROC || p_proc_actual->mutex_proc[desc] == NULL) {
		return -1;
	}

	//obtener mutex
	m = p_proc_actual->mutex_proc[desc];

	//comprobar que es el proceso que ha hecho el lock
	if (m->proc_blq != p_proc_actual->id) {
		return -2;
	}

	//hacer unlock
	m->blq_lock--;
	if (m->blq_lock == 0) m->proc_blq = -1;

	//despertar procesos esperando
	if (m->proc_blq == -1) {
		proc = lista_espera_lock.primero;
		while (proc && flag) {
			i = fijar_nivel_int(NIVEL_3);
			if (proc->mutex_proc[proc->blq_lock] == m) {
				flag = 0;
				proc->estado = LISTO;
				eliminar_elem(&lista_espera_lock, proc);
				insertar_ultimo(&lista_listos, proc);
			}
			//siguiente
			proc = lista_espera_lock.primero;
			fijar_nivel_int(i);
		}
	}
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/*
 *
 * Rutina de inicializaci�n invocada en arranque
 *
 */
int main(){
	/* se llega con las interrupciones prohibidas */

	instal_man_int(EXC_ARITM, exc_arit); 
	instal_man_int(EXC_MEM, exc_mem); 
	instal_man_int(INT_RELOJ, int_reloj); 
	instal_man_int(INT_TERMINAL, int_terminal); 
	instal_man_int(LLAM_SIS, tratar_llamsis); 
	instal_man_int(INT_SW, int_sw); 

	iniciar_cont_int();		/* inicia cont. interr. */
	iniciar_cont_reloj(TICK);	/* fija frecuencia del reloj */
	iniciar_cont_teclado();		/* inici cont. teclado */

	iniciar_tabla_proc();		/* inicia BCPs de tabla de procesos */

	/* crea proceso inicial */
	if (crear_tarea((void *)"init")<0)
		panico("no encontrado el proceso inicial");
	
	/* activa proceso inicial */
	p_proc_actual=planificador();
	cambio_contexto(NULL, &(p_proc_actual->contexto_regs));
	panico("S.O. reactivado inesperadamente");
	return 0;
}
