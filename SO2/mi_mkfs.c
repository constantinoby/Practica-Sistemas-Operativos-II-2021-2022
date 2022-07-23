//AUTORES: Aina Maria Tur Serrano y Constantino Byelov.
#include "directorios.h"
int main(int argc, char **argv)
{
    //Crear dispositivo virtual, abrir fichero
    bmount(argv[1]);
    //Número de bloques
    int nbloques= atoi(argv[2]);
    //Número de inodos
    int ninodos=nbloques/4;
    
    //buffer tamano del bloque 
    unsigned char buf[BLOCKSIZE];
    //Llenamos buffer a 0s
    memset(&buf, 0, BLOCKSIZE);
    //Escribimos el buffer en todos los bloques del fichero
    for (int i = 0; i < atoi(argv[2]); i++)
    {
        bwrite(i, buf);
    }
    //Inicializamos el superbloque y introducimos nbloques y ninodos
    initSB(nbloques, ninodos);
    //Inicializamos el mapa de bits y el array de inodos
    initMB();
    initAI();
    reservar_inodo('d',7);
    //Desmontamos dispositivo virtual, cerrar fichero
    bumount();
    
}
    
