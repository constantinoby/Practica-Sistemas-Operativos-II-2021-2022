//AUTORES: AINA MARIA TUR SERRANO Y CONSTANTINO BYELOV
#include "directorios.h"
int main(int argc, char const *argv[])
{
    //compruebo sintaxis
    if (argc != 4)
    {
        printf("Error de sintaxis: ./mi_chmod <nombre_dispositivo> <permisos> </ruta>\n");
        return -1;
    }

    //Asigno permisos, compruebo se encuentran dentro del rangpo
    unsigned char permisos=atoi(argv[2]);
   if(permisos<0 || permisos>7)
    {

        printf("Error de sintaxis: permisos incorrectos.\n");
        return -1;
    }
    
    // Monta el disco en el sistema. 
    if (bmount(argv[1]) == -1)
    {
        printf("Error de montaje de disco.\n");
        return -1;
    }
    //Función mi_chmod de directorios
    if(mi_chmod(argv[3], permisos)<0){
        return -1;
    }
    //Desmonto dispositivo virtual
    bumount();

}

