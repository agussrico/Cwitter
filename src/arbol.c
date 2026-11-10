#include "arbol.h"

///Funciones auxiliares recursivas (solo se usan dentro de este archivo)
static void recorrerInorden(tNodoArbol *nodo, void (*accion)(void *));
static void recorrerPreorden(tNodoArbol *nodo, void (*accion)(void *));
static void recorrerPosorden(tNodoArbol *nodo, void (*accion)(void *));
static int altura(tNodoArbol *nodo);
static void vaciar(tNodoArbol *nodo, void (*liberar)(void *));

void arbolInicializar(tArbol *arbol){
    arbol->raiz = NULL;
    arbol->cant = 0;
}

int arbolVacio(tArbol *arbol){
    return arbol->raiz == NULL;
}

int arbolLleno(size_t dataSize){
    void *nodo = malloc(sizeof(tNodoArbol));
    void *data;
    if(!nodo){
        return ARBOL_LLENO;
    }
    data = malloc(dataSize);
    if(!data){
        free(nodo);
        return ARBOL_LLENO;
    }
    free(nodo);
    free(data);
    return TODO_OK;
}

int arbolInsertar(tArbol *arbol, void *data, size_t dataSize, int (*cmp)(void *, void *)){
    ///Inserta respetando el orden de cmp. No admite claves repetidas
    tNodoArbol **actual = &arbol->raiz;
    tNodoArbol *nuevoNodo;
    int comp;

    ///bajo por el arbol hasta encontrar el lugar vacio donde va el nodo
    while(*actual != NULL){
        comp = cmp(data, (*actual)->data);
        if(comp == 0){
            return CLAVE_DUPLICADA;
        }
        actual = comp < 0 ? &(*actual)->izq : &(*actual)->der;
    }

    if(arbolLleno(dataSize)){
        return ARBOL_LLENO;
    }
    nuevoNodo = (tNodoArbol *)malloc(sizeof(tNodoArbol));
    nuevoNodo->data = malloc(dataSize);
    memcpy(nuevoNodo->data, data, dataSize);
    nuevoNodo->dataSize = dataSize;
    nuevoNodo->izq = NULL;
    nuevoNodo->der = NULL;

    *actual = nuevoNodo;
    arbol->cant++;
    return TODO_OK;
}

void *arbolBuscar(tArbol *arbol, void *clave, int (*cmp)(void *, void *)){
    ///devuelve el puntero a la data del nodo encontrado (se puede modificar), o NULL
    tNodoArbol *actual = arbol->raiz;
    int comp;
    while(actual != NULL){
        comp = cmp(clave, actual->data);
        if(comp == 0){
            return actual->data;
        }
        actual = comp < 0 ? actual->izq : actual->der;
    }
    return NULL;
}

int arbolEliminar(tArbol *arbol, void *clave, int (*cmp)(void *, void *), void (*liberar)(void *)){
    tNodoArbol **actual = &arbol->raiz;
    tNodoArbol **mayor;
    tNodoArbol *nodoAEliminar;
    int comp;

    ///busco el puntero que apunta al nodo a eliminar
    while(*actual != NULL && (comp = cmp(clave, (*actual)->data)) != 0){
        actual = comp < 0 ? &(*actual)->izq : &(*actual)->der;
    }
    if(*actual == NULL){
        return NODO_NO_ENCONTRADO;
    }

    nodoAEliminar = *actual;
    if(liberar){
        liberar(nodoAEliminar->data);
    }
    free(nodoAEliminar->data);

    if(nodoAEliminar->izq != NULL && nodoAEliminar->der != NULL){
        ///tiene dos hijos: lo reemplazo por el mayor del subarbol izquierdo
        mayor = &nodoAEliminar->izq;
        while((*mayor)->der != NULL){
            mayor = &(*mayor)->der;
        }
        nodoAEliminar->data = (*mayor)->data;
        nodoAEliminar->dataSize = (*mayor)->dataSize;
        ///el que se libera ahora es el nodo del mayor (su data ya se movio)
        actual = mayor;
        nodoAEliminar = *mayor;
        *actual = nodoAEliminar->izq;
    } else {
        ///tiene uno o ningun hijo: el padre pasa a apuntar a ese hijo
        *actual = nodoAEliminar->izq != NULL ? nodoAEliminar->izq : nodoAEliminar->der;
    }

    free(nodoAEliminar);
    arbol->cant--;
    return TODO_OK;
}

void arbolRecorrerInorden(tArbol *arbol, void (*accion)(void *)){
    ///recorre de menor a mayor segun cmp
    recorrerInorden(arbol->raiz, accion);
}

void arbolRecorrerPreorden(tArbol *arbol, void (*accion)(void *)){
    ///raiz antes que los hijos: sirve para guardar el arbol y reconstruirlo con la misma forma
    recorrerPreorden(arbol->raiz, accion);
}

void arbolRecorrerPosorden(tArbol *arbol, void (*accion)(void *)){
    ///hijos antes que la raiz
    recorrerPosorden(arbol->raiz, accion);
}

int arbolAltura(tArbol *arbol){
    return altura(arbol->raiz);
}

void arbolVaciar(tArbol *arbol, void (*liberar)(void *)){
    vaciar(arbol->raiz, liberar);
    arbol->raiz = NULL;
    arbol->cant = 0;
}

static void recorrerInorden(tNodoArbol *nodo, void (*accion)(void *)){
    if(nodo == NULL){
        return;
    }
    recorrerInorden(nodo->izq, accion);
    accion(nodo->data);
    recorrerInorden(nodo->der, accion);
}

static void recorrerPreorden(tNodoArbol *nodo, void (*accion)(void *)){
    if(nodo == NULL){
        return;
    }
    accion(nodo->data);
    recorrerPreorden(nodo->izq, accion);
    recorrerPreorden(nodo->der, accion);
}

static void recorrerPosorden(tNodoArbol *nodo, void (*accion)(void *)){
    if(nodo == NULL){
        return;
    }
    recorrerPosorden(nodo->izq, accion);
    recorrerPosorden(nodo->der, accion);
    accion(nodo->data);
}

static int altura(tNodoArbol *nodo){
    int altIzq;
    int altDer;
    if(nodo == NULL){
        return 0;
    }
    altIzq = altura(nodo->izq);
    altDer = altura(nodo->der);
    return 1 + (altIzq > altDer ? altIzq : altDer);
}

static void vaciar(tNodoArbol *nodo, void (*liberar)(void *)){
    ///posorden
    if(nodo == NULL){
        return;
    }
    vaciar(nodo->izq, liberar);
    vaciar(nodo->der, liberar);
    if(liberar){
        liberar(nodo->data);
    }
    free(nodo->data);
    free(nodo);
}
