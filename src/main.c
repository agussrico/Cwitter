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
int modoTest = 0; // muestra la cantidad de comparaciones de cada busqueda

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
    mostrarOpcion("R", "Responder Tweet");
    printf("\n");
    mostrarOpcion("1", "Ver Feed");
    mostrarOpcion("2", "Publicar Tweet");
    mostrarOpcion("3", "Buscar un Tweet");
    printf("\n");
    mostrarOpcion("V", "Ver Hilo Actual");
    mostrarOpcion("4", "Modificar Tweet");
    mostrarOpcion("5", "Eliminar Tweet");
    printf("\n");
    mostrarOpcion("6", "Top Verificados");
    mostrarOpcion("7", "Cerrar Sesion");
    printf("\n");
    mostrarOpcion("T", modoTest ? "Modo Test: ON" : "Modo Test: OFF");
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

void tweetImprimirHiloRecursivo(tLista *hilo, int nivel)
{
    tNodo *actual = hilo->inicio;
    while (actual != NULL)
    {
        Tweet *t = (Tweet *)actual->data;

        for(int i = 0; i < nivel; i++)
        {
            printf("    ");
        }
        printf("|-- ");

        struct tm *tm_info = localtime(&t->fecha);
        char fechaStr[20];
        strftime(fechaStr, sizeof(fechaStr), "%d/%m/%Y %H:%M:%S", tm_info);

        printf("[ID: %u] [%s] @%s", t->id, fechaStr, t->autor);
        if (t->verificado == 1) printf(" [V]");
        printf(" respondio:\n");

        for(int i = 0; i < nivel; i++)
        {
            printf("    ");
        }
        printf("    %s\n", t->mensaje);

        if (!listaVacia(&t->hilo))
        {
            tweetImprimirHiloRecursivo(&t->hilo, nivel + 1);
        }

        actual = actual->sig;
    }
}

int main()
{
    User *pUsuarioActual = NULL;

    leerConfig();

    ///famecheck: curl se inicia una sola vez para todo el programa
    if(famecheckIniciar() != FAMECHECK_OK){
        mostrarMensaje(ROJO, "  [!] Error al iniciar Famecheck (revisar apikey.key)\n");
        printf("  Presione ENTER para continuar...");
        getchar();
    }

    ///tweets
    tLista listaTweets;
    tNodo *nodoTweet = NULL;
    int posTweet = 0;
    tIndice indiceTweets; ///indice de busqueda por palabra
    listaInicializar(&listaTweets);
    indiceInicializar(&indiceTweets);
    tweetsAbrirConIndice(&listaTweets, &indiceTweets);

    ///usuarios
    int opcion;
    int resultadoRegistro;
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
                resultadoRegistro = userRegistrar(&listaUsuarios, user, pass);
                if(resultadoRegistro == TODO_OK)
                    mostrarMensaje(VERDE, "\n  [!] Usuario registrado con exito.\n");
                else if(resultadoRegistro == USER_DUPLICADO)
                    mostrarMensaje(ROJO, "\n  [!] Ese usuario ya existe, elegi otro nombre.\n");
                else
                    mostrarMensaje(ROJO, "\n  [!] No se pudo registrar el usuario (sin memoria).\n");
                printf("\n  Presione ENTER para continuar...");
                getchar(); getchar();
            }
            else if(opcion == 3){
                mostrarMensaje(AMARILLO, "\n  [!] Cerrando Cwitter y guardando datos...\n");
                usersGuardar(&listaUsuarios);
                tweetsGuardar(&listaTweets);
                listaVaciar(&listaUsuarios);
                indiceVaciar(&indiceTweets);
                listaVaciar(&listaTweets);
                famecheckFinalizar();

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

            char bufferMensaje[350]; // Buffer temporal grande
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

                    tweetPublicarConIndice(&listaTweets, &indiceTweets, pUsuarioActual, bufferMensaje, 0); // 0 = Sin padre
                    mostrarMensaje(VERDE, "\n  [!] Tweet publicado con exito.\n");
                    nodoTweet = listaTweets.inicio;
                    posTweet = (nodoTweet != NULL) ? 1 : 0;
                    break;

                case 'r':
                case 'R': // Responde a tweet o respuesta por su ID
                    unsigned int idResponder;
                    printf("\n  Ingresa el ID del tweet (o respuesta) al que queres responder: ");
                    scanf("%u", &idResponder);
                    while ((getchar()) != '\n');

                    Tweet *tweetDestino = tweetBuscarPorId(&listaTweets, idResponder);
                    if (tweetDestino == NULL)
                    {
                        mostrarMensaje(ROJO, "\n  [!] No se encontro un tweet con ese ID.\n");
                        break;
                    }

                    printf("  Escribi tu respuesta (max %d caracteres):\n  > ", MAX_TWEET);
                    fgets(bufferMensaje, sizeof(bufferMensaje), stdin);
                    bufferMensaje[strcspn(bufferMensaje, "\n")] = 0;

                    if (tweetPublicarConIndice(&listaTweets, &indiceTweets, pUsuarioActual, bufferMensaje, tweetDestino->id) == TODO_OK)
                        mostrarMensaje(VERDE, "\n  [!] Respuesta publicada con exito en el hilo.\n");
                    else
                        mostrarMensaje(ROJO, "\n  [!] No se pudo publicar la respuesta.\n");
                    break;

                case 'v':
                case 'V': // Ver hilo completo
                    if (nodoTweet == NULL)
                    {
                        mostrarMensaje(ROJO, "\n  [!] No hay tweets para mostrar.\n");
                        break;
                    }
                    limpiarPantalla();
                    Tweet *tSel = (Tweet *)nodoTweet->data;
                    printf("\n========= HILO DE CONVERSACION (ID: %u) =========\n", tSel->id);
                    tweetImprimir(tSel);
                    if (!listaVacia(&tSel->hilo))
                    {
                        printf("\n  --- Respuestas ---\n");
                        tweetImprimirHiloRecursivo(&tSel->hilo, 1); // Imprime las respuestas con sangria
                    } else {
                        printf("\n  (Este tweet todavia no tiene respuestas)\n");
                    }
                    printf("===================================================\n");

                    /// Opcion de responder directamente
                    printf("\n  Queres responder a este tweet? (S/N): ");
                    char respuestaOpc;
                    scanf(" %c", &respuestaOpc);
                    while ((getchar()) != '\n');
                    if (respuestaOpc == 's' || respuestaOpc == 'S')
                    {
                        unsigned int idDestino = tSel->id;

                        printf("\n  (Si queres responder a una respuesta interna, ingresa su ID. Si queres responder al principal, presiona 0): ");
                        unsigned int idIngresado;
                        scanf("%u", &idIngresado);
                        while ((getchar()) != '\n');

                        if (idIngresado != 0)
                        {
                            idDestino = idIngresado;
                        }

                        printf("  Escribi tu respuesta (max %d caracteres):\n  > ", MAX_TWEET);
                        fgets(bufferMensaje, sizeof(bufferMensaje), stdin);
                        bufferMensaje[strcspn(bufferMensaje, "\n")] = 0;

                        if (tweetPublicarConIndice(&listaTweets, &indiceTweets, pUsuarioActual, bufferMensaje, idDestino) == TODO_OK)
                            mostrarMensaje(VERDE, "\n  [!] Respuesta publicada con exito en el hilo.\n");
                        else
                            mostrarMensaje(ROJO, "\n  [!] No se encontro un tweet con ese ID.\n");
                    }
                    break;

                case '3':
                    printf("\n  Ingresa la palabra a buscar: ");
                    fgets(bufferMensaje, sizeof(bufferMensaje), stdin);
                    bufferMensaje[strcspn(bufferMensaje, "\n")] = 0;

                    tweetBuscarYMostrarConIndice(&indiceTweets, bufferMensaje);

                    if (modoTest) {
                        printf("\n  [TEST] Comparaciones con indice: %lu (palabras en el arbol: %d, altura: %d)\n",
                               indiceComparacionesUltimaBusqueda(), indiceTweets.arbol.cant, arbolAltura(&indiceTweets.arbol));
                        printf("  [TEST] Comparaciones sin indice: %d (un strstr por cada tweet y respuesta)\n", indiceTweets.cantRegistros);
                    }
                    break;

                case '4':
                    printf("\n  Ingresa el ID del tweet a modificar: ");
                    scanf("%u", &idTemp);
                    while ((getchar()) != '\n'); // Limpiar buffer

                    printf("  Escribi el nuevo mensaje:\n  > ");
                    fgets(bufferMensaje, sizeof(bufferMensaje), stdin);
                    bufferMensaje[strcspn(bufferMensaje, "\n")] = 0;

                    resultado = tweetModificarConIndice(&listaTweets, &indiceTweets, idTemp, bufferMensaje, pUsuarioActual->user);

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

                    resultado = tweetEliminarConIndice(&listaTweets, &indiceTweets, idTemp, pUsuarioActual->user);

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
                    listaInicializar(&listaFamecheckTop);
                    if(famecheckObtenerTop5(&listaFamecheckTop) != FAMECHECK_OK)
                        mostrarMensaje(ROJO, "Famecheck Error: Error de contacto con la API\n");
                    else if(listaVacia(&listaFamecheckTop))
                        mostrarMensaje(AMARILLO, "No hay usuarios verificados todavia\n");
                    else
                        listaRecorrerYAccion(&listaFamecheckTop, famecheckMostrarUsuarioTop);
                    listaVaciar(&listaFamecheckTop);
                    break;

                case 't':
                case 'T':
                    modoTest = !modoTest;
                    mostrarMensaje(AMARILLO, modoTest ? "\n  [!] Modo test activado.\n" : "\n  [!] Modo test desactivado.\n");
                    break;

                case '7':
                    pUsuarioActual = NULL;
                    mostrarMensaje(AMARILLO, "\n  [!] Cerrando sesion...\n");
                    break;

                case '0':
                    if(famecheckVerificarCuenta(pUsuarioActual->user, &pUsuarioActual->verficado) != FAMECHECK_OK){
                        mostrarMensaje(ROJO, "Famecheck Error: Error de contacto con la API\n");
                    }
                    else if(pUsuarioActual->verficado == 1){
                        mostrarMensaje(VERDE, "Famecheck Activdo: Usuario Verificado Correctamente\n");
                    }
                    else{
                        mostrarMensaje(AMARILLO, "Famecheck No Activado: Usuario no Verificado\n");
                    }
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
    indiceVaciar(&indiceTweets);
    listaVaciar(&listaTweets);
    famecheckFinalizar();
    return 0;
}
