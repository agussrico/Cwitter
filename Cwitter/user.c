#include "user.h"

void userImprimir(void *user)
{
    User *u = (User*)user;
    printf("User: %s\n", u->user);
}

int cmpUser(void* a, void* b)
{
    User* user1 = (User*)a;
    User* user2 = (User*)b;

    return strcmp(user1->user, user2->user) == 0;
}

int usersAbrir(tLista* lista)
{
    FILE* f = fopen(ARCH_USERS,"rb");
    if(!f)
        return ERROR_AP_ARCH;

    User u;
    while(fread(&u,sizeof(User),1,f))
    {
        listaInsertar(lista,&u,sizeof(User));
    }

    fclose(f);
    return TODO_OK;
}

static FILE* f = NULL;

void userEscribirArch(void* u)
{
    fwrite((User*)u,sizeof(User),1,f);
}

int usersGuardar(tLista* lista)
{
    f = fopen(ARCH_USERS,"wb");
    if(!f)
        return ERROR_AP_ARCH;

    listaRecorrerYAccion(lista, userEscribirArch);

    fclose(f);
    f = NULL;
    return TODO_OK;
}

int userRegistrar(tLista* lista, char* user, char* pass)
{
    User u;
    memset(&u, 0, sizeof(User));
    strncpy(u.pass, pass, sizeof(u.pass) - 1);
    strncpy(u.user, user, sizeof(u.user) - 1);

    if(!listaBuscarYDevolver(lista,&u,cmpUser)){
        return listaInsertar(lista,&u,sizeof(User));
    }
    return USER_DUPLICADO;
}

User* userIniciarSesion(tLista* lista, char* user, char* pass)
{
    User busqueda;
    memset(&busqueda, 0, sizeof(User));
    strncpy(busqueda.user, user, sizeof(busqueda.user) - 1);

    tNodo* nodo = listaBuscarYDevolver(lista,&busqueda,cmpUser);
    if(!nodo)
        return NULL;

    User* encontrado = (User*)nodo->data;
    if(strcmp(encontrado->pass,pass)!=0)
        return NULL;

    return encontrado;
}

