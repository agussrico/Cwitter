#include "lista.h"
#include "user.h"
#include "tweet.h"
#include "famecheck.h"
#include <stdio.h>
#include <stdlib.h>

User crearUser(char *nombre, int verificado)
{
    User u;
    memset(&u, 0, sizeof(User));
    strncpy(u.user, nombre, sizeof(u.user) - 1);
    u.verficado = verificado;
    return u;
}

void mostrarId(void *dato)
{
    printf("%u ", ((Tweet *)dato)->id);
}

int main()
{
    tLista feed;
    listaInicializar(&feed);

    // pepito123 es una cuenta verificada en FameCheck: sus tweets se reportan de verdad
    famecheckIniciar();
    User ana = crearUser("ana", 0);
    User beto = crearUser("beto", 0);
    User carlos = crearUser("carlos", 0);
    User pepito = crearUser("pepito123", 1);

    printf("=== 1. Prueba: Plataforma vacia e inicio de persistencia ===\n");
    int carga = tweetsAbrir(&feed);
    if(carga == TODO_OK && feed.cant == 0)
        printf("Plataforma vacia: No hay tweets previos o archivo nuevo.\n");
    else
        printf("Feed cargado desde archivo. Tweets actuales: %d\n", feed.cant);

    printf("\n=== 2. Setup: Publicacion de tweets ===\n");
    tweetPublicar(&feed, &ana, "Primer tweet de la plataforma"); //ID 1
    tweetPublicar(&feed, &beto, "Segundo tweet para probar permisos"); //ID 2
    tweetPublicar(&feed, &pepito, "Hola, soy una cuenta verificada"); //ID 3
    tweetPublicar(&feed, &ana, "Tweet intermedio que va a ser borrado"); //ID 4
    tweetPublicar(&feed, &carlos, "Carlos se suma a la prueba"); //ID 5
    tweetPublicar(&feed, &pepito, "Segundo tweet verificado"); //ID 6

    printf("\n=== 3. Prueba: Feed con verificados y no verificados ===\n");
    printf("Esperado: 6 3 5 4 2 1 (verificados primero, despues el mas reciente primero)\n");
    printf("Obtenido: ");
    listaRecorrerYAccion(&feed, mostrarId);
    printf("\n");
    tweetsMostrarFeed(&feed);

    printf("\n=== 4. Caso propio: tweet mas viejo no verificado va al final ===\n");
    Tweet viejo;
    memset(&viejo, 0, sizeof(Tweet));
    viejo.id = 99;
    strcpy(viejo.autor, "beto");
    strcpy(viejo.mensaje, "Tweet de hace una hora");
    viejo.fecha = time(NULL) - 3600;
    listaInsertarOrdenado(&feed, &viejo, sizeof(Tweet), cmpTweetFeed);
    printf("Esperado: 6 3 5 4 2 1 99\n");
    printf("Obtenido: ");
    listaRecorrerYAccion(&feed, mostrarId);
    printf("\n");
    tweetEliminar(&feed, 99, "beto");

    printf("\n=== 5. Prueba: Recorrer el feed hacia adelante y volver varias veces ===\n");
    tNodo *actual = feed.inicio;
    printf("Inicio: %u\n", ((Tweet *)actual->data)->id);
    actual = actual->sig; printf("Siguiente: %u\n", ((Tweet *)actual->data)->id);
    actual = actual->sig; printf("Siguiente: %u\n", ((Tweet *)actual->data)->id);
    actual = actual->ant; printf("Anterior: %u\n", ((Tweet *)actual->data)->id);
    actual = actual->ant; printf("Anterior: %u\n", ((Tweet *)actual->data)->id);
    actual = actual->sig; printf("Siguiente: %u\n", ((Tweet *)actual->data)->id);
    printf("Esperado: 6, 3, 5, 3, 6, 3\n");

    printf("\n=== 6. Prueba: Busqueda de tweets ===\n");
    printf("> Buscando 'permisos' (Deberia encontrar el de beto):\n");
    tweetBuscarYMostrar(&feed, "permisos");

    printf("\n> Buscando 'elefante' (No existe):\n");
    tweetBuscarYMostrar(&feed, "elefante");

    printf("\n=== 7. Prueba: Eliminacion en extremos e intermedio ===\n");
    printf("Borrando PRIMER tweet del feed (ID 6, pepito123): %d\n", tweetEliminar(&feed, 6, "pepito123"));
    printf("Borrando ULTIMO tweet del feed (ID 1, ana): %d\n", tweetEliminar(&feed, 1, "ana"));
    printf("Borrando tweet INTERMEDIO (ID 4, ana): %d\n", tweetEliminar(&feed, 4, "ana"));

    printf("\nFeed despues de las 3 eliminaciones (Deberian quedar los IDs 3, 5 y 2):\n");
    tweetsMostrarFeed(&feed);

    printf("\n=== 8. Casos Propios (Validacion de Permisos) ===\n");
    printf("> Intento de fraude: Carlos intenta borrar el tweet de Beto (ID 2): ");
    if(tweetEliminar(&feed, 2, "carlos") == SIN_PERMISO) {
        printf("BLOQUEADO (Correcto: Sin permiso)\n");
    } else {
        printf("ERROR DE SEGURIDAD\n");
    }

    printf("> Modificacion legitima: Beto edita su tweet (ID 2): ");
    if(tweetModificar(&feed, 2, "Beto edito este texto con exito", "beto") == TODO_OK) {
        printf("MODIFICADO (Correcto)\n");
    }

    tweetsMostrarFeed(&feed);

    printf("\n=== 9. Prueba: Persistencia ===\n");
    tweetsGuardar(&feed);
    listaVaciar(&feed);
    printf("Memoria vaciada (simulando cierre). Tweets actuales en RAM: %d\n", feed.cant);

    printf("\n=== Volviendo a cargar para confirmar conservacion y orden ===\n");
    tLista feedVerificacion;
    listaInicializar(&feedVerificacion);
    tweetsAbrir(&feedVerificacion);

    printf("Esperado: 3 5 2\n");
    printf("Obtenido: ");
    listaRecorrerYAccion(&feedVerificacion, mostrarId);
    printf("\n");
    tweetsMostrarFeed(&feedVerificacion);
    listaVaciar(&feedVerificacion);

    famecheckFinalizar();
    return 0;
}
