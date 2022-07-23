//AUTORES: Aina Maria Tur Serrano y Constantino Byelov.
#include "directorios.h"
#define DEBUG4 1
void mostrar_buscar_entrada(char *camino, char reservar){
  unsigned int p_inodo_dir = 0;
  unsigned int p_inodo = 0;
  unsigned int p_entrada = 0;
  int error;
  printf("\ncamino: %s, reservar: %d\n", camino, reservar);
  if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, reservar, 6)) < 0) {
    mostrar_error_buscar_entrada(error);
  }
  printf("**********************************************************************\n");
  return;
}
 
int main(int argc, char **argv)
{
    // Si hay más de un argumento, erro
    if (argc>2)
    {
        #if DEBUG
        fprintf(stderr, "ERROR %d: %s\n", errno, strerror(errno));
        #endif
        return -1;
    }
    bmount(argv[1]);
    /*struct tm *ts;
    char atime[80];
    char mtime[80];
    char ctime[80];*/
    struct superbloque SB;
    //struct inodo inodo;
    //int ninodo;
    if(bread(posSB, &SB)==-1){
        #if DEBUG
        fprintf(stderr, "ERROR %d: %s\n", errno, strerror(errno));
        #endif
        return -1;
    }    
    #if DEBUG4
    printf("DATOS DEL SUPERBLOQUE:\n");
    printf("posPrimerBloqueMB:%d\n", SB.posPrimerBloqueMB);
    printf("posUltimoBloqueMB:%d\n", SB.posUltimoBloqueMB);
    printf("posPrimerBloqueAI:%d\n", SB.posPrimerBloqueAI);
    printf("posUltimoBloqueAI: %d\n", SB.posUltimoBloqueAI);
    printf("posPrimerBloqueDatos:%d\n", SB.posPrimerBloqueDatos);
    printf("posUltimoBloque:%d\n", SB.posUltimoBloqueDatos);
    printf("posInodoRaiz:%d\n", SB.posInodoRaiz);
    printf("posPrimerInodoLibre: %d\n", SB.posPrimerInodoLibre);
    printf("cantBloquesLibres: %d\n", SB.cantBloquesLibres);
    printf("cantInodosLibres: %d\n", SB.cantInodosLibres);
    printf("totBloques: %d\n", SB.totBloques);
    printf("totInodos:%d\n", SB.totInodos);
    //printf("sizeof struct inodo is: %lu\n", sizeof(struct superbloque));
    //printf("sizeof struct inodo is: %lu\n", sizeof(struct inodo));
    #endif

   // lista enlazada de inodos
    //struct inodo inodos[BLOCKSIZE / INODOSIZE];
    //printf("RECORRIDO LISTA ENLAZADA DE INODOS LIBRES\n");
  /* for (int i =SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++)
    {
        bread(i,&inodos);
        for (int j = 0; j < BLOCKSIZE / INODOSIZE; j++)
        {                         // para cada inodo del AI
            if(inodos[j].tipo == 'l')// libre
            {
                 printf(" %d",inodos[j].punterosDirectos[0]);
            }   
        }
    }

    printf("RESERVAMOS UN BLOQUE Y LUEGO LO LIBERAMOS\n");
    reservar_bloque();
    bread(posSB, &SB);
    printf("Se ha reservado el bloque físico %d que era el 1º libre indicado por el MB\n",SB.posPrimerBloqueDatos);
    printf("SB.cantBloquesLibres: %d\n", SB.cantBloquesLibres);
    liberar_bloque(SB.posPrimerBloqueDatos);
    bread(posSB, &SB);
    printf("Liberamos ese bloque y después SB.cantBloquesLibres: %d\n", SB.cantBloquesLibres);

    printf("\n\n\nMAPA DE BITS CON BLOQUES DE METADATOS OCUPADOS\n");
    leer_bit(posSB);
    leer_bit(SB.posPrimerBloqueMB);
    leer_bit(SB.posUltimoBloqueMB);
    leer_bit(SB.posPrimerBloqueAI);
    leer_bit(SB.posUltimoBloqueAI);
    leer_bit(SB.posPrimerBloqueDatos);
    leer_bit(SB.posUltimoBloqueDatos);

    printf("\n\n\nDATOS DEL DIRECTORIO RAIZ\n");
    ninodo=SB.posInodoRaiz;
    leer_inodo(ninodo, &inodo);
    printf("tipo: %s\n",&inodo.tipo);
    printf("permisos: %d\n",inodo.permisos);
    ts = localtime(&inodo.atime);
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&inodo.mtime);
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&inodo.ctime);
    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);
    printf("ATIME: %s\n",atime);
    printf("MTIME: %s\n",mtime); 
    printf("CTIME: %s\n",ctime);
    printf("nlinks: %d\n",inodo.nlinks);
    printf("tamañoEnBytesLog: %d \n",inodo.tamEnBytesLog);
    printf("numBloquesOcupados: %d\n",inodo.numBloquesOcupados);
    */

   /*
   ninodo=reservar_inodo('f',6);
   bread(posSB,&SB);
   traducir_bloque_inodo(ninodo,8,1);
   traducir_bloque_inodo(ninodo,204,1);
   traducir_bloque_inodo(ninodo,30004,1);
   traducir_bloque_inodo(ninodo,400004,1);
   traducir_bloque_inodo(ninodo,468750,1);

    
    leer_inodo(ninodo, &inodo);
    printf("tipo: %s\n",&inodo.tipo);
    printf("permisos: %d\n",inodo.permisos);
    ts = localtime(&inodo.atime);
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&inodo.mtime);
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&inodo.ctime);
    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);
    printf("ATIME: %s\n",atime);
    printf("MTIME: %s\n",mtime); 
    printf("CTIME: %s\n",ctime);
    printf("nlinks: %d\n",inodo.nlinks);
    printf("tamañoEnBytesLog: %d \n",inodo.tamEnBytesLog);
    printf("numBloquesOcupados: %d\n",inodo.numBloquesOcupados);

  
    printf("\n\nSB.posPrimerInodoLibre: %d\n",SB.posPrimerInodoLibre);
  //Mostrar creación directorios y errores
  
  mostrar_buscar_entrada("pruebas/", 1); //ERROR_CAMINO_INCORRECTO
  mostrar_buscar_entrada("/pruebas/", 0); //ERROR_NO_EXISTE_ENTRADA_CONSULTA
  mostrar_buscar_entrada("/pruebas/docs/", 1); //ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO
  mostrar_buscar_entrada("/pruebas/", 1); // creamos /pruebas/
  mostrar_buscar_entrada("/pruebas/docs/", 1); //creamos /pruebas/docs/
  mostrar_buscar_entrada("/pruebas/docs/doc1", 1); //creamos /pruebas/docs/doc1
  mostrar_buscar_entrada("/pruebas/docs/doc1/doc11", 1);  
  //ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO
  mostrar_buscar_entrada("/pruebas/", 1); //ERROR_ENTRADA_YA_EXISTENTE
  mostrar_buscar_entrada("/pruebas/docs/doc1", 0); //consultamos /pruebas/docs/doc1
  mostrar_buscar_entrada("/pruebas/docs/doc1", 1); //creamos /pruebas/docs/doc1
  mostrar_buscar_entrada("/pruebas/casos/", 1); //creamos /pruebas/casos/
  mostrar_buscar_entrada("/pruebas/docs/doc2", 1); //creamos /pruebas/docs/doc2
  */
  bumount();
}