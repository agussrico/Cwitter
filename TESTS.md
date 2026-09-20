# Hoja de Tests – Cwitter

**Trabajo Práctico AyED – Sprint 1**

Este documento reúne los casos de prueba definidos para verificar el funcionamiento del sistema Cwitter durante el Sprint 1. Para cada caso se indica la situación que se busca probar, la operación concreta que se ejecuta y el resultado esperado según la consigna y el diseño acordado. A partir del Sprint 2, en la columna "Salida obtenida" se debe registrar la salida real obtenida para los casos definidos en el Sprint 1.

Los casos marcados como "caso propio" corresponden a los al menos 2 casos adicionales pensados por el equipo más allá de los casos obligatorios del enunciado.

## Usuarios

| N° | Situación a probar | Operación realizada | Resultado esperado | Salida obtenida |
|----|--------------------|---------------------|--------------------|-----------------|
| 1 | Plataforma vacía: no existe archivo de usuarios previo | `usersAbrir(&listaUsuarios)` al arrancar el programa por primera vez | Devuelve `ERROR_AP_ARCH` (no hay archivo previo); la lista queda inicializada con `cant = 0` | (se completa a partir del Sprint 2) |
| 2 | Registro de un usuario nuevo con nombre disponible | `userRegistrar(&lista, "ana", "1234")` | Devuelve código de éxito; "ana" queda agregada a la lista | (se completa a partir del Sprint 2) |
| 3 | Registro de un segundo usuario nuevo | `userRegistrar(&lista, "beto", "abcd")` | Devuelve código de éxito; "beto" queda agregado, "ana" sigue en la lista | (se completa a partir del Sprint 2) |
| 4 | Intento de registrar un usuario con un nombre ya existente | `userRegistrar(&lista, "ana", "otraPass")` | Devuelve código de error (usuario duplicado); no se agrega ni se pisa la cuenta original de "ana" | (se completa a partir del Sprint 2) |
| 5 | Inicio de sesión con usuario y contraseña correctos | `userIniciarSesion(&lista, "ana", "1234")` | Devuelve puntero válido al usuario "ana"; login exitoso | (se completa a partir del Sprint 2) |
| 6 | Inicio de sesión con usuario correcto pero contraseña incorrecta (**caso propio**) | `userIniciarSesion(&lista, "ana", "passIncorrecta")` | Devuelve `NULL`; login rechazado | (se completa a partir del Sprint 2) |
| 7 | Inicio de sesión con un usuario inexistente (**caso propio**) | `userIniciarSesion(&lista, "noexiste", "1234")` | Devuelve `NULL`; login rechazado | (se completa a partir del Sprint 2) |
| 8 | Persistencia: guardar usuarios en archivo | `usersGuardar(&lista)` | Devuelve `TODO_OK`; el archivo queda escrito con los usuarios registrados | (se completa a partir del Sprint 2) |
| 9 | Cierre simulado del programa | `listaVaciar(&lista)` | La lista en memoria queda en `cant = 0`; el archivo en disco no se ve afectado | (se completa a partir del Sprint 2) |
| 10 | Reapertura del programa y verificación de persistencia | `usersAbrir(&listaVerificacion)` sobre una lista nueva | Se recuperan todos los usuarios guardados previamente (mismo `cant` y mismos datos que antes del cierre) | (se completa a partir del Sprint 2) |

## Tweets

| Nº | Situación a probar | Operación realizada | Resultado esperado | Salida obtenida |
|----|--------------------|---------------------|--------------------|-----------------|
| 1 | Plataforma vacía: no existe archivo de tweets previo | `tweetsAbrir(&feed)` al arrancar el programa por primera vez | Devuelve `ERROR_AP_ARCH` (no hay archivo previo); la lista queda inicializada con `cant = 0` | (se completa a partir del Sprint 2) |
| 2 | Publicación de varios tweets de distintos usuarios | `tweetPublicar` ×5: ana, beto, ana, carlos, beto | Cada llamada devuelve `TODO_OK`; se asignan los IDs 1 a 5 en orden y `cant = 5` | (se completa a partir del Sprint 2) |
| 3 | Consulta del feed: muestra tweets de todos los usuarios, no solo los propios | `tweetsMostrarFeed(&feed)` con los 5 tweets cargados | Se muestran los 5 tweets (IDs 1 a 5) con su autor y texto | (se completa a partir del Sprint 2) |
| 4 | Búsqueda de un tweet que existe | `tweetBuscarYMostrar(&feed, "permisos")` | Muestra únicamente el tweet ID 2 de beto ("Segundo tweet para probar permisos") | (se completa a partir del Sprint 2) |
| 5 | Búsqueda de un tweet que no existe | `tweetBuscarYMostrar(&feed, "elefante")` | No muestra ningún tweet e informa que no hubo coincidencias; el programa no falla y la lista no se modifica | (se completa a partir del Sprint 2) |
| 6 | Eliminación del primer tweet de la lista | `tweetEliminar(&feed, 1, "ana")` | Devuelve `TODO_OK`; `cant = 4`; el nuevo primer tweet es el ID 2 | (se completa a partir del Sprint 2) |
| 7 | Eliminación del último tweet de la lista | `tweetEliminar(&feed, 5, "beto")` | Devuelve `TODO_OK`; `cant = 3`; el nuevo último tweet es el ID 4 | (se completa a partir del Sprint 2) |
| 8 | Eliminación de un tweet intermedio | `tweetEliminar(&feed, 3, "ana")` | Devuelve `TODO_OK`; `cant = 2`; los tweets vecinos (IDs 2 y 4) quedan enlazados correctamente | (se completa a partir del Sprint 2) |
| 9 | Estado del feed tras las tres eliminaciones | `tweetsMostrarFeed(&feed)` | Se muestran solo los tweets ID 2 (beto) e ID 4 (carlos), en ese orden | (se completa a partir del Sprint 2) |
| 10 | Un usuario intenta eliminar el tweet de otro (**caso propio**) | `tweetEliminar(&feed, 2, "carlos")` (el tweet 2 es de beto) | Devuelve `SIN_PERMISO`; el tweet ID 2 sigue en el feed y `cant` no cambia | (se completa a partir del Sprint 2) |
| 11 | El autor modifica su propio tweet (**caso propio**) | `tweetModificar(&feed, 2, "Beto edito este texto con exito", "beto")` | Devuelve `TODO_OK`; el tweet ID 2 muestra el nuevo texto y conserva su ID y autor | (se completa a partir del Sprint 2) |
| 12 | Persistencia: guardado y cierre del programa | `tweetsGuardar(&feed)` seguido de `listaVaciar(&feed)` | El archivo se escribe sin errores; tras vaciar, `feed.cant = 0` y la memoria queda liberada | (se completa a partir del Sprint 2) |
| 13 | Persistencia: reapertura del programa recupera todo | `tweetsAbrir(&feedVerificacion)` sobre una lista nueva, y luego `tweetsMostrarFeed` | Devuelve `TODO_OK`; se recuperan los IDs 2 y 4, con el texto modificado del tweet 2 y sin perder autor ni orden | (se completa a partir del Sprint 2) |
