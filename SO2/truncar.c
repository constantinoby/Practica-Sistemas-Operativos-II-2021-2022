// AUTORES: AINA MARIA TUR SERRANO Y CONSTANTINO BYELOV
#include "ficheros.h"
int main(int argc, char **argv)
{
    // valido que haya 4 parametros
    if (argc == 4)
    {
        char *nombre_dispositivo = argv[1];
        // Monto el dispositivo con el nombre del parametro
        bmount(nombre_dispositivo);
        // si nbytes es 0
        if (atoi(argv[3]) == 0)
        {
            liberar_inodo(atoi(argv[2]));
        }
        else
        {
            mi_truncar_f(atoi(argv[2]), atoi(argv[3]));
        }
#if DEBUG
        struct tm *ts;
        char atime[80];
        char mtime[80];
        char ctime[80];

        printf("\n\n\nDATOS DEL INDODO %d\n", atoi(argv[2]));
        struct inodo inodo;
        leer_inodo(atoi(argv[2]), &inodo);
        printf("tipo: %s\n", &inodo.tipo);
        printf("permisos: %d\n", inodo.permisos);
        ts = localtime(&inodo.atime);
        strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
        ts = localtime(&inodo.mtime);
        strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
        ts = localtime(&inodo.ctime);
        strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);
        printf("ATIME: %s\n", atime);
        printf("MTIME: %s\n", mtime);
        printf("CTIME: %s\n", ctime);
        printf("nlinks: %d\n", inodo.nlinks);
        printf("tamañoEnBytesLog: %d \n", inodo.tamEnBytesLog);
        printf("numBloquesOcupados: %d\n", inodo.numBloquesOcupados);
#endif
        // desmonto el dispositivo
        bumount();
    }
}