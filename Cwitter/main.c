#include <stdio.h>
#include <stdlib.h>
#include "lista.h"
#include "user.h"

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
            else{
                printf("\n  [!] Opcion no valida\n");
                printf("  Presione ENTER para continuar...");
                getchar(); getchar();
            }
        }
        system("cls");
        mostrarBanner();
        printf("\n  >> Sesion iniciada. Bienvenido, %s! <<\n\n", pUsuarioActual->user);
        break; ///aca iria el menu principal de twitter con el usuario iniciado
    }
    ///guardo la lista de usuarios en el archivo para persistencia
    usersGuardar(&listaUsuarios);
    ///vacio la lista
    listaVaciar(&listaUsuarios);
    return 0;
}
