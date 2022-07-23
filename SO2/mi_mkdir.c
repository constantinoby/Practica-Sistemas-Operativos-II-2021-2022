// AUTORES: AINA MARIA TUR SERRANO Y CONSTANTINO BYELOV
#include "directorios.h"
int main(int argc, char const *argv[])
{
    // Comprobamos la sintaxis
    if (argc != 4)
    {
        printf("Error de sintaxis: ./mi_mkdir <disco> <permisos> </ruta_directorio>\n");
        return -1;
    }

    // ahora miramos si el camino es un directorio
    if (argv[3][strlen(argv[3]) - 1] == '/')
    {
        // asigno permisos
        unsigned char permisos = atoi(argv[2]);
        // Compruebo los permisos se encuentran dentro del rango
        if ((permisos < 0) || (permisos > 7))
        {
            printf("Error: modo inválido: <<%d>>\n", permisos);
            return 0;
        }
        // Montamos dispositivo virtual
        if (bmount(argv[1]) == -1)
        {
            printf("Error de montaje de disco.\n");
            return -1;
        }
        // Función mi_creat de directorios
        if (mi_creat(argv[3], permisos) < 0)
        {
            bumount();
            return -1;
        }
        
    }
    else
    {
        // Se trata de un fichero. Emplear mi_touch
        printf("No es una ruta de directorio valida, se trata de un fichero.\n");
    }
    bumount();
}