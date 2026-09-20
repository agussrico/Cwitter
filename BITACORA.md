# Trabajo Práctico AyED – Cwitter – Sprint 1

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

**Entrega:** Checkpoint Sprint 1 – 21/09/2026

---

# Descripción del funcionamiento del programa

## 1. Resumen

Cwitter es un programa de consola en C que permite registrar usuarios, iniciar y cerrar sesión, publicar, modificar y eliminar tweets, ver el feed general y buscar tweets por texto. Los usuarios y los tweets se guardan en archivos binarios, así que se conservan entre ejecuciones.

Los datos se mantienen en memoria en dos listas doblemente enlazadas genéricas (una de usuarios y otra de tweets). Al iniciar se cargan desde disco y al salir se guardan.

## 2. Organización del código

| Archivo | Responsabilidad |
|---------|-----------------|
| `lista.h` / `lista.c` | Lista doblemente enlazada genérica (`void *`). No conoce ni usuarios ni tweets. |
| `user.h` / `user.c` | Entidad User: registro, inicio de sesión, carga y guardado de `users.bin`. |
| `tweet.h` / `tweet.c` | Entidad Tweet: publicar, modificar, eliminar, feed, búsqueda, carga y guardado de `tweets.bin`. |
| `main.c` | Interfaz de consola (menús) y flujo general del programa. |
| `config.ini` | Configuración: `linux=true` o `linux=false`, para elegir el comando de limpieza de pantalla (`clear` o `cls`). |
| `test_users.c` / `test_tweets.c` | Programas de prueba de cada módulo (ver sección 8). |

Las dependencias van en un solo sentido: `main` usa `user` y `tweet`, y ambos usan `lista`. La lista no depende de ninguno de los dos.

## 3. La lista genérica (`lista.c`)

Cada nodo guarda un puntero `void *data` a una copia del dato, el tamaño de ese dato (`dataSize`) y punteros al nodo siguiente y al anterior. La lista mantiene punteros a `inicio` y `fin` y un contador `cant`.

Operaciones disponibles:

- `listaInicializar`, `listaVacia`, `listaLlena`, `listaVaciar`.
- `listaInsertar`: inserta siempre al final. Reserva memoria para el nodo y para la copia del dato (`memcpy`).
- `listaBuscarYDevolver`: devuelve el primer nodo que cumple una condición, o `NULL`.
- `listaBuscarYAccion`: aplica una acción a todos los nodos que cumplen una condición y devuelve cuántos fueron.
- `listaRecorrerYAccion`: aplica una acción a todos los nodos.
- `listaEliminarNodo`: busca un nodo con una condición y lo desenlaza. Contempla eliminar el primero, el último y uno intermedio, y libera nodo y dato.

La condición de búsqueda se pasa como función de comparación (`int (*cmp)(void *, void *)`) y las acciones como función de acción (`void (*accion)(void *)`). Por eso una misma lista sirve para usuarios y tweets: cada módulo aporta sus propias funciones (`cmpUser`, `cmpTweetId`, `cmpTweetTexto`, `userImprimir`, `tweetImprimir`, etc.).

`listaLlena` prueba si se puede reservar memoria para un nodo y un dato. Si no se puede, `listaInsertar` devuelve `LISTA_LLENA`. No hay un máximo fijo de usuarios ni de tweets; el único límite es la memoria disponible.

## 4. Entidades

### User

```c
typedef struct {
    char user[20]; /* nombre de usuario */
    char pass[20]; /* contraseña */
} User;
```

El nombre de usuario es la clave de identificación: no puede haber dos usuarios con el mismo nombre. Como `main` lee con `%19s`, nombre y contraseña admiten hasta 19 caracteres sin espacios.

### Tweet

```c
typedef struct {
    unsigned int id;
    char autor[20];
    char mensaje[MAX_TWEET + 1]; /* MAX_TWEET = 140, más el '\0' */
} Tweet;
```

Cada tweet tiene un `id` numérico único. El autor se guarda como texto (el nombre de usuario), sin referencia a la lista de usuarios.

## 5. Flujo del programa (`main.c`)

1. Se lee `config.ini` (`leerConfig`) para saber si la pantalla se limpia con `cls` o con `clear`, se inicializan las dos listas y se cargan desde disco (`tweetsAbrir`, `usersAbrir`).
2. **Menú sin sesión:** iniciar sesión, registrarse o salir. Sin sesión iniciada no se puede publicar ni ver el feed, porque esas opciones no existen en este menú.
3. **Menú con sesión** (se muestra mientras `pUsuarioActual != NULL`): ver feed, publicar, buscar, modificar, eliminar o cerrar sesión.
4. Al elegir Salir en el menú sin sesión, se guardan usuarios y tweets en disco, se libera la memoria de ambas listas y termina el programa.

La sesión activa se representa con un puntero (`User *pUsuarioActual`) al dato del usuario dentro de la lista. Cerrar sesión lo pone en `NULL` y vuelve al primer menú.

## 6. Funcionalidades

**Registro (`userRegistrar`).** Busca el nombre en la lista de usuarios. Si no existe, inserta el nuevo usuario. Si existe, devuelve `USER_DUPLICADO`. Los campos se inicializan en cero y los textos se copian con `strncpy`, con un máximo de 19 caracteres.

**Inicio de sesión (`userIniciarSesion`).** Busca al usuario por nombre y compara la contraseña. Devuelve un puntero al usuario si coincide y `NULL` si el usuario no existe o la contraseña es incorrecta (no distingue los dos casos).

**Cierre de sesión.** Opción 6 del menú con sesión: descarta el usuario actual.

**Publicar (`tweetPublicar`).** Asigna el siguiente id, copia el autor y el mensaje (truncado a 140 caracteres) e inserta el tweet al final de la lista. El autor es siempre el usuario con sesión iniciada.

**Modificar (`tweetModificar`).** Busca el tweet por id. Devuelve `NODO_NO_ENCONTRADO` si no existe y `SIN_PERMISO` si el autor no es quien intenta modificarlo. Si todo está bien, reemplaza el mensaje (también truncado a 140).

**Eliminar (`tweetEliminar`).** Mismas validaciones que modificar; si pasan, elimina el nodo de la lista.

**Feed (`tweetsMostrarFeed`).** Recorre la lista completa e imprime los tweets de todos los usuarios, en orden de publicación (el más antiguo primero).

**Búsqueda (`tweetBuscarYMostrar`).** Recorre la lista y muestra los tweets cuyo mensaje contiene el texto buscado (`strstr`, distingue mayúsculas de minúsculas). Si no hay coincidencias, informa que no se encontró ninguno.

**Identificación de tweets.** Los ids se generan con una variable estática `ultimo_id` que se incrementa en cada publicación. Al cargar el archivo, `ultimo_id` se actualiza al id más alto encontrado, para que los nuevos tweets no repitan ids existentes.

## 7. Persistencia

- **Formato:** archivos binarios, uno para cada entidad: `users.bin` y `tweets.bin`.
- **Contenido:** la secuencia de `struct` completos, uno detrás de otro, escritos con `fwrite` y leídos con `fread`, sin encabezado ni separadores. El tamaño de cada registro es `sizeof(User)` (40 bytes) y `sizeof(Tweet)` (depende del alineamiento del compilador).
- **Cuándo se guarda:** al salir del programa (opción 3 del menú principal), sobrescribiendo el archivo completo (`"wb"`).
- **Cuándo se carga:** al iniciar el programa, leyendo el archivo registro por registro e insertando cada uno en la lista. Si el archivo no existe, se arranca con la lista vacía.
- **Comprobación sobre el archivo real:** el `users.bin` de ejemplo pesa 120 bytes, es decir 3 usuarios de 40 bytes cada uno, lo que coincide con `sizeof(User)`.

Solo se usan funciones de archivos binarios (`fopen` `"rb"`/`"wb"`, `fread`, `fwrite`) y nunca se mezclan con funciones de texto.

## 8. Pruebas realizadas hasta ahora

- **`test_users.c`:** carga inicial, registro de usuarios, registro de un usuario duplicado, login correcto, login con contraseña incorrecta, login con usuario inexistente, guardado, vaciado de la lista y recarga para verificar la persistencia.
- **`test_tweets.c`:** plataforma vacía, publicación de varios tweets, búsqueda existente e inexistente, eliminación del primero, del último y de uno intermedio, intento de borrar un tweet ajeno, modificación de un tweet propio, guardado, vaciado y recarga.

(La hoja de tests completa, con situación, operación y resultado esperado de cada caso, va en una sección aparte.)

## 9. Códigos de retorno

| Constante | Valor | Significado |
|-----------|-------|-------------|
| `TODO_OK` | 0 | Operación exitosa |
| `LISTA_LLENA` | 3 | No hay memoria para insertar |
| `NODO_NO_ENCONTRADO` | 4 | No existe el elemento buscado |
| `ERROR_AP_ARCH` | 5 | No se pudo abrir el archivo |
| `USER_DUPLICADO` | 6 | El nombre de usuario ya existe |
| `SIN_PERMISO` | 7 | El tweet pertenece a otro usuario |

## 10. Limitaciones conocidas de esta versión

- **Guardado solo al salir por el menú.** Si el programa se cierra de otra forma (ventana, corte de luz), se pierden los cambios de esa sesión.
- **Truncado silencioso:** un tweet de más de 140 caracteres se corta sin avisar al usuario, y `main` informa "publicado con éxito" sin revisar el valor de retorno.
- **Registro duplicado sin aviso:** `main` no muestra un mensaje cuando `userRegistrar` devuelve `USER_DUPLICADO`.
- **Entrada de datos:** ingresar letras donde se espera un número en el menú deja al programa en un bucle.
- **Búsqueda:** distingue mayúsculas y minúsculas.

---

# Problemas, decisiones y registro individual

## 1. Línea de tiempo del proyecto

| Fecha | Quién | Qué se hizo |
|-------|-------|-------------|
| Antes del 8/9 | agussrico | Primer commit: `cwitter.h` con estructuras y prototipos (diseño inicial, ver decisión 1). |
| 8/9 | LagooFran | Se crea `llista.h` (con error de tipeo en el nombre) y se elimina. Se agrega `.gitignore` para archivos de Code::Blocks. Se implementa la lista genérica (`lista.h`/`lista.c`). |
| 9/9 | agussrico | Implementación de registro, login y persistencia de usuarios y pruebas. |
| 9/9 | agussrico | Se corrigen los prototipos de punteros a función en `lista.h` (rama `fix/lista-prototipos`, merge a `develop`). Se implementan registro, login y persistencia de usuarios, con sus tests. |
| 11/9 | LagooFran | Menú de inicio de sesión y registro, arreglo del inicio de sesión, cambios en la lista y estilo. Se mergea el PR #1 (`feature/usuarios`). |
| 15/9 | mateofeiertag | Integración del módulo de tweets y del feed en `main`. |
| 15/9 | Garbaaa | Se corrige la búsqueda de tweets y se restringe modificar/eliminar al autor. |
| 17/9 | mateofeiertag | Casos de prueba del Sprint 1 para tweets. |
| 20/9 | LagooFran | Arreglos generales y soporte multisistema (`7478fea`): `config.ini` y `limpiarPantalla()`, se elimina `LISTA_VACIA` y el parámetro sin uso de `listaLlena`. Luego agrega `tweet.c` y `tweet.h` al proyecto de Code::Blocks (`11fdc8a`). |
| 20/9 | agussrico | Inicialización de structs y copia con límite en usuarios y tweets (`8f13ca8`). |

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

### Decisión 2: quién puede modificar o eliminar un tweet

**Problema a resolver.** El enunciado dice que hay que estar identificado para "corregir o eliminar" un tweet, pero no aclara si se refiere solo a los propios.

**Primera versión.** Cualquier usuario con sesión iniciada podía modificar o eliminar cualquier tweet (`tweetModificar(feed, id, mensaje)` y `tweetEliminar(feed, id)`).

**Qué elegimos (commit `71bae31`, Garbaaa, 15/9).** Solo el autor puede modificar o eliminar su tweet. Ambas funciones reciben ahora el autor que intenta la operación, primero buscan el tweet por id y después comparan el autor; si no coincide devuelven `SIN_PERMISO` (código 7). En `main` se muestran tres mensajes distintos: éxito, "ese tweet no es tuyo" y "no se encontró el tweet".

**Por qué.** El feed muestra tweets de todos los usuarios, y dejar que cualquiera borre los de otros no tiene sentido en una red social. De esto nos dimos cuenta una vez hecho la parte de modificación y eliminación de tweets, por lo que fue añadido más adelante.

**Efecto en los tests.** `test_tweets.c` incluye ahora un caso donde un usuario intenta borrar el tweet de otro y se espera `SIN_PERMISO`, y otro donde el autor modifica el suyo.

### Decisión 3: formato de persistencia

Archivos binarios (`users.bin`, `tweets.bin`) con la secuencia de `struct` completos, escritos con `fwrite`/`fread`. Se carga al iniciar y se guarda al salir.

**Por qué.** No hay que parsear texto ni escapar separadores dentro de un mensaje, y el código de carga y guardado es corto; además el requisito de la materia pide no mezclar funciones de archivos de texto con binarios.

### Decisión 4: un módulo por entidad y tests por módulo

Se separó el código en `user.c/h` y `tweet.c/h`, y cada uno con su archivo de pruebas (`test_users.c`, `test_tweets.c`). El trabajo se repartió en ramas de funcionalidad (`feature/usuarios`, `fix/lista-prototipos`) mergeadas a `develop`.

### Decisión 5: cómo hacer que el programa funcione en Windows y en Linux

**Problema a resolver.** `main` limpiaba la pantalla con `system("cls")`, un comando de Windows que no existe en Linux, y la consigna pide una solución en ANSI C con la mayor compatibilidad multiplataforma posible.

**Qué elegimos.** Un archivo `config.ini` con una línea `linux=false` o `linux=true`. Al iniciar, `leerConfig()` lo lee y guarda el resultado en una variable global (`usarLinux`). La función `limpiarPantalla()` ejecuta `clear` o `cls` según ese valor y reemplaza todas las llamadas directas a `system("cls")`.

**Límites de esta solución.** El archivo se busca en la carpeta desde donde se ejecuta el programa y solo se lee su primera línea; si falta, se usa `cls`, que es el comportamiento anterior. Quien use Linux tiene que cambiar `linux=false` por `linux=true` en su copia.

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

## 4. Contribuciones visibles en el historial

| Integrante | Trabajo visible en los commits |
|------------|--------------------------------|
| agussrico | Diseño inicial (`cwitter.h`), corrección de la lista, librería/registro/login/persistencia de usuarios y sus tests, inicialización de structs y copia con límite. |
| LagooFran | Lista genérica, configuración del IDE, menús de inicio de sesión y registro, soporte multisistema (`config.ini`) y limpieza de la lista (`listaLlena`, `LISTA_VACIA`). |
| mateofeiertag | Integración de tweets y feed en `main`, tests de tweets. |
| Garbaaa | Corrección de búsqueda, permisos de modificar/eliminar. |

## 5. Planteos individuales previos

### Agustina Rico (agussrico) – Estructura de datos (lista genérica)

- **Qué solución se me ocurrió inicialmente:** Armé una librería propia de Cwitter (`cwitter.h`) con listas simplemente enlazadas específicas para usuarios y tweets, y una estructura `Sistema` que agrupaba todo el estado.
- **Qué dudas tenía:** Pensaba que una lista genérica no era posible o que nos iba a traer conflictos de casteo de estructuras a la hora de utilizarlas y que era mejor desarrollar réplicas de cada función.
- **Qué cambió después de discutirlo o investigarlo:** Un compañero propuso hacer una lista genérica. Lo discutimos y lo probamos en el código, y vimos que era práctico. Más adelante, desarrollando el programa, nos dimos cuenta de que la estructura `Sistema` no era necesaria y la sacamos.

### Franco Lago (LagooFran) – Interfaz de consola

- **Qué solución se me ocurrió inicialmente:** Al armar la interfaz de consola utilicé funciones de sistema que me permitían mejorar la legibilidad de la interfaz.
- **Qué dudas tenía:** Me di cuenta que estas funciones que utilicé no eran compatibles con varios sistemas operativos por lo tanto el programa no iba a funcionar correctamente en sistemas Linux.
- **Qué cambió después de discutirlo o investigarlo:** Propuse crear un archivo de configuración que permita seleccionar el sistema operativo. Este archivo se lee al inicio del programa y configura el mismo para que funcione correctamente en todos los sistemas.

### Mateo Feiertag (mateofeiertag) – Módulo de tweets

- **Qué solución se me ocurrió inicialmente:** Diseñar una estructura independiente para los tweets con el límite estricto de 140 caracteres y un campo de ID autoincremental, manejando las operaciones básicas de publicación y visualización de manera aislada.
- **Qué dudas tenía:** Tenía dudas sobre cómo integrar las publicaciones a la lista doblemente enlazada genérica sin romper los tipos de datos, y cómo garantizar la seguridad para que un usuario no pudiera modificar o eliminar tweets ajenos.
- **Qué cambió después de discutirlo o investigarlo:** Adapté las funciones comparadoras para que trabajen con la lista genérica. Además, desarrollé un conjunto de pruebas (`test_tweets.c`) para validar los casos de borde exigidos (plataforma vacía, borrado en extremos e intermedio, y control de permisos).

### Francisco Garbaccio (Garbaaa) – Permisos de edición

- **Qué solución se me ocurrió inicialmente:** Tener en cuenta qué usuario está logueado, para que la modificación y eliminación de tweets no sea global sino dependiente del usuario.
- **Qué dudas tenía:** Tenía dudas de cómo buscar de la forma más eficiente el usuario para poder comparar con el tweet para llegar a la mejor solución posible.
- **Qué cambió después de discutirlo o investigarlo:** La mejor forma que se encontró fue pasar el autor por la función y compararlo con el tweet para poder vigilar correctamente los derechos del usuario.

---

# Operaciones que podrían pedirnos más adelante

## 1. Operaciones nuevas

| # | Operación | Por qué creemos que la van a pedir | Cambio de diseño que implicaría | Origen |
|---|-----------|------------------------------------|---------------------------------|--------|
| 1 | **Feed público:** ver el feed y buscar sin iniciar sesión; la sesión solo sirve para publicar, modificar y eliminar. | Es lo habitual en una red social, y en el enunciado el fundador dice que "cualquier persona" puede ver lo publicado. Abrir la lectura ayuda al crecimiento que él espera. | **Bajo.** `tweetsMostrarFeed` y `tweetBuscarYMostrar` ya no dependen del usuario. Cambia el menú de `main`: el menú sin sesión pasa a tener "ver feed" y "buscar". Cambia un requisito, así que hay que actualizar el caso de test "ver el feed sin estar logueado" (ver sección 4). | Equipo |
| 2 | **Comentarios e hilos:** responder a un tweet y ver la conversación completa. | Es el paso natural después de publicar mensajes: una red social sin respuestas se queda en un tablón. | **Alto.** `Tweet` necesita un campo nuevo (por ejemplo `idPadre`, con 0 para los tweets raíz). El feed muestra solo los raíces y el hilo se arma buscando los tweets con `idPadre` igual al id consultado. Ver decisiones abiertas (sección 2). | Equipo |
| 3 | **Feed ordenado** del más reciente al más viejo, y paginado. | Hoy sale por orden de inserción (el más viejo primero). Con muchos tweets no se puede leer entero y lo primero que se busca es lo último publicado. | **Medio.** La lista doble permite recorrer desde `fin` hacia atrás usando `ant`, sin tocar la estructura. Para paginar se saltan N nodos (recorrido O(n) por página). Si se quiere ordenar por fecha real hay que volver a guardar `fechaHora`, que estaba en el diseño inicial y se descartó. | Equipo |
| 4 | **Búsquedas más completas:** por autor, sin distinguir mayúsculas y por hashtag. | Hoy solo se busca un texto exacto con `strstr` (distingue mayúsculas). Buscar "lo que dijo alguien" o "un tema" es el uso típico. | **Bajo/medio.** Funciones `cmp` nuevas para usar con `listaBuscarYAccion`/`listaBuscarYDevolver`, que ya existen en la lista y sirven sin modificarla. Sin distinguir mayúsculas implica pasar a minúsculas al comparar (sin `strcasestr`, que no es ANSI C). | Sugerida por IA, evaluada por el equipo |
| 5 | **Perfil de usuario:** ver todos los tweets de una persona y cuántos tiene. | Complementa la búsqueda por autor y es lo primero que se mira de otra persona. | **Bajo.** Reutiliza `listaBuscarYAccion` con un `cmp` por autor y un contador. | Sugerida por IA, evaluada por el equipo |
| 6 | **Seguir usuarios** y ver un feed solo con los tweets de a quienes sigo. | Es el pedido clásico cuando el feed general crece demasiado para ser útil. | **Alto.** Aparece una relación usuario–usuario (N a N). No se puede guardar dentro de `User` con `fwrite` si contiene una lista (tendría punteros). Habría que persistirla aparte, por ejemplo un `seguidores.bin` con pares (seguidor, seguido), y reconstruirla al cargar. Ver decisiones abiertas. | Sugerida por IA, evaluada por el equipo |
| 7 | **Gestión de cuenta:** cambiar contraseña, eliminar la cuenta y, más adelante, un rol de moderador que pueda borrar cualquier tweet. | Toda plataforma con registro termina necesitándolo. El moderador es la respuesta natural al cambio de permisos de la decisión 2 del md 02. | **Medio.** Cambiar contraseña: modificar el dato en la lista y volver a guardar. Eliminar cuenta: `listaEliminarNodo` sobre usuarios, con una decisión abierta sobre sus tweets. Moderador: campo `rol` en `User`, lo que cambia el struct y obliga a migrar `users.bin`. | Sugerida por IA, evaluada por el equipo |

## 2. Decisiones abiertas que estas operaciones nos van a obligar a tomar

### Hilos (operación 2)

- **¿Dónde se guarda la relación?** A decidir entre (a) un campo `idPadre` en cada `Tweet`, o (b) una lista de respuestas dentro de cada tweet. La opción (a) mantiene el registro de tamaño fijo y se puede seguir guardando con `fwrite` directo; la (b) no, porque tendría punteros.
- **¿Qué pasa al eliminar un tweet con respuestas?** A decidir entre borrar también las respuestas, o dejar el hilo con un "tweet eliminado".
- Los comentarios deberían respetar también el límite de 140 caracteres y los permisos de autor al modificar y eliminar.

### Seguir usuarios (operación 6)

- **Estructura para la relación:** una lista de pares en memoria, o una lista de seguidos por usuario.
- Al eliminar una cuenta hay que limpiar también sus relaciones.

### Eliminar una cuenta (operación 7)

- **¿Qué pasa con sus tweets?** Borrarlos, o conservarlos como "usuario eliminado".
- **Riesgo que ya existe en el diseño actual:** el `Tweet` guarda el autor como texto, sin un identificador de usuario. Si se elimina una cuenta y alguien se registra después con el mismo nombre, esa persona quedaría como autora de los tweets viejos y podría modificarlos o borrarlos. Una posible salida es darle a cada usuario un id numérico, y que el tweet guarde ese id.

## 3. Evolutivos que surgen de las limitaciones conocidas (md 01)

| Limitación actual | Evolución esperada | Diseño / observaciones |
|-------------------|--------------------|------------------------|
| Los datos se guardan solo al salir por el menú; un cierre inesperado los pierde. | Guardar en cada operación (o periódicamente). | Reescribir el archivo completo tras cada cambio es simple pero recorre todo el archivo cada vez. La consigna pide que los recorridos de archivos sean mínimos y evitar archivos temporales, así que conviene evaluar agregar registros al final (`"ab"`) para publicaciones y modificar en el lugar con `fseek` para ediciones. |
| Un tweet largo se trunca sin avisar y el registro duplicado no muestra mensaje. | Validar y avisar al usuario. | Devolver un código nuevo (por ejemplo `TWEET_MUY_LARGO`) desde `tweetPublicar` y que `main` muestre el mensaje. Para `USER_DUPLICADO` alcanza con revisar el retorno en `main`. |
| Bytes basura en `users.bin` y uso de `strcpy` sin límite. | Inicializar con `memset` y copiar con límite (`strncpy`). | Cambio interno, sin efecto en el formato salvo que el archivo queda determinístico. |

---

# Uso de Inteligencia Artificial

## 1. Cómo decidimos usar la IA

La consigna permite usar IA y a la vez pide que aprendamos a analizar un problema, decidir, probar y defender lo que hicimos. Por eso, en lugar de usarla para que resuelva el trabajo, la usamos donde nos ayuda a avanzar sin sacarnos el razonamiento:

- **Como interlocutora**, para contrastar ideas y encontrar puntos que no habíamos visto. Las decisiones de diseño siguieron siendo del equipo: la IA aportaba opciones y contraargumentos, y nosotros elegíamos.
- **Como ayuda en tareas repetitivas**, que llevan tiempo pero no aportan aprendizaje (tipear un `.h`, armar un banner, escribir la estructura de un archivo de pruebas).
- **Como apoyo para entender y ordenar**, tanto la consigna como nuestra propia documentación.

Más allá de algunos arreglos y revisiones puntuales del código, los usos principales fueron los cuatro que siguen.

## 2. Usos principales

### 2.1 Consultas sobre los archivos del trabajo práctico y el flujo de trabajo

La usamos para entender qué pedía la consigna y cómo cumplirla. Por ejemplo:

- ¿Qué debo incluir en la bitácora?
- ¿Cómo se usa GitFlow y cómo lo aplicamos a nuestro repositorio (rama `develop`, ramas de funcionalidad y de arreglo, rama de release y tags por sprint)?
- ¿Cómo se entrega el trabajo?
- ¿Falta alguna funcionalidad para cubrir el sprint 1?

### 2.2 Decisiones de diseño

La usamos para contrastar ideas y ordenar el razonamiento antes de decidir. Por ejemplo:

- "Pensé este flujo para este proceso, ¿qué te parece?"
- "¿Qué funciones debería desarrollar para cubrir esta parte?"
- "¿Me conviene hacer una librería con funciones de lista estándar, o réplicas de las funciones personalizadas para tweets y usuarios?"
- "¿Está bien este código? ¿Detectás algún bug?"

También consultamos con IA las operaciones que podrían pedirnos en el próximo sprint. En el documento de operaciones futuras, la columna "Origen" indica cuáles ideas fueron nuestras y cuáles surgieron de esa consulta.

### 2.3 Generación de código de apoyo

La usamos para no tipear cosas repetitivas que no son la lógica propia del programa. Por ejemplo:

- "Hice este `.c`, armame el `.h` con todas las firmas de las funciones."
- Ayuda con la interfaz de `main`, como el logo del banner.
- "Armame un archivo para testear todas estas funciones."

### 2.4 Documentación: bitácora y hoja de tests

También la usamos para armar la documentación del trabajo: la descripción del funcionamiento del programa, el registro de problemas y decisiones, las operaciones futuras, el uso de IA y la hoja de tests. La forma de trabajo fue la siguiente:

- Le pasamos el código, los archivos de datos, el historial de commits y las consignas.
- A partir de eso, la IA armó borradores de cada sección, con el formato que pide la consigna (por ejemplo, para cada decisión: problema, alternativas, qué elegimos, por qué, qué observamos y si cambió).
- Lo que no se podía saber leyendo el código (los motivos, las alternativas que discutimos, cómo detectamos cada problema) quedó marcado como pendiente para que lo completemos nosotros, en lugar de inventarlo.
- Cada sección la revisamos y ajustamos entre todos antes de incorporarla, porque la defensa individual exige poder explicar cualquier punto de la bitácora.

## 3. Un caso en que la IA recomendó algo distinto a lo que resultó mejor

**Qué consultamos.** Cuando estábamos decidiendo cómo guardar usuarios y tweets, le preguntamos a la IA por listas enlazadas.

**Qué respondió.** Nos recomendó no hacer una lista genérica con `void *` y armar en su lugar dos listas tipadas, una para `Usuario` y otra para `Tweet`, aunque hubiera que duplicar código (dos "insertar", dos "eliminar", etc.). Sus argumentos: la genérica obliga a castear en cada lugar donde se lee un dato, el compilador no avisa si se castea al tipo equivocado (queda un error que solo aparece al ejecutar), hace falta pasar funciones de comparación como parámetro, y las listas tipadas son más simples de leer y de defender.

**Qué evaluamos.** Los riesgos que mencionó eran reales, pero no nos convencía el costo de duplicar toda la lógica de la lista por cada entidad, y menos sabiendo que van a aparecer nuevos pedidos en los próximos sprints. Lo discutimos entre nosotros y decidimos probar la lista genérica en el código.

**Qué observamos.**

- La lista genérica resultó práctica: hay una sola implementación (`lista.c`) que usan tanto usuarios como tweets, y cada módulo le pasa sus propias funciones de comparación y de acción.
- Nuestra versión copia el dato dentro del nodo (`memcpy` con `dataSize`), así que quien la usa no tiene que reservar ni liberar memoria por su cuenta.
- El casteo quedó acotado: hay 11 casteos en total, todos en `user.c` y `tweet.c`, y ninguno en `main.c`.

**Qué solución adoptamos.** Nos quedamos con la lista doblemente enlazada genérica como base común. La recomendación de la IA nos sirvió para tener presentes los riesgos del casteo, pero la decisión final fue del equipo, y para nuestro caso resultó mejor que la alternativa que nos sugirió.

## 4. Ventajas que le vimos

- **Ahorro de tiempo en lo mecánico.** Headers, banner, estructura de pruebas y armado del formato de la documentación nos dejaron más tiempo para pensar las decisiones de diseño.
- **Contraste de ideas antes de decidir.** Nos sirvió tener alguien con quien discutir alternativas, por ejemplo entre una librería genérica de listas y funciones repetidas por entidad, y ver consecuencias que no habíamos pensado.
- **Una segunda lectura del código.** Al armar la documentación aparecieron limitaciones que conviene tener anotadas, que después pudimos convertir en casos de prueba propios y en evolutivos.
- **Interpretación de la consigna.** Nos ayudó a no dejar afuera nada de lo que se pide entregar (bitácora, hoja de tests, tags, ramas, formato de entrega).
- **Documentación más ordenada y consistente.** Mantener un mismo formato en todas las secciones nos evitó rehacer el trabajo al juntarlo en un solo documento.

## 5. Criterios con los que la usamos

- **La decisión final es del equipo.** La IA propone; nosotros evaluamos, aceptamos, ajustamos o descartamos.
- **Todo lo que entra al proyecto tiene que poder explicarse.** Si no entendemos una propuesta o una porción de código, no la incorporamos.
- **Lo generado se verifica.** Compilamos, probamos y contrastamos con la consigna antes de darlo por bueno, porque una respuesta puede salir incompleta o con errores.
- **La documentación cuenta lo que realmente pasó.** Los motivos y las observaciones los escribimos nosotros; la IA ayuda a redactarlos y ordenarlos.
