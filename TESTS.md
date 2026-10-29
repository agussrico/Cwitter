# Hoja de Tests â€“ Cwitter

**Trabajo PrÃ¡ctico AyED â€“ Sprint 1 y Sprint 2**

Este documento reÃºne los casos de prueba definidos para verificar el funcionamiento del sistema Cwitter. Para cada caso se indica la situaciÃ³n que se busca probar, la operaciÃ³n concreta que se ejecuta, el resultado esperado segÃºn la consigna y el diseÃ±o acordado, y la salida real obtenida al ejecutar los programas de prueba.

Los casos marcados como "caso propio" corresponden a los casos adicionales pensados por el equipo mÃ¡s allÃ¡ de los casos obligatorios del enunciado.

## CÃ³mo se ejecutan

Cada `test_*.c` tiene su propio `main`, asÃ­ que se compila por separado de `main.c`. Los tests que usan tweets o FameCheck necesitan curl y cJSON (ver README) y el archivo `apikey.key` en la carpeta desde donde se ejecutan.

```
gcc -Wall -Iinclude tests/test_users.c src/lista.c src/user.c -o test_users
gcc -Wall -Iinclude -IC:/libs/curl/include tests/test_tweets.c src/lista.c src/user.c src/tweet.c src/famecheck.c src/cJSON.c -LC:/libs/curl/lib -lcurl -o test_tweets
gcc -Wall -Iinclude -IC:/libs/curl/include tests/test_famecheck.c src/lista.c src/user.c src/tweet.c src/famecheck.c src/cJSON.c -LC:/libs/curl/lib -lcurl -o test_famecheck
```

Las salidas de esta hoja se obtuvieron ejecutando los tres tests en una carpeta vacÃ­a, en el orden `test_users`, `test_tweets`, `test_famecheck`.

---

# Sprint 1

## Usuarios (`test_users.c`)

| NÂ° | SituaciÃ³n a probar | OperaciÃ³n realizada | Resultado esperado | Salida obtenida |
|----|--------------------|---------------------|--------------------|-----------------|
| 1 | Plataforma vacÃ­a: no existe archivo de usuarios previo | `usersAbrir(&listaUsuarios)` al arrancar el programa por primera vez | Devuelve `ERROR_AP_ARCH` (no hay archivo previo); la lista queda inicializada con `cant = 0` | `No existe archivo previo (plataforma vacia) o no se pudo abrir.` âœ” |
| 2 | Registro de un usuario nuevo con nombre disponible | `userRegistrar(&lista, "ana", "1234")` | Devuelve cÃ³digo de Ã©xito; "ana" queda agregada a la lista | `Registrar ana: 0` âœ” |
| 3 | Registro de un segundo usuario nuevo | `userRegistrar(&lista, "beto", "abcd")` | Devuelve cÃ³digo de Ã©xito; "beto" queda agregado, "ana" sigue en la lista | `Registrar beto: 0`; la lista muestra `ana` y `beto` âœ” |
| 4 | Intento de registrar un usuario con un nombre ya existente | `userRegistrar(&lista, "ana", "otraPass")` | Devuelve cÃ³digo de error (usuario duplicado); no se agrega ni se pisa la cuenta original de "ana" | `Registrar ana de nuevo: 6` (`USER_DUPLICADO`); la lista sigue con 2 usuarios âœ” |
| 5 | Inicio de sesiÃ³n con usuario y contraseÃ±a correctos | `userIniciarSesion(&lista, "ana", "1234")` | Devuelve puntero vÃ¡lido al usuario "ana"; login exitoso | `Login OK para 'ana'` âœ” |
| 6 | Inicio de sesiÃ³n con usuario correcto pero contraseÃ±a incorrecta (**caso propio**) | `userIniciarSesion(&lista, "ana", "passIncorrecta")` | Devuelve `NULL`; login rechazado | `Login rechazado correctamente: password incorrecta` âœ” |
| 7 | Inicio de sesiÃ³n con un usuario inexistente (**caso propio**) | `userIniciarSesion(&lista, "noexiste", "1234")` | Devuelve `NULL`; login rechazado | `Login rechazado correctamente: usuario inexistente` âœ” |
| 8 | Persistencia: guardar usuarios en archivo | `usersGuardar(&lista)` | Devuelve `TODO_OK`; el archivo queda escrito con los usuarios registrados | `Usuarios guardados correctamente.` âœ” |
| 9 | Cierre simulado del programa | `listaVaciar(&lista)` | La lista en memoria queda en `cant = 0`; el archivo en disco no se ve afectado | `Lista vaciada en memoria (simulando cierre del programa).` âœ” |
| 10 | Reapertura del programa y verificaciÃ³n de persistencia | `usersAbrir(&listaVerificacion)` sobre una lista nueva | Se recuperan todos los usuarios guardados previamente (mismo `cant` y mismos datos que antes del cierre) | `Usuarios recuperados del archivo: 2` (`ana`, `beto`) âœ” |

## Tweets (`test_tweets.c`)

En el Sprint 2 `tweetPublicar` pasÃ³ a recibir el `User` completo y el feed pasÃ³ a estar ordenado (verificados primero, despuÃ©s del mÃ¡s reciente al mÃ¡s viejo). Por eso el test se adaptÃ³: se suma un usuario verificado (`pepito123`) y los casos de eliminaciÃ³n se ajustaron al nuevo orden. Se publican 6 tweets: ana (ID 1), beto (2), pepito123 (3), ana (4), carlos (5), pepito123 (6). El feed queda `6 3 5 4 2 1`.

| NÂº | SituaciÃ³n a probar | OperaciÃ³n realizada | Resultado esperado | Salida obtenida |
|----|--------------------|---------------------|--------------------|-----------------|
| 1 | Plataforma vacÃ­a: no existe archivo de tweets previo | `tweetsAbrir(&feed)` al arrancar el programa por primera vez | Devuelve `ERROR_AP_ARCH` (no hay archivo previo); la lista queda inicializada con `cant = 0` | `Plataforma vacia: No hay tweets previos o archivo nuevo.` âœ” (en el Sprint 2 se corrigiÃ³ `tweetsAbrir`, que devolvÃ­a `TODO_OK` en este caso) |
| 2 | PublicaciÃ³n de varios tweets de distintos usuarios | `tweetPublicar` Ã—6: ana, beto, pepito123, ana, carlos, pepito123 | Cada llamada devuelve `TODO_OK`; se asignan los IDs 1 a 6 en orden y `cant = 6` | Se publican los 6 tweets con IDs 1 a 6; los 2 de `pepito123` se reportan a FameCheck (`[!] Tweet reportado correctamente [!]`) âœ” |
| 3 | Consulta del feed: muestra tweets de todos los usuarios, no solo los propios | `tweetsMostrarFeed(&feed)` con los 6 tweets cargados | Se muestran los 6 tweets con su autor y texto | Se muestran los 6 tweets de los 4 autores âœ” |
| 4 | BÃºsqueda de un tweet que existe | `tweetBuscarYMostrar(&feed, "permisos")` | Muestra Ãºnicamente el tweet ID 2 de beto ("Segundo tweet para probar permisos") | `[ID: 2] @beto dijo: Segundo tweet para probar permisos` âœ” |
| 5 | BÃºsqueda de un tweet que no existe | `tweetBuscarYMostrar(&feed, "elefante")` | No muestra ningÃºn tweet e informa que no hubo coincidencias; el programa no falla y la lista no se modifica | `No se encontraron tweets que contengan esa palabra.` âœ” |
| 6 | EliminaciÃ³n del primer tweet de la lista | `tweetEliminar(&feed, 6, "pepito123")` | Devuelve `TODO_OK`; el nuevo primer tweet es el ID 3 | `Borrando PRIMER tweet del feed (ID 6, pepito123): 0` âœ” |
| 7 | EliminaciÃ³n del Ãºltimo tweet de la lista | `tweetEliminar(&feed, 1, "ana")` | Devuelve `TODO_OK`; el nuevo Ãºltimo tweet es el ID 2 | `Borrando ULTIMO tweet del feed (ID 1, ana): 0` âœ” |
| 8 | EliminaciÃ³n de un tweet intermedio | `tweetEliminar(&feed, 4, "ana")` | Devuelve `TODO_OK`; los tweets vecinos (IDs 5 y 2) quedan enlazados correctamente | `Borrando tweet INTERMEDIO (ID 4, ana): 0` âœ” |
| 9 | Estado del feed tras las tres eliminaciones | `tweetsMostrarFeed(&feed)` | Se muestran solo los tweets ID 3 (pepito123), ID 5 (carlos) e ID 2 (beto), en ese orden | Feed: ID 3, ID 5, ID 2 âœ” |
| 10 | Un usuario intenta eliminar el tweet de otro (**caso propio**) | `tweetEliminar(&feed, 2, "carlos")` (el tweet 2 es de beto) | Devuelve `SIN_PERMISO`; el tweet ID 2 sigue en el feed y `cant` no cambia | `BLOQUEADO (Correcto: Sin permiso)` âœ” |
| 11 | El autor modifica su propio tweet (**caso propio**) | `tweetModificar(&feed, 2, "Beto edito este texto con exito", "beto")` | Devuelve `TODO_OK`; el tweet ID 2 muestra el nuevo texto y conserva su ID y autor | `MODIFICADO (Correcto)`; el feed muestra `[ID: 2] @beto dijo: Beto edito este texto con exito` âœ” |
| 12 | Persistencia: guardado y cierre del programa | `tweetsGuardar(&feed)` seguido de `listaVaciar(&feed)` | El archivo se escribe sin errores; tras vaciar, `feed.cant = 0` y la memoria queda liberada | `Memoria vaciada (simulando cierre). Tweets actuales en RAM: 0` âœ” |
| 13 | Persistencia: reapertura del programa recupera todo | `tweetsAbrir(&feedVerificacion)` sobre una lista nueva, y luego `tweetsMostrarFeed` | Se recuperan los IDs 3, 5 y 2 en ese orden, con el texto modificado del tweet 2 y sin perder autor | `Obtenido: 3 5 2`, con el texto modificado del tweet 2 âœ” |

---

# Sprint 2

## Feed y recorrido (`test_tweets.c`)

| NÂº | SituaciÃ³n a probar | OperaciÃ³n realizada | Resultado esperado | Salida obtenida |
|----|--------------------|---------------------|--------------------|-----------------|
| 14 | Feed con una mezcla de verificados y no verificados | Publicar los 6 tweets del caso 2 y recorrer el feed | Primero los tweets de verificados y despuÃ©s el resto, cada grupo del mÃ¡s reciente al mÃ¡s viejo: `6 3 5 4 2 1` | `Obtenido: 6 3 5 4 2 1` âœ” |
| 15 | Un tweet no verificado mÃ¡s viejo que todos queda al final (**caso propio**) | `listaInsertarOrdenado` de un tweet de beto (ID 99) con fecha de hace una hora | Queda Ãºltimo, detrÃ¡s de todos los no verificados: `6 3 5 4 2 1 99` | `Obtenido: 6 3 5 4 2 1 99` âœ” |
| 16 | Recorrer el feed hacia adelante y volver varias veces seguidas | Desde el inicio: siguiente, siguiente, anterior, anterior, siguiente (punteros `sig` y `ant` de la lista doble, igual que las teclas D y A del menÃº) | Pasa por los IDs `6, 3, 5, 3, 6, 3` sin recorrer el feed desde el principio | `Inicio: 6`, `Siguiente: 3`, `Siguiente: 5`, `Anterior: 3`, `Anterior: 6`, `Siguiente: 3` âœ” |
| 17 | El orden del feed se conserva al reiniciar | `tweetsGuardar`, `listaVaciar` y `tweetsAbrir` | El feed recargado mantiene el orden `3 5 2` | `Obtenido: 3 5 2` âœ” |

## FameCheck (`test_famecheck.c`)

FameCheck decide quiÃ©n queda verificado y, una vez dada de alta una cuenta, siempre devuelve el mismo resultado. Se usan `pepito123` (verificada) y `messi` (no verificada).

| NÂº | SituaciÃ³n a probar | OperaciÃ³n realizada | Resultado esperado | Salida obtenida |
|----|--------------------|---------------------|--------------------|-----------------|
| 1 | Inicio del servicio | `famecheckIniciar()` con `apikey.key` presente | Devuelve `FAMECHECK_OK` | `FameCheck iniciado` âœ” |
| 2 | Una cuenta que se verifica y otra que no | `famecheckVerificarCuenta("pepito123", &v)` y `famecheckVerificarCuenta("messi", &v)` | Ambas devuelven `FAMECHECK_OK`; `pepito123` queda con `verificado = 1` y `messi` con `verificado = 0` | `Verificar 'pepito123': 0 -> verificado = 1`, `Verificar 'messi': 0 -> verificado = 0` âœ” |
| 3 | Verificar dos veces la misma cuenta (**caso propio**) | `famecheckVerificarCuenta("pepito123", &v)` otra vez | FameCheck responde 200 con el mismo resultado, sin duplicar la cuenta | `'pepito123' de nuevo -> verificado = 1` âœ” |
| 4 | Reportar la publicaciÃ³n de un verificado | `famecheckReportarPublicacion("pepito123", "tweet de prueba", &r)` | Devuelve `FAMECHECK_OK` y `reportado = 1` | `Resultado: 0 -> reportado = 1` âœ” |
| 5 | Reportar la publicaciÃ³n de un no verificado (**caso propio**) | `famecheckReportarPublicacion("messi", "tweet de prueba", &r)` | FameCheck lo rechaza (400): devuelve `FAMECHECK_ERROR_RESPUESTA` (-2) y `reportado = 0` | `Resultado: -2 -> reportado = 0` âœ” |
| 6 | Reportar una publicaciÃ³n sin conexiÃ³n al servicio externo | `tweetPublicar` de un verificado con un proxy inexistente (`https_proxy=http://127.0.0.1:9`) | El tweet se publica igual (`TODO_OK`, queda en el feed) y el fallo queda registrado en `logs.log` | `tweetPublicar: 0`, `Tweets en el feed: 1`; `logs.log`: `[10/10/2026 20:45:11] Error al reportar tweet 1 de @pepito123: FAMECHECK_ERROR_RED` âœ” |
| 7 | Top 5 con menos de 5 usuarios verificados | `famecheckInsertarEnTop` con ana 2, beto 7, carla 4 | Se muestran los 3, de mayor a menor: beto 7, carla 4, ana 2 | `beto 7`, `carla 4`, `ana 2` âœ” |
| 8 | Top 5 con empates | `famecheckInsertarEnTop` con dario 3, beto 5, ana 5, eva 1, carla 3, fede 3, gaby 9 | Quedan 5, de mayor a menor y los empatados por orden alfabÃ©tico: gaby 9, ana 5, beto 5, carla 3, dario 3 (fede y eva quedan afuera) | `gaby 9`, `ana 5`, `beto 5`, `carla 3`, `dario 3` âœ” |
| 9 | Top 5 real con los datos de FameCheck | `famecheckObtenerTop5(&top)` | Devuelve `FAMECHECK_OK` y como mÃ¡ximo 5 cuentas, de mayor a menor | `Resultado: 0`; `pepito123 7`, `prueba_claude 3`, `Franco 0`, `taylorswift 0`, `zz_comun 0` âœ” |
| 10 | Reiniciar el programa y confirmar que la verificaciÃ³n de un usuario no se pierde | Registrar `pepito123` con `verificado = 1`, `usersGuardar`, `listaVaciar` y `usersAbrir` | Al recargar, `pepito123` sigue con `verificado = 1` | `'pepito123' al recargar -> verificado = 1` âœ” |

## Prueba manual con el programa completo

| NÂº | SituaciÃ³n a probar | OperaciÃ³n realizada | Resultado esperado | Salida obtenida |
|----|--------------------|---------------------|--------------------|-----------------|
| 1 | Reiniciar el programa y confirmar que la verificaciÃ³n no se pierde | Registrarse, iniciar sesiÃ³n, verificar con la opciÃ³n `0`, publicar, salir con la opciÃ³n 3, volver a abrir Cwitter e iniciar sesiÃ³n | Al volver a entrar, el usuario sigue verificado y los tweets siguen en el feed | `Usuario VERIFICADO!` y `Tweet 1 de 2` al volver a iniciar sesiÃ³n âœ” |
