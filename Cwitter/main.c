#include <stdio.h>
#include <stdlib.h>
#include "lista.h"
#include "user.h"

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
            printf("Bienvenido a CTwitter:\n1.Iniciar Sesion\n2.Registrarse");
            scanf("%d", &opcion);
            if(opcion == 1){
                printf("Ingrese su usuario: ");
                scanf("%s", user);
                printf("Ingrese su psw: ");
                scanf("%s", pass);
                pUsuarioActual = userIniciarSesion(&listaUsuarios, user, pass);
                if(pUsuarioActual == NULL){
                    printf("Error al iniciar sesion\n");
                }
            }
            else if(opcion == 2){
                printf("Registro de nuevo usuario: \n");
                printf("Ingrese su usuario: ");
                scanf("%s", user);
                printf("Ingrese su psw: ");
                scanf("%s", pass);
                userRegistrar(&listaUsuarios, user, pass);
                ///faltaria control de errores usuario duplicado/no se pudo cargar.
            }
            else{
                printf("Opcion no valida");
            }
        }
        printf("Sesion iniciada\n");
        break; ///aca iria el menu principal de twitter con el usuario iniciado
    }
    ///guardo la lista de usuarios en el archivo para persistencia
    usersGuardar(&listaUsuarios);
    ///vacio la lista
    listaVaciar(&listaUsuarios);
    return 0;
}
