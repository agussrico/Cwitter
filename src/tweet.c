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
    if(padre == NULL)
        return NODO_NO_ENCONTRADO;
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

// ================== VERSIONES CON INDICE ==================

// Busca el nodo de un tweet en el feed y en todos los hilos. Si contenedor no es NULL,
// devuelve ahi la lista donde esta el nodo (el feed o el hilo del tweet padre)
static tNodo *tweetBuscarNodoPorId(tLista *lista, unsigned int id, tLista **contenedor) {

    tNodo *actual = lista->inicio;

    while (actual != NULL) {
        Tweet *t = (Tweet *)actual->data;
        if (t->id == id) {
            if (contenedor != NULL)
                *contenedor = lista;
            return actual;
        }

        tNodo *encontradoEnHilo = tweetBuscarNodoPorId(&t->hilo, id, contenedor);
        if (encontradoEnHilo != NULL)
            return encontradoEnHilo;

        actual = actual->sig;
    }
    return NULL;
}

// Indexa todos los tweets de una lista y, recursivamente, sus respuestas
static int tweetIndexarLista(tIndice *indice, tLista *lista) {

    tNodo *actual = lista->inicio;

    while (actual != NULL) {
        Tweet *t = (Tweet *)actual->data;
        if (indiceCargar(indice, actual, t->mensaje) == INDICE_ERROR)
            return INDICE_ERROR;
        if (tweetIndexarLista(indice, &t->hilo) == INDICE_ERROR)
            return INDICE_ERROR;
        actual = actual->sig;
    }
    return TODO_OK;
}

// Saca del indice todos los tweets de una lista y, recursivamente, sus respuestas
static void tweetDesindexarLista(tIndice *indice, tLista *lista) {

    tNodo *actual = lista->inicio;

    while (actual != NULL) {
        Tweet *t = (Tweet *)actual->data;
        tweetDesindexarLista(indice, &t->hilo);
        indiceQuitar(indice, actual, t->mensaje);
        actual = actual->sig;
    }
}

int tweetsAbrirConIndice(tLista *feed, tIndice *indice) {
    // Carga el feed y los hilos con la funcion original y despues indexa todos los tweets
    int resultado = tweetsAbrir(feed);
    if (resultado != TODO_OK) {
        return resultado;
    }
    return tweetIndexarLista(indice, feed);
}

int tweetPublicarConIndice(tLista *feed, tIndice *indice, User *autor, char *mensaje, unsigned int idPadre) {
    // Si es una respuesta, el tweet padre tiene que existir
    if (idPadre != 0 && tweetBuscarPorId(feed, idPadre) == NULL) {
        return NODO_NO_ENCONTRADO;
    }

    // Publica con la funcion original (incluye el reporte a FameCheck) y despues indexa el tweet nuevo
    int resultado = tweetPublicar(feed, autor, mensaje, idPadre);
    if (resultado != TODO_OK) {
        return resultado;
    }

    // listaInsertarOrdenado no devuelve el nodo: lo busco por id (el tweet nuevo tiene el ultimo id)
    tNodo *nodo = tweetBuscarNodoPorId(feed, ultimo_id, NULL);
    if (nodo == NULL) {
        return NODO_NO_ENCONTRADO;
    }

    if (indiceCargar(indice, nodo, ((Tweet *)nodo->data)->mensaje) == INDICE_ERROR) {
        return INDICE_ERROR;
    }
    return TODO_OK;
}

int tweetModificarConIndice(tLista *feed, tIndice *indice, unsigned int id, char *nuevoMensaje, char *autor) {

    tNodo *nodo = tweetBuscarNodoPorId(feed, id, NULL);

    if (nodo == NULL)
        return NODO_NO_ENCONTRADO;

    Tweet *t = (Tweet *)nodo->data;

    if (strcmp(t->autor, autor) != 0)
        return SIN_PERMISO;

    // Se quita del indice con el mensaje viejo antes de cambiarlo, y se vuelve a cargar con el nuevo
    indiceQuitar(indice, nodo, t->mensaje);

    strncpy(t->mensaje, nuevoMensaje, MAX_TWEET);
    t->mensaje[MAX_TWEET] = '\0';

    if (indiceCargar(indice, nodo, t->mensaje) == INDICE_ERROR)
        return INDICE_ERROR;

    return TODO_OK;
}

int tweetEliminarConIndice(tLista *feed, tIndice *indice, unsigned int id, char *autor) {

    tLista *contenedor;
    tNodo *nodo = tweetBuscarNodoPorId(feed, id, &contenedor);

    if (nodo == NULL)
        return NODO_NO_ENCONTRADO;

    Tweet *t = (Tweet *)nodo->data;

    if (strcmp(t->autor, autor) != 0)
        return SIN_PERMISO;

    // Primero se saca del indice el tweet y todas sus respuestas: despues se liberan
    tweetDesindexarLista(indice, &t->hilo);
    indiceQuitar(indice, nodo, t->mensaje);

    tweetVaciarHilo(&t->hilo);
    return listaEliminarNodo(contenedor, &id, cmpTweetId);
}

void tweetBuscarYMostrarConIndice(tIndice *indice, char *textoBuscado) {

    printf("\n--- RESULTADOS DE BUSQUEDA: '%s' ---\n", textoBuscado);

    // El indice guarda palabras sueltas: se busca la primera palabra de lo ingresado
    char copia[MAX_PALABRA];
    strncpy(copia, textoBuscado, MAX_PALABRA - 1);
    copia[MAX_PALABRA - 1] = '\0';
    char *palabra = strtok(copia, SEPARADORES);

    tLista *resultados = (palabra != NULL) ? indiceBuscar(indice, palabra) : NULL;

    if (resultados == NULL) {
        printf("No se encontraron tweets que contengan esa palabra.\n");
        return;
    }

    // Cada elemento de la lista es un tNodo* que apunta al nodo del tweet (en el feed o en un hilo)
    tNodo *actual = resultados->inicio;
    while (actual != NULL) {
        Tweet *t = (Tweet *)(*(tNodo **)actual->data)->data;
        if (t->idPadre != 0)
            printf("\n(respuesta al tweet ID: %u)", t->idPadre);
        tweetImprimir(t);
        actual = actual->sig;
    }
}
