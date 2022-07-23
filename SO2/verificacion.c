// AUTORES: AINA MARIA TUR SERRANO Y CONSTANTINO BYELOV
#include "verificacion.h"
#define DEBUG3 1
int main(int argc, char const *argv[])
{
    // Comprobar la sintaxis  //Uso: verificacion <nombre_dispositivo> <directorio_simulación>
    if (argc != 3)
    {
        fprintf(stderr,"Error de sintaxis: ./verificacion <nombre_dispositivo> <directorio_simulación>\n");
        return -1;
    }
    //Montar el dispositivo
    if(bmount(argv[1])<0){
        fprintf(stderr,"Error al montar el dispositivo\n");
        return -1;
    }
    //El directorio es /simul_aaaammddhhmmss/
    #if DEBUG3
    fprintf(stderr,"El directorio simulacion:%s\n",argv[2]);
    #endif
    //Calcular el nº de entradas del directorio de simulación a partir del stat de su inodo
    struct STAT stat;
    mi_stat(argv[2],&stat);
    int num_entradas= stat.tamEnBytesLog/sizeof(struct entrada);
    #if DEBUG3
    fprintf(stderr,"Num_entradas: %d NUMPROCESOS: %d\n",num_entradas,NUMPROCESOS);
    #endif
    if(num_entradas!=NUMPROCESOS){
        fprintf(stderr,"Error:el número de entradas es diferente al número de procesos\n");
        //desmontamos dispositivos y salimos
        bumount();
        return -1;
    }
    //creamos fichero informe.txt
    char informe[100];
    sprintf(informe,"%s%s",argv[2],"informe.txt");
    if(mi_creat(informe,6)<0){
        //Si error:
        fprintf(stderr,"Error al crear el fichero informe.txt \n");
        bumount();
        exit(0);
    }
    //Leemos entradas 
    //MEJORA IMPLEMENTADA: Se leen todas las entradas al principio del bucle 
    //Declaramos buffer del tamaño NUMPROCESOS *sizeof(struct entrada)
    struct entrada entradas[ NUMPROCESOS * sizeof (struct entrada)];
    //Rellenamos el buffer con 0
    memset(entradas,0,sizeof(entradas));
    mi_read(argv[2],entradas,0,sizeof(entradas));
    int nbytes=0;
    //Bucle extrae PID cada entrada y guarda registro INFO
    for(int pos=0; pos<num_entradas; pos++){
        //sacar pid y guardarlo en INFORMACION
        pid_t pid= atoi(strchr(entradas[pos].nombre,'_')+1);
        struct INFORMACION info;
        info.pid=pid;
        info.nEscrituras=0;
        //Fichero prueba.dat en el directorio simulacion 
        char prueba[128];
        sprintf(prueba,"%s%s%s",argv[2],entradas[pos].nombre,"/prueba.dat");
        //Inicializar buffer N registros de escritura
        int cant_registros_buffer_escrituras = 256; 
        struct REGISTRO buffer_escrituras [cant_registros_buffer_escrituras];
        memset(buffer_escrituras, 0, sizeof(buffer_escrituras));
        int offset=0;
        int leidos;
        //Recorrer fichero prueba.dat mientras haya escrituras
        while ((leidos=mi_read(prueba, buffer_escrituras, offset, sizeof(buffer_escrituras))) > 0) {
            //Leer una escritura
            offset+=leidos;
            int num=0;

            while(num<cant_registros_buffer_escrituras){
                //Comprobar escritura válida
                if(buffer_escrituras[num].pid==info.pid){
                    //Si primera escritura validada
                    if(info.nEscrituras==0){
                    //Inicializar los registros significativos con los datos de esa escritura 
                    //ya será la de menor posición puesto que hacemos un barrido secuencial
                    info.MenorPosicion=buffer_escrituras[num];
                    info.MayorPosicion=buffer_escrituras[num];
                    info.PrimeraEscritura=buffer_escrituras[num];
                    info.UltimaEscritura=buffer_escrituras[num];
                    info.nEscrituras++;
                    }else{
                    //Sino comparar la fecha de las escrituras y la nEscritura para actualizar primera y ultima
                    if(((difftime(buffer_escrituras[num].fecha, info.PrimeraEscritura.fecha))<=0)&&(buffer_escrituras[num].nEscritura<info.PrimeraEscritura.nEscritura)){
                        info.PrimeraEscritura=buffer_escrituras[num];
                    } 
                    if(((difftime(buffer_escrituras[num].fecha, info.UltimaEscritura.fecha))>=0)&&(buffer_escrituras[num].nEscritura >info.UltimaEscritura.nEscritura)){
                        info.UltimaEscritura=buffer_escrituras[num];
                    }  
                    //Comparar registro para actualizar el de mayor o menor posicion   
                    if(buffer_escrituras[num].nRegistro<info.MenorPosicion.nRegistro){
                        info.MenorPosicion=buffer_escrituras[num];
                    }
                    if(buffer_escrituras[num].nRegistro>info.MayorPosicion.nRegistro){
                        info.MayorPosicion=buffer_escrituras[num];
                    }
                    //Incrementamos contador de escrituras validadas
                    info.nEscrituras++;
                    }
                }
                //Siguiente registro
                num++;
            }
        //Limpiamos buffer escrituras    
        memset(&buffer_escrituras,0,sizeof(buffer_escrituras));
        }
       
    //Tiempo de las escrituras
    char tiempo_MenorPosicion[24];
    char tiempo_MayorPosicion[24];
    char tiempo_PrimeraEscritura[24];
    char tiempo_UltimaEscritura[24];
    struct tm *tm;
    //FORMATEAMOS FECHA
    tm=localtime(&info.MenorPosicion.fecha);
    strftime(tiempo_MenorPosicion,sizeof(tiempo_MenorPosicion),"%a %d-%m-%Y %H:%M:%S",tm);
    tm=localtime(&info.MayorPosicion.fecha);
    strftime(tiempo_MayorPosicion,sizeof(tiempo_MayorPosicion),"%a %d-%m-%Y %H:%M:%S",tm);
    tm=localtime(&info.PrimeraEscritura.fecha);
    strftime(tiempo_PrimeraEscritura,sizeof(tiempo_PrimeraEscritura),"%a %d-%m-%Y %H:%M:%S",tm);
    tm=localtime(&info.UltimaEscritura.fecha);
    strftime(tiempo_UltimaEscritura,sizeof(tiempo_UltimaEscritura),"%a %d-%m-%Y %H:%M:%S",tm);

    //Inicializamos un buffer para añadir la información del struct al informe
    char buffer_informacion[BLOCKSIZE];
    memset(&buffer_informacion,0,BLOCKSIZE);
    //Almacenamos en el buffer la info del struct
    sprintf(buffer_informacion,"PID:%i\nNumero de escrituras:\t%d\nPrimera Escritura\t%d\t%d\t%s\nUltima Escritura\t%d\t%d\t%s\nMenor posicion\t%d\t%d\t%s\nMayor posicion\t%d\t%d\t%s\n\n", info.pid, info.nEscrituras,info.PrimeraEscritura.nEscritura, info.PrimeraEscritura.nRegistro,tiempo_PrimeraEscritura,info.UltimaEscritura.nEscritura, info.UltimaEscritura.nRegistro,tiempo_UltimaEscritura,info.MenorPosicion.nEscritura, info.MenorPosicion.nRegistro,tiempo_MenorPosicion,info.MayorPosicion.nEscritura, info.MayorPosicion.nRegistro,tiempo_MenorPosicion);
    //Escribimos en informe.txt
    if((mi_write(informe,&buffer_informacion,nbytes,strlen(buffer_informacion)))<0){
        bumount();
        exit(0);
    }
    nbytes+=strlen(buffer_informacion);
    #if DEBUG3 
    fprintf(stderr,"%d) 50 escrituras validadas en %s\n",pos+1, prueba);
    #endif
    }
} 
