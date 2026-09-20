#ifndef USER_H
#define USER_H
#include "lista.h"

#define ARCH_USERS "users.bin"

#define ERROR_AP_ARCH 5
#define USER_DUPLICADO 6

typedef struct {
    char user[20];
    char pass[20];
} User;

void userImprimir(void *user);
int cmpUser(void *a, void *b);
int usersAbrir(tLista *lista);
int usersGuardar(tLista *lista);
int userRegistrar(tLista *lista, char *user, char *pass);
User *userIniciarSesion(tLista *lista, char *user, char *pass);

#endif