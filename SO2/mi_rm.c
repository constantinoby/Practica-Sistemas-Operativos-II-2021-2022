// AUTORES: AINA MARIA TUR SERRANO Y CONSTANTINO BYELOV
#include "directorios.h"
int main(int argc, char const *argv[])
{
     //Comprobamos la sintaxis
    if(argc!=3)
    {
        printf("Error de sintaxis: Sintaxis: ./mi_rm disco /ruta\n");
        return -1;
    } 
     // ahora miramos si el camino es un fichero
    if (argv[2][strlen(argv[2]) - 1] != '/')
    {
    //Montamos disco virtual
    if(bmount(argv[1])<0){
        printf("Error de montaje de disco.\n");
        return -1;
    }
    mi_unlink(argv[2]);
    bumount();
    return 0;
    }else{
         // Se trata de un directorio. Emplear mi_rmdir
        printf("No es una ruta de fichero valida, se trata de un directorio.\n"); 
    }
}    