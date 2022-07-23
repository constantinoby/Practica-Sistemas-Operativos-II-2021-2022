// AUTORES: AINA MARIA TUR SERRANO Y CONSTANTINO BYELOV
#include "directorios.h"
int main(int argc, char const *argv[])
{
    // Comprobamos la sintaxis
    if (argc != 5)
    {
        printf("Error de sintaxis: ./mi_escribir <disco> </ruta_fichero> <texto> <offset>\n");
        return 0;
    }

if(argv[2][strlen(argv[2])-1]!='/'){//es un fichero
//monto el dispositivo virtual con el nombre que me pasan por parametro
if(bmount(argv[1])<0){
    return -1;
}
int longitud_texto=strlen(argv[3]);

printf("Longitud texto: %i\n",longitud_texto);
int offset= atoi(argv[4]);
int bytes_escritos;
bytes_escritos=mi_write(argv[2],argv[3],offset,strlen(argv[3]));
//Si da error indicamos que hemos escritos 0 bytes
if(bytes_escritos==-1){
    bytes_escritos=0;
}
printf("Bytes escritos: %i\n",bytes_escritos);
//Desmontamos el dispositivo virtual
bumount();
}else{
    printf("No es fichero\n");
    return -1;
}
}