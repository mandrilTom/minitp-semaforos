#include <stdio.h>      // libreria estandar
#include <stdlib.h>     // para usar exit y funciones de la libreria standard
#include <string.h>
#include <pthread.h>    // para usar threads
#include <semaphore.h>  // para usar semaforos
#include <unistd.h>

#define LIMITE 50

//creo estructura de semaforos 
struct semaforos {
    // Semaforos
    sem_t sem_mezclar;
    sem_t sem_salar;
    sem_t sem_armar_med;
    sem_t sem_cocinar;
    sem_t sem_armar_ham_carne;
    sem_t sem_armar_ham_pan;
    sem_t sem_armar_ham_extra;
    // "Mutex"
    sem_t salero_mutex;
    sem_t plancha_mutex;
    sem_t horno_mutex;
};

//creo los pasos con los ingredientes
struct paso {
    char accion [LIMITE];
    char ingredientes[4][LIMITE];
};

//creo los parametros de los hilos 
struct parametro {
    int equipo_param;
    struct semaforos semaforos_param;
    struct paso pasos_param[8];
    FILE* receta;
    FILE* resultado;
};

//funcion para imprimir las acciones y los ingredientes de la accion
void* imprimirAccion(void *data, char *accionIn) {
	struct parametro *mydata = data;
	//calculo la longitud del array de pasos 
	int sizeArray = (int)( sizeof(mydata->pasos_param) / sizeof(mydata->pasos_param[0]));
	//indice para recorrer array de pasos 
	int i;
	for(i = 0; i < sizeArray; i ++){
		//pregunto si la accion del array es igual a la pasada por parametro (si es igual la funcion strcmp devuelve cero)
		if(strcmp(mydata->pasos_param[i].accion, accionIn) == 0){
		    printf("\tEquipo %d - accion %s \n " , mydata->equipo_param, mydata->pasos_param[i].accion);
		    //calculo la longitud del array de ingredientes
		    int sizeArrayIngredientes = (int)( sizeof(mydata->pasos_param[i].ingredientes) / sizeof(mydata->pasos_param[i].ingredientes[0]) );
		    //indice para recorrer array de ingredientes
		    int h;
		    printf("\tEquipo %d -----------ingredientes : ----------\n",mydata->equipo_param); 
			for(h = 0; h < sizeArrayIngredientes; h++) {
				//consulto si la posicion tiene valor porque no se cuantos ingredientes tengo por accion 
				if(strlen(mydata->pasos_param[i].ingredientes[h]) != 0) {
					printf("\tEquipo %d ingrediente  %d : %s \n",mydata->equipo_param,h,mydata->pasos_param[i].ingredientes[h]);
				}
			}
		}
	}
}

//funcion para tomar de ejemplo
void* picar(void *data) {
	//creo el nombre de la accion de la funcion 
	char *accion = "Picar";
	//creo el puntero para pasarle la referencia de memoria (data) del struct pasado por parametro (la cual es un puntero). 
	struct parametro *mydata = data;
	//llamo a la funcion imprimir le paso el struct y la accion de la funcion
	imprimirAccion(mydata,accion);
	//uso sleep para simular que que pasa tiempo
	usleep( 2000000 );
	//doy la señal a la siguiente accion (picar me habilita mezclar)
    sem_post(&mydata->semaforos_param.sem_mezclar);
	
    pthread_exit(NULL);
}

void* mezclar(void *data) {
    char *accion = "Mezclar";
    struct parametro *mydata = data;
    sem_wait(&mydata->semaforos_param.sem_mezclar);
    imprimirAccion(mydata,accion);
    usleep( 2000000 );
    sem_post(&mydata->semaforos_param.sem_salar);

    pthread_exit(NULL);
}

void* salar(void *data) {
    char *accion = "Salar";
    struct parametro *mydata = data;
    sem_wait(&mydata->semaforos_param.sem_mezclar);
    sem_wait(&mydata->semaforos_param.salero_mutex);
    imprimirAccion(mydata,accion);
    usleep( 1000000 );
    sem_post(&mydata->semaforos_param.salero_mutex);
    sem_post(&mydata->semaforos_param.sem_armar_med);

    pthread_exit(NULL);
}

void* armar_medallones(void *data) {
    // Deberia editarlo para armar ambos medallones.
    // Como???.
    char *accion = "Armar medallones";
    struct parametro *mydata = data;
    sem_wait(&mydata->semaforos_param.sem_salar);
    imprimirAccion(mydata,accion);
    usleep( 4000000 );
    sem_post(&mydata->semaforos_param.sem_cocinar);

    pthread_exit(NULL);
}

void* cocinar_medallones(void *data) {
    // Deberia editarlo para cocinar ambos medallones seguidos.
    // Como???.
    char *accion = "Cocinar";
    struct parametro *mydata = data;
    sem_wait(&mydata->semaforos_param.sem_armar_med);
    sem_wait(&mydata->semaforos_param.plancha_mutex);
    imprimirAccion(mydata,accion);
    usleep( 10000000 );
    sem_post(&mydata->semaforos_param.plancha_mutex);
    sem_post(&mydata->semaforos_param.sem_armar_ham_carne);

    pthread_exit(NULL);
}

void* hornear_panes(void *data) {
    char *accion = "Hornear";
    struct parametro *mydata = data;
    sem_wait(&mydata->semaforos_param.horno_mutex);
    imprimirAccion(mydata,accion);
    usleep( 10000000 );
    sem_post(&mydata->semaforos_param.sem_armar_ham_pan);

    pthread_exit(NULL);
}

void* cortar_extras(void *data) {
    char *accion = "Cortar";
    struct parametro *mydata = data;
    imprimirAccion(mydata,accion);
    usleep( 2000000 );
    sem_post(&mydata->semaforos_param.sem_armar_ham_extra);

    pthread_exit(NULL);
}

void* armar_hamburgesas(void *data) {
    // Deberia editarlo para armar ambas hamburguesas.
    // Como???.
    char *accion = "Armar hamburguesa";
    struct parametro *mydata = data;
    sem_wait(&mydata->semaforos_param.sem_armar_ham_carne);
    sem_wait(&mydata->semaforos_param.sem_armar_ham_pan);
    sem_wait(&mydata->semaforos_param.sem_armar_ham_extra);
    imprimirAccion(mydata,accion);
    usleep( 6000000 );

    pthread_exit(NULL);
}

void* ejecutarReceta(void *i) {
	//variables semaforos
	sem_t sem_mezclar;
	sem_t sem_salar;
	sem_t sem_armar_med;
	sem_t sem_cocinar;
	sem_t sem_armar_ham_carne;
	sem_t sem_armar_ham_pan;
	sem_t sem_armar_ham_extra;
	sem_t salero_mutex;
	sem_t plancha_mutex;
	sem_t horno_mutex;

	//variables hilos
	pthread_t p1; 
	pthread_t p2; 
	pthread_t p3; 
	pthread_t p4; 
	pthread_t p5; 
	pthread_t p6; 
	pthread_t p7; 
	pthread_t p8; 
	
	//numero del equipo (casteo el puntero a un int)
	int p = *((int *) i);
	
	printf("Ejecutando equipo %d \n", p);

	//reservo memoria para el struct
	struct parametro *pthread_data = malloc(sizeof(struct parametro));

	//seteo los valores al struct
	
	//seteo numero de grupo
	pthread_data->equipo_param = p;

	//seteo semaforos
	pthread_data->semaforos_param.sem_mezclar = sem_mezclar;
	pthread_data->semaforos_param.sem_salar = sem_salar;
	pthread_data->semaforos_param.sem_armar_med = sem_armar_med;
	pthread_data->semaforos_param.sem_cocinar = sem_cocinar;
	pthread_data->semaforos_param.sem_armar_ham_carne = sem_armar_ham_carne;
	pthread_data->semaforos_param.sem_armar_ham_pan = sem_armar_ham_pan;
	pthread_data->semaforos_param.sem_armar_ham_extra = sem_armar_ham_extra;
	pthread_data->semaforos_param.salero_mutex = salero_mutex;
	pthread_data->semaforos_param.plancha_mutex = plancha_mutex;
	pthread_data->semaforos_param.horno_mutex = horno_mutex;
	
    pthread_data->receta = fopen("receta.txt", "rt");

	//seteo las acciones y los ingredientes (A PARTIR DEL ARCHIVO TXT)
    char buffer[1024];
    fgets(buffer, 1024, pthread_data->receta);
    int param_index = 0;
    int ing_index = 0;
    char * renglon = strtok(buffer, "\n");
    char * instruccion;
    char * elemento;
    while (renglon != NULL) {
        instruccion = strtok(renglon, "-");
        strcpy(pthread_data->pasos_param[param_index].accion, instruccion);
        instruccion = strtok(NULL, "-");
        elemento = strtok(instruccion, "|");
        ing_index = 0;
        while (elemento != NULL) {
            strcpy(pthread_data->pasos_param[param_index].ingredientes[ing_index], elemento);
            elemento = strtok(NULL, "|");
            ing_index++;
        }
        renglon = strtok(NULL, "\n");
        param_index++;
    }

	//inicializo los semaforos
	sem_init(&(pthread_data->semaforos_param.sem_mezclar),0,0);
	sem_init(&(pthread_data->semaforos_param.sem_salar),0,0);
	sem_init(&(pthread_data->semaforos_param.sem_armar_med),0,0);
	sem_init(&(pthread_data->semaforos_param.sem_cocinar),0,0);
	sem_init(&(pthread_data->semaforos_param.sem_armar_ham_carne),0,0);
	sem_init(&(pthread_data->semaforos_param.sem_armar_ham_pan),0,0);
	sem_init(&(pthread_data->semaforos_param.sem_armar_ham_extra),0,0);
	sem_init(&(pthread_data->semaforos_param.salero_mutex),0,1);
	sem_init(&(pthread_data->semaforos_param.plancha_mutex),0,1);
	sem_init(&(pthread_data->semaforos_param.horno_mutex),0,1);

	//creo los hilos a todos les paso el struct creado (el mismo a todos los hilos) ya que todos comparten los semaforos 
    int rc;
    rc = pthread_create(&p1,                            //identificador unico
                        NULL,                           //atributos del thread
                        picar,                         //funcion a ejecutar
                        pthread_data);                  //parametros de la funcion a ejecutar, pasado por referencia

    rc = pthread_create(&p2,                            //identificador unico
                        NULL,                           //atributos del thread
                        mezclar,                         //funcion a ejecutar
                        pthread_data);                  //parametros de la funcion a ejecutar, pasado por referencia

    rc = pthread_create(&p3,                            //identificador unico
                        NULL,                           //atributos del thread
                        salar,                         //funcion a ejecutar
                        pthread_data);                  //parametros de la funcion a ejecutar, pasado por referencia

    rc = pthread_create(&p4,                            //identificador unico
                        NULL,                           //atributos del thread
                        armar_medallones,                         //funcion a ejecutar
                        pthread_data);                  //parametros de la funcion a ejecutar, pasado por referencia

    rc = pthread_create(&p5,                            //identificador unico
                        NULL,                           //atributos del thread
                        cocinar_medallones,                         //funcion a ejecutar
                        pthread_data);                  //parametros de la funcion a ejecutar, pasado por referencia

    rc = pthread_create(&p6,                            //identificador unico
                        NULL,                           //atributos del thread
                        hornear_panes,                         //funcion a ejecutar
                        pthread_data);                  //parametros de la funcion a ejecutar, pasado por referencia

    rc = pthread_create(&p7,                            //identificador unico
                        NULL,                           //atributos del thread
                        cortar_extras,                         //funcion a ejecutar
                        pthread_data);                  //parametros de la funcion a ejecutar, pasado por referencia

    rc = pthread_create(&p8,                            //identificador unico
                        NULL,                           //atributos del thread
                        armar_hamburgesas,                         //funcion a ejecutar
                        pthread_data);                  //parametros de la funcion a ejecutar, pasado por referencia
	
    //valido que el hilo se alla creado bien 
    if (rc) {
        printf("Error:unable to create thread, %d \n", rc);
        exit(-1);
    }

	//join de todos los hilos
	pthread_join (p1,NULL);
	pthread_join (p2,NULL);
	pthread_join (p3,NULL);
	pthread_join (p4,NULL);
	pthread_join (p5,NULL);
	pthread_join (p6,NULL);
	pthread_join (p7,NULL);
	pthread_join (p8,NULL);

	//destruccion de los semaforos 
	sem_destroy(&sem_mezclar);
	sem_destroy(&sem_salar);
	sem_destroy(&sem_armar_med);
	sem_destroy(&sem_cocinar);
	sem_destroy(&sem_armar_ham_carne);
	sem_destroy(&sem_armar_ham_pan);
	sem_destroy(&sem_armar_ham_extra);
	sem_destroy(&salero_mutex);
	sem_destroy(&plancha_mutex);
	sem_destroy(&horno_mutex);

	//salida del hilo
	 pthread_exit(NULL);
}


int main ()
{
	//creo los nombres de los equipos 
	int rc;
	int *equipoNombre1 =malloc(sizeof(*equipoNombre1));
	int *equipoNombre2 =malloc(sizeof(*equipoNombre2));
	int *equipoNombre3 =malloc(sizeof(*equipoNombre3));
	*equipoNombre1 = 1;
	*equipoNombre2 = 2;
	*equipoNombre3 = 3;

	//creo las variables los hilos de los equipos
	pthread_t equipo1; 
	pthread_t equipo2;
	pthread_t equipo3;
 
	//inicializo los hilos de los equipos
    rc = pthread_create(&equipo1,                       //identificador unico
                        NULL,                           //atributos del thread
                        ejecutarReceta,                 //funcion a ejecutar
                        equipoNombre1); 

    rc = pthread_create(&equipo2,                       //identificador unico
                        NULL,                           //atributos del thread
                        ejecutarReceta,                 //funcion a ejecutar
                        equipoNombre2);

    rc = pthread_create(&equipo3,                       //identificador unico
                        NULL,                           //atributos del thread
                        ejecutarReceta,                 //funcion a ejecutar
                        equipoNombre3);

    if (rc) {
        printf("Error:unable to create thread, %d \n", rc);
        exit(-1);
    } 

	//join de todos los hilos
	pthread_join (equipo1,NULL);
	pthread_join (equipo2,NULL);
	pthread_join (equipo3,NULL);

    pthread_exit(NULL);
}

//Para compilar:   gcc HellsBurgers.c -o ejecutable -lpthread
//Para ejecutar:   ./ejecutable
