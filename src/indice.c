#include "indice.h"
#include <ctype.h>

///Funciones auxiliares (solo se usan dentro de este archivo)
static int cmpEntrada(void *a, void *b);
static void liberarEntrada(void *entrada);
static void normalizar(char *destino, const char *origen);
static int indexarPalabra(tIndice *indice, const char *palabra, tNodo *nodo);
static void desindexarPalabra(tIndice *indice, const char *palabra, tNodo *nodo);
static int cmpNodo(void *busqueda, void *data);

///contador de comparaciones de palabras, para el modo test (se reinicia en cada indiceBuscar)
static unsigned long comparaciones = 0;

void indiceInicializar(tIndice *indice){
    arbolInicializar(&indice->arbol);
    indice->cantRegistros = 0;
}

int indiceCargar(tIndice *indice, tNodo *nodo, const char *texto){
    ///Separa el texto en palabras y asocia el nodo a cada una
    ///devuelve la cantidad de registros indexados, o INDICE_ERROR si no hay memoria
    char copia[MAX_PALABRA];
    char *palabra;

    strncpy(copia, texto, MAX_PALABRA - 1); ///strtok modifica el texto, trabajo sobre una copia
    copia[MAX_PALABRA - 1] = '\0';

    palabra = strtok(copia, SEPARADORES);
    while(palabra != NULL){
        if(indexarPalabra(indice, palabra, nodo) != TODO_OK){
            return INDICE_ERROR;
        }
        palabra = strtok(NULL, SEPARADORES);
    }

    indice->cantRegistros++;
    return indice->cantRegistros;
}

int indiceQuitar(tIndice *indice, tNodo *nodo, const char *texto){
    ///Saca el nodo de la lista de cada palabra del texto. Si una lista queda vacia, borra la palabra del arbol
    ///texto tiene que ser el mismo con el que se cargo (al modificar, llamar antes de cambiar el mensaje)
    ///devuelve la cantidad de registros indexados que quedan
    char copia[MAX_PALABRA];
    char *palabra;

    strncpy(copia, texto, MAX_PALABRA - 1);
    copia[MAX_PALABRA - 1] = '\0';

    palabra = strtok(copia, SEPARADORES);
    while(palabra != NULL){
        desindexarPalabra(indice, palabra, nodo);
        palabra = strtok(NULL, SEPARADORES);
    }

    if(indice->cantRegistros > 0){
        indice->cantRegistros--;
    }
    return indice->cantRegistros;
}

tLista *indiceBuscar(tIndice *indice, const char *palabra){
    ///devuelve la lista de tNodo* que contienen la palabra, o NULL si no esta indexada
    tEntradaIndice clave;
    tEntradaIndice *entrada;

    normalizar(clave.palabra, palabra);
    comparaciones = 0;
    entrada = (tEntradaIndice *)arbolBuscar(&indice->arbol, &clave, cmpEntrada);
    if(entrada == NULL){
        return NULL;
    }
    return &entrada->nodos;
}

unsigned long indiceComparacionesUltimaBusqueda(void){
    ///cantidad de palabras que comparo la ultima llamada a indiceBuscar
    return comparaciones;
}

void indiceVaciar(tIndice *indice){
    ///libera el arbol y las listas de cada entrada (no toca los nodos indexados)
    arbolVaciar(&indice->arbol, liberarEntrada);
    indice->cantRegistros = 0;
}

static int indexarPalabra(tIndice *indice, const char *palabra, tNodo *nodo){
    tEntradaIndice nuevaEntrada;
    tEntradaIndice *entrada;

    normalizar(nuevaEntrada.palabra, palabra);
    entrada = (tEntradaIndice *)arbolBuscar(&indice->arbol, &nuevaEntrada, cmpEntrada);

    if(entrada != NULL){
        ///la palabra ya existe. Si se repite en el mismo texto, el nodo ya es el ultimo de la lista
        if(*(tNodo **)entrada->nodos.fin->data == nodo){
            return TODO_OK;
        }
        return listaInsertar(&entrada->nodos, &nodo, sizeof(tNodo *));
    }

    ///palabra nueva: armo la entrada con su lista y la inserto (el arbol copia la tLista, no los nodos)
    listaInicializar(&nuevaEntrada.nodos);
    if(listaInsertar(&nuevaEntrada.nodos, &nodo, sizeof(tNodo *)) != TODO_OK){
        return INDICE_ERROR;
    }
    if(arbolInsertar(&indice->arbol, &nuevaEntrada, sizeof(tEntradaIndice), cmpEntrada) != TODO_OK){
        listaVaciar(&nuevaEntrada.nodos);
        return INDICE_ERROR;
    }
    return TODO_OK;
}

static void desindexarPalabra(tIndice *indice, const char *palabra, tNodo *nodo){
    tEntradaIndice clave;
    tEntradaIndice *entrada;

    normalizar(clave.palabra, palabra);
    entrada = (tEntradaIndice *)arbolBuscar(&indice->arbol, &clave, cmpEntrada);
    if(entrada == NULL){
        return; ///la palabra ya se borro (estaba repetida en el texto)
    }

    listaEliminarNodo(&entrada->nodos, &nodo, cmpNodo);
    if(listaVacia(&entrada->nodos)){
        arbolEliminar(&indice->arbol, &clave, cmpEntrada, liberarEntrada);
    }
}

static int cmpNodo(void *busqueda, void *data){
    ///compara direcciones: la lista guarda tNodo* y se busca un tNodo* (formato de cmp de lista: 1 si coincide)
    return *(tNodo **)busqueda == *(tNodo **)data;
}

static int cmpEntrada(void *a, void *b){
    comparaciones++;
    return strcmp(((tEntradaIndice *)a)->palabra, ((tEntradaIndice *)b)->palabra);
}

static void liberarEntrada(void *entrada){
    listaVaciar(&((tEntradaIndice *)entrada)->nodos);
}

static void normalizar(char *destino, const char *origen){
    ///copia la palabra en minusculas para que "Hola" y "hola" sean la misma clave
    int i = 0;
    while(origen[i] != '\0' && i < MAX_PALABRA - 1){
        destino[i] = tolower((unsigned char)origen[i]);
        i++;
    }
    destino[i] = '\0';
}
