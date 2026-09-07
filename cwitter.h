#ifndef CWITTER_H
#define CWITTER_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARCHIVO_USUARIOS "usuarios.dat"
#define ARCHIVO_TWEETS "tweets.dat"

/* 1. La carga útil: Esto es exactamente lo que lees y escribes en el archivo */
typedef struct {
    char nombreUsuario[20];
    char contrasenia[20];
} DatosUsuario;

/* 2. El nodo: Esto solo existe mientras el programa está abierto en la RAM */
typedef struct Usuario {
    DatosUsuario datos;
    struct Usuario *siguiente;
} Usuario;

/* 1. La carga útil: Esto es lo único que viaja al archivo con fwrite/fread */
typedef struct {
    int id;
    char autor[20];
    char contenido[200];
    char fechaHora[20];
    int activo; /* 1 = visible, 0 = eliminado (para el borrado lógico) */
} DatosTweet;

/* 2. El nodo: Esto forma la lista enlazada en la RAM */
typedef struct Tweet {
    DatosTweet datos;
    struct Tweet *siguiente;
} Tweet;
 
/* Sistema: agrupa todo el estado de la aplicacion */
typedef struct {
    Usuario *listaUsuarios;
    Tweet *listaTweets;
    int proximoIdTweet;
    Usuario *usuarioActual; /* NULL si no hay sesion iniciada */
} Sistema;

#endif