// AUTORES: AINA MARIA TUR SERRANO Y CONSTANTINO BYELOV
#include "simulacion.h"
#define DEBUG2 1
// Variable global cuenta nº procesos acabados
static int acabados = 0;
// Función enterrador
void reaper()
{
    pid_t ended;
    signal(SIGCHLD, reaper);
    while ((ended = waitpid(-1, NULL, WNOHANG)) > 0)
    {
        acabados++;
        //printf("Proceso_%d con PID %d Con total %d escrituras acabadas\n", ended, ended, acabados);
    }
}
// Función transforma tiempo en nanosegundos
void my_sleep(unsigned msec)
{ // recibe tiempo en milisegundos
    struct timespec req, rem;
    int err;
    req.tv_sec = msec / 1000;              // conversión a segundos
    req.tv_nsec = (msec % 1000) * 1000000; // conversión a nanosegundos
    while ((req.tv_sec != 0) || (req.tv_nsec != 0))
    {
        if (nanosleep(&req, &rem) == 0)
            // rem almacena el tiempo restante si una llamada al sistema
            // ha sido interrumpida por una señal
            break;
        err = errno;
        // Interrupted; continue
        if (err == EINTR)
        {
            req.tv_sec = rem.tv_sec;
            req.tv_nsec = rem.tv_nsec;
        }
    }
}

int main(int argc, char **argv)
{
    // En el main() asociar la señal SIGCHLD al enterrador
    // Comprobar la sintaxis del comando  // uso: ./simulacion <disco>
    if (argc != 2)
    {
        fprintf(stderr,"Error: Sintaxis: ./simulacion <disco>\n");
        return -1;
    }
    // Montar el dispositivo //padre
    if (bmount(argv[1]) < 0)
    {
        fprintf(stderr,"Error al montar el dispositivo\n");
        exit(0);
    }

    // Crear el directorio de simulación: /simul_aaaammddhhmmss/
    char simulacion[21] = "/simul_";
    struct tm *tm; // ver info: struct tm
    time_t timenow;
    time(&timenow);
    tm = localtime(&timenow);
    sprintf(simulacion + strlen(simulacion), "%d%02d%02d%02d%02d%02d/", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
    // Creamos directorio simulación
    if (mi_creat(simulacion, 6) < 0)
    {
        // Si error
        fprintf(stderr,"Error al crear el directorio simulacion %s\n", simulacion);
        bumount();
        exit(0);
    }
    // Mostramos el nombre del directorio y asociamos señal SIGCHLD al enterrador
    #if DEBUG2
    fprintf(stderr,"directorio simulacion:%s\n", simulacion);
    #endif
    signal(SIGCHLD, reaper);
    // Este bucle recorre los procesos hijos
    pid_t pid;
    for (int proceso = 1; proceso <= NUMPROCESOS; proceso++)
    {
        pid = fork();
        if (pid == 0)
        {
            // Montar dispositivo hijo
            bmount(argv[1]);
            // creamos el directorio del proceso hijo dentro del directorio /simul_
            char proc[38];
            sprintf(proc, "%sproceso_%d/", simulacion, getpid());
            if (mi_creat(proc, 6) < 0)
            {
                fprintf(stderr,"ERROR al crear el directorio del proceso hijo\n");
                bumount();
                exit(0);
            }
            // Creamos fichero prueba.dat
            char fichero[48];
            sprintf(fichero, "%sprueba.dat", proc);
            if (mi_creat(fichero, 6) < 0)
            {
                fprintf(stderr,"Error al crear el fichero prueba.dat del proceso\n");
                bumount();
                exit(0);
            }
            // Inicializar semilla de números aleatorios
            srand(time(NULL) + getpid());
            // Para cada escritura inicializar los registros
            for (int nescritura = 0; nescritura < NUMESCRITURAS; nescritura++)
            {
                // Inicializar registro
                struct REGISTRO registro;
                registro.fecha = time(NULL);
                registro.pid = getpid();
                registro.nEscritura = nescritura + 1;
                registro.nRegistro = rand() % REGMAX;
                mi_write(fichero, &registro, (registro.nRegistro * sizeof(struct REGISTRO)), sizeof(struct REGISTRO));
                //fprintf(stderr,"[simulacion.c-> Escritura %d en %s]\n", nescritura + 1, fichero);
                #if DEBUG2
                if(nescritura==NUMESCRITURAS-1){ 
                    fprintf(stderr,"[Proceso %d-> %d escrituras completadas en %s]\n", proceso, nescritura + 1, fichero);
                }
                #endif
                // usleep(50000);
                my_sleep(50);
            }
            bumount();
            exit(0);
        }
        // usleep(150000);
        my_sleep(150);
    }
    // Permitir que el padre espere por todos los hijos:
    while (acabados < NUMPROCESOS)
    {
        pause();
    }
    #if DEBUG2
    fprintf(stderr, "Total de procesos terminados: %d\n", acabados);
    #endif
    bumount();
    exit(0);
}
