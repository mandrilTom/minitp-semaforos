#include <stdio.h>      // libreria estandar
#include <stdlib.h>     // para usar exit y funciones de la libreria standard
#include <string.h>
#include <pthread.h>    // para usar threads
#include <semaphore.h>  // para usar semaforos
#include <unistd.h>

#define LIMITE 50

pthread_mutex_t salero_mutex;
pthread_mutex_t plancha_mutex;
pthread_mutex_t horno_mutex;
pthread_mutex_t salida_mutex;

int ganadores[3];
int indice_ganador;

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
void* realizarAccion(int equipo, char *accionIn, char *ingrediente, FILE *salida) {
    pthread_mutex_lock(&salida_mutex);
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "Equipo : %d, Accion : %s, Ingrediente : %s", equipo, accionIn, ingrediente);
    salida = fopen("resultado.txt", "at");
    fprintf(salida, "%s\n", buffer);
    fclose(salida);
    printf("%s\n", buffer);
    pthread_mutex_unlock(&salida_mutex);
}

void* accionTerminada(void *data, char *accionIn) {
    pthread_mutex_lock(&salida_mutex);
    struct parametro *mydata = data;
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "Equipo : %d, Terminado : %s\n", mydata->equipo_param, accionIn);
    mydata->resultado = fopen("resultado.txt", "at");
    fprintf(mydata->resultado, "%s\n", buffer);
    fclose(mydata->resultado);
    printf("%s\n", buffer);
    pthread_mutex_unlock(&salida_mutex);
}

//funcion para tomar de ejemplo
void* picar(void *data) {
	//creo el nombre de la accion de la funcion 
	char *accion = "Picar";
	//creo el puntero para pasarle la referencia de memoria (data) del struct pasado por parametro (la cual es un puntero). 
	struct parametro *mydata = data;
	//llamo a la funcion imprimir le paso el struct y la accion de la funcion
	realizarAccion(mydata->equipo_param, accion, mydata->pasos_param[0].ingredientes[0], mydata->resultado);
	usleep( 1000000 );
	realizarAccion(mydata->equipo_param, accion, mydata->pasos_param[0].ingredientes[1], mydata->resultado);
	usleep( 1000000 );
	realizarAccion(mydata->equipo_param, accion, mydata->pasos_param[0].ingredientes[2], mydata->resultado);
	//uso sleep para simular que que pasa tiempo
	usleep( 1000000 );
	//doy la señal a la siguiente accion (picar me habilita mezclar)
	accionTerminada(mydata,accion);
    sem_post(&mydata->semaforos_param.sem_mezclar);
    pthread_exit(NULL);
}

void* mezclar(void *data) {
    char *accion = "Mezclar";
    struct parametro *mydata = data;
    sem_wait(&mydata->semaforos_param.sem_mezclar);
    realizarAccion(mydata->equipo_param, accion, mydata->pasos_param[1].ingredientes[0], mydata->resultado);
    realizarAccion(mydata->equipo_param, accion, mydata->pasos_param[1].ingredientes[1], mydata->resultado);
	usleep( 2000000 );
    accionTerminada(mydata,accion);
    sem_post(&mydata->semaforos_param.sem_salar);
    pthread_exit(NULL);
}

void* salar(void *data) {
    char *accion = "Salar";
    struct parametro *mydata = data;
    sem_wait(&mydata->semaforos_param.sem_salar);
    pthread_mutex_lock(&salero_mutex);
    realizarAccion(mydata->equipo_param, accion, mydata->pasos_param[2].ingredientes[0], mydata->resultado);
	usleep( 1000000 );
    accionTerminada(mydata,accion);
    pthread_mutex_unlock(&salero_mutex);
    sem_post(&mydata->semaforos_param.sem_armar_med);
    pthread_exit(NULL);
}

void* armar_medallones(void *data) {
    char *accion = "Armar medallones";
    char *priMed = "Armar primer medallon";
    char *secMed = "Armar segundo medallon";
    struct parametro *mydata = data;
    sem_wait(&mydata->semaforos_param.sem_armar_med);
    realizarAccion(mydata->equipo_param, accion, mydata->pasos_param[3].ingredientes[0], mydata->resultado);
	usleep( 2000000 );
    accionTerminada(mydata,priMed);
    realizarAccion(mydata->equipo_param, accion, mydata->pasos_param[3].ingredientes[0], mydata->resultado);
	usleep( 2000000 );
    accionTerminada(mydata,secMed);
    sem_post(&mydata->semaforos_param.sem_cocinar);
    pthread_exit(NULL);
}

void* cocinar_medallones(void *data) {
    // Deberia editarlo para cocinar ambos medallones seguidos.
    // Como???.
    char *accion = "Cocinar";
    char *priMed = "Cocinar primer medallon";
    char *secMed = "Cocinar segundo medallon";
    struct parametro *mydata = data;
    sem_wait(&mydata->semaforos_param.sem_cocinar);
    pthread_mutex_lock(&plancha_mutex);
    realizarAccion(mydata->equipo_param, accion, mydata->pasos_param[4].ingredientes[0], mydata->resultado);
    usleep( 5000000 );
    accionTerminada(mydata,priMed);
    pthread_mutex_unlock(&plancha_mutex);
    pthread_mutex_lock(&plancha_mutex);
    realizarAccion(mydata->equipo_param, accion, mydata->pasos_param[4].ingredientes[0], mydata->resultado);
    usleep( 5000000 );
    accionTerminada(mydata,secMed);
    pthread_mutex_unlock(&plancha_mutex);
    sem_post(&mydata->semaforos_param.sem_armar_ham_carne);
    pthread_exit(NULL);
}

void* hornear_panes(void *data) {
    char *accion = "Hornear";
    struct parametro *mydata = data;
    pthread_mutex_lock(&horno_mutex);
    realizarAccion(mydata->equipo_param, accion, mydata->pasos_param[5].ingredientes[0], mydata->resultado);
    usleep( 10000000 );
    accionTerminada(mydata,accion);
    pthread_mutex_unlock(&horno_mutex);
    sem_post(&mydata->semaforos_param.sem_armar_ham_pan);
    pthread_exit(NULL);
}

void* cortar_extras(void *data) {
    char *accion = "Cortar";
    struct parametro *mydata = data;
    realizarAccion(mydata->equipo_param, accion, mydata->pasos_param[6].ingredientes[0], mydata->resultado);
    usleep( 1000000 );
    realizarAccion(mydata->equipo_param, accion, mydata->pasos_param[6].ingredientes[1], mydata->resultado);
    usleep( 1000000 );
    accionTerminada(mydata,accion);
    sem_post(&mydata->semaforos_param.sem_armar_ham_extra);
    pthread_exit(NULL);
}

void* armar_hamburgesas(void *data) {
    // Deberia editarlo para armar ambas hamburguesas.
    // Como???.
    char *accion = "Armar hamburguesa";
    char *priHam = "Armar primer hamburgesa";
    char *secHam = "Armar segunda hamburgesa";
    struct parametro *mydata = data;
    sem_wait(&mydata->semaforos_param.sem_armar_ham_carne);
    sem_wait(&mydata->semaforos_param.sem_armar_ham_pan);
    sem_wait(&mydata->semaforos_param.sem_armar_ham_extra);
    realizarAccion(mydata->equipo_param, accion, mydata->pasos_param[7].ingredientes[0], mydata->resultado);
    realizarAccion(mydata->equipo_param, accion, mydata->pasos_param[7].ingredientes[1], mydata->resultado);
    realizarAccion(mydata->equipo_param, accion, mydata->pasos_param[7].ingredientes[2], mydata->resultado);
    realizarAccion(mydata->equipo_param, accion, mydata->pasos_param[7].ingredientes[3], mydata->resultado);
    usleep( 3000000 );
    accionTerminada(mydata,priHam);
    realizarAccion(mydata->equipo_param, accion, mydata->pasos_param[7].ingredientes[0], mydata->resultado);
    realizarAccion(mydata->equipo_param, accion, mydata->pasos_param[7].ingredientes[1], mydata->resultado);
    realizarAccion(mydata->equipo_param, accion, mydata->pasos_param[7].ingredientes[2], mydata->resultado);
    realizarAccion(mydata->equipo_param, accion, mydata->pasos_param[7].ingredientes[3], mydata->resultado);
    usleep( 3000000 );
    accionTerminada(mydata,secHam);
    ganadores[indice_ganador] = mydata->equipo_param;
    indice_ganador++;
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
	
    pthread_data->receta = fopen("receta.txt", "rt");

	//seteo las acciones y los ingredientes (A PARTIR DEL ARCHIVO TXT)
    int param_index = 0;
    int ing_index = 0;
    char renglon[128];
    fgets(renglon, 128, pthread_data->receta);
    char * instruccion;
    char * elemento;
    while (param_index < 8) {
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
        fgets(renglon, 128, pthread_data->receta);
        param_index++;
    }

    fclose(pthread_data->receta);

	//inicializo los semaforos
	sem_init(&(pthread_data->semaforos_param.sem_mezclar),0,0);
	sem_init(&(pthread_data->semaforos_param.sem_salar),0,0);
	sem_init(&(pthread_data->semaforos_param.sem_armar_med),0,0);
	sem_init(&(pthread_data->semaforos_param.sem_cocinar),0,0);
	sem_init(&(pthread_data->semaforos_param.sem_armar_ham_carne),0,0);
	sem_init(&(pthread_data->semaforos_param.sem_armar_ham_pan),0,0);
	sem_init(&(pthread_data->semaforos_param.sem_armar_ham_extra),0,0);

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

    FILE *salida = fopen("resultado.txt", "wt");
    fclose(salida);
    indice_ganador = 0;

	pthread_mutex_init(&salero_mutex, NULL);
	pthread_mutex_init(&plancha_mutex, NULL);
	pthread_mutex_init(&horno_mutex, NULL);
	pthread_mutex_init(&salida_mutex, NULL);

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

    char buffer[128];
    snprintf(buffer, sizeof(buffer), "¡Ganador Equipo %d!\n", ganadores[0]);
    salida = fopen("resultado.txt", "at");
    fprintf(salida, "%s\n", buffer);
    fclose(salida);
    printf("%s\n", buffer);

	pthread_mutex_destroy(&salero_mutex);
	pthread_mutex_destroy(&plancha_mutex);
	pthread_mutex_destroy(&horno_mutex);
	pthread_mutex_destroy(&salida_mutex);

    pthread_exit(NULL);
}

//Para compilar:   gcc HellsBurgers.c -o ejecutable -lpthread
//Para ejecutar:   ./ejecutable
