#include <stdio.h>
#include <stdlib.h>
#include "lista.h"
#include "user.h"
#include "tweet.h"

void mostrarBanner()
{
    printf("=====================================================\n");
    printf("   ____ _____                           \n");
    printf("  / ___|_   _|_      _ (_)|_| |_ ___ _ __\n");
    printf(" | |     | | \\ \\ /\\ / /| | __| __/ _ \\ '__|\n");
    printf(" | |___  | |  \\ V  V / | | |_| ||  __/ |\n");
    printf("  \\____| |_|   \\_/\\_/  |_|\\__|\\__\\___|_|\n");
    printf("=====================================================\n");
}

void mostrarMenuPrincipal()
{
    printf("\n");
    printf("  +-----------------------------------------+\n");
    printf("  |            BIENVENIDO A CTWITTER         |\n");
    printf("  +-----------------------------------------+\n");
    printf("  |  1. Iniciar Sesion                       |\n");
    printf("  |  2. Registrarse                          |\n");
    printf("  |  3. Salir de Cwitter(guardar)            |\n"); // para que termine el prgrama y guarde
    printf("  +-----------------------------------------+\n");
    printf("  >> Elegi una opcion: ");
}

void mostrarEncabezado(const char *titulo)
{
    int len = strlen(titulo);
    printf("\n  +");
    for(int i = 0; i < len + 2; i++) printf("-");
    printf("+\n");
    printf("  | %s |\n", titulo);
    printf("  +");
    for(int i = 0; i < len + 2; i++) printf("-");
    printf("+\n\n");
}

int main()
{
    User *pUsuarioActual = NULL;

    ///tweets
    tLista listaTweets;
    listaInicializar(&listaTweets);
    tweetsAbrir(&listaTweets);

    ///usuarios
    int opcion;
    char user[20], pass[20];
    tLista listaUsuarios;
    listaInicializar(&listaUsuarios);
    usersAbrir(&listaUsuarios);

    ///logica de inicio de sesion
    while(1){
        ///bucle menu
        while(pUsuarioActual == NULL){
            system("cls");
            mostrarBanner();
            mostrarMenuPrincipal();
            scanf("%d", &opcion);
            if(opcion == 1){
                system("cls");
                mostrarBanner();
                mostrarEncabezado("INICIAR SESION");
                printf("  Usuario: ");
                scanf("%19s", user);
                printf("  Password: ");
                scanf("%19s", pass);
                pUsuarioActual = userIniciarSesion(&listaUsuarios, user, pass);
                if(pUsuarioActual == NULL){
                    printf("\n  [!] Error al iniciar sesion\n");
                    printf("  Presione ENTER para continuar...");
                    getchar(); getchar();
                }
            }
            else if(opcion == 2){
                system("cls");
                mostrarBanner();
                mostrarEncabezado("REGISTRO DE NUEVO USUARIO");
                printf("  Usuario: ");
                scanf("%19s", user);
                printf("  Password: ");
                scanf("%19s", pass);
                userRegistrar(&listaUsuarios, user, pass);
                ///faltaria control de errores usuario duplicado/no se pudo cargar.
                printf("\n  Presione ENTER para continuar...");
                getchar(); getchar();
            }
            else if(opcion == 3){
                printf("\n  [!] Cerrando Cwitter y guardando datos...\n");
                usersGuardar(&listaUsuarios);
                tweetsGuardar(&listaTweets);
                listaVaciar(&listaUsuarios);
                listaVaciar(&listaTweets);

                return 0;
            }
            else{
                printf("\n  [!] Opcion no valida\n");
                printf("  Presione ENTER para continuar...");
                getchar(); getchar();
            }
        }
        system("cls");
        mostrarBanner();
        printf("\n  >> Sesion iniciada. Bienvenido, %s! <<\n\n", pUsuarioActual->user);
        //break; ///aca iria el menu principal de twitter con el usuario iniciado
        while(pUsuarioActual != NULL) {
            system("cls");
            mostrarBanner();
            printf("\n  [ Usuario: @%s ]\n\n", pUsuarioActual->user);
            printf("  1. Ver Feed\n");
            printf("  2. Publicar un Tweet\n");
            printf("  3. Buscar un Tweet\n");
            printf("  4. Modificar un Tweet\n");
            printf("  5. Eliminar un Tweet\n");
            printf("  6. Cerrar Sesion\n");
            printf("  >> Elegi una opcion: ");

            int opcMenu;
            scanf("%d", &opcMenu);

            // Limpiamos el buffer de entrada (CRÍTICO antes de leer strings con fgets)
            while ((getchar()) != '\n');

            char bufferMensaje[300]; // Buffer temporal grande
            unsigned int idTemp;

            switch(opcMenu) {
                case 1:
                    system("cls");
                    tweetsMostrarFeed(&listaTweets);
                    break;

                case 2:
                    printf("\n  Escribi tu tweet (max %d caracteres):\n  > ", MAX_TWEET);
                    fgets(bufferMensaje, sizeof(bufferMensaje), stdin);
                    bufferMensaje[strcspn(bufferMensaje, "\n")] = 0; // Saca el enter final

                    tweetPublicar(&listaTweets, pUsuarioActual->user, bufferMensaje);
                    printf("\n  [!] Tweet publicado con exito.\n");
                    break;

                case 3:
                    printf("\n  Ingresa la palabra a buscar: ");
                    fgets(bufferMensaje, sizeof(bufferMensaje), stdin);
                    bufferMensaje[strcspn(bufferMensaje, "\n")] = 0;

                    tweetBuscarYMostrar(&listaTweets, bufferMensaje);
                    break;

                case 4:
                    printf("\n  Ingresa el ID del tweet a modificar: ");
                    scanf("%u", &idTemp);
                    while ((getchar()) != '\n'); // Limpiar buffer

                    printf("  Escribi el nuevo mensaje:\n  > ");
                    fgets(bufferMensaje, sizeof(bufferMensaje), stdin);
                    bufferMensaje[strcspn(bufferMensaje, "\n")] = 0;

                    if (tweetModificar(&listaTweets, idTemp, bufferMensaje) == TODO_OK) {
                        printf("\n  [!] Tweet modificado.\n");
                    } else {
                        printf("\n  [!] No se encontro el tweet.\n");
                    }
                    break;

                case 5:
                    printf("\n  Ingresa el ID del tweet a eliminar: ");
                    scanf("%u", &idTemp);
                    while ((getchar()) != '\n');

                    if (tweetEliminar(&listaTweets, idTemp) == TODO_OK) {
                        printf("\n  [!] Tweet eliminado.\n");
                    } else {
                        printf("\n  [!] No se encontro el tweet.\n");
                    }
                    break;

                case 6: // Inicio y cierre de sesión
                    pUsuarioActual = NULL;
                    printf("\n  [!] Cerrando sesion...\n");
                    break;

                default:
                    printf("\n  [!] Opcion invalida.\n");
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
