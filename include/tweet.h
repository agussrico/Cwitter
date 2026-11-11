#ifndef TWEET_H_INCLUDED
#define TWEET_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "famecheck.h"
#include "lista.h"
#include "user.h"
#include "indice.h"

#define MAX_TWEET 280
#define ARCH_TWEETS "tweets.bin"
#define SIN_PERMISO 7   // el tweet no es del usuario logueado

typedef struct {
    unsigned int id;
    unsigned int idPadre;
    char autor[20];
    char mensaje[MAX_TWEET + 1]; // 280 caracteres mas el /0
    time_t fecha;
    int verificado;
    int reportadoAFamecheck;
    tLista hilo;
} Tweet;

int tweetsAbrir(tLista *feed);
int tweetsGuardar(tLista *feed);
void tweetGuardarHilo(FILE *pf, tLista *lista);
int tweetPublicar(tLista *feed, User *autor, char *mensaje, unsigned int idPadre);
int tweetModificar(tLista *feed, unsigned int id, char *nuevoMensaje, char *autor);
int tweetEliminar(tLista *feed, unsigned int id, char *autor);
void tweetVaciarHilo(tLista *hilo);
Tweet *tweetBuscarPorId(tLista *lista, unsigned int id);
void tweetsMostrarFeed(tLista *feed);
void tweetBuscarYMostrar(tLista *feed, char *textoBuscado);
void tweetImprimir(void *tweet);
int cmpTweetId(void *a, void *b);
int cmpTweetTexto(void *a, void *b);
int cmpTweetFeed(void *a, void *b);

// Versiones con indice: hacen lo mismo que las originales y mantienen actualizado el indice de busqueda
int tweetsAbrirConIndice(tLista *feed, tIndice *indice);
int tweetPublicarConIndice(tLista *feed, tIndice *indice, User *autor, char *mensaje, unsigned int idPadre);
int tweetModificarConIndice(tLista *feed, tIndice *indice, unsigned int id, char *nuevoMensaje, char *autor);
int tweetEliminarConIndice(tLista *feed, tIndice *indice, unsigned int id, char *autor);
void tweetBuscarYMostrarConIndice(tIndice *indice, char *textoBuscado);

#endif // TWEET_H_INCLUDED
