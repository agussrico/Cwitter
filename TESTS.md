# Hoja de Tests – Cwitter

**Trabajo Práctico AyED – Sprint 1 y Sprint 2**

Este documento reúne los casos de prueba definidos para verificar el funcionamiento del sistema Cwitter. Para cada caso se indica la situación que se busca probar, la operación concreta que se ejecuta, el resultado esperado y la salida real obtenida.

Los casos marcados como "caso propio" corresponden a las pruebas adicionales pensadas por el equipo más allá de los casos obligatorios del enunciado.

## Usuarios (Sprint 1)

| N° | Situación a probar | Operación realizada | Resultado esperado | Salida obtenida |
|----|--------------------|---------------------|--------------------|-----------------|
| 1 | Plataforma vacía: no existe archivo previo | `usersAbrir(&listaUsuarios)` al arrancar por primera vez | Devuelve `ERROR_AP_ARCH`; la lista queda inicializada con `cant = 0` | `ERROR_AP_ARCH`, lista.cant = 0 |
| 2 | Registro de usuario nuevo con nombre disponible | `userRegistrar(&lista, "ana", "1234")` | Devuelve código de éxito; "ana" se agrega a la lista | `TODO_OK`, usuario agregado |
| 3 | Registro de un segundo usuario nuevo | `userRegistrar(&lista, "beto", "abcd")` | Devuelve código de éxito; "beto" se agrega a la lista | `TODO_OK`, usuario agregado |
| 4 | Intento de registrar un usuario ya existente | `userRegistrar(&lista, "ana", "otraPass")` | Devuelve `USER_DUPLICADO`; no pisa la cuenta original | `USER_DUPLICADO`, cuenta intacta |
| 5 | Inicio de sesión correcto | `userIniciarSesion(&lista, "ana", "1234")` | Devuelve puntero válido al usuario "ana" | Puntero válido a "ana" |
| 6 | Inicio de sesión con password incorrecta (**caso propio**) | `userIniciarSesion(&lista, "ana", "passIncorrecta")` | Devuelve `NULL`; login rechazado | `NULL`, login rechazado |
| 7 | Inicio de sesión con usuario inexistente (**caso propio**) | `userIniciarSesion(&lista, "noexiste", "1234")` | Devuelve `NULL`; login rechazado | `NULL`, login rechazado |
| 8 | Persistencia: guardar usuarios en archivo | `usersGuardar(&lista)` | Devuelve `TODO_OK`; se crea/sobrescribe el archivo | `TODO_OK` |
| 9 | Cierre simulado del programa | `listaVaciar(&lista)` | La lista en memoria queda con `cant = 0` | lista.cant = 0 |
| 10 | Reapertura y verificación de persistencia | `usersAbrir(&listaVerificacion)` sobre una lista nueva | Se recuperan los usuarios guardados | `TODO_OK`, lista.cant = 2 |

## Tweets (Sprint 1)

| Nº | Situación a probar | Operación realizada | Resultado esperado | Salida obtenida |
|----|--------------------|---------------------|--------------------|-----------------|
| 1 | Plataforma vacía: no existe archivo previo | `tweetsAbrir(&feed)` al arrancar por primera vez | Devuelve `TODO_OK` (no lee nada); `cant = 0` | `TODO_OK`, feed.cant = 0 |
| 2 | Publicación de varios tweets | `tweetPublicar` ×5: ana, beto, ana, carlos, beto | Cada llamada devuelve `TODO_OK`; `cant = 5` | `TODO_OK`, IDs incrementales |
| 3 | Consulta del feed global | `tweetsMostrarFeed(&feed)` | Se muestran los 5 tweets | Muestra los 5 tweets impresos |
| 4 | Búsqueda de un tweet que existe | `tweetBuscarYMostrar(&feed, "permisos")` | Muestra el tweet de beto | Muestra 1 resultado (tweet de beto) |
| 5 | Búsqueda de un tweet que no existe | `tweetBuscarYMostrar(&feed, "elefante")` | Indica que no hubo coincidencias | "No se encontraron tweets..." |
| 6 | Eliminación del primer tweet de la lista | `tweetEliminar(&feed, 1, "ana")` | Devuelve `TODO_OK`; `cant = 4` | `TODO_OK` |
| 7 | Eliminación del último tweet de la lista | `tweetEliminar(&feed, 5, "beto")` | Devuelve `TODO_OK`; `cant = 3` | `TODO_OK` |
| 8 | Eliminación de un tweet intermedio | `tweetEliminar(&feed, 3, "ana")` | Devuelve `TODO_OK`; `cant = 2` | `TODO_OK` |
| 9 | Estado del feed tras eliminaciones | `tweetsMostrarFeed(&feed)` | Se muestran solo los tweets 2 y 4 | Muestra solo IDs 2 y 4 |
| 10 | Un usuario intenta eliminar el tweet de otro (**caso propio**) | `tweetEliminar(&feed, 2, "carlos")` | Devuelve `SIN_PERMISO` | `SIN_PERMISO` |
| 11 | El autor modifica su propio tweet (**caso propio**) | `tweetModificar(&feed, 2, "texto", "beto")` | Devuelve `TODO_OK`; se actualiza el texto | `TODO_OK`, texto modificado |
| 12 | Persistencia: guardado y cierre | `tweetsGuardar(&feed)` y `listaVaciar(&feed)` | `TODO_OK` y memoria liberada | `TODO_OK`, feed.cant = 0 |
| 13 | Persistencia: reapertura recupera todo | `tweetsAbrir(&feedVerificacion)` | Se recuperan los IDs restantes y sus textos | Muestra IDs 2 y 4 modificados |

---

## Integración FameCheck (Sprint 2)

| Nº | Situación a probar | Operación realizada | Resultado esperado | Salida obtenida |
|----|--------------------|---------------------|--------------------|-----------------|
| 1 | Cuenta que se verifica y cuenta que no | `famecheckVerificarCuenta` con "pepito123" y "messi" | "pepito123" devuelve `1`, "messi" devuelve `0` | 1 y 0 respectivamente |
| 2 | Verificar dos veces la misma cuenta (**caso propio**) | `famecheckVerificarCuenta` con "pepito123" otra vez | Devuelve 1; no duplica cuenta | 1, sin duplicar cuenta |
| 3 | Reportar publicación de un verificado | `famecheckReportarPublicacion` con "pepito123" | Devuelve `FAMECHECK_OK`, `reportado = 1` | `FAMECHECK_OK`, reportado = 1 |
| 4 | Reportar publicación de un NO verificado (**caso propio**) | `famecheckReportarPublicacion` con "messi" | Devuelve `FAMECHECK_ERROR_RESPUESTA`, `reportado = 0` | `FAMECHECK_ERROR_RESPUESTA`, reportado = 0 |
| 5 | Reporte sin conexión a FameCheck | Modificar proxy a `http://127.0.0.1:9` y publicar tweet | Falla reporte (Error Red) pero tweet se publica igual | Tweet publicado, Log de error registrado |
| 6 | Top 5 con menos de 5 verificados | `famecheckObtenerTop5` simulado con 3 usuarios | Retorna lista ordenada (beto 7, carla 4, ana 2) | beto 7, carla 4, ana 2 |
| 7 | Top 5 con empates y descartes | Inserción de 7 usuarios con puntajes iguales | Ordena por cant desc, luego alfabeto; descarta el 6to y 7mo | gaby 9, ana 5, beto 5, carla 3, dario 3 |
| 8 | Obtener Top 5 real de la API | `famecheckObtenerTop5(&top)` | Devuelve `TODO_OK` y muestra los resultados de la API | `TODO_OK`, lista de 5 usuarios |
| 9 | Persistencia de estado de verificación | `usersGuardar` usuario verificado, vaciar y recargar | El flag `verficado` se mantiene en 1 al reabrir | verificado = 1 tras carga |

## Feed y Navegación (Sprint 2)

| Nº | Situación a probar | Operación realizada | Resultado esperado | Salida obtenida |
|----|--------------------|---------------------|--------------------|-----------------|
| 1 | Inserción ordenada de feed mixto | Publicar tweets de verificados y no verificados mezclados | Verificados primero; luego orden por fecha/ID | 6 3 5 4 2 1 |
| 2 | Tweet viejo no verificado va al final (**caso propio**) | Insertar tweet ID 99 con fecha restada en 3600 segs | El tweet viejo se posiciona en la cola del feed mixto | 6 3 5 4 2 1 99 |
| 3 | Navegación hacia adelante y atrás | Mover puntero con `.sig` y `.ant` desde el inicio | Secuencia de IDs: 6, 3, 5, 3, 6, 3 | 6, 3, 5, 3, 6, 3 |
