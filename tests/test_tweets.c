#include "lista.h"
#include "user.h"
#include "tweet.h"
#include "famecheck.h"
#include "indice.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    remove(ARCH_TWEETS); // Asegura arrancar desde plataforma vacia en la prueba

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
    if(carga == ERROR_AP_ARCH && feed.cant == 0)
        printf("Plataforma vacia: No hay tweets previos o archivo nuevo.\n");
    else
        printf("Feed cargado desde archivo. Tweets actuales: %d\n", feed.cant);

    printf("\n=== 2. Setup: Publicacion de tweets ===\n");
    tweetPublicar(&feed, &ana, "Primer tweet de la plataforma", 0); //ID 1
    tweetPublicar(&feed, &beto, "Segundo tweet para probar permisos", 0); //ID 2
    tweetPublicar(&feed, &pepito, "Hola, soy una cuenta verificada", 0); //ID 3
    tweetPublicar(&feed, &ana, "Tweet intermedio que va a ser borrado", 0); //ID 4
    tweetPublicar(&feed, &carlos, "Carlos se suma a la prueba", 0); //ID 5
    tweetPublicar(&feed, &pepito, "Segundo tweet verificado", 0); //ID 6

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
    viejo.idPadre = 0;
    strcpy(viejo.autor, "beto");
    strcpy(viejo.mensaje, "Tweet de hace una hora");
    viejo.fecha = time(NULL) - 3600;
    listaInicializar(&viejo.hilo);
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
    tweetVaciarHilo(&feed);
    printf("Memoria vaciada (simulando cierre). Tweets actuales en RAM: %d\n", feed.cant);

    printf("\n=== Volviendo a cargar para confirmar conservacion y orden ===\n");
    tLista feedVerificacion;
    tIndice indice;
    listaInicializar(&feedVerificacion);
    indiceInicializar(&indice);
    tweetsAbrirConIndice(&feedVerificacion, &indice);

    printf("Esperado: 3 5 2\n");
    printf("Obtenido: ");
    listaRecorrerYAccion(&feedVerificacion, mostrarId);
    printf("\n");

    // =========================================================================
    // PRUEBAS SPRINT 3: 280 CARACTERES, HILOS MULTINIVEL E INDICE SOBRE ABB
    // =========================================================================
    printf("\n===================================================================\n");
    printf("             PRUEBAS SPRINT 3 (HILOS, 280 CHARS E INDICE)\n");
    printf("===================================================================\n");

    printf("\n=== 10. Sprint 3 - Caso 1: Tweet de 280 caracteres y truncado de >280 ===\n");
    char textoLargo[330];
    memset(textoLargo, 'A', 320);
    textoLargo[320] = '\0';
    tweetPublicarConIndice(&feedVerificacion, &indice, &ana, textoLargo, 0); // ID 7
    Tweet *tLargo = tweetBuscarPorId(&feedVerificacion, 7);
    printf("Largo enviado: 320 | Largo guardado: %d (Esperado: %d)\n",
           tLargo ? (int)strlen(tLargo->mensaje) : -1, MAX_TWEET);

    printf("\n=== 11. Sprint 3 - Caso 2: Hilos multinivel (respuesta a respuesta sin limite) ===\n");
    // Respondemos al tweet raiz ID 3 (de pepito123):
    tweetPublicarConIndice(&feedVerificacion, &indice, &ana, "Respuesta nivel 1 de Ana al tweet 3", 3);       // ID 8 (padre 3)
    tweetPublicarConIndice(&feedVerificacion, &indice, &beto, "Respuesta nivel 2 de Beto a Ana", 8);          // ID 9 (padre 8)
    tweetPublicarConIndice(&feedVerificacion, &indice, &carlos, "Respuesta nivel 3 profunda de Carlos", 9);   // ID 10 (padre 9)
    tweetPublicarConIndice(&feedVerificacion, &indice, &carlos, "Segunda respuesta nivel 1 al tweet 3", 3);   // ID 11 (padre 3)

    Tweet *t3 = tweetBuscarPorId(&feedVerificacion, 3);
    Tweet *t8 = tweetBuscarPorId(&feedVerificacion, 8);
    Tweet *t9 = tweetBuscarPorId(&feedVerificacion, 9);
    Tweet *t10 = tweetBuscarPorId(&feedVerificacion, 10);
    printf("Hijos directos de Tweet #3: %d (Esperado: 2 -> IDs 8 y 11)\n", t3->hilo.cant);
    printf("Nivel 1 (ID 8) idPadre=%u, respuestas=%d\n", t8->idPadre, t8->hilo.cant);
    printf("Nivel 2 (ID 9) idPadre=%u, respuestas=%d\n", t9->idPadre, t9->hilo.cant);
    printf("Nivel 3 (ID 10) idPadre=%u, respuestas=%d\n", t10->idPadre, t10->hilo.cant);

    printf("\n=== 12. Sprint 3 - Caso 3: Responder a un ID inexistente ===\n");
    int rInexistente = tweetPublicarConIndice(&feedVerificacion, &indice, &ana, "Respuesta a fantasma", 9999);
    printf("Resultado de responder al ID 9999: %d (Esperado: %d = NODO_NO_ENCONTRADO)\n",
           rInexistente, NODO_NO_ENCONTRADO);

    printf("\n=== 13. Sprint 3 - Caso 4: Busqueda con Indice ABB en hilos y mayusculas/minusculas ===\n");
    printf("> Buscando 'PROFUNDA' en el indice (debe encontrar la respuesta ID 10 de nivel 3):\n");
    tweetBuscarYMostrarConIndice(&indice, "PROFUNDA");

    printf("\n=== 14. Sprint 3 - Caso 5: Permisos y actualizacion del Indice al modificar respuesta en hilo ===\n");
    int rSinPermiso = tweetModificarConIndice(&feedVerificacion, &indice, 9, "Intento hackear respuesta", "carlos");
    printf("Carlos intenta modificar respuesta de Beto (ID 9): %d (Esperado: %d = SIN_PERMISO)\n",
           rSinPermiso, SIN_PERMISO);

    int rModOk = tweetModificarConIndice(&feedVerificacion, &indice, 9, "Respuesta nivel 2 editada con palabra clave relampago", "beto");
    printf("Beto modifica su respuesta (ID 9): %d (Esperado: 0 = TODO_OK)\n", rModOk);
    printf("> Buscando palabra nueva 'relampago' en el indice:\n");
    tweetBuscarYMostrarConIndice(&indice, "relampago");

    printf("\n=== 15. Sprint 3 - Caso 6: Eliminacion en cascada de un sub-hilo y limpieza del indice ===\n");
    // Si Ana borra su respuesta ID 8 (Nivel 1), deben desaparecer tambien el ID 9 (Nivel 2) y el ID 10 (Nivel 3),
    // pero debe seguir existiendo el ID 11 (la otra respuesta de Nivel 1 al tweet 3).
    int rBorradoHilo = tweetEliminarConIndice(&feedVerificacion, &indice, 8, "ana");
    printf("Ana elimina su respuesta ID 8 (con hijos 9 y 10): %d (Esperado: 0 = TODO_OK)\n", rBorradoHilo);
    printf("Respuestas que quedaron en Tweet #3: %d (Esperado: 1 -> solo ID 11)\n", t3->hilo.cant);
    printf("Buscar ID 8: %s | Buscar ID 9: %s | Buscar ID 10: %s | Buscar ID 11: %s\n",
           tweetBuscarPorId(&feedVerificacion, 8) ? "EXISTE" : "NULL",
           tweetBuscarPorId(&feedVerificacion, 9) ? "EXISTE" : "NULL",
           tweetBuscarPorId(&feedVerificacion, 10) ? "EXISTE" : "NULL",
           tweetBuscarPorId(&feedVerificacion, 11) ? "EXISTE" : "NULL");
    printf("> Buscando 'profunda' (del nieto eliminado ID 10) en el indice:\n");
    tweetBuscarYMostrarConIndice(&indice, "profunda");

    printf("\n=== 16. Sprint 3 - Caso 7: Persistencia de hilos en tweets.bin y recarga completa ===\n");
    // Agregamos una respuesta al ID 11 (Nivel 2) antes de guardar para persistir 2 niveles activos
    tweetPublicarConIndice(&feedVerificacion, &indice, &beto, "Respuesta persistente en segundo nivel", 11); // ID 12 (padre 11)
    tweetsGuardar(&feedVerificacion);
    indiceVaciar(&indice);
    tweetVaciarHilo(&feedVerificacion);
    printf("Guardado y vaciado en RAM. Feed cant: %d, Arbol cant: %d\n",
           feedVerificacion.cant, indice.arbol.cant);

    tLista feedRecargado;
    tIndice indiceRecargado;
    listaInicializar(&feedRecargado);
    indiceInicializar(&indiceRecargado);
    tweetsAbrirConIndice(&feedRecargado, &indiceRecargado);

    Tweet *t3Rec = tweetBuscarPorId(&feedRecargado, 3);
    Tweet *t11Rec = tweetBuscarPorId(&feedRecargado, 11);
    Tweet *t12Rec = tweetBuscarPorId(&feedRecargado, 12);
    printf("Recargado desde archivo -> Tweet #3 respuestas: %d (Esperado: 1 -> ID 11)\n",
           t3Rec ? t3Rec->hilo.cant : -1);
    printf("Recargado desde archivo -> Respuesta #11 idPadre=%u, respuestas=%d (Esperado: padre 3, 1 hijo -> ID 12)\n",
           t11Rec ? t11Rec->idPadre : 0, t11Rec ? t11Rec->hilo.cant : -1);
    printf("Recargado desde archivo -> Sub-respuesta #12 idPadre=%u, mensaje='%s'\n",
           t12Rec ? t12Rec->idPadre : 0, t12Rec ? t12Rec->mensaje : "NULL");
    printf("> Buscando 'persistente' en el indice reconstruido:\n");
    tweetBuscarYMostrarConIndice(&indiceRecargado, "persistente");

    indiceVaciar(&indiceRecargado);
    tweetVaciarHilo(&feedRecargado);
    remove(ARCH_TWEETS);

    famecheckFinalizar();
    return 0;
}
