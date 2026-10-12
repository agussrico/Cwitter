#include "tweet.h"



// Variable global est�tica para autoincrementar el ID de los tweets durante la ejecuci�n
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
    char strFecha[20];
    struct tm *infoTiempo = localtime(&(t->fechaHora));
    strftime(strFecha, sizeof(strFecha), "%d/%m/%Y %H:%M", infoTiempo);
    printf("\n[ID: %u] @%s dijo:\n%s\n", t->id, t->autor, t->mensaje);
    printf("\nFecha: %s\n",strFecha);
    printf("----------------------------------------\n");
}

int compararTweets(void *a, void *b) {
    Tweet *nuevo = (Tweet *)a;
    Tweet *actual = (Tweet *)b;

    /* Único caso donde frena antes del final:
       el nuevo es verificado (1) y chocó contra el primer NO verificado (0) */
    if (nuevo->reportadoAFamecheck == 0 && actual->reportadoAFamecheck == 1) {
        return -1;
    }

    /* En cualquier otro caso devuelve 1 para que el while siga avanzando
       e inserte al final de su bloque, igual que hacía listaInsertar */
    return 1;
}

int tweetPublicar(tLista *feed, User *autor, char *mensaje) {
    //Se modifico esta funcion para que cuando se ingrese la funcion reciba el USER y no solo su nombre
    //De esta manera se chequea si esta verificado y se evitan consultas de mas a la API
    Tweet nuevoTweet;
    memset(&nuevoTweet, 0, sizeof(Tweet));

    ultimo_id++;
    nuevoTweet.id = ultimo_id;

    nuevoTweet.reportadoAFamecheck = 0;

    strncpy(nuevoTweet.autor, autor->user, sizeof(nuevoTweet.autor) - 1);
    nuevoTweet.autor[sizeof(nuevoTweet.autor) - 1] = '\0';

    strncpy(nuevoTweet.mensaje, mensaje, MAX_TWEET);
    nuevoTweet.mensaje[MAX_TWEET] = '\0';

    nuevoTweet.fechaHora = time(NULL);

    // LOGICA FAMECHECK SUMADA A TWEET----
    if(autor->verficado == 1){
        printf("Reportando Tweet en FameCheck...\n");
        if (famecheckReportarPublicacion(autor->user, mensaje, &nuevoTweet.reportadoAFamecheck) != FAMECHECK_OK){
            printf("Error al Reportar el tweet a FameCheck\n");
        }
        else{
            printf("[!] Tweet reportado correctamente [!]\n");
        }
    }
    // LOGICA FAMECHECK SUMADA A TWEET----

    return listaInsertarOrdenado(feed, &nuevoTweet, sizeof(Tweet),compararTweets);
}

int tweetModificar(tLista *feed, unsigned int id, char *nuevoMensaje, char *autor) {

    tNodo *nodo = listaBuscarYDevolver(feed, &id, cmpTweetId);

    if (nodo == NULL)
        return NODO_NO_ENCONTRADO;

    Tweet *t = (Tweet *)nodo->data;

    if (strcmp(t->autor, autor) != 0)
        return SIN_PERMISO;

    strncpy(t->mensaje, nuevoMensaje, MAX_TWEET);
    t->mensaje[MAX_TWEET] = '\0';

    return TODO_OK;
}

int tweetEliminar(tLista *feed, unsigned int id, char *autor) {

    tNodo *nodo = listaBuscarYDevolver(feed, &id, cmpTweetId);

    if (nodo == NULL)
        return NODO_NO_ENCONTRADO;

    Tweet *t = (Tweet *)nodo->data;

    if (strcmp(t->autor, autor) != 0)
        return SIN_PERMISO;

    return listaEliminarNodo(feed, &id, cmpTweetId);
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
