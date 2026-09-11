#ifndef LISTA_H_INCLUDED
#define LISTA_H_INCLUDED
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define TODO_OK 0
#define LISTA_VACIA 2
#define LISTA_LLENA 3
#define NODO_NO_ENCONTRADO 4

typedef struct sNodo{
    void *data;
    size_t dataSize;
    struct sNodo *sig;
    struct sNodo *ant;
}tNodo;

typedef struct{
    tNodo *inicio;
    tNodo *fin;
    int cant;
}tLista;


void listaInicializar(tLista *lista);
int listaVacia(tLista *lista);
int listaLlena(tLista *lista, size_t dataSize);
int listaInsertar(tLista *lista, void *data, size_t dataSize);
int listaBuscarYAccion(tLista *lista, void *busqueda, int (*cmp)(void *, void*), void (*accion)(void *));
tNodo *listaBuscarYDevolver(tLista *lista, void *busqueda, int (*cmp)(void *, void*));
int listaEliminarNodo(tLista *lista, void *busqueda, int (*cmp)(void *, void*));
void listaVaciar(tLista *lista);
void listaRecorrerYAccion(tLista *lista, void (*accion)(void *));


#endif // LISTA_H_INCLUDED
