#include <stdio.h>
#include <stdlib.h>
#include "lista.h"
#include "user.h"
#include "tweet.h"
#include "famecheck.h"

#define RESET     "\033[0m"
#define NEGRITA   "\033[1m"
#define INVERSO   "\033[7m"
#define ROJO      "\033[31m"
#define VERDE     "\033[32m"
#define AMARILLO  "\033[33m"
#define AZUL      "\033[94m"
#define CIAN      "\033[36m"

#define ANCHO_CAJA      41
#define ANCHO_PANTALLA  76

int usarLinux = 0;

void leerConfig()
{
    char linea[50];
    FILE *f = fopen("config.ini", "r");
    if(f == NULL) return;
    fgets(linea, sizeof(linea), f);
    if(strstr(linea, "linux=true")) usarLinux = 1;
    fclose(f);
}

void limpiarPantalla()
{
    if(usarLinux) system("clear");
    else system("cls");
}

void color(const char *codigo)
{
    if(usarLinux) printf("%s", codigo);
}

void mostrarMensaje(const char *codigoColor, const char *texto)
{
    color(codigoColor);
    printf("%s", texto);
    color(RESET);
}

void mostrarPrompt(const char *texto)
{
    color(VERDE);
    printf("%s", texto);
    color(RESET);
}

void dibujarSeparador(char caracter, int ancho)
{
    int i;

    color(CIAN);
    printf("  ");
    for(i = 0; i < ancho; i++) printf("%c", caracter);
    printf("\n");
    color(RESET);
}

void dibujarLinea()
{
    int i;

    color(CIAN);
    printf("  +");
    for(i = 0; i < ANCHO_CAJA; i++) printf("-");
    printf("+\n");
    color(RESET);
}

void dibujarTitulo(const char *titulo)
{
    int largo = strlen(titulo);
    int izquierda = (ANCHO_CAJA - largo) / 2;

    dibujarLinea();
    color(CIAN);
    printf("  |");
    color(RESET);
    color(NEGRITA);
    printf("%*s%s%*s", izquierda, "", titulo, ANCHO_CAJA - largo - izquierda, "");
    color(RESET);
    color(CIAN);
    printf("|\n");
    color(RESET);
    dibujarLinea();
}

void mostrarFilaVacia()
{
    color(CIAN);
    printf("  |");
    color(RESET);
    printf("%*s", ANCHO_CAJA, "");
    color(CIAN);
    printf("|\n");
    color(RESET);
}

void mostrarFilaMenu(const char *tecla, const char *texto)
{
    color(CIAN);
    printf("  |");
    color(RESET);
    printf("   ");
    color(NEGRITA);
    color(AMARILLO);
    printf("[%s]", tecla);
    color(RESET);
    printf("  %-*s", ANCHO_CAJA - 8, texto);
    color(CIAN);
    printf("|\n");
    color(RESET);
}

void mostrarBanner()
{
    color(AZUL);
    printf("=====================================================\n");
    color(RESET);
    color(NEGRITA);
    color(CIAN);
    printf("   ____ _____                           \n");
    printf("  / ___|_   _|_      _ (_)|_| |_ ___ _ __\n");
    printf(" | |     | | \\ \\ /\\ / /| | __| __/ _ \\ '__|\n");
    printf(" | |___  | |  \\ V  V / | | |_| ||  __/ |\n");
    printf("  \\____| |_|   \\_/\\_/  |_|\\__|\\__\\___|_|\n");
    color(RESET);
    color(AZUL);
    printf("=====================================================\n");
    color(RESET);
}

void mostrarMenuPrincipal()
{
    printf("\n");
    dibujarTitulo("BIENVENIDO A CTWITTER");
    mostrarFilaVacia();
    mostrarFilaMenu("1", "Iniciar Sesion");
    mostrarFilaMenu("2", "Registrarse");
    mostrarFilaMenu("3", "Salir de Cwitter(guardar)"); // para que termine el prgrama y guarde
    mostrarFilaVacia();
    dibujarLinea();
    mostrarPrompt("  >> Elegi una opcion: ");
}

void mostrarEncabezado(const char *titulo)
{
    printf("\n");
    dibujarTitulo(titulo);
    printf("\n");
}

void mostrarOpcion(const char *tecla, const char *texto)
{
    printf("  ");
    if(usarLinux){
        color(CIAN);
        color(INVERSO);
        printf(" %s ", tecla);
        color(RESET);
    }
    else printf("[%s]", tecla);
    printf(" %-20s", texto);
}

void mostrarBarraOpciones()
{
    printf("\n");
    dibujarSeparador('-', ANCHO_PANTALLA);
    mostrarOpcion("A", "Tweet Anterior");
    mostrarOpcion("D", "Tweet Siguiente");
    mostrarOpcion("1", "Ver Feed");
    printf("\n");
    mostrarOpcion("2", "Publicar un Tweet");
    mostrarOpcion("3", "Buscar un Tweet");
    mostrarOpcion("4", "Modificar un Tweet");
    printf("\n");
    mostrarOpcion("5", "Eliminar un Tweet");
    mostrarOpcion("6", "Top Verificados");
    mostrarOpcion("7", "Cerrar Sesion");
    printf("\n");
    dibujarSeparador('-', ANCHO_PANTALLA);
}

void mostrarCentrado(const char *texto)
{
    int largo = strlen(texto);

    printf("  %*s%s\n", (ANCHO_PANTALLA - largo) / 2, "", texto);
}

void mostrarTweetActual(tNodo *nodo, int posicion, int total)
{
    char indicador[64];

    printf("\n");
    dibujarSeparador('=', ANCHO_PANTALLA);
    if(nodo == NULL)
    {
        color(AMARILLO);
        mostrarCentrado("Todavia no hay tweets para mostrar");
        color(RESET);
        dibujarSeparador('=', ANCHO_PANTALLA);
        printf("\n");
        return;
    }
    sprintf(indicador, "%c  Tweet %d de %d  %c", (nodo->ant != NULL) ? '<' : ' ', posicion, total, (nodo->sig != NULL) ? '>' : ' ');
    color(NEGRITA);
    color(CIAN);
    mostrarCentrado(indicador);
    color(RESET);
    dibujarSeparador('=', ANCHO_PANTALLA);
    tweetImprimir(nodo->data);
    printf("\n");
}

int main()
{
    User *pUsuarioActual = NULL;

    leerConfig();

    ///tweets
    tLista listaTweets;
    tNodo *nodoTweet = NULL;
    int posTweet = 0;
    listaInicializar(&listaTweets);
    tweetsAbrir(&listaTweets);

    ///usuarios
    int opcion;
    char user[20], pass[20];
    tLista listaUsuarios;
    listaInicializar(&listaUsuarios);
    usersAbrir(&listaUsuarios);

    ///Lista de usuarios con mayor interaccion
    tLista listaFamecheckTop;

    ///logica de inicio de sesion
    while(1){
        ///bucle menu
        while(pUsuarioActual == NULL){
            limpiarPantalla();
            mostrarBanner();
            mostrarMenuPrincipal();
            scanf("%d", &opcion);
            if(opcion == 1){
                limpiarPantalla();
                mostrarBanner();
                mostrarEncabezado("INICIAR SESION");
                mostrarPrompt("  Usuario: ");
                scanf("%19s", user);
                mostrarPrompt("  Password: ");
                scanf("%19s", pass);
                pUsuarioActual = userIniciarSesion(&listaUsuarios, user, pass);
                if(pUsuarioActual == NULL){
                    mostrarMensaje(ROJO, "\n  [!] Error al iniciar sesion\n");
                    printf("  Presione ENTER para continuar...");
                    getchar(); getchar();
                }
            }
            else if(opcion == 2){
                limpiarPantalla();
                mostrarBanner();
                mostrarEncabezado("REGISTRO DE NUEVO USUARIO");
                mostrarPrompt("  Usuario: ");
                scanf("%19s", user);
                mostrarPrompt("  Password: ");
                scanf("%19s", pass);
                userRegistrar(&listaUsuarios, user, pass);
                ///faltaria control de errores usuario duplicado/no se pudo cargar.
                printf("\n  Presione ENTER para continuar...");
                getchar(); getchar();
            }
            else if(opcion == 3){
                mostrarMensaje(AMARILLO, "\n  [!] Cerrando Cwitter y guardando datos...\n");
                usersGuardar(&listaUsuarios);
                tweetsGuardar(&listaTweets);
                listaVaciar(&listaUsuarios);
                listaVaciar(&listaTweets);

                return 0;
            }
            else{
                mostrarMensaje(ROJO, "\n  [!] Opcion no valida\n");
                printf("  Presione ENTER para continuar...");
                getchar(); getchar();
            }
        }
        limpiarPantalla();
        mostrarMensaje(VERDE, "\n  >> Sesion iniciada. Bienvenido, ");
        printf("%s", pUsuarioActual->user);
        mostrarMensaje(VERDE, "! <<\n\n");
        nodoTweet = listaTweets.inicio;
        posTweet = (nodoTweet != NULL) ? 1 : 0;

        //break; ///aca iria el menu principal de twitter con el usuario iniciado
        while(pUsuarioActual != NULL) {
            limpiarPantalla();
            color(NEGRITA);
            color(CIAN);
            printf("\n  [ Usuario: @%s ]\n\n", pUsuarioActual->user);
            color(RESET);
            if(pUsuarioActual->verficado == 1){
                mostrarMensaje(VERDE, "Usuario VERIFICADO!\n");
            }
            else{
                mostrarMensaje(AMARILLO, "!!! Usuario no verificado !!!. Ingrese 0 para verificar\n");
            }
            mostrarTweetActual(nodoTweet, posTweet, listaTweets.cant);
            mostrarBarraOpciones();
            mostrarPrompt("  >> Elegi una opcion: ");

            char opcMenu;
            scanf(" %c", &opcMenu);

            // Limpiamos el buffer de entrada (CRÍTICO antes de leer strings con fgets)
            while ((getchar()) != '\n');

            char bufferMensaje[300]; // Buffer temporal grande
            unsigned int idTemp;
            int resultado;

            switch(opcMenu) {
                case 'a':
                case 'A':
                    if(nodoTweet != NULL && nodoTweet->ant != NULL){
                        nodoTweet = nodoTweet->ant;
                        posTweet--;
                    }
                    continue;

                case 'd':
                case 'D':
                    if(nodoTweet != NULL && nodoTweet->sig != NULL){
                        nodoTweet = nodoTweet->sig;
                        posTweet++;
                    }
                    continue;

                case '1':
                    limpiarPantalla();
                    tweetsMostrarFeed(&listaTweets);
                    break;

                case '2':
                    printf("\n  Escribi tu tweet (max %d caracteres):\n  > ", MAX_TWEET);
                    fgets(bufferMensaje, sizeof(bufferMensaje), stdin);
                    bufferMensaje[strcspn(bufferMensaje, "\n")] = 0; // Saca el enter final

                    tweetPublicar(&listaTweets, pUsuarioActual, bufferMensaje);
                    mostrarMensaje(VERDE, "\n  [!] Tweet publicado con exito.\n");
                    if(nodoTweet == NULL){
                        nodoTweet = listaTweets.inicio;
                        posTweet = (nodoTweet != NULL) ? 1 : 0;
                    }

                    break;

                case '3':
                    printf("\n  Ingresa la palabra a buscar: ");
                    fgets(bufferMensaje, sizeof(bufferMensaje), stdin);
                    bufferMensaje[strcspn(bufferMensaje, "\n")] = 0;

                    tweetBuscarYMostrar(&listaTweets, bufferMensaje);
                    break;

                case '4':
                    printf("\n  Ingresa el ID del tweet a modificar: ");
                    scanf("%u", &idTemp);
                    while ((getchar()) != '\n'); // Limpiar buffer

                    printf("  Escribi el nuevo mensaje:\n  > ");
                    fgets(bufferMensaje, sizeof(bufferMensaje), stdin);
                    bufferMensaje[strcspn(bufferMensaje, "\n")] = 0;

                    resultado = tweetModificar(&listaTweets, idTemp, bufferMensaje, pUsuarioActual->user);

                    if (resultado == TODO_OK)
                        mostrarMensaje(VERDE, "\n  [!] Tweet modificado.\n");
                    else if (resultado == SIN_PERMISO)
                        mostrarMensaje(ROJO, "\n  [!] Ese tweet no es tuyo, no podes modificarlo.\n");
                    else
                        mostrarMensaje(ROJO, "\n  [!] No se encontro el tweet.\n");
                    break;

                case '5':
                    printf("\n  Ingresa el ID del tweet a eliminar: ");
                    scanf("%u", &idTemp);
                    while ((getchar()) != '\n');

                    resultado = tweetEliminar(&listaTweets, idTemp, pUsuarioActual->user);

                    if (resultado == TODO_OK)
                        mostrarMensaje(VERDE, "\n  [!] Tweet eliminado.\n");
                    else if (resultado == SIN_PERMISO)
                        mostrarMensaje(ROJO, "\n  [!] Ese tweet no es tuyo, no podes eliminarlo.\n");
                    else
                        mostrarMensaje(ROJO, "\n  [!] No se encontro el tweet.\n");
                    if (resultado == TODO_OK){
                        nodoTweet = listaTweets.inicio;
                        posTweet = (nodoTweet != NULL) ? 1 : 0;
                    }
                    break;

                 case '6':
                    mostrarMensaje(CIAN, "Usuarios verificados con mayor cantidad de interacciones\n");
                    famecheckIniciar();
                    listaInicializar(&listaFamecheckTop);
                    famecheckObtenerTop5(&listaFamecheckTop);
                    listaRecorrerYAccion(&listaFamecheckTop, famecheckMostrarUsuarioTop);
                    listaVaciar(&listaFamecheckTop);
                    famecheckFinalizar();
                    break;

                case '7':
                    pUsuarioActual = NULL;
                    mostrarMensaje(AMARILLO, "\n  [!] Cerrando sesion...\n");
                    break;

                case '0':
                    if(famecheckIniciar() != FAMECHECK_OK){
                        mostrarMensaje(ROJO, "Error al leer la apikey necesaria para activar Famecheck\n");
                    }
                    if(famecheckVerificarCuenta(pUsuarioActual->user, &pUsuarioActual->verficado) != FAMECHECK_OK){
                        mostrarMensaje(ROJO, "Famecheck Error: Error de contacto con la API\n");
                    }
                    else if(pUsuarioActual->verficado == 1){
                        mostrarMensaje(VERDE, "Famecheck Activdo: Usuario Verificado Correctamente\n");
                    }
                    else{
                        mostrarMensaje(AMARILLO, "Famecheck No Activado: Usuario no Verificado\n");
                    }
                    famecheckFinalizar();
                    break;

                default:
                    mostrarMensaje(ROJO, "\n  [!] Opcion invalida.\n");
            }

            if (pUsuarioActual != NULL) {
                printf("\n  Presione ENTER para continuar...");
                getchar();
            }
        }
    }

    ///guardo la lista de usuarios en el archivo para persistencia
    usersGuardar(&listaUsuarios);
    tweetsGuardar(&listaTweets); // Persistencia del feed
    ///vacio la lista
    listaVaciar(&listaUsuarios);
    listaVaciar(&listaTweets);
    return 0;
}