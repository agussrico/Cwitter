#ifndef INDICE_H_INCLUDED
#define INDICE_H_INCLUDED
#include "arbol.h"
#include "lista.h"

#define MAX_PALABRA 141 // un tweet de una sola palabra puede ocupar los 140 caracteres mas el /0
#define SEPARADORES " \t\n.,;:!?\"'()[]{}-" // caracteres que separan las palabras de un texto
#define INDICE_ERROR -1 // no hay memoria para seguir indexando

typedef struct{
    tArbol arbol;           // arbol de tEntradaIndice ordenado por palabra
    int cantRegistros;      // cantidad de registros (tweets) indexados
}tIndice;

typedef struct{
    char palabra[MAX_PALABRA];  // palabra indexada (clave del arbol)
    tLista nodos;               // lista de tNodo* de los tweets que contienen la palabra
}tEntradaIndice;

void indiceInicializar(tIndice *indice);
int indiceCargar(tIndice *indice, tNodo *nodo, const char *texto);
int indiceQuitar(tIndice *indice, tNodo *nodo, const char *texto);
tLista *indiceBuscar(tIndice *indice, const char *palabra);
void indiceVaciar(tIndice *indice);
unsigned long indiceComparacionesUltimaBusqueda(void);

#endif // INDICE_H_INCLUDED
