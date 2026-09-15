#include "tweet.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Variable global estática para autoincrementar el ID de los tweets durante la ejecución
static unsigned int ultimo_id = 0;


int cmpTweetId(void *a, void *b) {
    Tweet *tweetA = (Tweet *)a;
    unsigned int *idB = (unsigned int *)b;

    if (tweetA->id == *idB) {
        return 1; //  (Encontrado)
    }
    return 0; //  (No coincide)
}

int cmpTweetTexto(void *a, void *b) {
    Tweet *tweetA = (Tweet *)a;
    char *textoBuscado = (char *)b;

    if (strstr(tweetA->mensaje, textoBuscado) != NULL) {
        return 1; // Encontrado
    }
    return 0; // No encontrado
}

void tweetImprimir(void *dato) {
    Tweet *t = (Tweet *)dato;
    printf("\n[ID: %u] @%s dijo:\n%s\n", t->id, t->autor, t->mensaje);
    printf("----------------------------------------\n");
}


int tweetPublicar(tLista *feed, char *autor, char *mensaje) {

    Tweet nuevoTweet;

    ultimo_id++;
    nuevoTweet.id = ultimo_id;

    strncpy(nuevoTweet.autor, autor, sizeof(nuevoTweet.autor) - 1);
    nuevoTweet.autor[sizeof(nuevoTweet.autor) - 1] = '\0';

    strncpy(nuevoTweet.mensaje, mensaje, MAX_TWEET);
    nuevoTweet.mensaje[MAX_TWEET] = '\0';

    return listaInsertar(feed, &nuevoTweet, sizeof(Tweet));
}

int tweetModificar(tLista *feed, unsigned int id, char *nuevoMensaje) {

    tNodo *nodo = listaBuscarYDevolver(feed, &id, cmpTweetId);

    if (nodo != NULL) {
        Tweet *t = (Tweet *)nodo->data;

        strncpy(t->mensaje, nuevoMensaje, MAX_TWEET);
        t->mensaje[MAX_TWEET] = '\0';

        return TODO_OK;
    }

    return NODO_NO_ENCONTRADO;
}

int tweetEliminar(tLista *feed, unsigned int id) {

    int resultado = listaEliminarNodo(feed, &id, cmpTweetId);

    return resultado;
}

int tweetsAbrir(tLista *feed) {

    FILE *pf = fopen(ARCH_TWEETS, "rb");

    if (pf == NULL) {
        return TODO_OK;
    }

    Tweet aux;
    while (fread(&aux, sizeof(Tweet), 1, pf) == 1) {

        listaInsertar(feed, &aux, sizeof(Tweet));

        if (aux.id > ultimo_id) {
            ultimo_id = aux.id;
        }
    }

    fclose(pf);
    return TODO_OK;
}

int tweetsGuardar(tLista *feed) {

    FILE *pf = fopen(ARCH_TWEETS, "wb");
    if (pf == NULL) {
        return -1; // Error al intentar crear o abrir el archivo||crear macro?
    }

    tNodo *actual = feed->inicio;

    while (actual != NULL) {
        Tweet *t = (Tweet *)actual->data;
        fwrite(t, sizeof(Tweet), 1, pf);
        actual = actual->sig;
    }

    fclose(pf);
    return TODO_OK;
}

void tweetsMostrarFeed(tLista *feed) {

    printf("\n========= FEED DE CWITTER =========\n");


    listaRecorrerYAccion(feed, tweetImprimir);

    printf("===================================\n");
}

void tweetBuscarYMostrar(tLista *feed, char *textoBuscado) {

    printf("\n--- RESULTADOS DE BUSQUEDA: '%s' ---\n", textoBuscado);

    tNodo *actual = feed->inicio;
    int encontrados = 0;

    while (actual != NULL) {
        if (cmpTweetTexto(actual->data, textoBuscado) == 1) {
            tweetImprimir(actual->data);
            encontrados++;
        }
        actual = actual->sig;
    }

    if (encontrados == 0) {
        printf("No se encontraron tweets que contengan esa palabra.\n");
    }
}
