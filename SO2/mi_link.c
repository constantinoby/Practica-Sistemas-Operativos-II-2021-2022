// AUTORES: AINA MARIA TUR SERRANO Y CONSTANTINO BYELOV
#include "directorios.h"
int main(int argc, char const *argv[])
{
    // Comprobamos la sintaxis
    if (argc != 4)
    {
        printf("Error de sintaxis: ./mi_link disco /ruta_fichero_original /ruta_enlace\n");
        return -1;
    }
    // Los dos son ficheros
    if ((argv[2][strlen(argv[2]) - 1] != '/') && (argv[3][strlen(argv[3]) - 1] != '/'))
    {
        // Montamos disco virtual
        if (bmount(argv[1]) < 0)
        {
            printf("Error de montaje de disco.\n");
            return -1;
        }
        mi_link(argv[2], argv[3]);
        bumount();
        return 0;
    }
    else
    {
        printf("Error: Los archivos deben ser ficheros\n");
        return -1;
    }
}
