//AUTORES: Aina Maria Tur Serrano y Constantino Byelov.
#include "bloques.h"
#include "semaforo_mutex_posix.h"

//Variable global descriptor del fichero
static int descriptor = 0;
// Variable global para el semáforo
static sem_t *mutex;
// Variable global permite evitar wait dos veces
static unsigned int inside_sc = 0;

//Funcion para montar el dispositivo virtual
int bmount(const char *camino)
{
    if(descriptor>0){
        close(descriptor);
    }
    //Funcion muestra los permisos de creacion de archivos 
    umask(000);
    //Crea y abre el fichero con permisos de lectura y escritura
    descriptor = open(camino, O_RDWR | O_CREAT, 0666);
    //Si descritor -1, se ha producido un error
    if (descriptor == -1)
    {
    #if DEBUG    
        fprintf(stderr, "ERROR %d: %s\n", errno, strerror(errno));
    #endif    
    }
    if (!mutex)
    { // el semáforo es único en el sistema y sólo se ha de inicializar 1 vez (padre)
        mutex = initSem();
        if (mutex == SEM_FAILED)
        {
            return -1;
        }
    }
    
    //Devolvemos descriptor
    return descriptor;
}
//Funcion que desmonta el dispositivo virtual
int bumount()
{
    descriptor = close(descriptor);
    //Liberamos el descriptor
    //Si error
    if(close(descriptor) == -1)
    {
        #if DEBUG   
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
        #endif
        return -1;
    }
     // Eliminamos semáforo
    deleteSem();
    return 0;

}
//Funcion escribe 1 bloque en el dispositivo virtual
int bwrite(unsigned int nbloque, const void *buf)
{
    //Posicion en bytes al bloque que queremos escribir
    off_t desplazamiento = nbloque * BLOCKSIZE;
    //Mover puntero, comenzando desde el inicio fichero
    if (lseek(descriptor, desplazamiento, SEEK_SET) == -1)
    {
        //Si error
        #if DEBUG   
        fprintf(stderr, "ERROR %d: %s\n", errno, strerror(errno));
        #endif
    }
    //Escribir el bloque
    int escrito = write(descriptor, buf, BLOCKSIZE);
    //Si error
    if (escrito == -1)
    {
        #if DEBUG   
        fprintf(stderr, "ERROR %d: %s\n", errno, strerror(errno));
        #endif
    }
    //Numero bytes escritos(BLOCKSIZE)
    return escrito;
}
//Funcion lee 1 bloque del dispositivo virtual
int bread(unsigned int nbloque, void *buf)
{
    //Posicion en bytes al bloque que queremos leer
    off_t desplazamiento = nbloque * BLOCKSIZE;
    //Mover puntero, comenzando desde el inicio fichero
    if (lseek(descriptor, desplazamiento, SEEK_SET) == -1)
    {
        //Si error
        #if DEBUG   
        fprintf(stderr, "ERROR %d: %s\n", errno, strerror(errno));
        #endif
    }
    //Leer bloque
    int lectura = read(descriptor, buf, BLOCKSIZE);
    //Si error
    if (lectura == -1)
    {
        #if DEBUG   
        fprintf(stderr, "ERROR %d: %s\n", errno, strerror(errno));
        #endif
    }
    //Numero bytes leidos
    return lectura;
}
//Realiza wait semaforo
void mi_waitSem()
{
    if (!inside_sc)
    { // inside_sc==0
        waitSem(mutex);
    }
    inside_sc++;
}
//Realiza signal semaforo
void mi_signalSem()
{
    inside_sc--;
    if (!inside_sc)
    {
        signalSem(mutex);
    }
}
