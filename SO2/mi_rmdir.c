// AUTORES: AINA MARIA TUR SERRANO Y CONSTANTINO BYELOV
#include "directorios.h"
int main(int argc, char const *argv[])
{
    // Comprobamos la sintaxis
    if (argc != 3)
    {
        printf("Error de sintaxis: Sintaxis: ./mi_rmdir disco /ruta\n");
        return -1;
    }

    // ahora miramos si el camino es un directorio
    if (argv[2][strlen(argv[2]) - 1] == '/')
    {
        // Montamos dispositivo virtual
        if (bmount(argv[1])<0)
        {
            printf("Error de montaje de disco.\n");
            return -1;
        }

        mi_unlink(argv[2]);
        bumount();
        return 0;
    }
    else
    {
        // Se trata de un fichero. Emplear mi_rm
        printf("No es una ruta de directorio valida, se trata de un fichero.\n");
    }
    bumount();
}