# Sistemas Operativos y Redes

## Trabajo Práctico de hilos y semáforos

### Alumno: Tomas Eduardo Esteban Mandril
### Legajo: 40142645/2017

En el presente informe se presenta el trabajo realizado.
El codigo fuente simula una competencia entre **3 equipos** de cocina que deben armar **2 hamburguesas** cada uno.
Solo tienen acceso a un salero, una plancha que cocina un medallon de carne a la vez, y un horno con lugar para dos panes.
Por lo tanto, deben coordinar entre ellos el acceso a los mismos.
Asimismo, cada equipo realiza sus tareas de forma paralela, por lo que tambien deben coordinarse entre si.

Para coordinar las tareas *entre equipos*, se utilizaron mutex globales para los elementos a compartir (salero, plancha, horno).
Para las tareas *dentro de un mismo equipo*, se utilizaron semaforos **unicos para cada equipo** que habilitaban la tarea siguiente a una requerida para continuar.
En un momento se penso que los mutex debian ir junto con los semaforos, pero esto hacia que no se compartiesen entre equipos, por lo que se los separo.

El codigo fuente **NO** debia tener en si mismo el orden de pasos a seguir para realizar la receta, sino que debia levantarse de un archivo.
Para esto se creo un txt con las instrucciones y, mediante fgets, strtok y strcpy, se leian los datos en el archivo.
Los mismos se almacenaban en una estructura construida para que cada equipo pueda acceder a los pasos a seguir.

Tambien debia luego, a medida que funcionaba el programa, escribirse el progreso y resultado de la competencia paso a paso en un archivo de salida.
Esto se hizo con dos funciones *realizarAccion* y *accionTerminada*, que utilizar snprintf y fprintf para escribir la informacion de salida.
Para evitar la condicion de carrera, se utilizo un mutex especifico para el manejo del archivo de salida.
