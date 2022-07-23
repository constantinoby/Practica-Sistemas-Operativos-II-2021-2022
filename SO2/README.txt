GRUPO ASISTEMADOS: Constantino Byelov y Aina Maria Tur Serrano.
MEJORAS REALIZADAS:
Se han puesto las secciones críticas granulizadas en ficheros.c. Se han puesto en todas aquellas zonas que impliquen reservar y liber ar inodos, se reserven o liberen bloques,modifiquen campos inodos.
En verificacion.c hemos leido todas las entradas una vez antes del bucle declarando un buffer del tamaño NUMPROCESOS * sizeof (struct entrada).

En la funcion liberar_bloques_inodos del fichero ficheros_basicos.c se ha intentado iterar bloques por BL.

En la función mi_dir, se listan directorios y ficheros cada uno con un color diferente.

RESTRICCIONES PROGRAMA:
El programa no contiene ninguna restricción a parte de las sintaxis.

SINTAXIS ESPECÍFICA:
./simulacion disco

Crea 100 procesos hijos en el directorio /simul_aaaaddmmhhmmss/ y realiza 50 escrituras por proceso.

./verificacion disco /simul_aaaaddmmhhmmss/ 

Recorre el fichero prueba.dat de cada proceso del directorio creado anteriormente y guarda la siguiente informacion.
PID: El PID del proceso.
Nº de escrituras: el contador de los registros validados dentro del fichero.
Primera escritura.
última escritura.
Menor posición.
Mayor posición.
De los cuatro registros más significativos muestra el nº de la escritura, el nº de registro y la fecha.

