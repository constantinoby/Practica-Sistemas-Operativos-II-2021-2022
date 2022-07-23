//AUTORES: AINA MARIA TUR SERRANO Y CONSTANTINO BYELOV
#include "directorios.h"
int main(int argc, char const *argv[])
{
    // comprobamos la sintaxis
    if (argc != 3)
    {
        printf("Error de sintaxis: ./mi_stat <disco> </ruta>\n");
        return -1;
    }
    struct STAT p_stat;
    int p_inodo;
    struct tm *ts;
    char atime[80];
    char mtime[80];
    char ctime[80];
    
    //Montamos disco virtual
    if (bmount(argv[1]) == -1)
    {
        printf("Error de montaje de disco.\n");
        return -1;
    }
    
    //Función mi_stat de directorios
    if((p_inodo=mi_stat(argv[2],&p_stat))<0){
        return -1;
    }
    
    //Mostrar información p_stat
    printf("Nº de inodo:%d\n", p_inodo);
    printf("tipo: %c\n",p_stat.tipo); 
    printf("permisos: %d\n",p_stat.permisos); 
    ts = localtime(&p_stat.atime);
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&p_stat.mtime);
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&p_stat.ctime);
    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);
    printf("ATIME: %s\n",atime);
    printf("MTIME: %s\n",mtime); 
    printf("CTIME: %s\n",ctime);
    printf("nlinks: %d\n",p_stat.nlinks);
    printf("tamEnBytesLog: %d\n",p_stat.tamEnBytesLog);
    printf("numBloquesOcupados: %d\n",p_stat.numBloquesOcupados);
    //Desmontamos disco
    bumount();
}
