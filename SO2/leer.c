//AUTORES: Aina Maria Tur Serrano y Constantino Byelov.
#include "ficheros.h"
#define tambuffer 1500
//Este programa lee el contenido del inodo pasado por parametro
int main(int argc, char **argv)
{
    // Si hay menos de 3 erro
    if (argc != 3)
    {
        #if DEBUG
        fprintf(stderr, "ERROR %d: %s\n", errno, strerror(errno));
        #endif
        return -1;
    }
    //Montamos dispositivo virtual
    bmount(argv[1]);
    // buffer tamano del bloque
    unsigned char buffer_texto[tambuffer];
    memset(buffer_texto, 0, tambuffer);
    //Convertimos entero nº inodo
    int ninodo=atoi(argv[2]);
    int total_leidos=0;
    int offset=0;
    //Comenzamos la lectura
    int leidos = mi_read_f(ninodo, buffer_texto, offset, tambuffer);
    while (leidos > 0)
    {
        //Muestra por pantalla el contenido leído
        write(1, buffer_texto, leidos);
        total_leidos+= leidos;
        //Siguiente offset
        offset+= tambuffer;
        // Llenamos buffer a 0s para limpiarlo
        memset(buffer_texto, 0, tambuffer);
        leidos = mi_read_f(ninodo, buffer_texto, offset, tambuffer);  
    }
    memset(buffer_texto, 0, tambuffer);
    //Mostramos el total de bytes leídos y el tamaño en Bytes lógicos del fichero
    struct inodo ino;
    leer_inodo(ninodo, &ino);
    char string[128];
    #if DEBUG
    sprintf(string, "total bytes leídos %d\n",total_leidos);
    #endif
    write(2, string, strlen(string));
    #if DEBUG
    sprintf(string,"tamEnBytesLog: %d\n",ino.tamEnBytesLog);
    #endif
    write(2, string, strlen(string));
    //Desmontamos el dispositivo virtual
    bumount();
}
