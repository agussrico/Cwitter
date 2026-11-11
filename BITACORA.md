# Trabajo Práctico AyED – Cwitter – Sprint 3

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
- Sprint 2 – 12/10/2026 (tag `v2-procesamiento`)
- Sprint 3 – 11/11/2026 (tag `v3-final`)

---

# Descripción del funcionamiento del programa

## 1. Resumen

Cwitter es un programa de consola en C que permite registrar usuarios, iniciar y cerrar sesión, publicar, modificar y eliminar tweets, ver el feed general y buscar tweets por texto. Los usuarios y los tweets se guardan en archivos binarios, así que se conservan entre ejecuciones.

En el Sprint 2 se sumó la integración con **FameCheck**, un servicio externo que decide si una cuenta queda verificada. Cwitter le consulta la verificación de un usuario, le reporta cada publicación de una cuenta verificada y le pide la actividad para armar el **top 5 de verificados con más publicaciones**. Además, el **feed muestra primero los tweets de cuentas verificadas** y después el resto, del más reciente al más viejo, y se puede **recorrer de a un tweet por vez**, hacia adelante y hacia atrás. Si FameCheck no responde al reportar, la publicación se hace igual y el fallo queda registrado en un **archivo de log**.

En el **Sprint 3** se amplió el límite de cada publicación de 140 a **280 caracteres** y se incorporaron **hilos de conversación sin límite de profundidad**: cualquier usuario puede responder a un tweet principal o a una respuesta dentro de un hilo, formando árboles de respuestas anidados. Desde el menú principal se puede desplegar el **hilo completo de un tweet (`V`)**, diferenciando visualmente los niveles de profundidad con sangrías (`|--`), y responder directamente desde allí o por ID (`R`). La relación padre-hijo entre tweets y respuestas **se persiste en `tweets.bin`** mediante un recorrido en preorden y se reconstruye al abrir el programa. Finalmente, para sostener el crecimiento del volumen de publicaciones, se implementó un **Árbol Binario de Búsqueda genérico (`tArbol`)** y un **índice invertido en memoria por palabra (`tIndice`)**, que reemplaza el recorrido lineal de todo el feed y los hilos al buscar texto por una búsqueda logarítmica sobre el árbol.

Los datos se mantienen en memoria en listas doblemente enlazadas genéricas (una de usuarios, una de tweets —donde cada tweet contiene a su vez la lista de sus respuestas directas— y, mientras se muestra, una para el top 5) y en un árbol binario de búsqueda para el índice de palabras. Al iniciar se cargan desde disco y al salir se guardan.

## 2. Organización del código

| Archivo | Responsabilidad |
|---------|-----------------|
| `include/lista.h` / `src/lista.c` | Lista doblemente enlazada genérica (`void *`). No conoce ni usuarios ni tweets. |
| `include/arbol.h` / `src/arbol.c` | Árbol binario de búsqueda (ABB) genérico (`void *`). No conoce las entidades que almacena. |
| `include/indice.h` / `src/indice.c` | Índice de búsqueda por palabra sobre `tArbol`: normaliza cada palabra a minúsculas y guarda la `tLista` de punteros `tNodo *` de los tweets y respuestas que la contienen. |
| `include/user.h` / `src/user.c` | Entidad User: registro, inicio de sesión, carga y guardado de `users.bin`. |
| `include/tweet.h` / `src/tweet.c` | Entidad Tweet: publicar tweets y respuestas en hilos (y reportar a FameCheck), modificar, eliminar en cascada, feed ordenado, búsqueda con y sin índice, carga y guardado recursivo de `tweets.bin`. |
| `include/famecheck.h` / `src/famecheck.c` | Comunicación con FameCheck: verificar una cuenta, reportar una publicación, obtener el top 5 y registrar los fallos en `logs.log`. |
| `include/cJSON.h` / `src/cJSON.c` | Librería externa (DaveGamble/cJSON) para armar y leer JSON. Se compila junto con el proyecto. |
| `src/main.c` | Interfaz de consola (menús), recorrido del feed de a un tweet, visualización de hilos y flujo general del programa. |
| `config.ini` | Configuración: `linux=true` o `linux=false`, para elegir el comando de limpieza de pantalla (`clear` o `cls`) y si se usan colores. |
| `apikey.key` | Clave de API de FameCheck, en una sola línea. Está en el `.gitignore`: cada integrante tiene la suya en su copia local. |
| `Cwitter.cbp` | Proyecto de Code::Blocks. Actualizado con las rutas `include/` y `src/` y las unidades `arbol.c` e `indice.c`. |
| `tests/test_users.c` / `tests/test_tweets.c` / `tests/test_famecheck.c` | Programas de prueba de cada módulo (ver sección 8). |

Las dependencias principales van en un solo sentido: `main` usa `user`, `tweet`, `indice` y `famecheck`; `tweet` usa `famecheck` para reportar cada publicación e `indice` para mantener indexadas las palabras; `indice` usa `arbol` y `lista`; `famecheck` usa `lista`, `curl` y `cJSON`. Ni `lista` ni `arbol` dependen de ningún módulo de dominio.

## 3. Estructuras de datos genéricas (`lista.c`, `arbol.c` e `indice.c`)

### 3.1 La lista genérica (`lista.c`)

Cada nodo guarda un puntero `void *data` a una copia del dato, el tamaño de ese dato (`dataSize`) y punteros al nodo siguiente y al anterior. La lista mantiene punteros a `inicio` y `fin` y un contador `cant`.

Operaciones disponibles:

- `listaInicializar`, `listaVacia`, `listaLlena`, `listaVaciar`.
- `listaInsertar`: inserta siempre al final. Reserva memoria para el nodo y para la copia del dato (`memcpy`).
- **`listaInsertarOrdenado` (nueva en el Sprint 2):** recorre la lista desde el inicio mientras `cmp(nuevo, actual) > 0` y deja el nuevo nodo justo antes del primero donde eso deja de cumplirse (o al final). Se usa para el top 5, para el feed y para las respuestas dentro de cada hilo.
- **`listaEliminarUltimo` (nueva en el Sprint 2):** desenlaza y libera el último nodo. Se usa para recortar el top 5.
- `listaBuscarYDevolver`: devuelve el primer nodo que cumple una condición, o `NULL`.
- `listaBuscarYAccion`: aplica una acción a todos los nodos que cumplen una condición y devuelve cuántos fueron.
- `listaRecorrerYAccion`: aplica una acción a todos los nodos.
- `listaEliminarNodo`: busca un nodo con una condición y lo desenlaza. Contempla eliminar el primero, el último y uno intermedio, y libera nodo y dato.

La condición de búsqueda se pasa como función de comparación (`int (*cmp)(void *, void *)`) y las acciones como función de acción (`void (*accion)(void *)`). Por eso una misma lista sirve para usuarios, tweets, respuestas de hilos, entradas del índice y el top 5: cada módulo aporta sus propias funciones (`cmpUser`, `cmpTweetId`, `cmpTweetTexto`, `cmpTweetFeed`, `famecheckCmpActividad`, `userImprimir`, `tweetImprimir`, etc.).

En `listaInsertarOrdenado` el comparador indica posición: un valor negativo significa que el primer dato va antes que el segundo, y uno positivo que va después. Para ordenar de mayor a menor (por ejemplo, más publicaciones primero) el comparador devuelve negativo cuando el primero es más grande.

`listaLlena` prueba si se puede reservar memoria para un nodo y un dato. Si no se puede, las funciones de inserción devuelven `LISTA_LLENA`. No hay un máximo fijo de usuarios ni de tweets; el único límite es la memoria disponible.

### 3.2 El árbol binario de búsqueda genérico (`arbol.c`, nuevo en el Sprint 3)

Cada nodo (`tNodoArbol`) guarda un puntero `void *data` a una copia del dato, su tamaño (`dataSize`) y los punteros `izq` y `der` a sus hijos. La estructura `tArbol` mantiene el puntero a `raiz` y la cantidad de nodos `cant`.

Operaciones disponibles:

- `arbolInicializar`, `arbolVacio`, `arbolLleno`, `arbolAltura`.
- `arbolInsertar`: desciende iterativamente con un doble puntero (`tNodoArbol **actual`) comparando con `cmp` (`< 0` va a `izq`, `> 0` va a `der`). No admite claves duplicadas (`CLAVE_DUPLICADA`).
- `arbolBuscar`: desciende desde la raíz comparando con `cmp` y devuelve el puntero `void *data` del nodo encontrado, o `NULL` si llega a una hoja sin coincidencia.
- `arbolEliminar`: busca el nodo a eliminar; si tiene un hijo o ninguno lo puentea directamente, y si tiene dos hijos lo reemplaza por el mayor del subárbol izquierdo. Acepta un puntero a función `void (*liberar)(void *)` opcional por si el dato alojado tiene memoria dinámica interna (como una `tLista`).
- `arbolRecorrerInorden`, `arbolRecorrerPreorden`, `arbolRecorrerPosorden` y `arbolVaciar` (libera en posorden).

### 3.3 El índice de búsqueda por palabra (`indice.c`, nuevo en el Sprint 3)

Construido sobre `tArbol` y `tLista`:

- Cada nodo del árbol es una `tEntradaIndice` cuya clave es una `palabra` (normalizada a minúsculas) y cuyo valor es una `tLista nodos` donde cada elemento guarda un puntero `tNodo *` al nodo del tweet o respuesta que contiene esa palabra.
- `indiceCargar`: hace una copia dinámica del mensaje, separa las palabras con `strtok` usando `SEPARADORES` (`" \t\n.,;:!?\"'()[]{}-"`), las pasa a minúsculas y agrega el `tNodo *` en la entrada del árbol (evitando duplicar el mismo nodo si la palabra aparece repetida en el mismo tweet).
- `indiceQuitar`: recorre las palabras del mensaje y quita el `tNodo *` de la lista de cada palabra; si la lista de una palabra queda vacía, elimina esa palabra del árbol (`arbolEliminar`).
- `indiceBuscar`: normaliza la palabra buscada, la busca en el ABB con `arbolBuscar` y devuelve la `tLista *` de nodos asociados (o `NULL` si no existe).

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
    unsigned int idPadre;        /* nuevo en el Sprint 3: 0 si es tweet principal; id del padre si es respuesta */
    char autor[20];
    char mensaje[MAX_TWEET + 1]; /* MAX_TWEET = 280 en el Sprint 3, más el '\0' */
    time_t fecha;                /* momento de publicación */
    int verificado;              /* si el autor estaba verificado al publicar */
    int reportadoAFamecheck;     /* 1 si FameCheck aceptó el reporte */
    tLista hilo;                 /* nuevo en el Sprint 3: lista de respuestas directas a este tweet */
} Tweet;
```

Cada tweet tiene un `id` numérico único. El autor se guarda como texto (el nombre de usuario), sin referencia a la lista de usuarios. Por eso el tweet guarda también si su autor estaba verificado: el feed se ordena comparando solo dos tweets, sin tener que buscar al autor en la lista de usuarios.

En el Sprint 3 se subió `MAX_TWEET` a 280 y se agregaron dos campos para modelar los hilos: `idPadre` (`0` para los tweets raíz del feed principal, o el `id` del tweet/respuesta al que responde) y `tLista hilo` (la lista doblemente enlazada de sus respuestas directas). Como cada respuesta es a su vez un `Tweet` con su propia `tLista hilo`, las respuestas pueden anidarse sin límite de profundidad.

### ActividadFame

```c
typedef struct {
    char nombre[20];
    int cantidadCweets;
} ActividadFame;
```

Representa una cuenta verificada con su cantidad de publicaciones, tal como la devuelve FameCheck. Solo vive en memoria, mientras se arma y se muestra el top 5.

### tIndice y tEntradaIndice (nuevas en el Sprint 3)

```c
typedef struct {
    tArbol arbol;           /* árbol de tEntradaIndice ordenado por palabra */
    int cantRegistros;      /* cantidad de tweets y respuestas indexados */
} tIndice;

typedef struct {
    char palabra[MAX_PALABRA];  /* palabra en minúsculas (clave del árbol) */
    tLista nodos;               /* lista de tNodo* de los tweets que contienen la palabra */
} tEntradaIndice;
```

Solo viven en memoria durante la ejecución: se construyen al cargar `tweets.bin` (`tweetsAbrirConIndice`) y se actualizan en cada publicación, modificación o eliminación.

## 5. Flujo del programa (`main.c`)

1. Se lee `config.ini` (`leerConfig`) y se inicia FameCheck una sola vez (`famecheckIniciar`: inicializa curl y lee `apikey.key`). Si falla, se avisa y el programa sigue funcionando: las operaciones que usan FameCheck devuelven error. Después se inicializan las listas y el índice (`indiceInicializar`) y se cargan desde disco (`tweetsAbrirConIndice`, `usersAbrir`).
2. **Menú sin sesión:** iniciar sesión, registrarse o salir. Al registrarse, se revisa el retorno de `userRegistrar` y se informa en pantalla si el usuario fue registrado con éxito o si el nombre ya existe (`USER_DUPLICADO`). Sin sesión iniciada no se puede publicar ni ver el feed.
3. **Menú con sesión** (se muestra mientras `pUsuarioActual != NULL`): arriba se indica si el usuario está verificado y se muestra el tweet actual del feed ("Tweet X de Y") junto con la cantidad de respuestas directas (`Respuestas: N`). Las opciones son: `A`/`D` tweet anterior/siguiente, `R` responder a un tweet o respuesta por su ID, `1` ver feed completo, `2` publicar tweet principal, `3` buscar por palabra (usando el índice ABB), `V` ver el hilo completo del tweet actual, `4` modificar, `5` eliminar, `6` top 5 de verificados, `7` cerrar sesión y `0` verificar la cuenta con FameCheck.
4. Al elegir Salir en el menú sin sesión, se guardan usuarios y tweets (con todos sus hilos) en disco, se libera la memoria de las listas y del índice (`indiceVaciar`), se cierra curl (`famecheckFinalizar`) y termina el programa.

La sesión activa se representa con un puntero (`User *pUsuarioActual`) al dato del usuario dentro de la lista. Cerrar sesión lo pone en `NULL` y vuelve al primer menú. Para el recorrido del feed, `main` guarda un puntero al nodo del tweet actual (`nodoTweet`) y su posición (`posTweet`).

En Linux (`linux=true`) los menús usan colores con códigos de escape ANSI; en Windows se muestran sin colores, porque la consola clásica no siempre los interpreta.

## 6. Funcionalidades

**Registro (`userRegistrar`).** Busca el nombre en la lista de usuarios. Si no existe, inserta el nuevo usuario (con `verficado = 0`). Si existe, devuelve `USER_DUPLICADO`. Los campos se inicializan en cero y los textos se copian con `strncpy`, con un máximo de 19 caracteres.

**Inicio de sesión (`userIniciarSesion`).** Busca al usuario por nombre y compara la contraseña. Devuelve un puntero al usuario si coincide y `NULL` si el usuario no existe o la contraseña es incorrecta (no distingue los dos casos).

**Cierre de sesión.** Opción 7 del menú con sesión: descarta el usuario actual.

**Verificación de cuenta (`famecheckVerificarCuenta`).** Opción 0 del menú con sesión. Hace un `POST /api/Cwitter/Usuario` con el nombre y guarda en `verficado` lo que responde FameCheck. Si la cuenta ya existía en FameCheck, el servicio responde 200 con el mismo resultado, así que consultar dos veces no duplica nada. Si el servicio no responde, se informa el error y el usuario queda como estaba.

**Publicar y responder (`tweetPublicar` / `tweetPublicarConIndice`).** En el Sprint 3 recibe también `unsigned int idPadre` (`0` para un tweet principal en el feed, o el ID del tweet/respuesta al que se responde). Inicializa la sub-lista `nuevoTweet.hilo`, copia el mensaje truncado a **280 caracteres** (`MAX_TWEET`) y reporta a FameCheck si el autor está verificado. Si `idPadre == 0`, inserta ordenado en el `feed`; si `idPadre != 0`, busca al padre en toda la jerarquía con `tweetBuscarPorId` (si no existe devuelve `NODO_NO_ENCONTRADO`) e inserta la respuesta ordenada dentro de `&padre->hilo`. La versión `tweetPublicarConIndice` además indexa las palabras del nuevo tweet en `tIndice`.

**Reporte a FameCheck (`famecheckReportarPublicacion`).** Hace un `POST /api/Cwitter/Cweet` con el usuario y el mensaje. Solo se llama para autores verificados, porque FameCheck rechaza los reportes de cuentas no verificadas. FameCheck responde 204 cuando acepta el reporte (la documentación dice 201, ver problema 18), y en ese caso el tweet queda con `reportadoAFamecheck = 1`.

**Log de fallos (`famecheckRegistrarLog`).** Si el reporte falla, se agrega una línea a `logs.log` con la fecha y hora, el id del tweet, el autor y el motivo (`FAMECHECK_ERROR_RED`, `FAMECHECK_ERROR_RESPUESTA`, etc.). No se reintenta el envío: como pide la consigna, alcanza con dejar constancia.

**Búsqueda por ID en hilos (`tweetBuscarPorId`, nueva en el Sprint 3).** Recorre la lista recibida y, por cada tweet, si su `id` no coincide, se invoca recursivamente sobre `&t->hilo`. Esto permite localizar cualquier tweet o respuesta sin importar en qué nivel de profundidad del hilo se encuentre.

**Visualización de hilos (`tweetImprimirHiloRecursivo`, nueva en el Sprint 3).** Opción `V` del menú. Imprime el tweet actual del carrusel y recorre recursivamente su lista `hilo` aumentando el parámetro `nivel`. En cada nivel agrega `nivel * 4` espacios de sangría seguidos del prefijo `|--`, diferenciando visualmente qué responde a qué. Desde esa misma pantalla ofrece responder al tweet raíz (ingresando `0`) o a cualquier respuesta interna del hilo (ingresando su ID visible en pantalla).

**Modificar (`tweetModificar` / `tweetModificarConIndice`).** Busca el tweet o respuesta por ID en toda la jerarquía (`tweetBuscarPorId` / `tweetBuscarNodoPorId`). Devuelve `NODO_NO_ENCONTRADO` si no existe y `SIN_PERMISO` si el autor no es quien intenta modificarlo. En `tweetModificarConIndice`, antes de sobrescribir el mensaje (hasta 280 caracteres), quita del índice ABB las palabras del texto anterior (`indiceQuitar`) y luego indexa las del texto nuevo (`indiceCargar`).

**Eliminar (`tweetEliminar` / `tweetEliminarConIndice` y `tweetVaciarHilo`).** Valida existencia y autoría en cualquier nivel de profundidad. Si el tweet a eliminar es una respuesta (`idPadre != 0`), localiza a su padre para desenlazar el nodo de `&tPadre->hilo` en lugar de `feed`. Antes de borrar el nodo, desindexa (`tweetDesindexarLista`) y libera recursivamente todas las respuestas anidadas debajo de él (`tweetVaciarHilo(&t->hilo)`), evitando fugas de memoria y punteros colgantes en el índice.

**Feed (`tweetsMostrarFeed`).** Recorre la lista principal e imprime los tweets raíz (los que tienen `idPadre == 0`) junto con la cantidad de respuestas directas (`Respuestas: N`). Como la lista se mantiene ordenada (`cmpTweetFeed`), aparecen primero los tweets de cuentas verificadas y después el resto; dentro de cada grupo, del más reciente al más viejo.

**Recorrido de a un tweet.** En el menú con sesión se ve un tweet por vez. `D` pasa al siguiente (`nodo->sig`) y `A` vuelve al anterior (`nodo->ant`), sin recorrer el feed desde el principio. En los extremos no se mueve. Después de publicar o eliminar, el recorrido vuelve al primer tweet.

**Top 5 de verificados (`famecheckObtenerTop5`).** Opción 6. Hace un `GET /api/Cwitter/Usuario`, que devuelve las cuentas verificadas con su cantidad de publicaciones, sin ordenar. Cada cuenta se inserta ordenada en una lista (`famecheckInsertarEnTop`) y, si la lista pasa de 5, se elimina el último. Queda de mayor a menor cantidad; los empates se ordenan alfabéticamente por nombre.

**Búsqueda por palabra (`tweetBuscarYMostrarConIndice`, actualizada en el Sprint 3).** Toma la palabra buscada, la normaliza a minúsculas y consulta el índice ABB (`indiceBuscar`). Si la palabra está en el árbol, recorre directamente su lista de nodos asociados e imprime cada tweet (indicando `(respuesta al tweet ID: X)` cuando se trata de una respuesta dentro de un hilo), sin recorrer el feed.

**Identificación de tweets.** Los ids se generan con una variable estática `ultimo_id` que se incrementa en cada publicación. Al cargar el archivo, `ultimo_id` se actualiza al id más alto encontrado, para que los nuevos tweets no repitan ids existentes.

## 7. Persistencia

- **Formato:** archivos binarios, uno para cada entidad: `users.bin` y `tweets.bin`.
- **Contenido:** la secuencia de `struct` completos, uno detrás de otro, escritos con `fwrite` y leídos con `fread`, sin encabezado ni separadores. Con el compilador del proyecto (MinGW-w64 de 64 bits), `sizeof(User)` es 48 bytes y `sizeof(Tweet)` pasó de 184 a **344 bytes** en el Sprint 3 (por el aumento de `mensaje` a 281 bytes, el campo `idPadre` y la estructura `tLista hilo`).
- **Cuándo se guarda:** al salir del programa (opción 3 del menú principal), sobrescribiendo el archivo completo (`"wb"`).
- **Cómo se guardan y reconstruyen los hilos en `tweets.bin`:**
  - **Al guardar (`tweetsGuardar` + `tweetGuardarHilo`):** como cada `Tweet` tiene una sub-lista `hilo` con punteros en RAM (`inicio` y `fin`) que pierden validez al cerrar el programa, no alcanza con recorrer solo el nivel principal. `tweetGuardarHilo` recorre recursivamente en **preorden**: primero escribe en disco el `Tweet` padre con `fwrite` y enseguida se llama a sí misma sobre `&t->hilo` para escribir todas sus respuestas hijas antes de pasar al siguiente hermano.
  - **Al cargar (`tweetsAbrir` / `tweetsAbrirConIndice`):** se lee cada `Tweet` con `fread(&aux, sizeof(Tweet), 1, pf)`. Lo primero que se hace es `listaInicializar(&aux.hilo)` para limpiar los punteros viejos que venían grabados desde la ejecución anterior. Luego, si `aux.idPadre == 0`, se inserta ordenado en el `feed` principal; si `aux.idPadre != 0`, como el archivo fue escrito en preorden, su tweet padre **ya fue cargado previamente**, por lo que se lo busca con `tweetBuscarPorId(feed, aux.idPadre)` y se inserta la respuesta dentro de `&padre->hilo`. Finalmente, `tweetsAbrirConIndice` recorre la jerarquía cargada (`tweetIndexarLista`) para poblar el índice ABB en memoria.
- **Archivos de sprints anteriores:** como `Tweet` cambió de tamaño en el Sprint 3, los `tweets.bin` generados en el Sprint 1 o 2 son incompatibles y deben borrarse.
- **Verificación persistente:** `verficado` forma parte de `User`, así que se guarda y se recupera con el resto del usuario.

Archivos de texto, separados de los binarios:

- **`logs.log`:** se abre en modo `"a"` (agregar al final) cada vez que falla un reporte, se escribe una línea y se cierra. No se lee desde el programa: es para revisarlo a mano.
- **`apikey.key`:** se lee una vez al iniciar.

Cada archivo se maneja solo con funciones de su tipo: los binarios con `fopen` `"rb"`/`"wb"`, `fread` y `fwrite`, y los de texto con `fgets` y `fprintf`. Nunca se mezclan sobre un mismo archivo.

## 8. Pruebas realizadas hasta ahora

- **`test_users.c`:** carga inicial, registro de usuarios, registro de un usuario duplicado, login correcto, login con contraseña incorrecta, login con usuario inexistente, guardado, vaciado de la lista y recarga para verificar la persistencia.
- **`test_tweets.c`:** plataforma vacía, publicación de tweets de usuarios verificados y no verificados, feed con la mezcla de ambos, un tweet más viejo que queda al final, recorrido hacia adelante y hacia atrás varias veces, búsqueda existente e inexistente, eliminación del primero, del último y de uno intermedio, intento de borrar un tweet ajeno, modificación de un tweet propio, guardado, vaciado y recarga conservando el orden. En el Sprint 3 se sumaron 7 casos adicionales de borde (casos 18 a 24): publicación de más de 280 caracteres verificando el truncado exacto en 280, creación de hilos de 3 niveles de profundidad y múltiples respuestas por nivel, intento de responder a un ID inexistente (`NODO_NO_ENCONTRADO`), búsqueda con el índice ABB (`tweetBuscarYMostrarConIndice`) encontrando palabras en respuestas profundas sin importar mayúsculas/minúsculas, bloqueo con `SIN_PERMISO` y reindexación al modificar una respuesta en un hilo, eliminación en cascada de un sub-hilo completo verificando que sus descendientes desaparecen tanto de memoria como del índice ABB, y persistencia completa de hilos multinivel en `tweets.bin`.
- **`test_famecheck.c`:** inicio del servicio, una cuenta que se verifica y otra que no, verificar dos veces la misma cuenta, reportar la publicación de un verificado y la de uno no verificado, reportar sin conexión (simulada con un proxy inexistente) y revisar el log, top 5 con menos de 5 verificados, top 5 con empates, top 5 real del servicio y verificación que se conserva al reiniciar.

La hoja de tests completa, con situación, operación, resultado esperado y salida obtenida de cada caso, está en `TESTS.md`.

## 9. Códigos de retorno

| Constante | Valor | Significado |
|-----------|-------|-------------|
| `TODO_OK` | 0 | Operación exitosa |
| `LISTA_LLENA` | 3 | No hay memoria para insertar en lista |
| `NODO_NO_ENCONTRADO` | 4 | No existe el elemento buscado |
| `ERROR_AP_ARCH` | 5 | No se pudo abrir el archivo |
| `USER_DUPLICADO` | 6 | El nombre de usuario ya existe |
| `SIN_PERMISO` | 7 | El tweet pertenece a otro usuario |
| `ARBOL_LLENO` | 8 | No hay memoria para insertar en el ABB |
| `CLAVE_DUPLICADA` | 9 | La clave ya existe en el ABB |
| `INDICE_ERROR` | -1 | No hay memoria para seguir indexando palabras |
| `FAMECHECK_OK` | 0 | FameCheck respondió lo esperado |
| `FAMECHECK_ERROR_RED` | -1 | No se pudo contactar al servicio (sin conexión, timeout) |
| `FAMECHECK_ERROR_RESPUESTA` | -2 | El servicio respondió algo inesperado (400, 401, JSON inválido) |
| `FAMECHECK_ERROR_MEMORIA` | -3 | No se pudo reservar memoria para armar el pedido o la respuesta |
| `FAMECHECK_ERROR_CONFIG` | -4 | No se inició FameCheck o falta la clave en `apikey.key` |

## 10. Limitaciones conocidas de esta versión

- **Guardado solo al salir por el menú.** Si el programa se cierra de otra forma (ventana, corte de luz), se pierden los cambios de esa sesión.
- **Truncado silencioso:** un tweet de más de 280 caracteres se corta sin avisar al usuario.
- ~~**Registro duplicado sin aviso:** `main` no muestra un mensaje cuando `userRegistrar` devuelve `USER_DUPLICADO`.~~ **(resuelto en el Sprint 3, commit `ac68387`)**
- **Entrada de datos:** en el menú sin sesión, ingresar letras donde se espera un número deja al programa en un bucle. En el menú con sesión ya no pasa, porque la opción se lee como carácter.
- ~~**Búsqueda:** distingue mayúsculas y minúsculas.~~ **(resuelto en el Sprint 3 con `normalizar` en `indice.c`, que pasa las palabras a minúsculas)**
- **Búsqueda por palabras completas en el índice:** el índice ABB separa el texto por signos de puntuación y espacios (`strtok`), por lo que busca por palabra completa (la primera palabra ingresada) y no por subcadena parcial (por ejemplo, buscar `"hol"` no encuentra `"hola"`).
- **Árbol del índice sin autobalanceo (AVL):** el ABB del índice inserta las palabras en el orden en que aparecen; si se insertaran en orden estrictamente alfabético podría desbalancearse (ver análisis detallado en Decisión 14).
- **Verificarse después de publicar:** los tweets guardan si el autor estaba verificado al momento de publicar. Si alguien se verifica después, sus tweets anteriores no suben en el feed.
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

### Sprint 3

| Fecha | Quién | Qué se hizo |
|-------|-------|-------------|
| 28/10 | agussrico | Se reorganiza el repositorio en las carpetas `src/`, `include/` y `tests/`, actualizando `Cwitter.cbp` y la documentación (`5e38fff`). Se implementa el límite de 280 caracteres y el soporte de hilos en `tweet.h` y `tweet.c`: campos `idPadre` y `tLista hilo`, `tweetPublicar` unificada, `tweetBuscarPorId` recursiva, eliminación en cascada (`tweetVaciarHilo`) y persistencia recursiva en preorden (`tweetGuardarHilo` y `tweetsAbrir`, `9ee0c50`, rama `feature/hilos`). |
| 1/11 | mateofeiertag | Se actualiza el menú en `main.c` con la opción `R` para responder tweets/respuestas por ID y la opción `V` para visualizar el hilo completo con sangrías por nivel (`tweetImprimirHiloRecursivo`) y responder directamente desde allí (`2a726b0`). |
| 10/11 | LagooFran | Se implementa el TDA Árbol Binario de Búsqueda genérico (`arbol.h`/`arbol.c`, `fe38135`) y el módulo de índice invertido por palabra sobre el árbol (`indice.h`/`indice.c`, `ea17643`, rama `feature/busquedaIndexada`). Se agregan las funciones `*ConIndice` en `tweet.c`, se avisa en `main.c` al intentar registrar un usuario duplicado (`ac68387`), se integran los hilos con el índice para indexar y desindexar también las respuestas (`9d574ee`) y se mergea a `develop` (`670856a`). |
| 11/11 | agussrico | Se actualizan `test_tweets.c`, `TESTS.md`, `BITACORA.md` y `README.md` con los casos de prueba del Sprint 3 y las mediciones de escalabilidad; release `v3-final`. |

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

**Cómo resultó en el Sprint 3.** Volvió a confirmarse: la misma `tLista` se reutilizó tanto para guardar la sub-lista `hilo` de respuestas dentro de cada `Tweet` como para guardar la lista de nodos `tNodo *` dentro de cada entrada del índice de palabras (`tEntradaIndice`), sin tener que tocar `lista.c`.

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

En el Sprint 3 se mantuvo la misma separación modular agregando `arbol.c/h` (TDA ABB genérico) e `indice.c/h` (índice de palabras), trabajando sobre las ramas `feature/hilos` y `feature/busquedaIndexada`.

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

### Decisión 11 (Sprint 3): qué estructura elegimos para representar los hilos y por qué

**Problema a resolver.** Un tweet tiene que poder recibir respuestas, y cada respuesta tiene que poder recibir sus propias respuestas, sin ningún límite de profundidad ni de cantidad de respuestas por tweet. Además, hay que poder mostrar el hilo completo diferenciando los niveles, eliminar un tweet junto con sus respuestas y persistir esa relación en archivo.

**Alternativas consideradas.**

1. **Una única lista plana (`feed`) donde todos los tweets y respuestas viven juntos**, diferenciados solo por un campo `idPadre` (`0` si es raíz, o el ID del tweet al que responde):
   - *Ventaja:* guardar en archivo es trivial (`fwrite` lineal).
   - *Por qué la descartamos:* para mostrar un hilo de $k$ niveles habría que recorrer toda la lista completa una y otra vez buscando qué tweets tienen `idPadre == X`, y además en el carrusel del feed principal aparecerían mezcladas las respuestas sueltas o habría que filtrarlas constantemente.
2. **Un árbol N-ario clásico con punteros `primerHijo` y `hermanoDerecho` dentro de `Tweet`:**
   - *Ventaja:* representa cualquier jerarquía usando dos punteros por nodo.
   - *Por qué la descartamos:* obligaría a programar desde cero toda la lógica de inserción ordenada, búsqueda y desenlace de hijos/hermanos que **ya teníamos resuelta y probada** en nuestra lista doblemente enlazada genérica (`tLista`).
3. **Un arreglo fijo de respuestas dentro de `Tweet` (por ejemplo `Tweet respuestas[50]`):**
   - *Por qué la descartamos:* el enunciado pide explícitamente que no haya límites artificiales impuestos por nosotros, y además un `struct Tweet` no puede contenerse a sí mismo por valor de forma infinita.
4. **Lista de hijos dentro de cada `Tweet` (`tLista hilo` por valor + `unsigned int idPadre`):**
   - Cada `Tweet` guarda `idPadre` (`0` para los tweets raíz) y una estructura `tLista hilo` (que ocupa solo 24 bytes: punteros `inicio`, `fin` y entero `cant`). Los tweets principales viven en `feed`, y las respuestas directas a un tweet viven dentro de su `t->hilo`.

**Qué elegimos.** La opción 4: combinar `unsigned int idPadre` y `tLista hilo` (por valor, no como puntero `tLista *`) dentro de `Tweet`, y unificar la publicación en `tweetPublicar(..., unsigned int idPadre)`.

**Por qué nos pareció mejor que las otras opciones.**

- **Reutilización total de `tLista`:** cada nivel de respuestas es simplemente otra `tLista`, por lo que reutilizamos `listaInicializar`, `listaInsertarOrdenado`, `listaVacia` y `listaEliminarNodo` sin duplicar código.
- **El feed principal queda limpio:** en `listaTweets` (`feed`) solo están los tweets raíz (`idPadre == 0`), así que el carrusel `A`/`D` y `tweetsMostrarFeed` no se ensucian con respuestas sueltas, y en cada tweet podemos mostrar inmediatamente `Respuestas: t->hilo.cant` en $O(1)$.
- **Acceso directo al subárbol:** parado sobre un tweet, todas sus respuestas directas están en `t->hilo` sin tener que buscarlas en el resto del sistema. Y tener también `idPadre` nos permite saber en todo momento a qué tweet responde y reconstruir el árbol al leer `tweets.bin`.
- **Por qué `tLista hilo` por valor y no `tLista *hilo`:** inicialmente probamos declarar `tLista *hilo` y asignarle la dirección de una variable local en `tweetPublicar`, pero al terminar la función esa dirección de la pila quedaba inválida (puntero colgante). Guardando `tLista hilo` por valor dentro del `struct Tweet`, el `memcpy` de `listaInsertarOrdenado` copia limpiamente `{NULL, NULL, 0}` dentro del nodo dinámico sin pedir otro `malloc` extra.

---

### Decisión 12 (Sprint 3): cómo modificamos el formato de persistencia en `tweets.bin`

**Problema a resolver.** En los sprints anteriores cada tweet era un registro independiente y `tweetsGuardar` solo recorría la lista principal `feed`. Ahora un tweet puede tener hijos en `t->hilo`, nietos dentro de cada hijo, etc. Además, el campo `tLista hilo` dentro de `Tweet` contiene punteros de memoria (`inicio` y `fin`) que al grabarse en disco con `fwrite` quedan apuntando a direcciones de RAM viejas que ya no existen cuando el programa vuelve a abrirse.

**Qué modificamos.**

1. **Guardado recursivo en preorden (`tweetGuardarHilo`):**
   Al guardar `tweets.bin`, en lugar de un `while` plano sobre `feed`, usamos `tweetGuardarHilo(FILE *pf, tLista *lista)`: por cada nodo de la lista escribe primero el `Tweet` padre con `fwrite` y acto seguido se invoca recursivamente con `tweetGuardarHilo(pf, &t->hilo)` para escribir todas sus respuestas antes de pasar al siguiente tweet. Así garantizamos que **en el archivo todo padre aparece siempre antes que cualquiera de sus hijos**.
2. **Reconstrucción al abrir (`tweetsAbrir`):**
   Al leer cada registro `aux` con `fread`:
   - Primero ejecutamos `listaInicializar(&aux.hilo)` para **blanquear los punteros `inicio` y `fin`** que venían grabados del archivo y dejar su lista de respuestas vacía en `NULL`.
   - Si `aux.idPadre == 0`, es un tweet principal y se inserta en `feed` con `listaInsertarOrdenado`.
   - Si `aux.idPadre != 0`, es una respuesta: como el archivo se guardó en preorden, su tweet padre ya fue leído y cargado en memoria en pasos anteriores. Lo buscamos con `tweetBuscarPorId(feed, aux.idPadre)` e insertamos `aux` dentro de `&padre->hilo`.
3. **Vaciado recursivo en memoria (`tweetVaciarHilo`):**
   Como `listaVaciar` solo libera `nodo->data` y no conoce que adentro de un `Tweet` hay otra `tLista`, agregamos `tweetVaciarHilo`: recorre los hermanos con un `while`, vacía primero en profundidad los sub-hilos de cada uno y al final llama a `listaVaciar(hilo)`. Se usa tanto al eliminar un tweet con respuestas (borrado en cascada) como al cerrar el programa.

---

### Decisión 13 (Sprint 3): visualización de hilos y cómo responder desde consola (`main.c`)

**Problema a resolver.** El usuario necesita ver el hilo completo de un tweet distinguiendo los niveles de profundidad y poder responder tanto a un tweet principal como a una respuesta dentro de un hilo, sin que el carrusel principal se vuelva inmanejable ni tener que adivinar de memoria el ID interno de un tweet.

**Alternativas discutidas en el equipo.**

- **Mostrar los hilos siempre desplegados en el carrusel principal:** descartada porque un solo tweet con muchas respuestas llenaría la pantalla y rompería la idea de recorrer el feed de a un tweet con `A` y `D`.
- **Navegar el hilo nivel por nivel con `A`/`D` (entrando a las respuestas de a una):** la analizamos con un simulador porque escala muy bien si un tweet tiene 100 respuestas, pero no mostraba el árbol completo de un vistazo como pide el enunciado (*"ver el hilo completo, de alguna forma que deje claro qué responde a qué"*).
- **Mantener el carrusel limpio y agregar una vista dedicada `[V] Ver Hilo Actual` + `[R] Responder Tweet`:** en el carrusel principal se sigue viendo un solo tweet por vez (indicando `Respuestas: N`). Al presionar `V`, se abre el hilo completo del tweet actual imprimiendo recursivamente todas sus respuestas con sangría proporcional al nivel (`nivel * 4` espacios + `|--`), y ahí mismo al pie pregunta si se desea responder al tweet principal (presionando `0`) o a alguna de las respuestas internas (cuyo `[ID: X]` el usuario tiene a la vista en esa misma pantalla).

**Qué elegimos.** La tercera opción (`[V] Ver Hilo Actual` con `tweetImprimirHiloRecursivo` y respuesta integrada, más `[R] Responder Tweet` desde el menú).

---

### Decisión 14 (Sprint 3): escalabilidad y búsqueda indexada con Árbol Binario de Búsqueda (ABB)

**Problema a resolver.** Con el crecimiento de la plataforma a miles de publicaciones y respuestas anidadas en hilos, necesitábamos evaluar cómo se comportaban las búsquedas en función de la cantidad de tweets ($N = 100, 1.000, 10.000\text{ y }100.000$) y evitar que la búsqueda creciera en proporción directa a la cantidad total de tweets.

#### 1. Medición inicial con la búsqueda lineal (sin índice)

Antes de indexar, medimos cuántas comparaciones y cuánto tiempo demoraba recorrer la estructura de listas e hilos en el peor caso (cuando el elemento buscado está al final o no existe) para $100, 1.000, 10.000\text{ y }100.000$ tweets:

| Cantidad de Tweets ($n$) | Comparaciones en Peor Caso (Sin índice) | Tiempo Búsqueda por ID (`tweetBuscarPorId`) | Tiempo Búsqueda de Texto (`strstr` sin índice) |
| :--- | :--- | :--- | :--- |
| **100** | 100 | `0.00030 ms` | `0.00140 ms` |
| **1.000** | 1.000 | `0.00240 ms` | `0.01460 ms` |
| **10.000** | 10.000 | `0.05600 ms` | `0.40400 ms` |
| **100.000** | 100.000 | `2.77600 ms` | `8.25200 ms` |

#### 2. Búsqueda indexada: análisis de comparaciones

- **Búsqueda sin índice:**
  La búsqueda original recorre la lista completa de tweets y, para cada uno, usa `strstr` para ver si el texto buscado aparece en el mensaje. Con $n$ tweets (contando las respuestas de los hilos), cada búsqueda hace siempre **$n$ comparaciones**, haya resultados o no. Su complejidad es **$O(n)$**. Además, cada comparación con `strstr` puede recorrer el mensaje entero, de hasta 280 caracteres.
- **Búsqueda con índice (`tIndice` sobre `tArbol`):**
  El índice es un **árbol binario de búsqueda (ABB)** con una entrada (`tEntradaIndice`) por cada palabra distinta que aparece en los tweets y respuestas. Las palabras se guardan en minúsculas (`normalizar`) y sin signos de puntuación (`SEPARADORES`). Cada entrada tiene una lista (`tLista nodos`) de punteros a los nodos (`tNodo *`) de los tweets que contienen esa palabra.
  Para buscar, se baja por el árbol comparando la palabra buscada con la de cada nodo (`strcmp`) hasta encontrarla o llegar a una hoja. La cantidad de comparaciones depende de la **altura $h$ del árbol**, no de la cantidad de tweets:
  - **Mejor caso** (árbol balanceado): $\lceil \log_2(p + 1) \rceil$ comparaciones, con $p =$ cantidad de palabras distintas.
  - **Peor caso** (árbol degenerado): $p$ comparaciones.
  - **En la práctica**, las palabras se insertan en el orden en que aparecen en los tweets, que es variado, así que el árbol queda cerca del caso balanceado.

  Encontrada la palabra, los resultados se obtienen directamente de su lista, sin recorrer el feed. Cada comparación es un `strcmp` entre dos palabras cortas, que suele resolverse en los primeros caracteres.

#### 3. Mediciones comparativas: Búsqueda sin índice vs. Búsqueda con índice ABB

**a) Evolución por volumen de tweets ($100$ a $100.000$ tweets, buscando en el peor caso):**

| Tweets ($n$) | Comparaciones sin índice ($O(n)$) | Tiempo sin índice | Comparaciones con índice ABB | Altura del ABB ($h$) | Tiempo con índice ABB |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **100** | 100 | `0.00140 ms` | 4 | 18 | `0.00010 ms` |
| **1.000** | 1.000 | `0.01460 ms` | 4 | 23 | `0.00012 ms` |
| **10.000** | 10.000 | `0.40400 ms` | 4 | 30 | `0.00010 ms` |
| **100.000** | 100.000 | `8.25200 ms` | 4 | 36 | `0.00036 ms` |

**b) Mediciones por escenario de vocabulario (con $10.000$ tweets):**

| Escenario | Tweets ($n$) | Palabras distintas ($p$) | Altura del árbol | Comparaciones con índice | Comparaciones sin índice | Ahorro |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **Vocabulario chico, con hilos** | 10.001 | 107 | 14 | 5 | 10.001 | **99,95 %** |
| **Vocabulario amplio** | 10.000 | 4.349 | 32 | 5 a 18 | 10.000 | **99,8 % a 99,95 %** |
| **Palabra inexistente** | 10.000 | 4.349 | 32 | 10 | 10.000 | **99,9 %** |

Con 10.000 tweets, la búsqueda sin índice siempre hace 10.000 comparaciones. Con índice, ninguna búsqueda hizo más de 18. Si la cantidad de tweets crece, la búsqueda lineal crece en la misma proporción. Con índice, solo crece si aparecen palabras nuevas, y aun así de forma logarítmica.

#### 4. Costo del índice y compensación

- **Construcción:** el índice vive en memoria y se arma al iniciar el programa recorriendo todos los tweets (`tweetsAbrirConIndice`). Para cada palabra de cada tweet se hace una búsqueda en el árbol. El costo es **$O(n \cdot w \cdot h)$**, con $w =$ palabras por tweet. Como un tweet tiene como máximo 280 caracteres, $w$ está acotado. Con 10.000 tweets, la carga completa del programa tardó alrededor de 0,1 segundos.
- **Mantenimiento:** publicar (`tweetPublicarConIndice`), modificar (`tweetModificarConIndice`) o eliminar (`tweetEliminarConIndice`) un tweet actualiza solo las entradas de sus palabras: **$O(w \cdot h)$** por operación. Al eliminar un tweet, también se quitan del índice todas las respuestas de su hilo (`tweetDesindexarLista`).
- **Compensación:** el costo de construcción se paga una sola vez por ejecución. A partir de ahí, cada búsqueda cuesta unas pocas comparaciones en lugar de una por cada tweet.

#### 5. Limitaciones del índice actual

- **Palabras completas:** se busca por palabra completa, no por subcadena. Por ejemplo, `"hol"` no encuentra `"hola"`.
- **Árbol sin balancear:** con 4.349 palabras la altura fue 32, mientras que la mínima posible es 13. Un árbol autobalanceado (AVL) garantizaría el mejor caso, a cambio de una implementación más compleja.

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
| 15 | Había un ejecutable y un archivo viejo de `main` en el repositorio. | Se subieron `cwitter` (el binario de Linux, sin extensión, que ninguna regla ignoraba) y `mainOLD.c` (una copia de respaldo). | Se sacaron del repositorio y se agregaron `cwitter`, `*.log`, `bin/` y `obj/` al `.gitignore`. | `8670e77` |
| 16 | `test_tweets.c` dejó de compilar. | En el Sprint 2 `tweetPublicar` pasó a recibir un `User *`, y el test le seguía pasando el nombre como texto. | Se reescribió el test con usuarios armados a mano (verificados y no verificados) y se sumaron los casos del feed ordenado y del recorrido. | `44948f1` (agussrico, 10/10) |
| 17 | Dos tweets publicados en el mismo segundo quedaban en orden invertido. | `time()` tiene resolución de un segundo: si la fecha era igual, el comparador dejaba primero al más viejo. | Se agregó un desempate por id: el de id más alto es el más nuevo. | `77d01da` (agussrico, 10/10) |
| 18 | Los reportes correctos se tomaban como error. | La documentación de FameCheck dice que el reporte responde 201, pero el servicio responde 204 (sin body). | `famecheckReportarPublicacion` considera éxito el 204. Quedó anotado en el código. | `31d686e` (LagooFran, 8/10) |
| 19 | `tweetsAbrir` devolvía `TODO_OK` cuando no existía el archivo, a diferencia de `usersAbrir`. | Se encontró al completar la columna "Salida obtenida" de la hoja de tests: el caso esperaba `ERROR_AP_ARCH`. | Ahora devuelve `ERROR_AP_ARCH`, igual que `usersAbrir`. `main` no usa ese valor, así que no cambia el comportamiento del programa. | `6f73401` (agussrico, 10/10) |
| 20 | Al desarrollar `tweetVaciarHilo`, una primera versión con `if (nodo != NULL) return tweetVaciarHilo(&t->hilo);` solo miraba el primer hijo y salía sin llamar a `listaVaciar(hilo)`. | Usar `if` + `return` cortaba la función en la primera rama en lugar de recorrer todos los hermanos de ese nivel y vaciar la lista al final. | Se reemplazó el `if` por un `while (nodo != NULL)` sin `return` intermedio, vaciando primero recursivamente cada `&t->hilo` y llamando al final a `listaVaciar(hilo)`. | `9ee0c50` (agussrico, 28/10) |
| 21 | Al integrar `feature/hilos` con el índice ABB, las respuestas dentro de los hilos no se indexaban y `MAX_PALABRA` seguía en 141. | La primera versión del índice solo recorría `feed` y asumía tweets de 140 caracteres. | Se subió `MAX_PALABRA` a 281 y se agregaron `tweetBuscarNodoPorId`, `tweetIndexarLista` y `tweetDesindexarLista` recursivas para que el índice abarque también todas las respuestas de los hilos. | `9d574ee` (LagooFran, 10/11) |

## 4. Contribuciones visibles en el historial

| Integrante | Trabajo visible en los commits |
|------------|--------------------------------|
| agussrico | **Sprint 1:** diseño inicial (`cwitter.h`), corrección de la lista, librería/registro/login/persistencia de usuarios y sus tests, inicialización de structs y copia con límite, documentación. **Sprint 2:** coautoría del reporte y el top 5, corrección del top 5 y del inicio de curl, log de fallos, limpieza del repositorio, feed ordenado, `test_famecheck.c`, adaptación de `test_tweets.c` y bitácora. **Sprint 3:** reorganización del repositorio en `src/`, `include/` y `tests/`, diseño e implementación de hilos en `tweet.h`/`tweet.c` (`idPadre`, `tLista hilo`, `tweetPublicar` unificada, `tweetBuscarPorId`, `tweetVaciarHilo` y persistencia recursiva `tweetGuardarHilo`/`tweetsAbrir`), casos de prueba del Sprint 3 en `test_tweets.c`, `TESTS.md`, `BITACORA.md` y `README.md`. |
| LagooFran | **Sprint 1:** lista genérica, configuración del IDE, menús de inicio de sesión y registro, soporte multisistema (`config.ini`) y limpieza de la lista (`listaLlena`, `LISTA_VACIA`). **Sprint 2:** integración con curl y cJSON, verificación de usuarios, reporte de tweets, top 5 (`listaInsertarOrdenado`, `listaEliminarUltimo`), menús nuevos con colores y recorrido del feed con A/D. **Sprint 3:** TDA Árbol Binario de Búsqueda genérico (`arbol.h`/`arbol.c`), índice invertido de búsqueda por palabra (`indice.h`/`indice.c`), funciones `*ConIndice` en `tweet.c` con soporte para hilos y 280 caracteres, aviso de usuario duplicado en `main.c` y análisis de comparaciones. |
| mateofeiertag | **Sprint 1:** integración de tweets y feed en `main`, tests de tweets. **Sprint 2:** hoja de tests y formato de los tweets en consola (fecha y marca de verificado). **Sprint 3:** visualización recursiva de hilos con sangría por nivel (`tweetImprimirHiloRecursivo`, opción `V`) y respuesta a tweets/hilos (`R` y respuesta directa desde `V`) en `main.c`. |
| Garbaaa | **Sprint 1:** corrección de búsqueda, permisos de modificar/eliminar. |

## 5. Planteos individuales previos

#### Agustina Rico (agussrico) – Estructura de datos (lista genérica e hilos)

- **Qué solución se me ocurrió inicialmente:** Armé una librería propia de Cwitter (`cwitter.h`) con listas simplemente enlazadas específicas para usuarios y tweets, y una estructura `Sistema` que agrupaba todo el estado.
- **Qué dudas tenía:** Pensaba que una lista genérica no era posible o que nos iba a traer conflictos de casteo de estructuras a la hora de utilizarlas y que era mejor desarrollar réplicas de cada función.
- **Qué cambió después de discutirlo o investigarlo:** Un compañero propuso hacer una lista genérica. Lo discutimos y lo probamos en el código, y vimos que era práctico. Más adelante, desarrollando el programa, nos dimos cuenta de que la estructura `Sistema` no era necesaria y la sacamos. En el Sprint 3 esa misma lista genérica nos permitió meter una `tLista hilo` dentro de cada `Tweet` para armar hilos de cualquier profundidad sin programar otra lista nueva.

#### Franco Lago (LagooFran) – Interfaz de consola e índice sobre ABB

- **Qué solución se me ocurrió inicialmente:** Al armar la interfaz de consola utilicé funciones de sistema que me permitían mejorar la legibilidad de la interfaz.
- **Qué dudas tenía:** Me di cuenta que estas funciones que utilicé no eran compatibles con varios sistemas operativos por lo tanto el programa no iba a funcionar correctamente en sistemas Linux.
- **Qué cambió después de discutirlo o investigarlo:** Propuse crear un archivo de configuración que permita seleccionar el sistema operativo. Este archivo se lee al inicio del programa y configura el mismo para que funcione correctamente en todos los sistemas. En el Sprint 3, para resolver el problema de escalabilidad en las búsquedas sin acoplar las estructuras, diseñé un TDA `tArbol` genérico (`void *`) siguiendo el mismo esquema de `tLista` y monté sobre él el índice invertido de palabras (`tIndice`).

#### Mateo Feiertag (mateofeiertag) – Módulo de tweets y visualización de hilos

- **Qué solución se me ocurrió inicialmente:** Diseñar una estructura independiente para los tweets con el límite estricto de 140 caracteres y un campo de ID autoincremental, manejando las operaciones básicas de publicación y visualización de manera aislada.
- **Qué dudas tenía:** Tenía dudas sobre cómo integrar las publicaciones a la lista doblemente enlazada genérica sin romper los tipos de datos, y cómo garantizar la seguridad para que un usuario no pudiera modificar o eliminar tweets ajenos.
- **Qué cambió después de discutirlo o investigarlo:** Adapté las funciones comparadoras para que trabajen con la lista genérica. Además, desarrollé un conjunto de pruebas (`test_tweets.c`) para validar los casos de borde exigidos (plataforma vacía, borrado en extremos e intermedio, y control de permisos), y en el Sprint 3 implementé la impresión recursiva de los hilos en consola (`tweetImprimirHiloRecursivo`) escalonando visualmente cada nivel de respuesta.

#### Francisco Garbaccio (Garbaaa) – Permisos de edición

- **Qué solución se me ocurrió inicialmente:** Tener en cuenta qué usuario está logueado, para que la modificación y eliminación de tweets no sea global sino dependiente del usuario.
- **Qué dudas tenía:** Tenía dudas de cómo buscar de la forma más eficiente el usuario para poder comparar con el tweet para llegar a la mejor solución posible.
- **Qué cambió después de discutirlo o investigarlo:** La mejor forma que se encontró fue pasar el autor por la función y compararlo con el tweet para poder vigilar correctamente los derechos del usuario.

---

# Operaciones que podrían pedirnos más adelante

## 1. Operaciones nuevas

| # | Operación | Por qué creemos que la van a pedir | Cambio de diseño que implicaría | Origen |
|---|-----------|------------------------------------|---------------------------------|--------|
| 1 | **Feed público:** ver el feed y buscar sin iniciar sesión; la sesión solo sirve para publicar, modificar y eliminar. | Es lo habitual en una red social, y en el enunciado el fundador dice que "cualquier persona" puede ver lo publicado. Abrir la lectura ayuda al crecimiento que él espera. | **Bajo.** `tweetsMostrarFeed` y `tweetBuscarYMostrarConIndice` ya no dependen del usuario. Cambia el menú de `main`: el menú sin sesión pasa a tener "ver feed" y "buscar". Cambia un requisito, así que hay que actualizar el caso de test "ver el feed sin estar logueado". | Equipo |
| 2 | ~~**Comentarios e hilos:** responder a un tweet y ver la conversación completa.~~ **(resuelto en el Sprint 3)** | Se pidió en el Sprint 3 y se resolvió agregando `idPadre` y `tLista hilo` dentro de `Tweet` (ver decisiones 11, 12 y 13). | — | Equipo |
| 3 | ~~**Feed ordenado** del más reciente al más viejo~~ **(resuelto en el Sprint 2)** y **paginado**. | El orden se pidió en el Sprint 2 (verificados primero, después por fecha) y se resolvió con la lista ordenada (decisión 7). Queda pendiente paginar: con muchos tweets no se puede leer el feed entero. | **Bajo.** El recorrido de a un tweet ya resuelve parte de esto. Para paginar se saltan N nodos desde el inicio o desde el nodo actual. | Equipo |
| 4 | **Búsquedas más completas:** por autor, ~~sin distinguir mayúsculas~~ **(resuelto en el Sprint 3 con `tIndice`)** y por hashtag. | En el Sprint 3 el índice ABB normaliza a minúsculas, resolviendo la búsqueda insensible a mayúsculas. Queda pendiente buscar por autor o por `#hashtag`. | **Bajo.** Para hashtags alcanza con no incluir `#` en `SEPARADORES` de `indice.h`, o tener un segundo `tIndice` para autores. | Sugerida por IA, evaluada por el equipo |
| 5 | **Perfil de usuario:** ver todos los tweets de una persona y cuántos tiene. | Complementa la búsqueda por autor y es lo primero que se mira de otra persona. | **Bajo.** Reutiliza `listaBuscarYAccion` con un `cmp` por autor y un contador. El campo `cantTweets` de `User`, hoy sin uso, podría servir para esto. | Sugerida por IA, evaluada por el equipo |
| 6 | **Seguir usuarios** y ver un feed solo con los tweets de a quienes sigo. | Es el pedido clásico cuando el feed general crece demasiado para ser útil. | **Alto.** Aparece una relación usuario–usuario (N a N). No se puede guardar dentro de `User` con `fwrite` si contiene una lista (tendría punteros). Habría que persistirla aparte, por ejemplo un `seguidores.bin` con pares (seguidor, seguido), y reconstruirla al cargar. Ver decisiones abiertas. | Sugerida por IA, evaluada por el equipo |
| 7 | **Gestión de cuenta:** cambiar contraseña, eliminar la cuenta y, más adelante, un rol de moderador que pueda borrar cualquier tweet. | Toda plataforma con registro termina necesitándolo. El moderador es la respuesta natural al cambio de permisos de la decisión 2. | **Medio.** Cambiar contraseña: modificar el dato en la lista y volver a guardar. Eliminar cuenta: `listaEliminarNodo` sobre usuarios, con una decisión abierta sobre sus tweets. Moderador: campo `rol` en `User`, lo que cambia el struct y obliga a migrar `users.bin` (ver lo que pasó con los `.bin` en el Sprint 2, decisión 3). | Sugerida por IA, evaluada por el equipo |
| 8 | **Reintentar los reportes fallidos** a FameCheck. | Hoy los fallos quedan en `logs.log` y nadie los reenvía; el fundador quiere tener el registro de actividad completo. | **Bajo/medio.** El tweet ya tiene `reportadoAFamecheck`: al iniciar o con una opción del menú se recorren los tweets de verificados con ese campo en 0 y se vuelven a reportar. | Equipo |
| 9 | **Subir en el feed los tweets de quien se verifica después de publicar.** | Hoy un tweet guarda si su autor estaba verificado al publicar; si se verifica después, sus tweets viejos no suben. | **Bajo.** Al verificar, recorrer los tweets de ese autor, marcar `verificado = 1` y reubicarlos en la lista (sacarlos e insertarlos ordenados). | Equipo |

## 2. Decisiones abiertas que estas operaciones nos van a obligar a tomar

### ~~Hilos (operación 2)~~ **(decisiones tomadas e implementadas en el Sprint 3)**

- **¿Dónde se guarda la relación?** Se combinaron ambas ideas: cada `Tweet` guarda `idPadre` y su sub-lista `tLista hilo`, persistiendo en preorden en `tweets.bin` y limpiando los punteros de `hilo` con `listaInicializar` al cargar (decisiones 11 y 12).
- **¿Qué pasa al eliminar un tweet con respuestas?** Se decidió eliminar en cascada todas sus respuestas y sub-respuestas (`tweetVaciarHilo`) y quitarlas también del índice ABB (`tweetDesindexarLista`).
- **¿Qué límite y orden tienen las respuestas?** Respetan el nuevo límite de 280 caracteres, los permisos de autor (`SIN_PERMISO`) y se insertan ordenadas dentro de `padre->hilo` con `cmpTweetFeed`.

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
| Un tweet largo se trunca sin avisar ~~y el registro duplicado no muestra mensaje~~. | Validar y avisar al usuario al truncar (el aviso de `USER_DUPLICADO` ya se incorporó en el Sprint 3, commit `ac68387`). | Devolver un código nuevo (por ejemplo `TWEET_MUY_LARGO`) desde `tweetPublicar` y que `main` muestre el mensaje. |
| ~~Bytes basura en `users.bin` y uso de `strcpy` sin límite.~~ | **Resuelto en el Sprint 1** (`8f13ca8`). | — |
| Cambiar un `struct` deja inservibles los `.bin` anteriores. | Poder agregar campos sin perder los datos. | Guardar al principio del archivo un número de versión, o escribir una migración que lea el formato viejo y escriba el nuevo. |
| Sin conexión, publicar un tweet de un verificado puede demorar hasta 10 segundos. | Que publicar no espere al servicio. | Bajar el tiempo de espera, o publicar primero y reportar después (por ejemplo, marcando el tweet como pendiente y reportando los pendientes al salir). |
| El ABB del índice (`tArbol`) no es autobalanceado y `tweetBuscarPorId` sigue siendo recursiva sobre listas. | Balancear el ABB (AVL) y/o agregar un segundo índice en ABB por `id`. | Con un árbol AVL la altura quedaría garantizada en $\lceil \log_2(p + 1) \rceil$. Además, reutilizando el mismo `tArbol` para indexar pares `(id, tNodo *)` se llevaría también `tweetBuscarPorId` de $O(n)$ a $O(\log n)$. |

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
- "¿Cómo conviene representar los hilos dentro de `Tweet` reutilizando `tLista` y qué pasa con los punteros al guardar en `tweets.bin`?"
- "¿Me podés mostrar visualmente con un simulador cómo sería la interacción del usuario en la pantalla para ver y responder hilos?"
- "¿Cómo medimos y comparamos la cantidad de comparaciones de la búsqueda lineal frente al índice con un Árbol Binario de Búsqueda?"

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
- En el Sprint 2 y en el Sprint 3, para completar la columna "Salida obtenida" de la hoja de tests y las tablas de escalabilidad, ejecutó los programas de prueba y copió la salida real.
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

### 3.3 Sprint 3: separar `tweetPublicar` y `tweetResponder` vs. unificar con `idPadre`

**Qué consultamos.** Al empezar a diseñar los hilos de respuestas, le consultamos a la IA qué funciones agregar en `tweet.c`.

**Qué respondió.** Inicialmente nos propuso crear una función nueva `tweetResponder(tLista *feed, User *autor, unsigned int idPadre, char *mensaje)` separada de `tweetPublicar(tLista *feed, User *autor, char *mensaje)`.

**Qué evaluamos.** Al comparar ambas funciones vimos que hacían exactamente el mismo trabajo (inicializar el `Tweet`, asignarle `ultimo_id`, fecha, autor, `verificado`, truncar a 280 caracteres, inicializar `nuevoTweet.hilo` y reportar a FameCheck si era verificado), cambiando únicamente la última línea: si va a `feed` o a `&padre->hilo`. Tener dos funciones duplicaba más de 40 líneas de código.

**Qué solución adoptamos.** Unificamos todo en una sola función `tweetPublicar(tLista *feed, User *autor, char *mensaje, unsigned int idPadre)` que recibe `0` cuando es un tweet principal y el ID del padre cuando es una respuesta.

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
