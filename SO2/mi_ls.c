//AUTORES: AINA MARIA TUR SERRANO Y CONSTANTINO BYELOV
#include "directorios.h"
int main(int argc, char const *argv[])
{
    // Comprobamos la sintaxis
    if (argc != 3)
    {
        printf("Error de sintaxis: Sintaxis: ./mi_ls <disco> </ruta_directorio>\n");
        return -1;
    }
    
    // Montamos disco virtual
    if (bmount(argv[1]) == -1)
    {
        printf("Error de montaje de disco.\n");
        return -1;
    }
    // Inicializamos buffer con 0
    char buffer[TAMBUFFER];
    memset(buffer, 0, TAMBUFFER);
    char tipo='d';
    int total_entradas;
    //Calculamos total entradas
    if ((total_entradas = mi_dir(argv[2], buffer, tipo)) < 0)
    {
        bumount();
        return -1;
    }
    if(total_entradas!=0){
        //Imprimimos buffer
        printf("Tipo\tModo\tmTime\t\t\tTamaño\tNombre\n");
        printf("----------------------------------------------------------"
               "----------------------\n");
        printf("%s\n", buffer);
    }
    // Desmontamos dispositivo virtual
    bumount();
}