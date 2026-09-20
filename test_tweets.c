#include "lista.h"
#include "tweet.h"
#include <stdio.h>
#include <stdlib.h>

int main()
{
    tLista feed;
    listaInicializar(&feed);

    printf("=== 1. Prueba: Plataforma vacia e inicio de persistencia ===\n");
    int carga = tweetsAbrir(&feed);
    if(carga == TODO_OK && feed.cant == 0)
        printf("Plataforma vacia: No hay tweets previos o archivo nuevo.\n");
    else
        printf("Feed cargado desde archivo. Tweets actuales: %d\n", feed.cant);

    printf("\n=== 2. Setup: Publicacion de tweets ===\n");
    tweetPublicar(&feed, "ana", "Primer tweet de la plataforma"); //ID 1
    tweetPublicar(&feed, "beto", "Segundo tweet para probar permisos"); //ID 2
    tweetPublicar(&feed, "ana", "Tweet intermedio que va a ser borrado"); //ID 3
    tweetPublicar(&feed, "carlos", "Carlos se suma a la prueba"); //ID 4
    tweetPublicar(&feed, "beto", "Ultimo tweet de la lista enlazada"); //ID 5

    printf("Feed original armado para las pruebas:\n");
    tweetsMostrarFeed(&feed);

    printf("\n=== 3. Prueba: Busqueda de tweets ===\n");
    printf("> Buscando 'permisos' (Deberia encontrar el de beto):\n");
    tweetBuscarYMostrar(&feed, "permisos");

    printf("\n> Buscando 'elefante' (No existe):\n");
    tweetBuscarYMostrar(&feed, "elefante");

    printf("\n=== 4. Prueba: Eliminacion en extremos e intermedio ===\n");
    printf("Borrando PRIMER tweet (ID 1, ana): %d\n", tweetEliminar(&feed, 1, "ana"));
    printf("Borrando ULTIMO tweet (ID 5, beto): %d\n", tweetEliminar(&feed, 5, "beto"));
    printf("Borrando tweet INTERMEDIO (ID 3, ana): %d\n", tweetEliminar(&feed, 3, "ana"));

    printf("\nFeed despues de las 3 eliminaciones (Deberian quedar solo los IDs 2 y 4):\n");
    tweetsMostrarFeed(&feed);

    printf("\n=== 5. Casos Propios (Validacion de Permisos) ===\n");
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

    printf("\n=== 6. Prueba: Persistencia ===\n");
    tweetsGuardar(&feed);
    listaVaciar(&feed);
    printf("Memoria vaciada (simulando cierre). Tweets actuales en RAM: %d\n", feed.cant);

    printf("\n=== Volviendo a cargar para confirmar conservacion ===\n");
    tLista feedVerificacion;
    listaInicializar(&feedVerificacion);
    tweetsAbrir(&feedVerificacion);

    printf("Feed recuperado del archivo:\n");
    tweetsMostrarFeed(&feedVerificacion);
    listaVaciar(&feedVerificacion);

    return 0;
}
