# Trabajo Práctico AyED – Cwitter – Sprint 2

**Universidad Nacional de La Matanza**
Departamento de Ingeniería e Investigaciones Tecnológicas
Centro Regional Universitario de Ituzaingó

## Bitácora de diseño

Algoritmo y estructura de datos – 3640 – Comisión 90-3900

**Grupo:** Puntero

### Integrantes

- Agustina Rico - 46028153 (@agussrico)
- Franco Lago - 45129873 (@LagooFran)
- Mateo Feiertag - 46293138 (@mateofeiertag)
- Francisco Garbaccio - 46021880 (@Garbaaa)

**Repositorio:** https://github.com/agussrico/Cwitter

**Entregas:**

- Checkpoint Sprint 1 – 21/09/2026 (tag `v1-fundamentos`)
- Sprint 2 – (completar fecha) (tag `v2-procesamiento`)

---

# Descripción del funcionamiento del programa

## 1. Resumen

Cwitter es un programa de consola en C que permite registrar usuarios, iniciar y cerrar sesión, publicar, modificar y eliminar tweets, ver el feed general y buscar tweets por texto. Los usuarios y los tweets se guardan en archivos binarios, así que se conservan entre ejecuciones.

En el Sprint 2 se sumó la integración con **FameCheck**, un servicio externo que decide si una cuenta queda verificada. Cwitter le consulta la verificación de un usuario, le reporta cada publicación de una cuenta verificada y le pide la actividad para armar el **top 5 de verificados con más publicaciones**. Además, el **feed muestra primero los tweets de cuentas verificadas** y después el resto, del más reciente al más viejo, y se puede **recorrer de a un tweet por vez**, hacia adelante y hacia atrás. Si FameCheck no responde al reportar, la publicación se hace igual y el fallo queda registrado en un **archivo de log**.

Los datos se mantienen en memoria en listas doblemente enlazadas genéricas (una de usuarios, una de tweets y, mientras se muestra, una para el top 5). Al iniciar se cargan desde disco y al salir se guardan.

## 2. Organización del código

| Archivo | Responsabilidad |
|---------|-----------------|
| `lista.h` / `lista.c` | Lista doblemente enlazada genérica (`void *`). No conoce ni usuarios ni tweets. |
| `user.h` / `user.c` | Entidad User: registro, inicio de sesión, carga y guardado de `users.bin`. |
| `tweet.h` / `tweet.c` | Entidad Tweet: publicar (y reportar a FameCheck), modificar, eliminar, feed ordenado, búsqueda, carga y guardado de `tweets.bin`. |
| `famecheck.h` / `famecheck.c` | **Nuevo.** Comunicación con FameCheck: verificar una cuenta, reportar una publicación, obtener el top 5 y registrar los fallos en `logs.log`. |
| `cJSON.h` / `cJSON.c` | **Nuevo.** Librería externa (DaveGamble/cJSON) para armar y leer JSON. Se compila junto con el proyecto. |
| `main.c` | Interfaz de consola (menús), recorrido del feed de a un tweet y flujo general del programa. |
| `config.ini` | Configuración: `linux=true` o `linux=false`, para elegir el comando de limpieza de pantalla (`clear` o `cls`) y si se usan colores. |
| `apikey.key` | **Nuevo.** Clave de API de FameCheck, en una sola línea. Está en el `.gitignore`: cada integrante tiene la suya en su copia local. |
| `Cwitter.cbp` | Proyecto de Code::Blocks. En el Sprint 2 se le agregaron `famecheck`, `cJSON` y la librería `curl`. |
| `test_users.c` / `test_tweets.c` / `test_famecheck.c` | Programas de prueba de cada módulo (ver sección 8). |

Las dependencias principales van en un solo sentido: `main` usa `user`, `tweet` y `famecheck`; `tweet` usa `famecheck` para reportar cada publicación; `famecheck` usa `lista`, `curl` y `cJSON`. La lista no depende de ningún módulo.

## 3. La lista genérica (`lista.c`)

Cada nodo guarda un puntero `void *data` a una copia del dato, el tamaño de ese dato (`dataSize`) y punteros al nodo siguiente y al anterior. La lista mantiene punteros a `inicio` y `fin` y un contador `cant`.

Operaciones disponibles:

- `listaInicializar`, `listaVacia`, `listaLlena`, `listaVaciar`.
- `listaInsertar`: inserta siempre al final. Reserva memoria para el nodo y para la copia del dato (`memcpy`).
- **`listaInsertarOrdenado` (nueva en el Sprint 2):** recorre la lista desde el inicio mientras `cmp(nuevo, actual) > 0` y deja el nuevo nodo justo antes del primero donde eso deja de cumplirse (o al final). Se usa para el top 5 y para el feed.
- **`listaEliminarUltimo` (nueva en el Sprint 2):** desenlaza y libera el último nodo. Se usa para recortar el top 5.
- `listaBuscarYDevolver`: devuelve el primer nodo que cumple una condición, o `NULL`.
- `listaBuscarYAccion`: aplica una acción a todos los nodos que cumplen una condición y devuelve cuántos fueron.
- `listaRecorrerYAccion`: aplica una acción a todos los nodos.
- `listaEliminarNodo`: busca un nodo con una condición y lo desenlaza. Contempla eliminar el primero, el último y uno intermedio, y libera nodo y dato.

La condición de búsqueda se pasa como función de comparación (`int (*cmp)(void *, void *)`) y las acciones como función de acción (`void (*accion)(void *)`). Por eso una misma lista sirve para usuarios, tweets y el top 5: cada módulo aporta sus propias funciones (`cmpUser`, `cmpTweetId`, `cmpTweetTexto`, `cmpTweetFeed`, `famecheckCmpActividad`, `userImprimir`, `tweetImprimir`, etc.).

En `listaInsertarOrdenado` el comparador indica posición: un valor negativo significa que el primer dato va antes que el segundo, y uno positivo que va después. Para ordenar de mayor a menor (por ejemplo, más publicaciones primero) el comparador devuelve negativo cuando el primero es más grande.

`listaLlena` prueba si se puede reservar memoria para un nodo y un dato. Si no se puede, las funciones de inserción devuelven `LISTA_LLENA`. No hay un máximo fijo de usuarios ni de tweets; el único límite es la memoria disponible.

## 4. Entidades

### User

```c
typedef struct {
    char user[20];   /* nombre de usuario */
    char pass[20];   /* contraseña */
    int cantTweets;  /* nuevo en el Sprint 2, hoy sin uso */
    int verficado;   /* nuevo en el Sprint 2: 1 si FameCheck lo verificó */
} User;
```

El nombre de usuario es la clave de identificación: no puede haber dos usuarios con el mismo nombre. Como `main` lee con `%19s`, nombre y contraseña admiten hasta 19 caracteres sin espacios.

`verficado` guarda lo que respondió FameCheck, así que se persiste en `users.bin` y no hace falta volver a consultar el servicio en cada inicio de sesión. `cantTweets` se agregó pensando en el top 5, pero la cantidad de publicaciones la lleva FameCheck y el campo quedó sin usar.

### Tweet

```c
typedef struct {
    unsigned int id;
    char autor[20];
    char mensaje[MAX_TWEET + 1]; /* MAX_TWEET = 140, más el '\0' */
    time_t fecha;                /* nuevo: momento de publicación */
    int verificado;              /* nuevo: si el autor estaba verificado al publicar */
    int reportadoAFamecheck;     /* nuevo: 1 si FameCheck aceptó el reporte */
} Tweet;
```

Cada tweet tiene un `id` numérico único. El autor se guarda como texto (el nombre de usuario), sin referencia a la lista de usuarios. Por eso el tweet guarda también si su autor estaba verificado: el feed se ordena comparando solo dos tweets, sin tener que buscar al autor en la lista de usuarios.

### ActividadFame (nueva)

```c
typedef struct {
    char nombre[20];
    int cantidadCweets;
} ActividadFame;
```

Representa una cuenta verificada con su cantidad de publicaciones, tal como la devuelve FameCheck. Solo vive en memoria, mientras se arma y se muestra el top 5.

## 5. Flujo del programa (`main.c`)

1. Se lee `config.ini` (`leerConfig`) y se inicia FameCheck una sola vez (`famecheckIniciar`: inicializa curl y lee `apikey.key`). Si falla, se avisa y el programa sigue funcionando: las operaciones que usan FameCheck devuelven error. Después se inicializan las listas y se cargan desde disco (`tweetsAbrir`, `usersAbrir`).
2. **Menú sin sesión:** iniciar sesión, registrarse o salir. Sin sesión iniciada no se puede publicar ni ver el feed, porque esas opciones no existen en este menú.
3. **Menú con sesión** (se muestra mientras `pUsuarioActual != NULL`): arriba se indica si el usuario está verificado y se muestra el tweet actual del feed ("Tweet X de Y"). Las opciones son: `A`/`D` tweet anterior/siguiente, `1` ver feed completo, `2` publicar, `3` buscar, `4` modificar, `5` eliminar, `6` top 5 de verificados, `7` cerrar sesión y `0` verificar la cuenta con FameCheck. La opción se lee como un carácter.
4. Al elegir Salir en el menú sin sesión, se guardan usuarios y tweets en disco, se libera la memoria de las listas, se cierra curl (`famecheckFinalizar`) y termina el programa.

La sesión activa se representa con un puntero (`User *pUsuarioActual`) al dato del usuario dentro de la lista. Cerrar sesión lo pone en `NULL` y vuelve al primer menú. Para el recorrido del feed, `main` guarda un puntero al nodo del tweet actual (`nodoTweet`) y su posición (`posTweet`).

En Linux (`linux=true`) los menús usan colores con códigos de escape ANSI; en Windows se muestran sin colores, porque la consola clásica no siempre los interpreta.

## 6. Funcionalidades

**Registro (`userRegistrar`).** Busca el nombre en la lista de usuarios. Si no existe, inserta el nuevo usuario (con `verficado = 0`). Si existe, devuelve `USER_DUPLICADO`. Los campos se inicializan en cero y los textos se copian con `strncpy`, con un máximo de 19 caracteres.

**Inicio de sesión (`userIniciarSesion`).** Busca al usuario por nombre y compara la contraseña. Devuelve un puntero al usuario si coincide y `NULL` si el usuario no existe o la contraseña es incorrecta (no distingue los dos casos).

**Cierre de sesión.** Opción 7 del menú con sesión: descarta el usuario actual.

**Verificación de cuenta (`famecheckVerificarCuenta`, nueva).** Opción 0 del menú con sesión. Hace un `POST /api/Cwitter/Usuario` con el nombre y guarda en `verficado` lo que responde FameCheck. Si la cuenta ya existía en FameCheck, el servicio responde 200 con el mismo resultado, así que consultar dos veces no duplica nada. Si el servicio no responde, se informa el error y el usuario queda como estaba.

**Publicar (`tweetPublicar`).** Recibe el `User` completo (antes recibía solo el nombre). Asigna el siguiente id, la fecha y hora actual (`time(NULL)`) y si el autor está verificado, copia el autor y el mensaje (truncado a 140 caracteres) y lo inserta ordenado en el feed. Si el autor está verificado, antes reporta la publicación a FameCheck (ver abajo). La publicación se concreta aunque el reporte falle.

**Reporte a FameCheck (`famecheckReportarPublicacion`, nueva).** Hace un `POST /api/Cwitter/Cweet` con el usuario y el mensaje. Solo se llama para autores verificados, porque FameCheck rechaza los reportes de cuentas no verificadas. FameCheck responde 204 cuando acepta el reporte (la documentación dice 201, ver problema 19), y en ese caso el tweet queda con `reportadoAFamecheck = 1`.

**Log de fallos (`famecheckRegistrarLog`, nueva).** Si el reporte falla, se agrega una línea a `logs.log` con la fecha y hora, el id del tweet, el autor y el motivo (`FAMECHECK_ERROR_RED`, `FAMECHECK_ERROR_RESPUESTA`, etc.). No se reintenta el envío: como pide la consigna, alcanza con dejar constancia.

**Modificar (`tweetModificar`).** Busca el tweet por id. Devuelve `NODO_NO_ENCONTRADO` si no existe y `SIN_PERMISO` si el autor no es quien intenta modificarlo. Si todo está bien, reemplaza el mensaje (también truncado a 140). No cambia la posición del tweet en el feed.

**Eliminar (`tweetEliminar`).** Mismas validaciones que modificar; si pasan, elimina el nodo de la lista.

**Feed (`tweetsMostrarFeed`).** Recorre la lista completa e imprime los tweets de todos los usuarios. Como la lista se mantiene ordenada (`cmpTweetFeed`), aparecen primero los tweets de cuentas verificadas y después el resto; dentro de cada grupo, del más reciente al más viejo. Si dos tweets tienen la misma fecha (mismo segundo), va primero el de id más alto. Cada tweet se imprime con su fecha y hora de publicación, y los de cuentas verificadas llevan la marca `[V]`.

**Recorrido de a un tweet (nuevo).** En el menú con sesión se ve un tweet por vez. `D` pasa al siguiente (`nodo->sig`) y `A` vuelve al anterior (`nodo->ant`), sin recorrer el feed desde el principio. En los extremos no se mueve. Después de publicar o eliminar, el recorrido vuelve al primer tweet.

**Top 5 de verificados (`famecheckObtenerTop5`, nueva).** Opción 6. Hace un `GET /api/Cwitter/Usuario`, que devuelve las cuentas verificadas con su cantidad de publicaciones, sin ordenar. Cada cuenta se inserta ordenada en una lista (`famecheckInsertarEnTop`) y, si la lista pasa de 5, se elimina el último. Queda de mayor a menor cantidad; los empates se ordenan alfabéticamente por nombre. Si hay menos de 5 verificados se muestran los que haya, y si no hay ninguno se avisa.

**Búsqueda (`tweetBuscarYMostrar`).** Recorre la lista y muestra los tweets cuyo mensaje contiene el texto buscado (`strstr`, distingue mayúsculas de minúsculas). Si no hay coincidencias, informa que no se encontró ninguno.

**Identificación de tweets.** Los ids se generan con una variable estática `ultimo_id` que se incrementa en cada publicación. Al cargar el archivo, `ultimo_id` se actualiza al id más alto encontrado, para que los nuevos tweets no repitan ids existentes.

## 7. Persistencia

- **Formato:** archivos binarios, uno para cada entidad: `users.bin` y `tweets.bin`.
- **Contenido:** la secuencia de `struct` completos, uno detrás de otro, escritos con `fwrite` y leídos con `fread`, sin encabezado ni separadores. Con el compilador del proyecto (MinGW-w64 de 64 bits), `sizeof(User)` es 48 bytes y `sizeof(Tweet)` es 184 bytes.
- **Cuándo se guarda:** al salir del programa (opción 3 del menú principal), sobrescribiendo el archivo completo (`"wb"`).
- **Cuándo se carga:** al iniciar el programa, leyendo el archivo registro por registro. Los usuarios se insertan al final y los tweets se insertan ordenados, así el feed queda bien aunque el archivo no lo esté. Si el archivo no existe, se arranca con la lista vacía y las dos funciones devuelven `ERROR_AP_ARCH`.
- **Archivos del Sprint 1:** como `User` y `Tweet` tienen campos nuevos, los `.bin` generados con el Sprint 1 ya no se leen correctamente y hay que borrarlos.
- **Verificación persistente:** `verficado` forma parte de `User`, así que se guarda y se recupera con el resto del usuario.

Archivos de texto, separados de los binarios:

- **`logs.log`:** se abre en modo `"a"` (agregar al final) cada vez que falla un reporte, se escribe una línea y se cierra. No se lee desde el programa: es para revisarlo a mano.
- **`apikey.key`:** se lee una vez al iniciar.

Cada archivo se maneja solo con funciones de su tipo: los binarios con `fopen` `"rb"`/`"wb"`, `fread` y `fwrite`, y los de texto con `fgets` y `fprintf`. Nunca se mezclan sobre un mismo archivo.

## 8. Pruebas realizadas hasta ahora

- **`test_users.c`:** carga inicial, registro de usuarios, registro de un usuario duplicado, login correcto, login con contraseña incorrecta, login con usuario inexistente, guardado, vaciado de la lista y recarga para verificar la persistencia.
- **`test_tweets.c`:** plataforma vacía, publicación de tweets de usuarios verificados y no verificados, feed con la mezcla de ambos, un tweet más viejo que queda al final, recorrido hacia adelante y hacia atrás varias veces, búsqueda existente e inexistente, eliminación del primero, del último y de uno intermedio, intento de borrar un tweet ajeno, modificación de un tweet propio, guardado, vaciado y recarga conservando el orden. Se adaptó en el Sprint 2 porque cambió la firma de `tweetPublicar`.
- **`test_famecheck.c` (nuevo):** inicio del servicio, una cuenta que se verifica y otra que no, verificar dos veces la misma cuenta, reportar la publicación de un verificado y la de uno no verificado, reportar sin conexión (simulada con un proxy inexistente) y revisar el log, top 5 con menos de 5 verificados, top 5 con empates, top 5 real del servicio y verificación que se conserva al reiniciar.

La hoja de tests completa, con situación, operación, resultado esperado y salida obtenida de cada caso, está en `TESTS.md`.

## 9. Códigos de retorno

| Constante | Valor | Significado |
|-----------|-------|-------------|
| `TODO_OK` | 0 | Operación exitosa |
| `LISTA_LLENA` | 3 | No hay memoria para insertar |
| `NODO_NO_ENCONTRADO` | 4 | No existe el elemento buscado |
| `ERROR_AP_ARCH` | 5 | No se pudo abrir el archivo |
| `USER_DUPLICADO` | 6 | El nombre de usuario ya existe |
| `SIN_PERMISO` | 7 | El tweet pertenece a otro usuario |
| `FAMECHECK_OK` | 0 | FameCheck respondió lo esperado |
| `FAMECHECK_ERROR_RED` | -1 | No se pudo contactar al servicio (sin conexión, timeout) |
| `FAMECHECK_ERROR_RESPUESTA` | -2 | El servicio respondió algo inesperado (400, 401, JSON inválido) |
| `FAMECHECK_ERROR_MEMORIA` | -3 | No se pudo reservar memoria para armar el pedido o la respuesta |
| `FAMECHECK_ERROR_CONFIG` | -4 | No se inició FameCheck o falta la clave en `apikey.key` |

Los códigos de FameCheck son negativos para no confundirse con los del resto del programa.

## 10. Limitaciones conocidas de esta versión

- **Guardado solo al salir por el menú.** Si el programa se cierra de otra forma (ventana, corte de luz), se pierden los cambios de esa sesión.
- **Truncado silencioso:** un tweet de más de 140 caracteres se corta sin avisar al usuario, y `main` informa "publicado con éxito" sin revisar el valor de retorno.
- **Registro duplicado sin aviso:** `main` no muestra un mensaje cuando `userRegistrar` devuelve `USER_DUPLICADO`.
- **Entrada de datos:** en el menú sin sesión, ingresar letras donde se espera un número deja al programa en un bucle. En el menú con sesión ya no pasa, porque la opción se lee como carácter.
- **Búsqueda:** distingue mayúsculas y minúsculas.
- **Verificarse después de publicar:** los tweets guardan si el autor estaba verificado al momento de publicar. Si alguien se verifica después, sus tweets anteriores no suben en el feed. Queda fuera del alcance de este sprint.
- **Reportes fallidos:** quedan en `logs.log` pero no se reintentan; habría que reenviarlos a mano.
- **Tiempo de espera:** cada pedido a FameCheck tiene un límite de 10 segundos. Sin conexión, publicar un tweet de un verificado puede demorar hasta ese tiempo antes de completarse.
- **`cantTweets` en `User`** quedó sin uso.

---

# Problemas, decisiones y registro individual

## 1. Línea de tiempo del proyecto

### Sprint 1

| Fecha | Quién | Qué se hizo |
|-------|-------|-------------|
| Antes del 8/9 | agussrico | Primer commit: `cwitter.h` con estructuras y prototipos (diseño inicial, ver decisión 1). |
| 8/9 | LagooFran | Se crea `llista.h` (con error de tipeo en el nombre) y se elimina. Se agrega `.gitignore` para archivos de Code::Blocks. Se implementa la lista genérica (`lista.h`/`lista.c`). |
| 9/9 | agussrico | Se corrigen los prototipos de punteros a función en `lista.h` (rama `fix/lista-prototipos`, merge a `develop`). Se implementan registro, login y persistencia de usuarios, con sus tests. |
| 11/9 | LagooFran | Menú de inicio de sesión y registro, arreglo del inicio de sesión, cambios en la lista y estilo. Se mergea el PR #1 (`feature/usuarios`). |
| 15/9 | mateofeiertag | Integración del módulo de tweets y del feed en `main`. |
| 15/9 | Garbaaa | Se corrige la búsqueda de tweets y se restringe modificar/eliminar al autor. |
| 17/9 | mateofeiertag | Casos de prueba del Sprint 1 para tweets. |
| 20/9 | LagooFran | Arreglos generales y soporte multisistema (`7478fea`): `config.ini` y `limpiarPantalla()`, se elimina `LISTA_VACIA` y el parámetro sin uso de `listaLlena`. Luego agrega `tweet.c` y `tweet.h` al proyecto de Code::Blocks (`11fdc8a`). |
| 20/9 | agussrico | Inicialización de structs y copia con límite en usuarios y tweets (`8f13ca8`). Bitácora, hoja de tests y README; release `v1-fundamentos`. |

### Sprint 2

| Fecha | Quién | Qué se hizo |
|-------|-------|-------------|
| 7/10 | LagooFran | Se agregan cJSON y un primer `famecheck.c`, y los campos `cantTweets` y `verficado` en `User` (`225d2dd`). Funciones generales para usar curl y cJSON y verificación de usuarios con la opción 0 del menú (`2a12cc7`, rama `feature/famecheck`). |
| 8/10 | LagooFran y agussrico | Reporte de tweets a FameCheck, top 5 de verificados con `listaInsertarOrdenado` y `listaEliminarUltimo`, `tweetPublicar` pasa a recibir el `User`, menús nuevos con colores y recorrido del feed con A/D (`31d686e`). |
| 10/10 | agussrico | Se corrige el orden del top 5 y curl pasa a iniciarse una sola vez (`7461930`). Se agregan famecheck, cJSON y curl al proyecto de Code::Blocks (`748ada8`). |
| 10/10 | agussrico | Log de fallos al reportar tweets (`5986abd`, rama `feature/logs`). Se corrige el `.gitignore` y se sacan del repo la clave, el binario y `mainOLD.c` (`8670e77`). Se agrega `test_famecheck.c` (`06cf809`). |
| 10/10 | agussrico | Feed con verificados primero y después por fecha (`77d01da`, rama `feature/feed`) y actualización de `test_tweets.c` (`44948f1`). `tweetsAbrir` pasa a devolver `ERROR_AP_ARCH` sin archivo. Bitácora del Sprint 2. |
| 11/10 | mateofeiertag | Hoja de tests del Sprint 2 (`4738645`). Nuevo formato de los tweets en consola: cada tweet muestra la fecha y hora de publicación y la marca `[V]` si el autor está verificado (`f980911`). |

## 2. Decisiones de diseño

### Decisión 1: de la librería `cwitter` a una lista genérica reutilizable

**Problema a resolver.** Necesitábamos guardar en memoria una cantidad de usuarios y tweets sin límite fijo (el cliente no dio un máximo) y poder recorrer, buscar, insertar y eliminar elementos.

**Primera versión (commit `177626c`).** Una librería `cwitter.h` con estructuras propias de la aplicación:

- `Usuario` y `Tweet`, cada uno como nodo de lista simplemente enlazada (`siguiente`), separando la "carga útil" que va al archivo (`DatosUsuario`, `DatosTweet`) del nodo que solo existe en RAM.
- Una estructura `Sistema` que agrupaba todo el estado: lista de usuarios, lista de tweets, próximo id y usuario actual.
- `DatosTweet` tenía `contenido[200]`, `fechaHora` y un campo `activo` para borrado lógico.

**Alternativas consideradas.**

- Mantener las listas específicas dentro de `cwitter.h`.
- Una lista genérica (`void *`) con funciones estándar, independiente de qué guarda.

**Qué elegimos.** Una lista doblemente enlazada genérica en `lista.h`/`lista.c`, que queda como base común en la rama `develop`. Cada nodo guarda un `void *data` con una copia del dato y su tamaño; la búsqueda y las acciones se pasan como funciones (`cmp`, `accion`).

**Por qué.** La misma lista sirve para usuarios y tweets, y también para los sprints siguientes, donde van a aparecer nuevos pedidos; sin límite máximo (solo la memoria); y permite eliminar en el medio sin recorrer desde el principio para "empalmar" el nodo anterior.

**Qué cambió respecto de la versión inicial (visible entre commits).**

- El borrado lógico (`activo`) desapareció: ahora eliminar un tweet saca el nodo de la lista y libera su memoria (`listaEliminarNodo`). Se optó por este cambio ya que brinda un uso de memoria más eficiente y prolijo, y nos evita el conflicto de filtrado de tweets para mostrar en el feed.
- Se eliminó la estructura de `Sistema`; el usuario actual pasó a ser un puntero en `main`, ya que con este puntero, y al menos para esta entrega, nos basta para gestionar la sesión de usuarios en Cwitter.
- `contenido[200]` pasó a `mensaje[141]` (140 + `'\0'`), ajustado al límite de 140 caracteres del enunciado.

**Cómo resultó en el Sprint 2.** La decisión se confirmó: el top 5 y el feed ordenado se resolvieron con la misma lista, agregando solo dos operaciones genéricas (`listaInsertarOrdenado` y `listaEliminarUltimo`), y el recorrido de a un tweet usa los punteros `ant` y `sig` que ya tenía. La fecha del tweet, que se había descartado del diseño inicial, volvió a agregarse porque ahora el feed se ordena por fecha.

### Decisión 2: quién puede modificar o eliminar un tweet

**Problema a resolver.** El enunciado dice que hay que estar identificado para "corregir o eliminar" un tweet, pero no aclara si se refiere solo a los propios.

**Primera versión.** Cualquier usuario con sesión iniciada podía modificar o eliminar cualquier tweet (`tweetModificar(feed, id, mensaje)` y `tweetEliminar(feed, id)`).

**Qué elegimos (commit `71bae31`, Garbaaa, 15/9).** Solo el autor puede modificar o eliminar su tweet. Ambas funciones reciben ahora el autor que intenta la operación, primero buscan el tweet por id y después comparan el autor; si no coincide devuelven `SIN_PERMISO` (código 7). En `main` se muestran tres mensajes distintos: éxito, "ese tweet no es tuyo" y "no se encontró el tweet".

**Por qué.** El feed muestra tweets de todos los usuarios, y dejar que cualquiera borre los de otros no tiene sentido en una red social. De esto nos dimos cuenta una vez hecho la parte de modificación y eliminación de tweets, por lo que fue añadido más adelante.

**Efecto en los tests.** `test_tweets.c` incluye un caso donde un usuario intenta borrar el tweet de otro y se espera `SIN_PERMISO`, y otro donde el autor modifica el suyo.

### Decisión 3: formato de persistencia

Archivos binarios (`users.bin`, `tweets.bin`) con la secuencia de `struct` completos, escritos con `fwrite`/`fread`. Se carga al iniciar y se guarda al salir.

**Por qué.** No hay que parsear texto ni escapar separadores dentro de un mensaje, y el código de carga y guardado es corto; además el requisito de la materia pide no mezclar funciones de archivos de texto con binarios.

**Consecuencia en el Sprint 2.** Al agregar campos a `User` y `Tweet` cambió el tamaño de cada registro, y los archivos del Sprint 1 quedaron incompatibles. Como todavía no hay datos reales que conservar, se decidió borrarlos en lugar de escribir una migración.

### Decisión 4: un módulo por entidad y tests por módulo

Se separó el código en `user.c/h` y `tweet.c/h`, y cada uno con su archivo de pruebas (`test_users.c`, `test_tweets.c`). El trabajo se repartió en ramas de funcionalidad (`feature/usuarios`, `fix/lista-prototipos`) mergeadas a `develop`.

En el Sprint 2 se siguió el mismo criterio: todo lo que habla con el servicio externo quedó en `famecheck.c/h`, con su propio `test_famecheck.c`, y el trabajo se hizo en las ramas `feature/famecheck`, `feature/logs` y `feature/feed`.

### Decisión 5: cómo hacer que el programa funcione en Windows y en Linux

**Problema a resolver.** `main` limpiaba la pantalla con `system("cls")`, un comando de Windows que no existe en Linux, y la consigna pide una solución en ANSI C con la mayor compatibilidad multiplataforma posible.

**Qué elegimos.** Un archivo `config.ini` con una línea `linux=false` o `linux=true`. Al iniciar, `leerConfig()` lo lee y guarda el resultado en una variable global (`usarLinux`). La función `limpiarPantalla()` ejecuta `clear` o `cls` según ese valor y reemplaza todas las llamadas directas a `system("cls")`.

**Límites de esta solución.** El archivo se busca en la carpeta desde donde se ejecuta el programa y solo se lee su primera línea; si falta, se usa `cls`, que es el comportamiento anterior. Quien use Linux tiene que cambiar `linux=false` por `linux=true` en su copia.

**Uso en el Sprint 2.** El mismo valor decide si se muestran colores en los menús: los códigos ANSI solo se imprimen en Linux.

### Decisión 6 (Sprint 2): cómo armar el top 5

**Problema a resolver.** FameCheck devuelve todas las cuentas verificadas con su cantidad de publicaciones, sin ordenar, y hay que mostrar las 5 con más publicaciones. Puede haber menos de 5 y puede haber empates.

**Alternativas consideradas.**

- Bajar todas las cuentas a una lista o vector y ordenarlo completo (por ejemplo, con burbujeo o `qsort`), y después mostrar los primeros 5.
- Un vector fijo de 5 posiciones, reemplazando el menor cada vez que llega uno más grande.
- Una lista ordenada que nunca pasa de 5: cada cuenta se inserta en su lugar y, si quedan 6, se saca la última.

**Qué elegimos.** La lista ordenada con corte (`famecheckInsertarEnTop`, que usa `listaInsertarOrdenado` y `listaEliminarUltimo`), con el comparador `famecheckCmpActividad`: más publicaciones primero y, en caso de empate, orden alfabético.

**Por qué.** Reutiliza la lista genérica en lugar de escribir un ordenamiento aparte. Nunca guarda más de 6 elementos, aunque FameCheck devuelva muchas cuentas. Con menos de 5 cuentas funciona sin casos especiales, y el desempate por nombre hace que el resultado sea siempre el mismo para los mismos datos.

**Qué observamos.** La primera versión ordenaba al revés: el comparador devolvía positivo cuando la cuenta nueva tenía más publicaciones, la lista quedaba de menor a mayor y al recortar se eliminaba al que más tenía. Ver problema 11.

### Decisión 7 (Sprint 2): feed con verificados primero

**Problema a resolver.** El fundador pide que los tweets de cuentas conocidas aparezcan primero y que el resto siga ordenado por fecha y hora de publicación.

**Alternativas consideradas.**

- Ordenar la lista cada vez que se muestra el feed.
- Dos listas separadas, una para tweets de verificados y otra para el resto, que se muestran una detrás de la otra.
- Mantener una sola lista siempre ordenada, insertando cada tweet en su lugar.

**Qué elegimos.** Una sola lista ordenada. Cada tweet se inserta con `listaInsertarOrdenado` y el comparador `cmpTweetFeed`: primero verificados, después del más reciente al más viejo y, si coinciden en el mismo segundo, el de id más alto. Al cargar `tweets.bin` también se inserta ordenado.

**Por qué.** El feed completo, la búsqueda y el recorrido de a un tweet trabajan sobre la misma lista y en el mismo orden, sin copiar ni reordenar nada al mostrar. Con dos listas, el recorrido tendría que saltar de una a otra y modificar o eliminar tendría que buscar en las dos. Insertar ordenado cuesta recorrer la lista hasta el lugar del tweet, que es aceptable para la cantidad de tweets del programa.

**Cómo sabe el comparador si el autor está verificado.** El comparador solo recibe dos tweets, así que se agregó al `Tweet` el campo `verificado`, que se copia del autor al publicar. La otra opción era buscar al autor en la lista de usuarios en cada comparación, pero el comparador no tiene acceso a esa lista y sería mucho más lento. La consecuencia es que si alguien se verifica después de publicar, sus tweets anteriores no suben (ver limitaciones).

**Qué cambió respecto del Sprint 1.** El feed pasó de mostrarse del más viejo al más nuevo (orden de inserción) a mostrarse del más reciente al más viejo, que es lo habitual en una red social.

### Decisión 8 (Sprint 2): recorrer el feed de a un tweet

**Problema a resolver.** El usuario tiene que poder ver un tweet, pasar al siguiente y volver al anterior sin recorrer todo el feed desde el principio.

**Alternativas consideradas.**

- Una pila con los tweets ya vistos: al avanzar se apila el actual y al volver se desapila.
- Guardar solo la posición (un número) y recorrer la lista desde el inicio hasta esa posición cada vez.
- Usar directamente los punteros `ant` y `sig` de la lista doble.

**Qué elegimos.** Los punteros de la lista doble. `main` guarda el nodo actual y su posición; `D` pasa a `nodo->sig` y `A` a `nodo->ant`.

**Por qué.** La lista ya es doblemente enlazada, así que volver al anterior es inmediato y no hace falta otra estructura ni memoria extra. La pila resolvería volver atrás, pero duplicaría información que la lista ya tiene. Guardar solo la posición obligaría a recorrer desde el principio, que es justo lo que el fundador no quiere.

**Qué hay que cuidar.** Si se publica o se elimina un tweet, el nodo actual puede cambiar de posición o dejar de existir. Por eso, después de esas operaciones el recorrido vuelve al primer tweet.

### Decisión 9 (Sprint 2): cómo registrar los fallos de FameCheck

**Problema a resolver.** Si FameCheck no responde al reportar una publicación, la publicación tiene que hacerse igual y el fallo tiene que quedar registrado para revisarlo después. No hace falta reintentar.

**Alternativas consideradas.**

- Guardar los fallos en una lista en memoria y escribirlos al salir.
- Un archivo binario con un `struct` por fallo.
- Un archivo de texto al que se agrega una línea por fallo.

**Qué elegimos.** Un archivo de texto `logs.log` abierto en modo `"a"`, con una línea por fallo: fecha y hora, id del tweet, autor y motivo.

**Por qué.** El log lo va a leer una persona, así que conviene que sea texto. Agregar al final evita reescribir el archivo y no se pierde nada si el programa se corta después del fallo, algo que sí pasaría con una lista en memoria. Es un archivo distinto de los binarios, así que no se mezclan funciones de texto y binarias sobre un mismo archivo. Guardar el motivo permite distinguir si el servicio no respondió (`FAMECHECK_ERROR_RED`) o si rechazó el reporte (`FAMECHECK_ERROR_RESPUESTA`).

### Decisión 10 (Sprint 2): integración con FameCheck

**Problema a resolver.** Hablar con un servicio HTTP desde C, armar y leer JSON y autenticarse con una clave, sin que una falla del servicio detenga a Cwitter.

**Qué elegimos.**

- **curl y cJSON**, a partir del `ejemplo_api.c` de la cátedra. Una sola función `enviarPeticion` hace los `POST` y los `GET` (si no hay body, es un `GET`), con un tiempo de espera de 10 segundos.
- **Todo en un módulo `famecheck.c/h`**, con códigos de error propios y negativos.
- **curl se inicia una sola vez** al arrancar el programa y se cierra al salir. La primera versión lo iniciaba y cerraba en cada publicación y en cada opción del menú (ver problema 12).
- **La clave en `apikey.key`**, fuera del código y del repositorio, para que cada integrante use la suya.
- **`tweetPublicar` recibe el `User` completo** en lugar del nombre, para saber si el autor está verificado sin consultar a FameCheck en cada publicación.
- **La verificación se pide con la opción 0 del menú** y el resultado se guarda en el usuario. Así, si el servicio no responde, se puede volver a intentar más tarde.

**Por qué.** Separar el módulo permite probarlo aparte (`test_famecheck.c`) y que el resto del programa no dependa de cómo se habla con el servicio. Que cada función devuelva un código en lugar de cortar el programa es lo que permite que una publicación se concrete aunque FameCheck falle.

## 3. Problemas encontrados y cómo se resolvieron

| # | Problema | Causa (según el diff) | Solución | Commit |
|---|----------|-----------------------|----------|--------|
| 1 | Prototipos de funciones con punteros a función mal escritos en `lista.h` y `lista.c`. | Se escribió `int (*cmp(void *, void*))`, que declara una función que devuelve un puntero, en lugar de `int (*cmp)(void *, void*)`, un puntero a función. | Se corrigieron todos los prototipos y definiciones. | `c4ddef8` (agussrico, 9/9) |
| 2 | Falta el prototipo de `listaRecorrerYAccion` en `lista.h`. | Se implementó en `lista.c` pero no se declaró en el header. | Se agregó al mismo fix. | `c4ddef8` |
| 3 | `listaEliminarNodo` podía fallar al borrar el primero, el último o el único nodo. | Hacía `ant->sig = ...` y `sig->ant = ...` sin comprobar si `ant` o `sig` eran `NULL`. | Se agregó el chequeo: si no hay anterior se actualiza `lista->inicio`, si no hay siguiente se actualiza `lista->fin`. | `c4ddef8` |
| 4 | `listaVacia` mezclaba un código de error con un valor booleano. | Devolvía `LISTA_VACIA` (2) o `TODO_OK` (0) en lugar de un verdadero/falso. | Ahora devuelve `lista->cant == 0`. | `c4ddef8` |
| 5 | La búsqueda de tweets mostraba los tweets que no contenían el texto. | En `tweetBuscarYMostrar` la condición era `cmpTweetTexto(...) == 0`, y `cmpTweetTexto` devuelve 1 cuando sí encuentra. | Se cambió la comparación a `== 1`. | `5df6020` (Garbaaa, 15/9) |
| 6 | Warning *unused parameter* en `listaLlena`. | La función recibía `tLista *lista` y no lo usaba; solo necesita el tamaño del dato. | Se quitó el parámetro y se actualizaron el prototipo y la llamada en `listaInsertar`. | `7478fea` (LagooFran, 20/9) |
| 7 | `LISTA_VACIA` estaba definido pero sin uso. | Quedó sin usar después de corregir `listaVacia` (problema 4). | Se eliminó la constante. | `7478fea` |
| 8 | La limpieza de pantalla solo funcionaba en Windows. | `main` llamaba directamente a `system("cls")`. | Se agregó `config.ini` y una función `limpiarPantalla()` (ver decisión 5). | `7478fea` |
| 9 | Bytes basura en `users.bin` y posible desborde de campos. | `userRegistrar` y `userIniciarSesion` usaban `strcpy` sobre un `User` sin inicializar; `Tweet` tiene además 3 bytes de relleno sin inicializar. | `memset` a cero de la estructura y `strncpy` con límite en usuarios; `memset` del Tweet en `tweetPublicar`. | `8f13ca8` (agussrico, 20/9) |
| 10 | El proyecto de Code::Blocks no incluía `tweet.c` ni `tweet.h`. | Según el diff, no figuraban como unidades en `Cwitter.cbp`. | Se agregaron ambos archivos al proyecto. | `11fdc8a` (LagooFran, 20/9) |
| 11 | El top 5 mostraba las cuentas con **menos** publicaciones. | `famecheckCmpActividad` devolvía 1 cuando la cuenta nueva tenía más publicaciones, así que `listaInsertarOrdenado` la dejaba más atrás: la lista quedaba de menor a mayor y `listaEliminarUltimo` sacaba al que más tenía. | Se invirtió el comparador: devuelve negativo cuando la cuenta nueva tiene más publicaciones. | `7461930` (agussrico, 10/10) |
| 12 | curl se iniciaba y cerraba en cada publicación y en cada opción del menú, incluso para usuarios no verificados. | `tweetPublicar` y las opciones 0 y 6 llamaban a `famecheckIniciar` y `famecheckFinalizar`. | Se inicia una sola vez al arrancar `main` y se cierra al salir. | `7461930` |
| 13 | Los mensajes de reporte y del top 5 quedaban pegados a la línea siguiente. | Faltaba el `\n` en el `printf` de "Tweet reportado correctamente" y en `famecheckMostrarUsuarioTop`. | Se agregaron los saltos de línea. El menú del top 5 ahora también avisa si la API falla o si no hay verificados. | `7461930` |
| 14 | El proyecto de Code::Blocks no compilaba con FameCheck. | `Cwitter.cbp` no incluía `famecheck.c` ni `cJSON.c`, ni linkeaba `curl`. | Se agregaron las unidades, el directorio de includes de curl y la librería `curl` al linker. | `748ada8` (agussrico, 10/10) |
| 15 | La clave de API quedó subida al repositorio. | El `.gitignore` tenía `.key`, que solo ignora un archivo llamado exactamente `.key`, y no `apikey.key`. | Se cambió a `*.key`, se sacó `apikey.key` del repositorio y hay que pedir una clave nueva a la cátedra, porque la anterior quedó en el historial. | `8670e77` (agussrico, 10/10) |
| 16 | Había un ejecutable y un archivo viejo de `main` en el repositorio. | Se subieron `cwitter` (el binario de Linux, sin extensión, que ninguna regla ignoraba) y `mainOLD.c` (una copia de respaldo). | Se sacaron del repositorio y se agregaron `cwitter`, `*.log`, `bin/` y `obj/` al `.gitignore`. | `8670e77` |
| 17 | `test_tweets.c` dejó de compilar. | En el Sprint 2 `tweetPublicar` pasó a recibir un `User *`, y el test le seguía pasando el nombre como texto. | Se reescribió el test con usuarios armados a mano (verificados y no verificados) y se sumaron los casos del feed ordenado y del recorrido. | `44948f1` (agussrico, 10/10) |
| 18 | Dos tweets publicados en el mismo segundo quedaban en orden invertido. | `time()` tiene resolución de un segundo: si la fecha era igual, el comparador dejaba primero al más viejo. | Se agregó un desempate por id: el de id más alto es el más nuevo. | `77d01da` (agussrico, 10/10) |
| 19 | Los reportes correctos se tomaban como error. | La documentación de FameCheck dice que el reporte responde 201, pero el servicio responde 204 (sin body). | `famecheckReportarPublicacion` considera éxito el 204. Quedó anotado en el código. | `31d686e` (LagooFran, 8/10) |
| 20 | `tweetsAbrir` devolvía `TODO_OK` cuando no existía el archivo, a diferencia de `usersAbrir`. | Se encontró al completar la columna "Salida obtenida" de la hoja de tests: el caso esperaba `ERROR_AP_ARCH`. | Ahora devuelve `ERROR_AP_ARCH`, igual que `usersAbrir`. `main` no usa ese valor, así que no cambia el comportamiento del programa. | `6f73401` (agussrico, 10/10) |

## 4. Contribuciones visibles en el historial

| Integrante | Trabajo visible en los commits |
|------------|--------------------------------|
| agussrico | **Sprint 1:** diseño inicial (`cwitter.h`), corrección de la lista, librería/registro/login/persistencia de usuarios y sus tests, inicialización de structs y copia con límite, documentación. **Sprint 2:** coautoría del reporte y el top 5, corrección del top 5 y del inicio de curl, log de fallos, limpieza del repositorio, feed ordenado, `test_famecheck.c`, adaptación de `test_tweets.c` y bitácora. |
| LagooFran | **Sprint 1:** lista genérica, configuración del IDE, menús de inicio de sesión y registro, soporte multisistema (`config.ini`) y limpieza de la lista (`listaLlena`, `LISTA_VACIA`). **Sprint 2:** integración con curl y cJSON, verificación de usuarios, reporte de tweets, top 5 (`listaInsertarOrdenado`, `listaEliminarUltimo`), menús nuevos con colores y recorrido del feed con A/D. |
| mateofeiertag | **Sprint 1:** integración de tweets y feed en `main`, tests de tweets. **Sprint 2:** hoja de tests y formato de los tweets en consola (fecha y marca de verificado). |
| Garbaaa | **Sprint 1:** corrección de búsqueda, permisos de modificar/eliminar. |

## 5. Planteos individuales previos

#### Agustina Rico (agussrico) – Estructura de datos (lista genérica)

- **Qué solución se me ocurrió inicialmente:** Armé una librería propia de Cwitter (`cwitter.h`) con listas simplemente enlazadas específicas para usuarios y tweets, y una estructura `Sistema` que agrupaba todo el estado.
- **Qué dudas tenía:** Pensaba que una lista genérica no era posible o que nos iba a traer conflictos de casteo de estructuras a la hora de utilizarlas y que era mejor desarrollar réplicas de cada función.
- **Qué cambió después de discutirlo o investigarlo:** Un compañero propuso hacer una lista genérica. Lo discutimos y lo probamos en el código, y vimos que era práctico. Más adelante, desarrollando el programa, nos dimos cuenta de que la estructura `Sistema` no era necesaria y la sacamos.

#### Franco Lago (LagooFran) – Interfaz de consola

- **Qué solución se me ocurrió inicialmente:** Al armar la interfaz de consola utilicé funciones de sistema que me permitían mejorar la legibilidad de la interfaz.
- **Qué dudas tenía:** Me di cuenta que estas funciones que utilicé no eran compatibles con varios sistemas operativos por lo tanto el programa no iba a funcionar correctamente en sistemas Linux.
- **Qué cambió después de discutirlo o investigarlo:** Propuse crear un archivo de configuración que permita seleccionar el sistema operativo. Este archivo se lee al inicio del programa y configura el mismo para que funcione correctamente en todos los sistemas.

#### Mateo Feiertag (mateofeiertag) – Módulo de tweets

- **Qué solución se me ocurrió inicialmente:** Diseñar una estructura independiente para los tweets con el límite estricto de 140 caracteres y un campo de ID autoincremental, manejando las operaciones básicas de publicación y visualización de manera aislada.
- **Qué dudas tenía:** Tenía dudas sobre cómo integrar las publicaciones a la lista doblemente enlazada genérica sin romper los tipos de datos, y cómo garantizar la seguridad para que un usuario no pudiera modificar o eliminar tweets ajenos.
- **Qué cambió después de discutirlo o investigarlo:** Adapté las funciones comparadoras para que trabajen con la lista genérica. Además, desarrollé un conjunto de pruebas (`test_tweets.c`) para validar los casos de borde exigidos (plataforma vacía, borrado en extremos e intermedio, y control de permisos).

#### Francisco Garbaccio (Garbaaa) – Permisos de edición

- **Qué solución se me ocurrió inicialmente:** Tener en cuenta qué usuario está logueado, para que la modificación y eliminación de tweets no sea global sino dependiente del usuario.
- **Qué dudas tenía:** Tenía dudas de cómo buscar de la forma más eficiente el usuario para poder comparar con el tweet para llegar a la mejor solución posible.
- **Qué cambió después de discutirlo o investigarlo:** La mejor forma que se encontró fue pasar el autor por la función y compararlo con el tweet para poder vigilar correctamente los derechos del usuario.

---

# Operaciones que podrían pedirnos más adelante

## 1. Operaciones nuevas

| # | Operación | Por qué creemos que la van a pedir | Cambio de diseño que implicaría | Origen |
|---|-----------|------------------------------------|---------------------------------|--------|
| 1 | **Feed público:** ver el feed y buscar sin iniciar sesión; la sesión solo sirve para publicar, modificar y eliminar. | Es lo habitual en una red social, y en el enunciado el fundador dice que "cualquier persona" puede ver lo publicado. Abrir la lectura ayuda al crecimiento que él espera. | **Bajo.** `tweetsMostrarFeed` y `tweetBuscarYMostrar` ya no dependen del usuario. Cambia el menú de `main`: el menú sin sesión pasa a tener "ver feed" y "buscar". Cambia un requisito, así que hay que actualizar el caso de test "ver el feed sin estar logueado". | Equipo |
| 2 | **Comentarios e hilos:** responder a un tweet y ver la conversación completa. | Es el paso natural después de publicar mensajes: una red social sin respuestas se queda en un tablón. | **Alto.** `Tweet` necesita un campo nuevo (por ejemplo `idPadre`, con 0 para los tweets raíz). El feed muestra solo los raíces y el hilo se arma buscando los tweets con `idPadre` igual al id consultado. Ver decisiones abiertas (sección 2). | Equipo |
| 3 | ~~**Feed ordenado** del más reciente al más viejo~~ **(resuelto en el Sprint 2)** y **paginado**. | El orden se pidió en el Sprint 2 (verificados primero, después por fecha) y se resolvió con la lista ordenada (decisión 7). Queda pendiente paginar: con muchos tweets no se puede leer el feed entero. | **Bajo.** El recorrido de a un tweet ya resuelve parte de esto. Para paginar se saltan N nodos desde el inicio o desde el nodo actual. | Equipo |
| 4 | **Búsquedas más completas:** por autor, sin distinguir mayúsculas y por hashtag. | Hoy solo se busca un texto exacto con `strstr` (distingue mayúsculas). Buscar "lo que dijo alguien" o "un tema" es el uso típico. | **Bajo/medio.** Funciones `cmp` nuevas para usar con `listaBuscarYAccion`/`listaBuscarYDevolver`, que ya existen en la lista y sirven sin modificarla. Sin distinguir mayúsculas implica pasar a minúsculas al comparar (sin `strcasestr`, que no es ANSI C). | Sugerida por IA, evaluada por el equipo |
| 5 | **Perfil de usuario:** ver todos los tweets de una persona y cuántos tiene. | Complementa la búsqueda por autor y es lo primero que se mira de otra persona. | **Bajo.** Reutiliza `listaBuscarYAccion` con un `cmp` por autor y un contador. El campo `cantTweets` de `User`, hoy sin uso, podría servir para esto. | Sugerida por IA, evaluada por el equipo |
| 6 | **Seguir usuarios** y ver un feed solo con los tweets de a quienes sigo. | Es el pedido clásico cuando el feed general crece demasiado para ser útil. | **Alto.** Aparece una relación usuario–usuario (N a N). No se puede guardar dentro de `User` con `fwrite` si contiene una lista (tendría punteros). Habría que persistirla aparte, por ejemplo un `seguidores.bin` con pares (seguidor, seguido), y reconstruirla al cargar. Ver decisiones abiertas. | Sugerida por IA, evaluada por el equipo |
| 7 | **Gestión de cuenta:** cambiar contraseña, eliminar la cuenta y, más adelante, un rol de moderador que pueda borrar cualquier tweet. | Toda plataforma con registro termina necesitándolo. El moderador es la respuesta natural al cambio de permisos de la decisión 2. | **Medio.** Cambiar contraseña: modificar el dato en la lista y volver a guardar. Eliminar cuenta: `listaEliminarNodo` sobre usuarios, con una decisión abierta sobre sus tweets. Moderador: campo `rol` en `User`, lo que cambia el struct y obliga a migrar `users.bin` (ver lo que pasó con los `.bin` en el Sprint 2, decisión 3). | Sugerida por IA, evaluada por el equipo |
| 8 | **Reintentar los reportes fallidos** a FameCheck. | Hoy los fallos quedan en `logs.log` y nadie los reenvía; el fundador quiere tener el registro de actividad completo. | **Bajo/medio.** El tweet ya tiene `reportadoAFamecheck`: al iniciar o con una opción del menú se recorren los tweets de verificados con ese campo en 0 y se vuelven a reportar. | Equipo |
| 9 | **Subir en el feed los tweets de quien se verifica después de publicar.** | Hoy un tweet guarda si su autor estaba verificado al publicar; si se verifica después, sus tweets viejos no suben. | **Bajo.** Al verificar, recorrer los tweets de ese autor, marcar `verificado = 1` y reubicarlos en la lista (sacarlos e insertarlos ordenados). | Equipo |

## 2. Decisiones abiertas que estas operaciones nos van a obligar a tomar

### Hilos (operación 2)

- **¿Dónde se guarda la relación?** A decidir entre (a) un campo `idPadre` en cada `Tweet`, o (b) una lista de respuestas dentro de cada tweet. La opción (a) mantiene el registro de tamaño fijo y se puede seguir guardando con `fwrite` directo; la (b) no, porque tendría punteros.
- **¿Qué pasa al eliminar un tweet con respuestas?** A decidir entre borrar también las respuestas, o dejar el hilo con un "tweet eliminado".
- Los comentarios deberían respetar también el límite de 140 caracteres y los permisos de autor al modificar y eliminar.
- **¿Las respuestas de verificados también van primero?** Habría que decidir si el orden del feed se aplica dentro de cada hilo.

### Seguir usuarios (operación 6)

- **Estructura para la relación:** una lista de pares en memoria, o una lista de seguidos por usuario.
- Al eliminar una cuenta hay que limpiar también sus relaciones.

### Eliminar una cuenta (operación 7)

- **¿Qué pasa con sus tweets?** Borrarlos, o conservarlos como "usuario eliminado".
- **¿Qué pasa en FameCheck?** La cuenta sigue registrada allá; habría que ver si el servicio ofrece dar de baja una sola cuenta (hoy solo existe el reset completo).
- **Riesgo que ya existe en el diseño actual:** el `Tweet` guarda el autor como texto, sin un identificador de usuario. Si se elimina una cuenta y alguien se registra después con el mismo nombre, esa persona quedaría como autora de los tweets viejos y podría modificarlos o borrarlos. Una posible salida es darle a cada usuario un id numérico, y que el tweet guarde ese id.

## 3. Evolutivos que surgen de las limitaciones conocidas

| Limitación actual | Evolución esperada | Diseño / observaciones |
|-------------------|--------------------|------------------------|
| Los datos se guardan solo al salir por el menú; un cierre inesperado los pierde. | Guardar en cada operación (o periódicamente). | Reescribir el archivo completo tras cada cambio es simple pero recorre todo el archivo cada vez. La consigna pide que los recorridos de archivos sean mínimos y evitar archivos temporales, así que conviene evaluar agregar registros al final (`"ab"`) para publicaciones y modificar en el lugar con `fseek` para ediciones. Con el feed ordenado, el orden en el archivo ya no importa, porque se reordena al cargar. |
| Un tweet largo se trunca sin avisar y el registro duplicado no muestra mensaje. | Validar y avisar al usuario. | Devolver un código nuevo (por ejemplo `TWEET_MUY_LARGO`) desde `tweetPublicar` y que `main` muestre el mensaje. Para `USER_DUPLICADO` alcanza con revisar el retorno en `main`. |
| ~~Bytes basura en `users.bin` y uso de `strcpy` sin límite.~~ | **Resuelto en el Sprint 1** (`8f13ca8`). | — |
| Cambiar un `struct` deja inservibles los `.bin` anteriores. | Poder agregar campos sin perder los datos. | Guardar al principio del archivo un número de versión, o escribir una migración que lea el formato viejo y escriba el nuevo. |
| Sin conexión, publicar un tweet de un verificado puede demorar hasta 10 segundos. | Que publicar no espere al servicio. | Bajar el tiempo de espera, o publicar primero y reportar después (por ejemplo, marcando el tweet como pendiente y reportando los pendientes al salir). |

---

# Uso de Inteligencia Artificial

## 1. Cómo decidimos usar la IA

La consigna permite usar IA y a la vez pide que aprendamos a analizar un problema, decidir, probar y defender lo que hicimos. Por eso, en lugar de usarla para que resuelva el trabajo, la usamos donde nos ayuda a avanzar sin sacarnos el razonamiento:

- **Como interlocutora**, para contrastar ideas y encontrar puntos que no habíamos visto. Las decisiones de diseño siguieron siendo del equipo: la IA aportaba opciones y contraargumentos, y nosotros elegíamos.
- **Como ayuda en tareas repetitivas**, que llevan tiempo pero no aportan aprendizaje (tipear un `.h`, armar un banner, escribir la estructura de un archivo de pruebas).
- **Como apoyo para entender y ordenar**, tanto la consigna como nuestra propia documentación.

Más allá de algunos arreglos y revisiones puntuales del código, los usos principales fueron los que siguen.

## 2. Usos principales

### 2.1 Consultas sobre los archivos del trabajo práctico y el flujo de trabajo

La usamos para entender qué pedía la consigna y cómo cumplirla. Por ejemplo:

- ¿Qué debo incluir en la bitácora?
- ¿Cómo se usa GitFlow y cómo lo aplicamos a nuestro repositorio (rama `develop`, ramas de funcionalidad y de arreglo, rama de release y tags por sprint)?
- ¿Cómo se entrega el trabajo?
- ¿Falta alguna funcionalidad para cubrir el sprint?
- ¿Qué orden de implementación me recomendas seguir?

### 2.2 Decisiones de diseño

La usamos para contrastar ideas y ordenar el razonamiento antes de decidir. Por ejemplo:

- "Pensé este flujo para este proceso, ¿qué te parece?"
- "¿Qué funciones debería desarrollar para cubrir esta parte?"
- "¿Me conviene hacer una librería con funciones de lista estándar, o réplicas de las funciones personalizadas para tweets y usuarios?"
- "¿Está bien este código? ¿Detectás algún bug?"
- "¿Cómo guardo la fecha y hora del momento?"

También consultamos con IA las operaciones que podrían pedirnos más adelante. En la tabla de operaciones futuras, la columna "Origen" indica cuáles ideas fueron nuestras y cuáles surgieron de esa consulta.

### 2.3 Generación de código de apoyo y revisión

La usamos para no tipear cosas repetitivas que no son la lógica propia del programa. Por ejemplo:

- "Hice este `.c`, armame el `.h` con todas las firmas de las funciones."
- Ayuda con la interfaz de `main`, como el logo del banner.
- "Armame un archivo para testear todas estas funciones."
- Revisión de errores y verificación de compilado y ejecución con detección de bugs.

### 2.4 Configuración del entorno

En el Sprint 2 hubo que instalar libcurl. La IA siguió el instructivo de la cátedra: detectó que el gcc de Code::Blocks ya estaba instalado (MinGW-w64 de 64 bits), descargó curl de 64 bits a `C:\libs\curl`, agregó las rutas al `PATH` y verificó que el proyecto compilara.

### 2.5 Documentación: bitácora y hoja de tests

También la usamos para armar la documentación del trabajo: la descripción del funcionamiento del programa, el registro de problemas y decisiones, las operaciones futuras, el uso de IA y la hoja de tests. La forma de trabajo fue la siguiente:

- Le pasamos el código, los archivos de datos, el historial de commits y las consignas.
- A partir de eso, la IA armó borradores de cada sección, con el formato que pide la consigna (por ejemplo, para cada decisión: problema, alternativas, qué elegimos, por qué, qué observamos y si cambió).
- En el Sprint 2, para completar la columna "Salida obtenida" de la hoja de tests, ejecutó los tres programas de prueba y copió la salida real. Así apareció que `tweetsAbrir` no devolvía lo que esperaba el caso del Sprint 1 (problema 20).
- Lo que no se podía saber leyendo el código (los motivos, las alternativas que discutimos, cómo detectamos cada problema, los planteos individuales) quedó marcado como pendiente para que lo completemos nosotros, en lugar de inventarlo.
- Cada sección la revisamos y ajustamos entre todos antes de incorporarla, porque la defensa individual exige poder explicar cualquier punto de la bitácora.

## 3. Casos en que la IA recomendó algo distinto a lo que resultó mejor

### 3.1 Sprint 1: lista genérica o listas tipadas

**Qué consultamos.** Cuando estábamos decidiendo cómo guardar usuarios y tweets, le preguntamos a la IA por listas enlazadas.

**Qué respondió.** Nos recomendó no hacer una lista genérica con `void *` y armar en su lugar dos listas tipadas, una para `Usuario` y otra para `Tweet`, aunque hubiera que duplicar código (dos "insertar", dos "eliminar", etc.). Sus argumentos: la genérica obliga a castear en cada lugar donde se lee un dato, el compilador no avisa si se castea al tipo equivocado (queda un error que solo aparece al ejecutar), hace falta pasar funciones de comparación como parámetro, y las listas tipadas son más simples de leer y de defender.

**Qué evaluamos.** Los riesgos que mencionó eran reales, pero no nos convencía el costo de duplicar toda la lógica de la lista por cada entidad, y menos sabiendo que van a aparecer nuevos pedidos en los próximos sprints. Lo discutimos entre nosotros y decidimos probar la lista genérica en el código.

**Qué observamos.**

- La lista genérica resultó práctica: hay una sola implementación (`lista.c`) que usan tanto usuarios como tweets, y cada módulo le pasa sus propias funciones de comparación y de acción.
- Nuestra versión copia el dato dentro del nodo (`memcpy` con `dataSize`), así que quien la usa no tiene que reservar ni liberar memoria por su cuenta.
- El casteo quedó acotado a los módulos de cada entidad (`user.c`, `tweet.c`, `famecheck.c`).
- En el Sprint 2 la misma lista resolvió el top 5, el feed ordenado y el recorrido, agregando solo dos funciones genéricas.

**Qué solución adoptamos.** Nos quedamos con la lista doblemente enlazada genérica como base común. La recomendación de la IA nos sirvió para tener presentes los riesgos del casteo, pero la decisión final fue del equipo, y para nuestro caso resultó mejor que la alternativa que nos sugirió.

### 3.2 Sprint 2: dirección del orden por fecha en el feed

**Qué consultamos.** Cómo ordenar el feed con los verificados primero y el resto por fecha.

**Qué respondió.** Propuso ordenar cada grupo del más viejo al más nuevo, con el argumento de que la consigna dice que el resto del feed "sigue ordenado como hasta ahora" y en el Sprint 1 el feed se mostraba en orden de publicación.

**Qué evaluamos.** La interpretación era defendible, pero en una red social lo esperable es ver primero lo último que se publicó, y la consigna solo dice "ordenado por fecha/hora de publicación", sin indicar la dirección.

**Qué solución adoptamos.** Del más reciente al más viejo. Lo dejamos registrado como un cambio respecto del Sprint 1 (decisión 7).

## 4. Ventajas que le vimos

- **Ahorro de tiempo en lo mecánico.** Headers, banner, estructura de pruebas, instalación de librerías y armado del formato de la documentación nos dejaron más tiempo para pensar las decisiones de diseño.
- **Contraste de ideas antes de decidir.** Nos sirvió tener alguien con quien discutir alternativas, por ejemplo entre una librería genérica de listas y funciones repetidas por entidad, y ver consecuencias que no habíamos pensado.
- **Una segunda lectura del código.** Al revisar y documentar aparecieron errores y limitaciones que conviene tener anotadas, que después pudimos convertir en casos de prueba y en evolutivos.
- **Interpretación de la consigna.** Nos ayudó a no dejar afuera nada de lo que se pide entregar (bitácora, hoja de tests, tags, ramas, formato de entrega).
- **Documentación más ordenada y consistente.** Mantener un mismo formato en todas las secciones nos evitó rehacer el trabajo al juntarlo en un solo documento.

## 5. Criterios con los que la usamos

- **La decisión final es del equipo.** La IA propone; nosotros evaluamos, aceptamos, ajustamos o descartamos.
- **Todo lo que entra al proyecto tiene que poder explicarse.** Si no entendemos una propuesta o una porción de código, no la incorporamos. Por eso, cuando algo no nos cerraba, le pedimos que lo explicara antes de usarlo.
- **Lo generado se verifica.** Compilamos, probamos y contrastamos con la consigna antes de darlo por bueno, porque una respuesta puede salir incompleta o con errores.
- **La documentación cuenta lo que realmente pasó.** Los motivos y las observaciones los escribimos nosotros; la IA ayuda a redactarlos y ordenarlos.
