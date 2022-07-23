//AUTORES: Aina Maria Tur Serrano y Constantino Byelov.
#include "ficheros.h"
// Función que escribe el contenido procedente de un buffer de nbytes en un fichero/directorio
int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes)
{
    // Primer y ultimo bloque lógico
    int primerBL;
    int ultimoBL;
    // byte dentro del bloque lógico 8 donde cae el offset
    int desp1;
    // byte lógico hasta donde escribimos
    int desp2;
    // Bloque físico
    int nbfisico;
    // bytes escritos
    int escritos;
    // guardaremos datos leidos bloque fisico
    unsigned char buf_bloque[BLOCKSIZE];
    // leer inodo de ninodo
    struct inodo inodo;
    leer_inodo(ninodo, &inodo);
    // Si no tiene permisos w=1, no se escribe(010, 011, 110 o 111)
    if ((inodo.permisos & 2) != 2)
    {
#if DEBUG
        printf("No tiene permisos de escritura");
#endif
        return -1;
    }

    primerBL = offset / BLOCKSIZE;
    ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;
    desp1 = offset % BLOCKSIZE;
    desp2 = (offset + nbytes - 1) % BLOCKSIZE;
    mi_waitSem();
    nbfisico = traducir_bloque_inodo(ninodo, primerBL, 1);
    mi_signalSem();
    if (bread(nbfisico, buf_bloque) == -1){
        return -1;
    }  
      
    // 1 solo bloque
    if (primerBL == ultimoBL)
    {
        // Escribimos en el bloque fisico el contenido de buf_bloque modificado
        memcpy(buf_bloque + desp1, buf_original, nbytes);
        if (bwrite(nbfisico, buf_bloque) == -1)
        {
#if DEBUG
            fprintf(stderr, "ERROR %d: %s\n", errno, strerror(errno));
#endif
            return -1;
        }
        escritos = nbytes;
    }
    else
    {
        // Si cabe en más de un bloque
        // Primer bloque
        memcpy(buf_bloque + desp1, buf_original, BLOCKSIZE - desp1);
        if (bwrite(nbfisico, buf_bloque) == -1)
        {
#if DEBUG
            fprintf(stderr, "ERROR %d: %s\n", errno, strerror(errno));
#endif
            return -1;
        }
        escritos = BLOCKSIZE - desp1;

        // Bloques intermedios
        for (int i = primerBL + 1; i < ultimoBL; i++)
        {
            mi_waitSem();
            nbfisico = traducir_bloque_inodo(ninodo, i, 1);
            mi_signalSem();
            if (bwrite(nbfisico, buf_original + (BLOCKSIZE - desp1) + (i - primerBL - 1) * BLOCKSIZE) == -1)
            {
#if DEBUG
                fprintf(stderr, "ERROR %d: %s\n", errno, strerror(errno));
#endif
                return -1;
            }
            escritos = escritos + BLOCKSIZE;
            
        }
        // Ultimo bloque
        mi_waitSem();
        nbfisico = traducir_bloque_inodo(ninodo, ultimoBL, 1);
        mi_signalSem();
        if (bread(nbfisico, buf_bloque) == -1){
            return -1;
        }    
        memcpy(buf_bloque, buf_original + (nbytes - desp2 - 1), desp2 + 1);
        if (bwrite(nbfisico, buf_bloque) == -1)
        {
#if DEBUG
            fprintf(stderr, "ERROR %d: %s\n", errno, strerror(errno));
#endif
            return -1;
        }
        escritos = escritos + desp2 + 1;
    }
    mi_waitSem();
    // Si hemos escrito más bytes, actualizar información, timestamps y escribir inodo actualizado.
    leer_inodo(ninodo, &inodo);
    if (inodo.tamEnBytesLog < (offset + escritos))
    {

        inodo.tamEnBytesLog = offset + escritos;
        inodo.ctime = time(NULL);
    }
    inodo.mtime = time(NULL);
    escribir_inodo(ninodo, inodo);
    mi_signalSem();
    return escritos;
}
// Función que lee información de un fichero/directorio y la guarda en un buffer
int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes)
{
    int primerBL;
    int ultimoBL;
    int desp1;
    int desp2;
    int nbfisico;
    int leidos = 0;
    unsigned char buf_bloque[BLOCKSIZE];
    // Lee inodo posicion ninodo
    struct inodo inodo;
    leer_inodo(ninodo, &inodo);
    if (offset >= inodo.tamEnBytesLog)
    {
        leidos = 0; // No podemos leer nada
        return leidos;
    }
    if ((offset + nbytes) >= inodo.tamEnBytesLog)
        // pretende leer más allá de EOF
        nbytes = inodo.tamEnBytesLog - offset;
    // leemos sólo los bytes que podemos desde el offset hasta EOF
    // Si no tiene permisos r=1, no se lee (100, 101, 110 o 111)
    if ((inodo.permisos & 4) != 4)
    {
#if DEBUG
        printf("No tiene permisos de lectura\n");
#endif
        return -1;
    }
    primerBL = offset / BLOCKSIZE;
    ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;
    desp1 = offset % BLOCKSIZE;
    desp2 = (offset + nbytes - 1) % BLOCKSIZE;
    nbfisico = traducir_bloque_inodo(ninodo, primerBL, 0);

    // 1 solo bloque
    if (primerBL == ultimoBL)
    {
        // Si no hay bloque físico asignado a un bloque lógico
        if (nbfisico != -1)
        { // leemos el contenido del nbfisico
            if (bread(nbfisico, buf_bloque) == -1){
                return -1;
            }
            memcpy(buf_original, buf_bloque + desp1, nbytes);
        }
        // sumamos los bytes leídos
        leidos = nbytes;
    }
    else
    {
        // Si cabe en más de un bloque
        // Primer bloque
        if (nbfisico != -1)
        {
            if (bread(nbfisico, buf_bloque) == -1){
                return -1;
            }    
            memcpy(buf_original, buf_bloque + desp1, BLOCKSIZE - desp1);
        }
        leidos = BLOCKSIZE - desp1;
        // Bloques intermedios
        for (int i = primerBL + 1; i < ultimoBL; i++)
        {
            nbfisico = traducir_bloque_inodo(ninodo, i, 0);
            if (nbfisico != -1)
            {
                if (bread(nbfisico, buf_bloque) == -1){
                    return -1;
                }    
                memcpy(buf_original + (BLOCKSIZE - desp1) + (i - primerBL - 1) * BLOCKSIZE, buf_bloque, BLOCKSIZE);
            }
            leidos = leidos + BLOCKSIZE;
        }
        // Ultimo bloque
        nbfisico = traducir_bloque_inodo(ninodo, ultimoBL, 0);
        if (nbfisico != -1)
        {
            if (bread(nbfisico, buf_bloque) == -1){   
                return -1;
            }    
            memcpy(buf_original + (nbytes - desp2 - 1), buf_bloque, desp2 + 1);
        }
        leidos = leidos + desp2 + 1;
    }
    mi_waitSem();
    leer_inodo(ninodo, &inodo);
    // Actualizar info del inodo
    inodo.atime = time(NULL);
    escribir_inodo(ninodo, inodo);
    mi_signalSem();
    // Devolver nº bytes leidos
    return leidos;
}

// Función que devuelve la metainformación de un fichero/directorio
int mi_stat_f(unsigned int ninodo, struct STAT *p_stat)
{
    // Leemos el inodo de la posición ninodo
    struct inodo ino;
    leer_inodo(ninodo, &ino);
    // asignamos info de inodo a p_stat
    p_stat->tipo = ino.tipo;
    p_stat->permisos = ino.permisos;
    p_stat->nlinks = ino.nlinks;
    p_stat->tamEnBytesLog = ino.tamEnBytesLog;
    p_stat->atime = ino.atime;
    p_stat->ctime = ino.ctime;
    p_stat->mtime = ino.mtime;
    p_stat->numBloquesOcupados = ino.numBloquesOcupados;

    return 0;
}
// Función que cambia los permisos de un fichero/directorio
int mi_chmod_f(unsigned int ninodo, unsigned char permisos)
{
    // Leemos el inodo de la posición ninodo
    struct inodo ino;
    mi_waitSem();
    leer_inodo(ninodo, &ino);
    // Asignamos nuevos permisos y actualizamos c_time
    ino.permisos = permisos;
    ino.ctime = time(NULL);
    // Escribimos el inodo en la posicion ninodo
    escribir_inodo(ninodo, ino);
    mi_signalSem();
    return 0;
}
int mi_truncar_f(unsigned int ninodo, unsigned int nbytes)
{
    int primerBL, liberados;
    struct inodo inodo;
    struct superbloque SB;
    if (bread(posSB, &SB) == -1)
        return -1;  
    mi_waitSem();     
    leer_inodo(ninodo, &inodo);
    // Si no tiene permisos w=1, no se escribe(010, 011, 110 o 111)
    if ((inodo.permisos & 2) != 2)
    {
#if DEBUG
        printf("No tiene permisos de escritura");
#endif
        return -1;
    }
    //Calculamos el primer bloque lógico
    if (nbytes % BLOCKSIZE == 0)
    {
        primerBL = nbytes / BLOCKSIZE;
    }
    else
    {
        primerBL = nbytes / BLOCKSIZE + 1;
    }
    //liberamos los bloques
    liberados = liberar_bloques_inodo(primerBL, &inodo);
    //Actualizamos información del inodo
    inodo.ctime = time(NULL);
    inodo.mtime = time(NULL);
    inodo.tamEnBytesLog = nbytes;
    inodo.numBloquesOcupados = inodo.numBloquesOcupados - liberados;
    SB.cantBloquesLibres+= liberados;
    //Escribimos la información actualizada
    escribir_inodo(ninodo, inodo);
    mi_signalSem();
    if (bwrite(posSB, &SB) == -1)
    {
#if DEBUG
        fprintf(stderr, "ERROR %d: %s\n", errno, strerror(errno));
#endif
        return -1;
    }
    return liberados;
}
