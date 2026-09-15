#ifndef TWEET_H_INCLUDED
#define TWEET_H_INCLUDED

#include "lista.h"

#define MAX_TWEET 140
#define ARCH_TWEETS "tweets.bin"

typedef struct {
    unsigned int id;
    char autor[20];
    char mensaje[MAX_TWEET + 1]; // 140 caracteres mas el /0
} Tweet;

int tweetsAbrir(tLista *feed);
int tweetsGuardar(tLista *feed);
int tweetPublicar(tLista *feed, char *autor, char *mensaje);
int tweetModificar(tLista *feed, unsigned int id, char *nuevoMensaje);
int tweetEliminar(tLista *feed, unsigned int id);
void tweetsMostrarFeed(tLista *feed);
void tweetBuscarYMostrar(tLista *feed, char *textoBuscado);
void tweetImprimir(void *tweet);
int cmpTweetId(void *a, void *b);
int cmpTweetTexto(void *a, void *b);

#endif // TWEET_H_INCLUDED
