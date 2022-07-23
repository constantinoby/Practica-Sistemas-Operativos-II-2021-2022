// AUTORES: AINA MARIA TUR SERRANO Y CONSTANTINO BYELOV
#include "ficheros.h"
//Este programa nos permite probar la función mi_chmod
int main(int argc, char **argv){
    // Si hay menos de 3 párametros error
    
    if (argc!=4)
    {
        #if DEBUG
        printf("Sintaxis: permitir <nombre_dispositivo> <ninodo> <permisos>\n");
        #endif
        return -1;
    }
    //Crear dispositivo virtual, abrir fichero
    bmount(argv[1]);
    //Probar mi_chmod
    mi_chmod_f(atoi(argv[2]),atoi( argv[3]));
    //Desmontamos dispositivo virtual, cerrar fichero
    bumount();
}