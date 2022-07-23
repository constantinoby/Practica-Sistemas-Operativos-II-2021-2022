//AUTORES: Aina Maria Tur Serrano y Constantino Byelov.
#include "ficheros.h"
//Este programa escribirá texto en uno o varios inodos
int main(int argc, char **argv){
//Miro si al meter el comando me meten todos los parametros
if (argc!=4)
{
    char c='"';
    printf("Sintaxis: escribir <nombre_dispositivo> <%c$(cat fichero)%c> <diferentes_inodos>\n",c,c);
    printf("Offsets: 9000, 209000, 30725000, 409605000, 480000000\n");
    printf("Si diferentes_inodos=0 se reserva un solo inodo para todos los offsets\n");
    return -1;
}
 
char* nombre_dispositivo=argv[1];
//monto el dispositivo virtual con el nombre que me pasan por parametro
bmount(nombre_dispositivo);
int longitud_texto=strlen(argv[2]);
#if DEBUG
printf("Longitud texto: %i\n\n",longitud_texto);
#endif
int ninodo=reservar_inodo('f',6);
int offsets[5]={9000, 209000, 30725000, 409605000, 480000000};
int diferentes_inodos=atoi(argv[3]);

int bytes_escritos;
struct STAT stat;
unsigned char buf_original[BLOCKSIZE];

for (int i = 0; i < 5; i++)
{
   #if DEBUG 
   printf("Nº inodo reservado: %i\n",ninodo);
   printf("offset: %i\n",offsets[i]);
   #endif
   memset(buf_original,0, BLOCKSIZE); 
   bytes_escritos=mi_write_f(ninodo,argv[2],offsets[i],longitud_texto);
   //Después de probar, comentar
   // memset(buf_original,0,BLOCKSIZE);
   // mi_read_f(ninodo,buf_original,offsets[i],BLOCKSIZE);
   //write(1,buf_original,longitud_texto);
   //Mostramos datos por pantalla
   mi_stat_f(ninodo,&stat);
   #if DEBUG
   printf("Bytes escritos: %i\n",bytes_escritos);
   printf("stat.tamEnBytesLog= %i\n",stat.tamEnBytesLog);
   printf("stat.numBloquesOcupados= %i\n\n",stat.numBloquesOcupados);
   #endif
    //Si diferentes inodos=0 reserva un solo inodo para todos los offsets
   if (diferentes_inodos!=0 && i!=4)
   {
        ninodo=reservar_inodo('f',6);
   }
}
//Desmontamos el dispositivo virtual
bumount();
}