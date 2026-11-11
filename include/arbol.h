#ifndef ARBOL_H_INCLUDED
#define ARBOL_H_INCLUDED
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

///Codigos compartidos con lista.h (se definen solo si no estan definidos)
#ifndef TODO_OK
#define TODO_OK 0
#endif
#ifndef NODO_NO_ENCONTRADO
#define NODO_NO_ENCONTRADO 4
#endif
#define ARBOL_LLENO 8
#define CLAVE_DUPLICADA 9

typedef struct sNodoArbol{
    void *data;
    size_t dataSize;
    struct sNodoArbol *izq;
    struct sNodoArbol *der;
}tNodoArbol;

typedef struct{
    tNodoArbol *raiz;
    int cant;
}tArbol;

///cmp devuelve <0 si a va antes que b, 0 si son iguales y >0 si a va despues (como strcmp)
///liberar se usa cuando la data tiene memoria propia adentro (ej: una tLista). Puede ser NULL

void arbolInicializar(tArbol *arbol);
int arbolVacio(tArbol *arbol);
int arbolLleno(size_t dataSize);
int arbolInsertar(tArbol *arbol, void *data, size_t dataSize, int (*cmp)(void *, void *));
void *arbolBuscar(tArbol *arbol, void *clave, int (*cmp)(void *, void *));
int arbolEliminar(tArbol *arbol, void *clave, int (*cmp)(void *, void *), void (*liberar)(void *));
void arbolRecorrerInorden(tArbol *arbol, void (*accion)(void *));
void arbolRecorrerPreorden(tArbol *arbol, void (*accion)(void *));
void arbolRecorrerPosorden(tArbol *arbol, void (*accion)(void *));
int arbolAltura(tArbol *arbol);
void arbolVaciar(tArbol *arbol, void (*liberar)(void *));

#endif // ARBOL_H_INCLUDED
