#include "lista.h"
#include "user.h"
#include "tweet.h"
#include "famecheck.h"

// Nombres fijos: FameCheck decide quien se verifica y despues siempre devuelve lo mismo
#define USER_VERIFICADO    "pepito123"
#define USER_NO_VERIFICADO "messi"

void cargarTop(tLista *top, char *nombre, int cantidad)
{
    ActividadFame a;
    memset(&a, 0, sizeof(ActividadFame));
    strncpy(a.nombre, nombre, sizeof(a.nombre) - 1);
    a.cantidadCweets = cantidad;
    famecheckInsertarEnTop(top, &a);
}

int main()
{
    int verificado;
    int reportado;
    tLista top;
    tLista feed;
    tLista usuarios;
    User autor;

    printf("=== 1. Inicio de FameCheck (lee apikey.key) ===\n");
    if(famecheckIniciar() != FAMECHECK_OK){
        printf("No se pudo iniciar FameCheck, revisar apikey.key\n");
        return 1;
    }
    printf("FameCheck iniciado\n");

    printf("\n=== 2. Cuenta que se verifica y cuenta que no ===\n");
    verificado = -1;
    printf("Verificar '%s': %d", USER_VERIFICADO, famecheckVerificarCuenta(USER_VERIFICADO, &verificado));
    printf(" -> verificado = %d (esperado 1)\n", verificado);
    verificado = -1;
    printf("Verificar '%s': %d", USER_NO_VERIFICADO, famecheckVerificarCuenta(USER_NO_VERIFICADO, &verificado));
    printf(" -> verificado = %d (esperado 0)\n", verificado);

    printf("\n=== 3. Caso propio: verificar dos veces la misma cuenta ===\n");
    verificado = -1;
    famecheckVerificarCuenta(USER_VERIFICADO, &verificado);
    printf("'%s' de nuevo -> verificado = %d (esperado 1, sin duplicar la cuenta)\n", USER_VERIFICADO, verificado);

    printf("\n=== 4. Reportar publicacion de un verificado ===\n");
    printf("Resultado: %d", famecheckReportarPublicacion(USER_VERIFICADO, "tweet de prueba", &reportado));
    printf(" -> reportado = %d (esperado 0 y reportado 1)\n", reportado);

    printf("\n=== 5. Caso propio: reportar publicacion de un no verificado ===\n");
    printf("Resultado: %d", famecheckReportarPublicacion(USER_NO_VERIFICADO, "tweet de prueba", &reportado));
    printf(" -> reportado = %d (esperado %d y reportado 0)\n", reportado, FAMECHECK_ERROR_RESPUESTA);

    printf("\n=== 6. Reportar publicacion sin conexion a FameCheck ===\n");
    listaInicializar(&feed);
    memset(&autor, 0, sizeof(User));
    strcpy(autor.user, USER_VERIFICADO);
    autor.verficado = 1;
    putenv("https_proxy=http://127.0.0.1:9"); // proxy inexistente: simula que no hay conexion
    printf("tweetPublicar: %d (esperado 0, el tweet se publica igual)\n", tweetPublicar(&feed, &autor, "tweet sin conexion"));
    putenv("https_proxy=");
    printf("Tweets en el feed: %d (esperado 1)\n", feed.cant);
    printf("Revisar logs.log: tiene que haber una linea con FAMECHECK_ERROR_RED\n");
    listaVaciar(&feed);

    printf("\n=== 7. Top 5 con menos de 5 verificados ===\n");
    listaInicializar(&top);
    cargarTop(&top, "ana", 2);
    cargarTop(&top, "beto", 7);
    cargarTop(&top, "carla", 4);
    printf("Esperado: beto 7, carla 4, ana 2\n");
    listaRecorrerYAccion(&top, famecheckMostrarUsuarioTop);
    listaVaciar(&top);

    printf("\n=== 8. Top 5 con empates ===\n");
    listaInicializar(&top);
    cargarTop(&top, "dario", 3);
    cargarTop(&top, "beto", 5);
    cargarTop(&top, "ana", 5);
    cargarTop(&top, "eva", 1);
    cargarTop(&top, "carla", 3);
    cargarTop(&top, "fede", 3);
    cargarTop(&top, "gaby", 9);
    printf("Esperado: gaby 9, ana 5, beto 5, carla 3, dario 3 (fede y eva quedan afuera)\n");
    listaRecorrerYAccion(&top, famecheckMostrarUsuarioTop);
    listaVaciar(&top);

    printf("\n=== 9. Top 5 real traido de FameCheck ===\n");
    listaInicializar(&top);
    printf("Resultado: %d (esperado 0)\n", famecheckObtenerTop5(&top));
    listaRecorrerYAccion(&top, famecheckMostrarUsuarioTop);
    listaVaciar(&top);

    printf("\n=== 10. La verificacion no se pierde al reiniciar ===\n");
    listaInicializar(&usuarios);
    userRegistrar(&usuarios, USER_VERIFICADO, "1234");
    ((User *)usuarios.inicio->data)->verficado = 1;
    usersGuardar(&usuarios);
    listaVaciar(&usuarios);
    printf("Guardado y lista vaciada (simulando cierre)\n");
    usersAbrir(&usuarios);
    printf("'%s' al recargar -> verificado = %d (esperado 1)\n", USER_VERIFICADO,
           ((User *)usuarios.inicio->data)->verficado);
    listaVaciar(&usuarios);

    famecheckFinalizar();
    return 0;
}
