// AUTORES: AINA MARIA TUR SERRANO Y CONSTANTINO BYELOV
#include "directorios.h"
int main(int argc, char const *argv[])
{
    // Comprobamos la sintaxis
    if (argc != 4)
    {
        printf("Error de sintaxis: ./mi_touch <disco> <permisos> </ruta>\n");
        return -1;
    }

    // Ahora miramos si el camino es un fichero
    if (argv[3][strlen(argv[3]) - 1] != '/')
    {
        // Asigno permisos
        unsigned char permisos = atoi(argv[2]);
        // Compruebo los permisos y si se encuentra dentro del rango válido
        if ((permisos < 0) || (permisos > 7))
        {
            printf("Error: modo inválido: <<%d>>\n", permisos);
            return -1;
        }
        // Montamos disco virtual
        if (bmount(argv[1]) == -1)
        {
            printf("Error de montaje de disco.\n");
            return -1;
        }
        // Función mi_creat de directorios
        if ((mi_creat(argv[3], permisos)) < 0)
        {
            // Desmontamos dispositivo
            bumount();
            return -1;
        }
    }
    else
    {
        // Es un directorio. Emplear mi_mkdir
        printf("No es una ruta de fichero válida, se trata de un directorio\n");
        return -1;
    }
    // Desmontamos dispositivo
    bumount();
}
