//AUTORES: Aina Maria Tur Serrano y Constantino Byelov.
#define DEBUG8 0
#include "directorios.h"
// struct UltimaEntrada UltimasEntradas[cache];
struct UltimaEntrada UltimaEntradaEscritura;
// Funcion nos permite extraer el nombre de un directorio o fichero
int extraer_camino(const char *camino, char *inicial, char *final, char *tipo)
{
    // Camino debe comenzar por '/'
    if (camino[0] != '/')
        return -1;
    // Buscamos el segundo '/'
    char *fi = strchr((camino + 1), '/');
    // Si no hay segundo '/
    if (fi == NULL)
    {
        // Es un fichero
        strcpy(inicial, (camino + 1));
        strcpy(tipo, "f");
        // No guardamos nada en final
        strcpy(final, "");
    }
    else
    {
        // Es un directorio
        strcpy(tipo, "d");
        // Guardar en inicial lo que hay entre los dos primeros '/'
        strcpy(inicial, (camino + 1));
        strtok(inicial, "/");
        // Guardar en final el resto de camino después del segundo '/
        strcpy(final, fi);
    }
    /*
    printf("Camino: %s\n\tInicio: %s\n\tFinal: %s\n", camino, inicial, final);
    printf("Tipo: %s\n", tipo);*/
    return 0;
}
// Función busca una entrada de entre las entradas del inodo del directorio padre
int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos)
{
    struct entrada entrada;
    memset(entrada.nombre, 0, sizeof(entrada.nombre));
    struct inodo inodo_dir;
    char inicial[sizeof(entrada.nombre)];
    memset(inicial, 0, sizeof(entrada.nombre));
    // Se suma uno para tener en cuenta el final
    char final[strlen(camino_parcial) + 1];
    memset(final, 0, strlen(camino_parcial) + 1);

    char tipo;
    int cant_entradas_inodo, num_entrada_inodo = 0; // nº de entrada inicial

    // Leemos el superbloque
    struct superbloque SB;
    if (bread(posSB, &SB) < 0)
    {
        return -1;
    }

    if ((strcmp(camino_parcial, "/")) == 0) // camino_parcial es “/”
    {
        *p_inodo = SB.posInodoRaiz; // nuestra raiz siempre estará asociada al inodo 0
        *p_entrada = 0;
        return 0;
    }
    // Obtenemos el nombre del directorio o fichero
    if (extraer_camino(camino_parcial, inicial, final, &tipo) < 0)
        return ERROR_CAMINO_INCORRECTO;
#if DEBUG8
    printf("[buscar_entrada()->inicial: %s, final: %s, reservar: %d]\n", inicial, final, reservar);
#endif
    // buscamos la entrada cuyo nombre se encuentra en inicial
    leer_inodo(*p_inodo_dir, &inodo_dir);
    // Si no tiene permisos r=1, no se lee (100, 101, 110 o 111)
    if ((inodo_dir.permisos & 4) != 4)
    {
#if DEBUG8
        printf("[buscar_entrada()->el inodo %d no tiene permisos de lectura]\n", *p_inodo_dir);
#endif
        return ERROR_PERMISO_LECTURA;
    }
    // inicializar el buffer de lectura con 0s
    //  el buffer de lectura puede ser un struct tipo entrada
    //  o mejor un array de las entradas que caben en un bloque, para optimizar la lectura en RAM
    struct entrada lectura[BLOCKSIZE / sizeof(struct entrada)];
    memset(lectura, 0, sizeof(lectura));
    // calcular cant_entradas_inodo // cantidad de entradas que contiene el inodo
    cant_entradas_inodo = (inodo_dir.tamEnBytesLog) / sizeof(struct entrada);
    int leidos = 0;
    if (cant_entradas_inodo > 0)
    {
        // leer entrada
        leidos += mi_read_f(*p_inodo_dir, lectura, leidos, BLOCKSIZE);
        // Mientras no sea la ultima entrada y el nombre de la entrada no sea la inicial
        while ((num_entrada_inodo < cant_entradas_inodo) && (strcmp(inicial, lectura[num_entrada_inodo % (BLOCKSIZE / sizeof(struct entrada))].nombre) != 0))
        {
            num_entrada_inodo++;
           if ((num_entrada_inodo % (BLOCKSIZE / sizeof(struct entrada))) == 0)
            {
                // leer siguiente entrada
                // previamente volver a inicializar el buffer de lectura con 0s
                memset(lectura, 0, sizeof(lectura));
                leidos += mi_read_f(*p_inodo_dir, lectura, leidos, BLOCKSIZE);
            }
        }
        // Guardamos la entrada en el struct entrada declarado
        memcpy(&entrada, &lectura[num_entrada_inodo % (BLOCKSIZE / sizeof(struct entrada))], sizeof(struct entrada));
    }
    if ((strcmp(inicial, entrada.nombre) != 0) && (num_entrada_inodo == cant_entradas_inodo))
    {
        // la entrada no existe
        switch (reservar)
        {
        case 0: // modo consulta. Como no existe retornamos error
            return ERROR_NO_EXISTE_ENTRADA_CONSULTA;
        case 1: // modo escritura
                // Creamos la entrada en el directorio referenciado por *p_inodo_dir
            // si es fichero no permitir escritura
            if (inodo_dir.tipo == 'f')
                return ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO;
            // si es directorio comprobar que tiene permiso de escritura
            // Si no tiene permisos w=1, no se escribe(010, 011, 110 o 111)
            if ((inodo_dir.permisos & 2) != 2)
            {
#if DEBUG8
                printf("[buscar_entrada()->el inodo %d no tiene permisos de escritura]\n", *p_inodo_dir);
#endif
                return ERROR_PERMISO_ESCRITURA;
            }
            else
            {
                // copiar *inicial en el nombre de la entrada
                strcpy(entrada.nombre, inicial);
                if (tipo == 'd')
                {
                    if (!strcmp(final, "/"))
                    {
                        mi_waitSem();
                        // reservar un inodo como directorio y asignarlo a la entrada
                        entrada.ninodo = reservar_inodo('d', permisos);
                        mi_signalSem();
#if DEBUG8
                        printf("[buscar_entrada()->reservado inodo %d tipo %c con permisos %d para %s]\n", entrada.ninodo, tipo, permisos, inicial);
#endif
                    }
                    else
                    { // cuelgan más diretorios o ficheros
                        return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;
                    }
                }
                else
                { // es un fichero
                  // reservar un inodo como fichero y asignarlo a la entrada
                    mi_waitSem();
                    entrada.ninodo = reservar_inodo('f', permisos);
                    mi_signalSem();
#if DEBUG8
                    printf("[buscar_entrada()->reservado inodo %d tipo %c con permisos %d para %s]\n", entrada.ninodo, tipo, permisos, inicial);
#endif
                }
#if DEBUG8
                printf("[buscar_entrada()->creada entrada: %s, %d]\n", entrada.nombre, entrada.ninodo);
#endif
                // escribir la entrada en el directorio padre
                if (mi_write_f(*p_inodo_dir, &entrada, num_entrada_inodo * sizeof(struct entrada), sizeof(struct entrada)) < 0)
                {
                    if (entrada.ninodo != -1)
                    { // entrada.inodo != -1
                        // liberar el inodo
                        mi_waitSem();
                        liberar_inodo(entrada.ninodo);
                        mi_signalSem();
#if DEBUG8
                        printf("[buscar_entrada()->liberado inodo: %s, %d]\n", entrada.nombre, entrada.ninodo);
#endif
                    }
                    return -1;
                }
            }
        }
    }
    // Si hemos legado al final del camino
    if (!strcmp(final, "/") || !strcmp(final, ""))
    {
        if ((num_entrada_inodo < cant_entradas_inodo) && (reservar == 1))
        {
            // modo escritura y la entrada ya existe
            return ERROR_ENTRADA_YA_EXISTENTE;
        }
        // cortamos la recursividad
        // asignar a *p_inodo el numero de inodo del directorio o fichero creado o leido
        *p_inodo = entrada.ninodo;
        // asignar a *p_entrada el número de su entrada dentro del último directorio que lo contiene
        *p_entrada = num_entrada_inodo;
        return 0;
    }
    else
    {
        // asignamos a *p_inodo_dir el puntero al inodo que se indica en la entrada encontrada;
        *p_inodo_dir = entrada.ninodo;
        // p_inodo_dir=lectura[num_entrada_inodo].ninodo;
        return buscar_entrada(final, p_inodo_dir, p_inodo, p_entrada, reservar, permisos);
    }
    return 0;
}
// Función muestra los errores encontrados al realizar buscar_entrada
void mostrar_error_buscar_entrada(int error)
{
    // fprintf(stderr, "Error: %d\n", error);
    switch (error)
    {
    case -1:
        fprintf(stderr, "Error: Camino incorrecto.\n");
        break;
    case -2:
        fprintf(stderr, "Error: Permiso denegado de lectura.\n");
        break;
    case -3:
        fprintf(stderr, "Error: No existe el archivo o el directorio.\n");
        break;
    case -4:
        fprintf(stderr, "Error: No existe algún directorio intermedio.\n");
        break;
    case -5:
        fprintf(stderr, "Error: Permiso denegado de escritura.\n");
        break;
    case -6:
        fprintf(stderr, "Error: El archivo ya existe.\n");
        break;
    case -7:
        fprintf(stderr, "Error: No es un directorio.\n");
        break;
    }
}
// Función que crea un fichero/directorio y su entrada de directorio.
int mi_creat(const char *camino, unsigned char permisos)
{
    mi_waitSem();
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int prob;
    // Leemos superbloque
    struct superbloque SB;
    if (bread(posSB, &SB) == -1)
    {
        return -1;
    }
    // Posicion inodo raíz
    p_inodo_dir = SB.posInodoRaiz;
    p_inodo = SB.posInodoRaiz;
    // Buscamos entrada
    if ((prob = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 1, permisos)) < 0)
    {
        mostrar_error_buscar_entrada(prob);
        mi_signalSem();
        return prob;
    }
    mi_signalSem();
    return 0;
}
// Función que pone el contenido del directorio en un buffer de memoria
int mi_dir(const char *camino, char *buffer, char tipo)
{
    struct inodo inodo;
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int total_entradas = 0;
    struct entrada entrada;

    // Para cada entrada buscamos si existe y comprobamos tipo_inodo=tipo
    int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4); // 4 son permisos de lectura
    if (error < 0)
    {
        mostrar_error_buscar_entrada(error);
        return -1;
    }

    // leemos para ver si tiene los persmisos
    leer_inodo(p_inodo, &inodo);
    if ((inodo.permisos & 4) != 4)
    {
        printf("Error: El inodo no tiene permisos de lectura");
        return -1;
    }
    if ((camino[strlen(camino) - 1] != '/') && (inodo.tipo == 'd'))
    {
        printf("Error: la sintaxis no concuerda con el tipo\n");
        return -1;
    }
    // Miramos el tipo del inodo
    if (inodo.tipo == 'd')
    {
        // es un directorio
        leer_inodo(p_inodo, &inodo);
        tipo = inodo.tipo;
        // Calulamos cant entradas inodo
        total_entradas = inodo.tamEnBytesLog / sizeof(struct entrada);
        // Inicializamos un array de entradas
        struct entrada lectura[BLOCKSIZE / sizeof(struct entrada)];
        memset(lectura, 0, sizeof(lectura));
        printf("Total: %d\n", total_entradas);
        // Comenzamos a leer y guardamos en el buffer
        int offset = 0;
        offset += mi_read_f(p_inodo, lectura, offset, BLOCKSIZE);
        for (int i = 0; i < total_entradas; i++)
        {
            // Leer el inodo
            leer_inodo(lectura[i % (BLOCKSIZE / sizeof(struct entrada))].ninodo, &inodo);
            // tipo
            if (inodo.tipo == 'd')
            {
                strcat(buffer, CYAN);
                strcat(buffer, "d\t");
            }
            else
            {
                strcat(buffer, MAGENTA);
                strcat(buffer, "f\t");
            }
            // permisos
            if (inodo.permisos & 4)
                strcat(buffer, "r");
            else
                strcat(buffer, "-");
            if (inodo.permisos & 2)
                strcat(buffer, "w");
            else
                strcat(buffer, "-");
            if (inodo.permisos & 1)
                strcat(buffer, "x");
            else
                strcat(buffer, "-");
            strcat(buffer, "\t");

            // mtime
            char tmp[100];
            struct tm *tm; // ver info: struct tm
            tm = localtime(&inodo.mtime);
            sprintf(tmp, "%d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
            strcat(buffer, tmp);
            strcat(buffer, "\t");

            // tamaño
            char inodo_tamEnBytesLog[3];
            sprintf(inodo_tamEnBytesLog, "%d", inodo.tamEnBytesLog);
            strcat(buffer, inodo_tamEnBytesLog);
            strcat(buffer, "\t");

            // NOMBRE
            strcat(buffer, lectura[i % (BLOCKSIZE / sizeof(struct entrada))].nombre);
            strcat(buffer, BLANCO);
            strcat(buffer, "\n");
            // Leemos siguiente entrada
            if ((offset % (BLOCKSIZE / sizeof(struct entrada))) == 0)
            {
                offset += mi_read_f(p_inodo, lectura, offset, BLOCKSIZE);
            }
        }
    }
    // MEJORA REALIZADA PARA LISTAR FICHEROS
    else
    {
        // Asignamos el tipo
        leer_inodo(entrada.ninodo, &inodo);
        tipo = inodo.tipo;
        // es un fichero
        mi_read_f(p_inodo_dir, &entrada, p_entrada * sizeof(struct entrada), sizeof(struct entrada));

        // tipo
        strcat(buffer, MAGENTA);
        strcat(buffer, "f\t");
        // permisos
        if (inodo.permisos & 4)
            strcat(buffer, "r");
        else
            strcat(buffer, "-");
        if (inodo.permisos & 2)
            strcat(buffer, "w");
        else
            strcat(buffer, "-");
        if (inodo.permisos & 1)
            strcat(buffer, "x");
        else
            strcat(buffer, "-");
        strcat(buffer, "\t");

        // mtime
        char tmp[100];
        struct tm *tm; // ver info: struct tm
        tm = localtime(&inodo.mtime);
        sprintf(tmp, "%d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
        strcat(buffer, tmp);
        strcat(buffer, "\t");

        // tamaño
        char inodo_tamEnBytesLog[3];
        sprintf(inodo_tamEnBytesLog, "%d", inodo.tamEnBytesLog);
        strcat(buffer, inodo_tamEnBytesLog);
        strcat(buffer, "\t");
        // NOMBRE
        strcat(buffer, entrada.nombre);

        strcat(buffer, BLANCO);
        strcat(buffer, "\n");
    }
    // Devolvemos el nº total de entradas
    return total_entradas;
}

// Función que cambia los permisos de un fichero o directorio
int mi_chmod(const char *camino, unsigned char permisos)
{
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int prob;
    struct superbloque SB;
    // Leemos el superbloque
    if (bread(posSB, &SB) == -1)
    {
        return -1;
    }
    // Asiganmos la posicion del inodo raíz a inodo_dir
    p_inodo_dir = SB.posInodoRaiz;
    // p_inodo = SB.posInodoRaiz;
    // Buscamos entrada
    if ((prob = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, permisos)) < 0)
    {
        mostrar_error_buscar_entrada(prob);
        return prob;
    }
    // Cambiamos permisos
    mi_chmod_f(p_inodo, permisos);
    return 0;
}
// Función que muestra la información acerca del inodo de un fichero o directorio
int mi_stat(const char *camino, struct STAT *p_stat)
{

    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    // Leemos el superbloque
    struct superbloque SB;
    if (bread(posSB, &SB) == -1)
    {
        return -1;
    }
    // Asignamos la posicion del inodo raíz
    p_inodo_dir = SB.posInodoRaiz;
    // p_inodo = SB.posInodoRaiz;
    int prob;
    // Buscamos entrada del camino
    if ((prob = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4)) < 0)
    {
        mostrar_error_buscar_entrada(prob);
        return prob;
    }
    // Llamamos a mi_stat_f
    if (mi_stat_f(p_inodo, p_stat) < 0)
    {
        return -1;
    }
    // Devolvemos el nº del inodo
    return p_inodo;
}
// Función que permite escribir contenido en un fichero
int mi_write(const char *camino, const void *buf, unsigned int offset, unsigned int nbytes)
{
    // Leemos el superbloque
    struct superbloque SB;
    if (bread(posSB, &SB) == -1)
    {
        return -1;
    }
    // Asiganmos la posicion inodo raíz
    unsigned int p_inodo_dir = SB.posInodoRaiz;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int bytes_escritos = 0;
    int error;
    // Miramos si la escritura es sobre el mismo inodo
    if (strcmp(UltimaEntradaEscritura.camino, camino) == 0)
    {
        p_inodo = UltimaEntradaEscritura.p_inodo;
    }
    else
    {
        // Actualizamos las variables buscando entrada del camino
        if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4)) < 0)
        {
            mostrar_error_buscar_entrada(error);
            return -1;
        }
        // Guardamos el camino y el p_inodo en la variable global
        strcpy(UltimaEntradaEscritura.camino, camino);
        UltimaEntradaEscritura.p_inodo = p_inodo;
#if DEBUG
        printf("[mi_write->Actualizamos la caché de escritura\n");
#endif
    }
    // Escribimos el inodo y devolvemos el número de bytes escritos
    bytes_escritos = mi_write_f(p_inodo, buf, offset, nbytes);
    return bytes_escritos;
}
// Función para leer los nbytes que tiene un fichero
int mi_read(const char *camino, void *buf, unsigned int offset, unsigned int nbytes)
{
    // leemos superbloque
    struct superbloque SB;
    if (bread(posSB, &SB) == -1)
    {
        return -1;
    }
    unsigned int p_inodo_dir = SB.posInodoRaiz;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int bytes_leidos = 0;
    int error;
    if (strcmp(UltimaEntradaEscritura.camino, camino) == 0)
    {
        p_inodo = UltimaEntradaEscritura.p_inodo;
    }
    else
    {
        // Buscamos la entrada para obtener el p_inodo
        if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4)) < 0)
        {
            mostrar_error_buscar_entrada(error);
            return -1;
        }
        strcpy(UltimaEntradaEscritura.camino, camino);
        UltimaEntradaEscritura.p_inodo = p_inodo;
    }
    struct inodo inodo;
    leer_inodo(p_inodo, &inodo);
#if DEBUG
    printf("Tamaño en bytes logicos del inodo: %d\n", inodo.tamEnBytesLog);
#endif
    // Llamamos mi_read_f para leer los nbytes a partir del offser pasados por parametro
    bytes_leidos = mi_read_f(p_inodo, buf, offset, nbytes);
    // Devolvemos los bytes leidos
    return bytes_leidos;
}
// Fúnción para crear un enlace de una entrada de directorio
int mi_link(const char *camino1, const char *camino2)
{
    // leemos el superbloque
    struct superbloque SB;
    if (bread(posSB, &SB) == -1)
    {
        return -1;
    }
    unsigned int p_inodo_dir1 = SB.posInodoRaiz;
    unsigned int p_inodo1;
    unsigned int p_entrada1 = 0;
    unsigned int p_inodo_dir2 = SB.posInodoRaiz;
    unsigned int p_inodo2;
    unsigned int p_entrada2 = 0;
    // Comprobamos camino1 exista y tiene permiso lectura
    int error;
    error = buscar_entrada(camino1, &p_inodo_dir1, &p_inodo1, &p_entrada1, 0, 0);
    if (error < 0)
    {
        mostrar_error_buscar_entrada(error);
        return -1;
    }
    // LEER P_INODO1 SI F Y PERMISOS 4
    struct inodo inodo1;
    leer_inodo(p_inodo1, &inodo1);
    if (inodo1.tipo != 'f')
    {
        return -1;
    }
    if ((inodo1.permisos & 4) != 4)
    {
        return -1;
    }
    // Creamos entrada camino 2. No debe existir
    if ((error = buscar_entrada(camino2, &p_inodo_dir2, &p_inodo2, &p_entrada2, 1, 6)) < 0)
    {
        mostrar_error_buscar_entrada(error);
        return -1;
    }
    // Leemos entrada asociada camino2(p_entrada2)
    struct entrada entrada;
    if (mi_read_f(p_inodo_dir2, &entrada, ((p_entrada2) * sizeof(struct entrada)), (sizeof(struct entrada))) < 0){
        return -1;
    }    
    // Creamos enlace y Asociamos p_inodo1
    entrada.ninodo = p_inodo1;
    // Escribimos la entrada modificada
    if (mi_write_f(p_inodo_dir2, &entrada, (p_entrada2 * sizeof(struct entrada)), (sizeof(struct entrada))) < 0){
        return -1;
    }    
    mi_waitSem();
    // Liberamos inodo entrada asociada
    liberar_inodo(p_inodo2);
    // Incrementamos enlaces de p_inodo1 y actualizamos mtime
    inodo1.nlinks++;
    inodo1.ctime = time(NULL);
    escribir_inodo(p_inodo1, inodo1);
    mi_signalSem();
    // Devolvemos 0 en caso de NO error
    return 0;
}
// Eliminamos enlace de un inodo
int mi_unlink(const char *camino)
{
    if (camino[0] == '/' && strlen(camino) == 1)
    {
        printf("Error: No se puede eliminar el directorio raíz\n");
        return -1;
    }
    struct superbloque SB;
    if (bread(posSB, &SB) == -1)
    {
        return -1;
    }
    unsigned int p_inodo_dir = SB.posInodoRaiz;
    unsigned int p_inodo = SB.posInodoRaiz;
    unsigned int p_entrada = 0;
    // Comprobar entrada camino exista y obtener p_entrada Permisos 0 porque no reserva
    int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0);
    if (error < 0)
    {
        mostrar_error_buscar_entrada(error);
        return -1;
    }

    // Leer p_entrada y obtener su tamEnBytesLog
    struct inodo inodo;
    leer_inodo(p_inodo, &inodo);
    // comprobamos si está vacío
    if ((inodo.tipo == 'd') && (inodo.tamEnBytesLog > 0))
    {
        printf("ERROR : El directorio no está vacío\n");
        return -1;
    }
    // Leer p_inodo_dir y calcular número entradas totales
    struct inodo inodo_dir;
    leer_inodo(p_inodo_dir, &inodo_dir);
    int cant_entradas = inodo_dir.tamEnBytesLog / sizeof(struct entrada);
    // Si entrada es la última
    struct entrada entrada;
    if (p_entrada == cant_entradas - 1)
    {
        mi_truncar_f(p_inodo_dir, inodo_dir.tamEnBytesLog - sizeof(struct entrada));
    }
    else
    {
        if (mi_read_f(p_inodo_dir, &entrada, (sizeof(struct entrada)) * (cant_entradas - 1), sizeof(struct entrada)) < 0)
        {
            return -1;
        }
        if (mi_write_f(p_inodo_dir, &entrada, sizeof(struct entrada) * p_entrada, sizeof(struct entrada)) < 0)
        {
            return -1;
        }

        mi_truncar_f(p_inodo_dir, inodo_dir.tamEnBytesLog - sizeof(struct entrada));
    }
    mi_waitSem();
    // Restamos un enlace y si es 0 liberamos inodo
    inodo.nlinks--;
    if (inodo.nlinks == 0)
    {
        liberar_inodo(p_inodo);
    }
    else
    {
        inodo.ctime = time(NULL);
        escribir_inodo(p_inodo, inodo);
    }
    mi_signalSem();
    return 0;
}