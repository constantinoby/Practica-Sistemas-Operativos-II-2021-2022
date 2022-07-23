// AUTORES: AINA MARIA TUR SERRANO Y CONSTANTINO BYELOV
#include "directorios.h"
#define tambuffer (BLOCKSIZE*4)

int main(int argc, char const *argv[])
{
    // Comprobamos la sintaxis
    if (argc != 3)
    {
        printf("Error de sintaxis: ./mi_cat <disco> </ruta_fichero>\n");
        return 0;
    }
    //Comprobamos que la ruta sea un fichero
    if(argv[2][strlen(argv[2])-1]!='/'){
        //Montamos dispositivo virtual
        if(bmount(argv[1])<0){
            return -1;
        }

    // buffer tamaño del bloque
    unsigned char buffer_texto[tambuffer];
    memset(buffer_texto, 0, tambuffer);
    int offset=0;
    int total_leidos=0;
    //Comenzamos la lectura
    int leidos = mi_read(argv[2], buffer_texto, offset, tambuffer);
    //printf("\nTOTAL_LEIDOS:%d\n",leidos);
    while (leidos > 0)
    {
        //Muestra por pantalla el contenido leído
        write(1, buffer_texto, leidos);
        total_leidos+=leidos;
        offset+=tambuffer;
        memset(buffer_texto,0,tambuffer);
        //Comenzamos la lectura
        leidos= mi_read(argv[2], buffer_texto, offset, tambuffer);
    }
    memset(buffer_texto,0,tambuffer);
    //Desmontamos el dispositivo virtual
    bumount();
    //Mostrar el total de bytes leidos
    char string[128];
    sprintf(string, "\ntotal bytes leídos %d\n",total_leidos);
    write(2, string, strlen(string));
    }else{
        printf("No es un fichero\n");
        return -1;
    }
}    