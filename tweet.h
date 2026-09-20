#ifndef TWEET_H_INCLUDED
#define TWEET_H_INCLUDED

#include "lista.h"

#define MAX_TWEET 140
#define ARCH_TWEETS "tweets.bin"
#define SIN_PERMISO 7   // el tweet no es del usuario logueado

typedef struct {
    unsigned int id;
    char autor[20];
    char mensaje[MAX_TWEET + 1]; // 140 caracteres mas el /0
} Tweet;

int tweetsAbrir(tLista *feed);
int tweetsGuardar(tLista *feed);
int tweetPublicar(tLista *feed, char *autor, char *mensaje);
int tweetModificar(tLista *feed, unsigned int id, char *nuevoMensaje, char *autor);
int tweetEliminar(tLista *feed, unsigned int id, char *autor);
void tweetsMostrarFeed(tLista *feed);
void tweetBuscarYMostrar(tLista *feed, char *textoBuscado);
void tweetImprimir(void *tweet);
int cmpTweetId(void *a, void *b);
int cmpTweetTexto(void *a, void *b);

#endif // TWEET_H_INCLUDED
