#include "lista.h"
#include "user.h"

int main()
{
    tLista listaUsuarios;
    listaInicializar(&listaUsuarios);

    printf("=== Cargando usuarios desde archivo ===\n");
    int carga = usersAbrir(&listaUsuarios);
    if(carga == ERROR_AP_ARCH)
        printf("No existe archivo previo (plataforma vacia) o no se pudo abrir.\n");
    else
        printf("Usuarios cargados: %d\n", listaUsuarios.cant);

    printf("\n=== Registrando usuarios ===\n");
    printf("Registrar ana: %d\n", userRegistrar(&listaUsuarios, "ana", "1234"));
    printf("Registrar beto: %d\n", userRegistrar(&listaUsuarios, "beto", "abcd"));
    printf("Registrar ana de nuevo: %d\n", userRegistrar(&listaUsuarios, "ana", "otraPass"));

    printf("\nUsuarios actuales:\n");
    listaRecorrerYAccion(&listaUsuarios, userImprimir);

    printf("\n=== Pruebas de inicio de sesion ===\n");

    User *sesion = userIniciarSesion(&listaUsuarios, "ana", "1234");
    if(sesion)
        printf("Login OK para '%s'\n", sesion->user);
    else
        printf("Login fallido para 'ana' (no deberia pasar)\n");

    sesion = userIniciarSesion(&listaUsuarios, "ana", "passIncorrecta");
    if(!sesion)
        printf("Login rechazado correctamente: password incorrecta\n");
    else
        printf("ERROR: dejo entrar con password incorrecta\n");

    sesion = userIniciarSesion(&listaUsuarios, "noexiste", "1234");
    if(!sesion)
        printf("Login rechazado correctamente: usuario inexistente\n");
    else
        printf("ERROR: dejo entrar a un usuario que no existe\n");

    printf("\n=== Guardando usuarios en archivo ===\n");
    int guardar = usersGuardar(&listaUsuarios);
    if(guardar == TODO_OK)
        printf("Usuarios guardados correctamente.\n");
    else
        printf("Error al guardar usuarios.\n");

    listaVaciar(&listaUsuarios);
    printf("\nLista vaciada en memoria (simulando cierre del programa).\n");

    printf("\n=== Volviendo a cargar para confirmar persistencia ===\n");
    tLista listaVerificacion;
    listaInicializar(&listaVerificacion);
    usersAbrir(&listaVerificacion);
    printf("Usuarios recuperados del archivo: %d\n", listaVerificacion.cant);
    listaRecorrerYAccion(&listaVerificacion, userImprimir);

    listaVaciar(&listaVerificacion);

    return 0;
}