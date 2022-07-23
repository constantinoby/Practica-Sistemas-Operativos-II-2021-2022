// AUTORES: Aina Maria Tur Serrano y Constantino Byelov.
#include "ficheros_basico.h"
#define DEBUG2 0
// Calcula el tamaño en bloques necesario para el mapa de bits.
int tamMB(unsigned int nbloques)
{
    int tamMB;
    // Se divide numero total bloque entre 8(agrupar bits en bytes)
    // dividir tamaño bloque
    if (((nbloques / 8) % BLOCKSIZE) == 0)
    {
        tamMB = (nbloques / 8) / BLOCKSIZE;
        // Si el modulo no da 0, se añade un bloque
    }
    else
    {
        tamMB = ((nbloques / 8) / BLOCKSIZE) + 1;
    }
    return tamMB;
}

// Calcula el tamaño en bloques del array de inodos.
int tamAI(unsigned int ninodos)
{
    int tamAI;
    // calcular los bytes que ocupan los inodos y dividirlo por el tamaño de un bloque
    if (((ninodos * INODOSIZE) % BLOCKSIZE) == 0)
    {
        tamAI = (ninodos * INODOSIZE) / BLOCKSIZE;
        // Si el módulo da diferente de 0, se añade un bloque
    }
    else
    {
        tamAI = ((ninodos * INODOSIZE) / BLOCKSIZE) + 1;
    }
    return tamAI;
}

// Inicializa los datos del Superbloque
int initSB(unsigned int nbloques, unsigned int ninodos)
{
    struct superbloque SB;
    SB.posPrimerBloqueMB = posSB + tamSB; // posSB = 0, tamSB = 1
    SB.posUltimoBloqueMB = SB.posPrimerBloqueMB + tamMB(nbloques) - 1;
    SB.posPrimerBloqueAI = SB.posUltimoBloqueMB + 1;
    SB.posUltimoBloqueAI = SB.posPrimerBloqueAI + tamAI(ninodos) - 1;
    SB.posPrimerBloqueDatos = SB.posUltimoBloqueAI + 1;
    SB.posUltimoBloqueDatos = nbloques - 1;
    SB.posInodoRaiz = 0;
    SB.posPrimerInodoLibre = 0;
    SB.cantBloquesLibres = nbloques;
    SB.cantInodosLibres = ninodos;
    SB.totBloques = nbloques;
    SB.totInodos = ninodos;
    // Hay que escribir los datos en la posicion del SB
    return bwrite(posSB, &SB);
}

// Inicializa el mapa de bits poniendo a 1 los bits que representan los metadatos.
int initMB()
{
    //unsigned char bufferMB[BLOCKSIZE];
    // Se lee el superbloque
    struct superbloque SB;
    // Si error leer SB, devuleve -1
    if (bread(posSB, &SB) == -1)
    {
#if DEBUG
        fprintf(stderr, "ERROR %d: %s\n", errno, strerror(errno));
#endif
        return -1;
    }
  
        // Poner a 1 los bits del SB, MB y el AI
        for (int i = posSB; i < SB.posPrimerBloqueDatos; i++)
        {
            escribir_bit(i, 1);
            SB.cantBloquesLibres--;
        }

        // Si escribir error, devuleve -1
        if (bwrite(posSB, &SB) == -1)
        {
    #if DEBUG
            fprintf(stderr, "ERROR %d: %s\n", errno, strerror(errno));
    #endif
            return -1;
        }
    
    // Si no se producen errores se devuelve 0
    return 0;
}

// Inicializa el array de inodos es lo pone a 1
int initAI()
{
    struct inodo inodos[BLOCKSIZE / INODOSIZE];
    // leer SB y localizar AI
    struct superbloque SB;
    // Si error leer SB, devuleve -1
    if (bread(posSB, &SB) == -1)
    {
#if DEBUG
        fprintf(stderr, "ERROR %d: %s\n", errno, strerror(errno));
#endif
        return -1;
    }
    int contInodos = SB.posPrimerInodoLibre + 1; // si hemos inicializado SB.posPrimerInodoLibre = 0
    for (int i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++)
    { // para cada bloque del AI
        // leer el bloque de inodos i del dispositivo virtual
        if (bread(i, &inodos) == -1)
        {
#if DEBUG
            fprintf(stderr, "ERROR %d: %s\n", errno, strerror(errno));
#endif
            return -1;
        }

        for (int j = 0; j < BLOCKSIZE / INODOSIZE; j++)
        {                         // para cada inodo del AI
            inodos[j].tipo = 'l'; // libre
            if (contInodos < SB.totInodos)
            {                                               // si no hemos llegado al último inodo
                inodos[j].punterosDirectos[0] = contInodos; // enlazamos con el siguiente
                contInodos++;
            }
            else
            { // hemos llegado al último inodo
                inodos[j].punterosDirectos[0] = UINT_MAX;
                // hay que salir del bucle, el último bloque no tiene por qué estar completo !!!
            }
        }
        // escribir el bloque de inodos i  en el dispositivo virtual
        if (bwrite(i, &inodos) == -1)
        {
#if DEBUG
            fprintf(stderr, "ERROR %d: %s\n", errno, strerror(errno));
#endif
            return -1;
        }
    }
    return 0;
}

// Escribe el valor de bit en un bit del MB
int escribir_bit(unsigned int nbloque, unsigned int bit)
{

    unsigned char bufferMB[BLOCKSIZE];
    unsigned char mascara = 128; // 10000000
    struct superbloque SB;
    // Calculo posicion byte en MB
    int posbyte = nbloque / 8;
    // Calculo posicion bit en posbyte
    int posbit = nbloque % 8;
    // Calculo bloque MB donde esta el bit
    int nbloqueMB = posbyte / BLOCKSIZE;

    // leer SB y localizar MB
    //  Si error leer SB, devuleve -1
    if (bread(posSB, &SB) == -1)
    {
#if DEBUG
        fprintf(stderr, "ERROR %d: %s\n", errno, strerror(errno));
#endif
        return -1;
    }
    // Calculo posicion absoluta bloque
    int nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB;
    // leer contenido bloque nbloquesabs, si error -1
    if (bread(nbloqueabs, bufferMB) == -1)
    {
#if DEBUG
        fprintf(stderr, "ERROR %d: %s\n", errno, strerror(errno));
#endif
        return -1;
    }
    // localizar posicion en array y aplicar mascara
    posbyte = posbyte % BLOCKSIZE;
    mascara >>= posbit; // desplazamiento de bits a la derecha
    // Si queremos escribir un 1 en el mapa de bits
    if (bit == 1)
    {
        bufferMB[posbyte] |= mascara; //  operador OR para bits
    }
    // Si queremos escribir un 0 en nuestro mapa de bits
    else if (bit == 0)
    {
        bufferMB[posbyte] &= ~mascara; // operadores AND y NOT para bits
    }
    // Escribimos el contenido del buffer en el bloque

    if (bwrite(nbloqueabs, bufferMB) == -1)
    {
#if DEBUG
        fprintf(stderr, "ERROR %d: %s\n", errno, strerror(errno));
#endif
        return -1;
    }

    return 0;
}
// Lee un determinado bit del MB y devuelve el valor del bit leído.
char leer_bit(unsigned int nbloque)
{
    unsigned char bufferMB[BLOCKSIZE];
    struct superbloque SB;
    // Calculo posicion byte en MB
    int posbyte = nbloque / 8;
    // Calculo posicion bit en posbyte
    int posbit = nbloque % 8;
    unsigned char mascara = 128; // 10000000
    // leer SB y localizar MB
    // Si error leer SB, devuleve -1
    if (bread(posSB, &SB) == -1)
    {
#if DEBUG
        fprintf(stderr, "ERROR %d: %s\n", errno, strerror(errno));
#endif
        return -1;
    }
    int nbloqueMB = posbyte / BLOCKSIZE;
    int nbloqueabs = nbloqueMB + SB.posPrimerBloqueMB;
    // leer contenido bloque nbloquesabs, si error -1
    if (bread(nbloqueabs, bufferMB) == -1)
    {
#if DEBUG
        fprintf(stderr, "ERROR %d: %s\n", errno, strerror(errno));
#endif
        return -1;
    }
#if DEBUG
    printf("[leer_bit(%d)-> posbyte: %d, posbit: %d, nbloqueMB: %d, nbloqueabs: %d]\n", nbloque, posbyte, posbit, nbloqueMB, nbloqueabs);
#endif
    posbyte = posbyte % BLOCKSIZE;
    mascara >>= posbit;           // desplazamiento de bits a la derecha
    mascara &= bufferMB[posbyte]; // operador AND para bits
    mascara >>= (7 - posbit);     // desplazamiento de bits a la derecha
#if DEBUG
    printf("leer_bit: %d\n", mascara);
#endif
    return mascara;
}
// Busca primer bloque libre en MB y lo ocupa
int reservar_bloque()
{
    struct superbloque SB;
    // Declaracion buffer tamaño de un bloque
    unsigned char bufferAux[BLOCKSIZE];
    unsigned char bufferMB[BLOCKSIZE];
    unsigned char buf[BLOCKSIZE];
    unsigned char mascara = 128; // 10000000
    int posbit = 0;
    int posbyte = 0;

    memset(bufferAux, 255, BLOCKSIZE); // llenamos el buffer auxiliar con 1s
    // leer SB y localizar MB
    // Si error leer SB, devuleve -1
    if (bread(posSB, &SB) == -1)
    {
#if DEBUG
        fprintf(stderr, "ERROR %d: %s\n", errno, strerror(errno));
#endif
        return -1;
    }
    int nbloqueabs = SB.posPrimerBloqueMB;
    // Si no quedan bloques libres, se devuelve -1
    if (SB.cantBloquesLibres == 0)
    {
        return -1;
    }
    // Se lee la posicion nbloquesabs y se guarda en el bufferMB
    if (bread(nbloqueabs, bufferMB) == -1)
    {
#if DEBUG
        fprintf(stderr, "ERROR %d: %s\n", errno, strerror(errno));
#endif
        return -1;
    }
    // Buscamos primer bloque libre( aquel que tenga un 0) antes de llegar al final del MB
    while ((memcmp(bufferAux, bufferMB, BLOCKSIZE) == 0) && (nbloqueabs <= SB.posUltimoBloqueMB))
    {
        nbloqueabs++;
        if (bread(nbloqueabs, bufferMB) == -1)
        {
#if DEBUG
            fprintf(stderr, "ERROR %d: %s\n", errno, strerror(errno));
#endif
            return -1;
        }
    }
    // localizar byte dentro de ese bloque tiene un 0
    while (bufferMB[posbyte] == 255)
    {
        posbyte++;
    }

    // encontrar el primer bit a 0 en ese byte
    while (bufferMB[posbyte] & mascara)
    {                            // operador AND para bits
        bufferMB[posbyte] <<= 1; // desplazamiento de bits a la izquierda
        posbit++;
    }
    // Calcular bloque a reservar y ocuparlo
    int nbloque = ((nbloqueabs - SB.posPrimerBloqueMB) * BLOCKSIZE + posbyte) * 8 + posbit;
    escribir_bit(nbloque, 1);

    // Limpiar bloque en la zona de datos, escribiendo el contenido de un buffer a 0
    memset(buf, 0, BLOCKSIZE);
    if (bwrite(nbloque, buf) == -1)
    {
#if DEBUG
        fprintf(stderr, "ERROR %d: %s\n", errno, strerror(errno));
#endif
        return -1;
    }
    // Restar el numero de bloques libres y guardamos el SB
    SB.cantBloquesLibres--;

    if (bwrite(posSB, &SB) == -1)
    {
#if DEBUG
        fprintf(stderr, "ERROR %d: %s\n", errno, strerror(errno));
#endif
        return -1;
    }
    // Devolvemos el numero de bloque
    return nbloque;
}
// Liberar un bloque determinado
int liberar_bloque(unsigned int nbloque)
{
    // leer SB y localizar MB
    struct superbloque SB;
    // Si error leer SB, devuleve -1
    if (bread(posSB, &SB) == -1)
    {
#if DEBUG
        fprintf(stderr, "ERROR %d: %s\n", errno, strerror(errno));
#endif
        return -1;
    }
    // Poner a 0 el bit del bloque del MB
    escribir_bit(nbloque, 0);
    // Aumentar bloques libres y guardar SB
    SB.cantBloquesLibres++;
    if (bwrite(posSB, &SB) == -1)
    {
#if DEBUG
        fprintf(stderr, "ERROR %d: %s\n", errno, strerror(errno));
#endif
        return -1;
    }
    // devolver numero de bloque
    return nbloque;
}
// Escribe el contenido de un inodo en un inodo del array de inodos
int escribir_inodo(unsigned int ninodo, struct inodo inodo)
{
    // leer SB y localizar AI
    struct superbloque SB;
    struct inodo inodos[BLOCKSIZE / INODOSIZE];
    // Si error leer SB, devuleve -1
    if (bread(posSB, &SB) == -1)
    {
#if DEBUG
        fprintf(stderr, "ERROR %d: %s\n", errno, strerror(errno));
#endif
        return -1;
    }
    // numero de bloque inodo en array de inodos
    int posBloqueAI = ninodo / (BLOCKSIZE / INODOSIZE);
    int nbloque = SB.posPrimerBloqueAI + posBloqueAI;
    // leemos el bloque
    if (bread(nbloque, inodos) == -1)
    {
#if DEBUG
        fprintf(stderr, "ERROR %d: %s\n", errno, strerror(errno));
#endif
        return -1;
    }
    // buscamos posicion en array de inodos y escribimos bloque
    int id = ninodo % (BLOCKSIZE / INODOSIZE);
    inodos[id] = inodo;

    if (bwrite(nbloque, inodos) == -1)
    {
#if DEBUG
        fprintf(stderr, "ERROR %d: %s\n", errno, strerror(errno));
#endif
        return -1;
    }
    // Si no hay error devolvemos 0
    return 0;
}
// Lee inodo del array de inodos y lo guarda en una variable tipo inodo
int leer_inodo(unsigned int ninodo, struct inodo *inodo)
{
    struct inodo inodos[BLOCKSIZE / INODOSIZE];
    // leer SB y localizar AI
    struct superbloque SB;
    // Si error leer SB, devuleve -1
    if (bread(posSB, &SB) == -1)
    {
#if DEBUG
        fprintf(stderr, "ERROR %d: %s\n", errno, strerror(errno));
#endif
        return -1;
    }
    // Calculo bloque contiene inodo
    int posBloqueAI = ninodo / (BLOCKSIZE / INODOSIZE);
    int nbloque = SB.posPrimerBloqueAI + posBloqueAI;
    // Volcar contenido inodo en un array de inodos tamaño del buffer
    if (bread(nbloque, inodos) == -1)
    {
#if DEBUG
        fprintf(stderr, "ERROR %d: %s\n", errno, strerror(errno));
#endif
        return -1;
    }
    // inodo esta en la posicion id
    int id = ninodo % (BLOCKSIZE / INODOSIZE);
    *inodo = inodos[id];
    return 0;
}
// Encuentra el primer inodo libre, devuelve el numero y actualiza lista
int reservar_inodo(unsigned char tipo, unsigned char permisos)
{
    // leer SB y localizar AI
    struct superbloque SB;
    // Si error leer SB, devuleve -1
    if (bread(posSB, &SB) == -1)
    {
#if DEBUG
        fprintf(stderr, "ERROR %d: %s\n", errno, strerror(errno));
#endif
        return -1;
    }
    // Si no hay inodos libres devuelve -1
    if (SB.cantInodosLibres == 0)
    {
        return -1;
    }
    // Reservamos la posicion del primer inodo libre
    int posInodoReservado = SB.posPrimerInodoLibre;
    // leemos el primer inodo libre, sabemos posicion siguiente libre
    struct inodo primero;
    leer_inodo(posInodoReservado, &primero);
    // Actualizamos lista
    SB.posPrimerInodoLibre = primero.punterosDirectos[0];
    // Inicializamos campos del inodo
    struct inodo inodo;
    inodo.tipo = tipo; // Tipo ('l':libre, 'd':directorio o 'f':fichero)
    inodo.permisos = permisos;
    inodo.nlinks = 1;
    inodo.tamEnBytesLog = 0;
    inodo.atime = time(NULL);
    inodo.mtime = time(NULL);
    inodo.ctime = time(NULL);
    inodo.numBloquesOcupados = 0;
    for (int i = 0; i < 12; i++)
    {
        inodo.punterosDirectos[i] = 0;
    }
    for (int i = 0; i < 3; i++)
    {
        inodo.punterosIndirectos[i] = 0;
    }
    // escribimos el inodo en la posición reservada
    escribir_inodo(posInodoReservado, inodo);
    // Decrementamos inodos libres y guardamos el SB
    SB.cantInodosLibres--;
    if (bwrite(posSB, &SB) == -1)
    {
#if DEBUG
        fprintf(stderr, "ERROR %d: %s\n", errno, strerror(errno));
#endif
        return -1;
    }
    // Devolvemos posicion reservada
    return posInodoReservado;
}
// Función que obtiene el rango de punteros en el que se sitúa el bloque lógico
int obtener_nRangoBL(struct inodo *inodo, unsigned int nblogico, unsigned int *ptr)
{
    // Si bloque lógico <  12
    if (nblogico < DIRECTOS)
    {
        *ptr = inodo->punterosDirectos[nblogico];
        return 0; // devolvemos nivel_punteros
    }             // si bloque lógico <268
    else if (nblogico < INDIRECTOS0)
    {
        *ptr = inodo->punterosIndirectos[0];
        return 1;
    } // si bloque lógico < 65.804
    else if (nblogico < INDIRECTOS1)
    {
        *ptr = inodo->punterosIndirectos[1];
        return 2;
    } // si bloque lógico < 16.843.020
    else if (nblogico < INDIRECTOS2)
    {
        *ptr = inodo->punterosIndirectos[2];
        return 3;
    }
    else // bloque lógico no se encuentra en ninguno de esos niveles, está fuera de rango. Muestra error
    {
        *ptr = 0;
#if DEBUG
        fprintf(stderr, "ERROR %d: %s Bloque lógico fuera de rango \n", errno, strerror(errno));
#endif
        return -1;
    }
}
// Función obtiene los índices de los bloques de punteros
int obtener_indice(unsigned int nblogico, int nivel_punteros)
{
    // Si bloque lógico < 12
    if (nblogico < DIRECTOS)
    {
        // devuelve propio bloque
        return nblogico;
    } // si bloque lógico <268
    else if (nblogico < INDIRECTOS0)
    { // bloque -12
        return nblogico - DIRECTOS;
    } // si bloque lógico < 65.804
    else if (nblogico < INDIRECTOS1)
    { // obtener índice bloque nivel punteros 2
        if (nivel_punteros == 2)
        {
            return (nblogico - INDIRECTOS0) / NPUNTEROS;
        } // resto divión indica bloque en nivel 1 apuntado por nivel 2
        else if (nivel_punteros == 1)
        {
            return (nblogico - INDIRECTOS0) % NPUNTEROS; // 256
        }
    } // si bloque lógico < 16.843.020
    else if (nblogico < INDIRECTOS2)
    { // obtener índice bloque nivel punteros 3
        if (nivel_punteros == 3)
        {
            return (nblogico - INDIRECTOS1) / (NPUNTEROS * NPUNTEROS); // 256^2
        }                                                              // resto división anterior indica indice bloque nivel 2 apuntado por nivel 3
        else if (nivel_punteros == 2)
        {
            return ((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) / NPUNTEROS;
        } // resto división anterior indica índice bloque nivel 1 apuntado por nivel 2
        else if (nivel_punteros == 1)
        {
            return ((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) % NPUNTEROS;
        }
    }
    // Si fuera de rango, error
    return -1;
}
// Función obtiene el nº de bloque físico correspondiente a un bloque lógico determinado del inodo indicado
int traducir_bloque_inodo(unsigned int ninodo, unsigned int nblogico, unsigned char reservar)
{
    struct inodo inodo;
    unsigned int ptr, ptr_ant;
    int salvar_inodo, nRangoBL, nivel_punteros, indice;
    unsigned int buffer[NPUNTEROS];
    leer_inodo(ninodo, &inodo);
    ptr = 0;
    ptr_ant = 0;
    salvar_inodo = 0;
    nRangoBL = obtener_nRangoBL(&inodo, nblogico, &ptr); // 0:D, 1:I0, 2:I1, 3:I2
    nivel_punteros = nRangoBL;                           // el nivel_punteros +alto es el que cuelga del inodo
    while (nivel_punteros > 0)
    { // iterar para cada nivel de punteros indirectos
        if (ptr == 0)
        { // no cuelgan bloques de punteros
            if (reservar == 0)
            {
                return -1; // bloque inexistente -> no imprimir nada por pantalla!!!
            }
            else
            { // reservar bloques de punteros y crear enlaces desde el  inodo hasta el bloque de datos
                salvar_inodo = 1;
                ptr = reservar_bloque(); // de punteros
                inodo.numBloquesOcupados++;
                inodo.ctime = time(NULL); // fecha actual
                if (nivel_punteros == nRangoBL)
                { // el bloque cuelga directamente del inodo
                    inodo.punterosIndirectos[nRangoBL - 1] = ptr;
#if DEBUG2
                    printf("traducir_bloque_inodo()→ inodo.punterosIndirectos[%d] =%d (reservado BF %d para punteros_nivel%d)]\n", nRangoBL - 1, inodo.punterosIndirectos[nRangoBL - 1],
                           inodo.punterosIndirectos[nRangoBL - 1], nivel_punteros);
#endif
                }
                else
                {
                    // el bloque cuelga de otro bloque de punteros
                    buffer[indice] = ptr;
                    // salvamos en el dispositivo el buffer de punteros modificado
                    if (bwrite(ptr_ant, buffer) == -1)
                    {
#if DEBUG
                        fprintf(stderr, "ERROR %d: %s\n", errno, strerror(errno));
#endif
                        return -1;
                    }
#if DEBUG2
                    printf("traducir_bloque_inodo()→ punteros_nivel%d[%d] =%d (reservado BF %d para punteros_nivel%d)]\n", nivel_punteros + 1, indice, buffer[indice], buffer[indice], nivel_punteros);
#endif
                }
                memset(buffer, 0, BLOCKSIZE); // ponemos a 0 todos los punteros del buffer
            }
        }
        else if (bread(ptr, buffer) == -1) // leemos del dispositivo el bloque de punteros ya existente
        {
#if DEBUG
            fprintf(stderr, "ERROR %d: %s\n", errno, strerror(errno));
#endif
            return -1;
        }

        indice = obtener_indice(nblogico, nivel_punteros);
        ptr_ant = ptr;        // guardamos el puntero actual
        ptr = buffer[indice]; // y lo desplazamos al siguiente nivel
        nivel_punteros--;
    } // al salir de este bucle ya estamos al nivel de datos

    if (ptr == 0)
    { // no existe bloque de datos
        if (reservar == 0)
        {
            return -1; // error lectura ∄ bloque
        }
        else
        {
            salvar_inodo = 1;
            ptr = reservar_bloque(); // de datos
            inodo.numBloquesOcupados++;
            inodo.ctime = time(NULL);
            if (nRangoBL == 0)
            {
                inodo.punterosDirectos[nblogico] = ptr;
#if DEBUG2
                printf("traducir_bloque_inodo()→ inodo.punterosDirectos[%d] =%d (reservado BF %d para BL %d)]\n", nblogico, inodo.punterosDirectos[nblogico],
                       inodo.punterosDirectos[nblogico], nblogico);
#endif
            }
            else
            {
                buffer[indice] = ptr; // asignamos la dirección del bloque de datos
#if DEBUG2
                printf("traducir_bloque_inodo()→ punteros_nivel%d[%d] =%d (reservado BF %d para BL %d)]\n", nivel_punteros + 1, indice, buffer[indice], buffer[indice], nblogico);
#endif
                // salvamos en el dispositivo el buffer de punteros modificado
                if (bwrite(ptr_ant, buffer) == -1)
                {
#if DEBUG2
                    fprintf(stderr, "ERROR %d: %s\n", errno, strerror(errno));
#endif
                    return -1;
                }
            }
        }
    }
    if (salvar_inodo == 1)
    {
        escribir_inodo(ninodo, inodo); // sólo si lo hemos actualizado
    }
    return ptr; // nº de bloque físico correspondiente al bloque de datos lógico, nblogico
}

int liberar_inodo(unsigned int ninodo)
{
    // Leer el inodo
    struct inodo inodo;
    leer_inodo(ninodo, &inodo);
    // leer SB
    struct superbloque SB;
    // Si error leer SB, devuleve -1
    if (bread(posSB, &SB) == -1)
    {
#if DEBUG
        fprintf(stderr, "ERROR %d: %s\n", errno, strerror(errno));
#endif
        return -1;
    }
    // Liberar bloques del inodo y restarlo a los ocupados
    //CASO ERROR
    int bloques_liberados = liberar_bloques_inodo(0, &inodo);
    if(bloques_liberados<0){
        return -1;
    }
    //CASO ERROR >0 Se tienen que haber liberado todos los bloques
    inodo.numBloquesOcupados = inodo.numBloquesOcupados - bloques_liberados;
    if(inodo.numBloquesOcupados>0){
        return -1;
    }
    // inodo pasa a libre
    inodo.tipo = 'l';
    inodo.tamEnBytesLog = 0;
    //inodo punteros directos 0 valga ninodo
    inodo.punterosDirectos[0]=SB.posPrimerInodoLibre;
    // Añadir bloques liberados
    SB.posPrimerInodoLibre = ninodo;
    SB.cantBloquesLibres += bloques_liberados;
    SB.cantInodosLibres++;
    escribir_inodo(ninodo, inodo);
    if (bwrite(posSB, &SB) == -1)
    {
#if DEBUG
        fprintf(stderr, "ERROR %d: %s\n", errno, strerror(errno));
#endif
        return -1;
    }
    return ninodo;
}
int liberar_bloques_inodo(unsigned int primerBL, struct inodo *inodo)
{
    // libera los bloques de datos e índices iterando desde el primer bloque lógico a liberar hasta el último
    // por tanto explora las ramificaciones de punteros desde las hojas hacia las raíces en el inodo
    unsigned int nivel_punteros, indice, ptr = 0, nBL, ultimoBL;
    int nRangoBL;
    unsigned int bloques_punteros[3][NPUNTEROS]; // array de bloques de punteros
    unsigned char bufAux_punteros[BLOCKSIZE];    // para llenar de 0s y comparar
    int ptr_nivel[3];                            // punteros a bloques de punteros de cada nivel
    int indices[3], liberados = 0;
    ; // indices de cada nivel   // nº de bloques liberados
    int contador_bread = 0, contador_bwrite = 0;
    if (inodo->tamEnBytesLog == 0) // el fichero está vacío
        return 0;
    // obtenemos el último bloque lógico del inodo
    if (inodo->tamEnBytesLog % BLOCKSIZE == 0)
    {
        ultimoBL = (inodo->tamEnBytesLog / BLOCKSIZE )- 1;
    }
    else
    {
        ultimoBL = inodo->tamEnBytesLog / BLOCKSIZE;
    }
#if DEBUG
    printf("[liberar_bloques_inodo()→primer BL: %d, último BL:%d\n]", primerBL, ultimoBL);
#endif

    memset(bufAux_punteros, 0, BLOCKSIZE);

    for (nBL = primerBL; nBL <= ultimoBL; nBL++)
    { // recorrido BLs
        // nivel de punteros donde esta bloque lógico
        nRangoBL = obtener_nRangoBL(inodo, nBL, &ptr); // 0:D, 1:I0, 2:I1, 3:I2
        if (nRangoBL < 0)
            // error fprintf
            return -1;
        nivel_punteros = nRangoBL; // el nivel_punteros +alto cuelga del inodo

        while (ptr > 0 && nivel_punteros > 0)
        { // cuelgan bloques de punteros
            indice = obtener_indice(nBL, nivel_punteros);
            if (indice == 0 || nBL == primerBL)
            {
                // solo hay que leer del dispositivo si no está ya cargado previamente en un buffer
                if (bread(ptr, bloques_punteros[nivel_punteros - 1]) == -1)
                {
                    return -1;
                }
                contador_bread++;
            }
            ptr_nivel[nivel_punteros - 1] = ptr;
            indices[nivel_punteros - 1] = indice;
            ptr = bloques_punteros[nivel_punteros - 1][indice];
            nivel_punteros--;
        }
        if (ptr > 0)
        { // si existe bloque de datos
            liberar_bloque(ptr);
#if DEBUG
            printf("[liberar_bloques_inodo()→liberado BF %d de datos correspondiente al BL %d ]\n", ptr, nBL);
#endif
            liberados++;
            if (nRangoBL == 0)
            { // es un puntero Directo
                inodo->punterosDirectos[nBL] = 0;
            }
            else
            {
                nivel_punteros = 1;
                while (nivel_punteros <= nRangoBL)
                {
                    indice = indices[nivel_punteros - 1];
                    bloques_punteros[nivel_punteros - 1][indice] = 0;
                    ptr = ptr_nivel[nivel_punteros - 1];
                    if (memcmp(bloques_punteros[nivel_punteros - 1], bufAux_punteros, BLOCKSIZE) == 0)
                    {
                        // No cuelgan más bloques ocupados, hay que liberar el bloque de punteros
                        liberar_bloque(ptr);
                        liberados++;
#if DEBUG
                        printf("[liberar_bloques_inodo()→liberado BF %d de punteros_nivel%d correspondiente al BL %d]\n", ptr, nivel_punteros, nBL);
#endif
                        // Incluir mejora saltando los bloques que no sea necesario explorar
                        // al eliminar bloque de punteros
                        //No funciona correctamente//
                        /*if (nivel_punteros == 2)
                        {
                            nBL += NPUNTEROS - 1;
                            nivel_punteros++;
                        }
                        // Si el nivel de punteros es 3 saltará los correspondientes bloques lógicos de los niveles 2 y 1
                        else if (nivel_punteros == 3)
                        {
                            nBL += (NPUNTEROS * NPUNTEROS) - 1;
                            nivel_punteros++;
                           
                        }*/
                        if (nivel_punteros == nRangoBL)
                            inodo->punterosIndirectos[nRangoBL - 1] = 0;
                        nivel_punteros++;
                    }
                    else
                    { // escribimos en el dispositivo el bloque de punteros modificado
                        if (bwrite(ptr, bloques_punteros[nivel_punteros - 1]) == -1)
                        {
#if DEBUG
                            fprintf(stderr, "ERROR %d: %s\n", errno, strerror(errno));
#endif
                            return -1;
                        }
                        contador_bwrite++;
                        // hemos de salir del bucle ya que no será necesario liberar los bloques de niveles
                        // superiores de los que cuelga
                        nivel_punteros = nRangoBL + 1;
                    }
                }
            }
        }
        else
        {
            // Incluir mejora saltando los bloques que no sea necesario explorar  al valer 0 un puntero
            // Si detecta que un puntero es 0 puede saltarse los bloques lógicos correspondiente al siguiente de nivel de punteros
            // Si el puntero pertenece a nivel 2 saltará los bloques lógicos correspondientes al nivel 1
            if (nivel_punteros == 2)
            {
                nBL += NPUNTEROS - 1;
            }
            // Si el nivel de punteros es 3 saltará los correspondientes bloques lógicos de los niveles 2 y 1
            else if (nivel_punteros == 3)
            {
                nBL += (NPUNTEROS * NPUNTEROS) - 1;
            }
        }
    }
#if DEBUG
    printf("[liberar_bloques_inodo()→ total bloques liberados:%d, total breads:%d, total_bwrites:%d]\n", liberados, contador_bread, contador_bwrite);
#endif
    return liberados;
}
