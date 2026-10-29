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

int cmpTweetFeed(void *a, void *b)
{
    Tweet *t1 = (Tweet *)a;
    Tweet *t2 = (Tweet *)b;

    if(t1->verificado != t2->verificado)
        return t2->verificado - t1->verificado; /*se muestra primero el verificado*/

    if(t1->fecha != t2->fecha)
        return t1->fecha > t2->fecha ? -1 : 1;/*si ambos estan verificados/no verificados, se muestra por fecha*/

    return t1->id > t2->id ? -1 : 1; // mismo segundo: el de id mas alto es el mas nuevo
}

// agregar impresion hilo
void tweetImprimir(void *dato) {
    Tweet *t = (Tweet *)dato;

    struct tm *tm_info = localtime(&t->fecha);
    char fechaStr[20];
    strftime(fechaStr, sizeof(fechaStr), "%d/%m/%Y %H:%M:%S", tm_info);

    printf("\n[ID: %u] [%s] @%s", t->id, fechaStr, t->autor);

    if (t->verificado == 1) {
        printf(" [V]");
    }

    printf(" dijo:\n%s\n", t->mensaje);
    printf("Respuestas: %d\n", t->hilo.cant);
    printf("----------------------------------------\n");
}

int tweetPublicar(tLista *feed, User *autor, char *mensaje, unsigned int idPadre) {
    //Se modifico esta funcion para que cuando se ingrese la funcion reciba el USER y no solo su nombre
    //De esta manera se chequea si esta verificado y se evitan consultas de mas a la API
    Tweet nuevoTweet;
    memset(&nuevoTweet, 0, sizeof(Tweet));

    ultimo_id++;
    nuevoTweet.id = ultimo_id;
    nuevoTweet.idPadre = idPadre;
    nuevoTweet.fecha = time(NULL);

    nuevoTweet.reportadoAFamecheck = 0;

    strncpy(nuevoTweet.autor, autor->user, sizeof(nuevoTweet.autor) - 1);
    nuevoTweet.autor[sizeof(nuevoTweet.autor) - 1] = '\0';
    nuevoTweet.verificado = autor->verficado;

    strncpy(nuevoTweet.mensaje, mensaje, MAX_TWEET);
    nuevoTweet.mensaje[MAX_TWEET] = '\0';

    listaInicializar(&nuevoTweet.hilo);

    // LOGICA FAMECHECK SUMADA A TWEET----
    if(autor->verficado == 1){
        printf("Reportando Tweet en FameCheck...\n");
        int reporte = famecheckReportarPublicacion(autor->user, mensaje, &nuevoTweet.reportadoAFamecheck);

        if(reporte == FAMECHECK_OK){
            printf("[!] Tweet reportado correctamente [!]\n");
        }
        else{
            printf("Error al Reportar el tweet a FameCheck\n");
            switch(reporte){
                case FAMECHECK_ERROR_RED:       famecheckRegistrarLog(autor->user, nuevoTweet.id, "FAMECHECK_ERROR_RED"); break;
                case FAMECHECK_ERROR_RESPUESTA: famecheckRegistrarLog(autor->user, nuevoTweet.id, "FAMECHECK_ERROR_RESPUESTA"); break;
                case FAMECHECK_ERROR_MEMORIA:   famecheckRegistrarLog(autor->user, nuevoTweet.id, "FAMECHECK_ERROR_MEMORIA"); break;
                case FAMECHECK_ERROR_CONFIG:    famecheckRegistrarLog(autor->user, nuevoTweet.id, "FAMECHECK_ERROR_CONFIG"); break;
                default:                        famecheckRegistrarLog(autor->user, nuevoTweet.id, "ERROR_DESCONOCIDO");
            }
        }
    }
    // LOGICA FAMECHECK SUMADA A TWEET----

    if(idPadre == 0)
        return listaInsertarOrdenado(feed,&nuevoTweet,sizeof(Tweet),cmpTweetFeed);

    Tweet *padre = tweetBuscarPorId(feed,idPadre);
    return listaInsertarOrdenado(&padre->hilo, &nuevoTweet, sizeof(Tweet),cmpTweetFeed);    
}

int tweetModificar(tLista *feed, unsigned int id, char *nuevoMensaje, char *autor) {

    Tweet *t = tweetBuscarPorId(feed, id);

    if (t == NULL)
        return NODO_NO_ENCONTRADO;

    if (strcmp(t->autor, autor) != 0)
        return SIN_PERMISO;

    strncpy(t->mensaje, nuevoMensaje, MAX_TWEET);
    t->mensaje[MAX_TWEET] = '\0';

    return TODO_OK;
}

int tweetEliminar(tLista *feed, unsigned int id, char *autor) {

    Tweet *t = tweetBuscarPorId(feed, id);

    if (t == NULL)
        return NODO_NO_ENCONTRADO;
    
    if (strcmp(t->autor, autor) != 0)
        return SIN_PERMISO;

    tLista *origen = feed;

    if(t->idPadre != 0)
    {
        Tweet *tPadre = tweetBuscarPorId(feed,t->idPadre);
        origen = &tPadre->hilo;
    }

    tweetVaciarHilo(&t->hilo);
    
    return listaEliminarNodo(origen, &id, cmpTweetId);
}

void tweetVaciarHilo(tLista *hilo)
{
    tNodo *nodo = hilo->inicio;
    
    while(nodo != NULL)
    {
        Tweet *t = (Tweet*)nodo->data;
        tweetVaciarHilo(&t->hilo);
        nodo = nodo->sig;
    }

    listaVaciar(hilo);
}

Tweet *tweetBuscarPorId(tLista *lista, unsigned int id)
{
    tNodo *actual = lista->inicio;

    while (actual != NULL)
    {
        Tweet *t = (Tweet *)actual->data;
        if (t->id == id) {
            return t;
        }

        // Busco recursivamente en el hilo
        Tweet *encontradoEnHilo = tweetBuscarPorId(&t->hilo, id);
        if (encontradoEnHilo != NULL) {
            return encontradoEnHilo;
        }

        actual = actual->sig;
    }

    return NULL;
}

int tweetsAbrir(tLista *feed) {

    FILE *pf = fopen(ARCH_TWEETS, "rb");

    if (pf == NULL) {
        return ERROR_AP_ARCH;
    }

    Tweet aux;
    while (fread(&aux, sizeof(Tweet), 1, pf) == 1)
    {
        listaInicializar(&aux.hilo);

        if (aux.idPadre == 0)
        {
            listaInsertarOrdenado(feed, &aux, sizeof(Tweet), cmpTweetFeed);
        }
        
        else
        {
            Tweet *padre = tweetBuscarPorId(feed, aux.idPadre);
            listaInsertarOrdenado(&padre->hilo, &aux, sizeof(Tweet), cmpTweetFeed);
        }

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
        return ERROR_AP_ARCH;
    }

    tweetGuardarHilo(pf,feed);

    fclose(pf);
    return TODO_OK;
}

void tweetGuardarHilo(FILE *pf, tLista *lista)
{
    tNodo *actual = lista->inicio;

    while (actual != NULL)
    {
        Tweet *t = (Tweet *)actual->data;
        fwrite(t, sizeof(Tweet), 1, pf);
        tweetGuardarHilo(pf, &t->hilo);
        actual = actual->sig;
    }
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
