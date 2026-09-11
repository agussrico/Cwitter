#include "lista.h"

void listaInicializar(tLista *lista){
    lista->inicio = NULL;
    lista->fin = NULL;
    lista->cant = 0;
}

int listaVacia(tLista *lista){
    if(!lista->inicio && !lista->fin){
        return LISTA_VACIA;
    }
    return TODO_OK;
}

int listaLlena(tLista *lista, size_t dataSize){
    void *nodo = malloc(sizeof(tNodo));
    void *data;
    if(!nodo){
        return LISTA_LLENA;
    }
    data = malloc(dataSize);
    if(!data){
        free(nodo);
        return LISTA_LLENA;
    }
    free(nodo);
    free(data);
    return TODO_OK;
}

int listaInsertar(tLista *lista, void *data, size_t dataSize){
    tNodo *nuevoNodo;
    void *nuevaData;
    if(listaLlena(lista, dataSize)){
        return LISTA_LLENA;
    }
    ///Declaro el nodo
    nuevoNodo = (tNodo *)malloc(sizeof(tNodo));
    nuevaData = malloc(dataSize);
    memcpy(nuevaData, data, dataSize);
    nuevoNodo->ant = NULL;
    nuevoNodo->sig = NULL;
    nuevoNodo->dataSize = dataSize;
    nuevoNodo->data = nuevaData;

    if(listaVacia(lista)){
        ///Nuevo nodo es el primer nodo por lo tanto es inicio y fin al mismo tiempo
        lista->inicio = nuevoNodo;
        lista->fin = nuevoNodo;
    }
    else{
        ///Agrego al final de la lista
        lista->fin->sig = nuevoNodo;
        nuevoNodo->ant = lista->fin;
        lista->fin = nuevoNodo;
    }
    lista->cant++;

    return TODO_OK;
}

int listaBuscarYAccion(tLista *lista, void *busqueda, int (*cmp)(void *, void*), void (*accion)(void *)){
    ///Busca todos los nodos que cumplan con cmp y ejecuta accion en la data de cada uno
    int cont = 0;
    tNodo *nodoActual = lista->inicio;
    while(nodoActual != NULL){
        if(cmp(busqueda, nodoActual->data)){
            accion(nodoActual->data);
            cont++;
        }
        nodoActual = nodoActual->sig;
    }
    return cont; ///devuelve la cantidad de nodos accionados
}

tNodo *listaBuscarYDevolver(tLista *lista, void *busqueda, int (*cmp)(void *, void*)){
    ///devuelve el puntero al primer nodo que cumple con cmp encontrado
    tNodo *nodoActual = lista->inicio;
    while(nodoActual != NULL){
        if(cmp(busqueda, nodoActual->data)){
            return nodoActual; ///puntero al nodo encontrado
        }
        nodoActual = nodoActual->sig;
    }
    return NULL; ///si no encuentra retorna NULL
}

void listaRecorrerYAccion(tLista *lista, void (*accion)(void *)){
    ///Ejecuta una accion en todos los nodos de la lista
    tNodo *nodoActual = lista->inicio;
    while(nodoActual != NULL){
        accion(nodoActual->data);
        nodoActual = nodoActual->sig;
    }
}

int listaEliminarNodo(tLista *lista, void *busqueda, int (*cmp)(void *, void*)){
    tNodo *nodoAEliminar = listaBuscarYDevolver(lista, busqueda, cmp);
    if(!nodoAEliminar){
        return NODO_NO_ENCONTRADO;
    }
    nodoAEliminar->ant->sig = nodoAEliminar->sig;
    nodoAEliminar->sig->ant = nodoAEliminar->ant;
    free(nodoAEliminar->data);
    free(nodoAEliminar);
    lista->cant--;
    return TODO_OK;
}

void listaVaciar(tLista *lista){
    tNodo *nodoActual = lista->fin;
    tNodo *nodoAnterior;
    if(listaVacia(lista)){
        return;
    }

    while(nodoActual != NULL){
        nodoAnterior = nodoActual->ant;
        free(nodoActual->data);
        free(nodoActual);
        nodoActual = nodoAnterior;
    }

    lista->inicio = NULL;
    lista->fin = NULL;
    lista->cant = 0;

}
