# Cwitter

Red social de consola escrita en **ANSI C**, desarrollada como Trabajo Práctico de *Algoritmo y Estructura de Datos* (3640 – Comisión 90-3900) en la Universidad Nacional de La Matanza (UNLaM), Departamento de Ingeniería e Investigaciones Tecnológicas, Centro Regional Universitario de Ituzaingó.

**Grupo:** Puntero
**Estado actual:** Sprint 2 – `v2-procesamiento`

## Integrantes

| Nombre | Usuario de GitHub |
|--------|-------------------|
| Agustina Rico | [@agussrico](https://github.com/agussrico) |
| Franco Lago | [@LagooFran](https://github.com/LagooFran) |
| Mateo Feiertag | [@mateofeiertag](https://github.com/mateofeiertag) |
| Francisco Garbaccio | [@Garbaaa](https://github.com/Garbaaa) |

## Funcionalidades

### Sprint 1

- Registro de nuevos usuarios.
- Inicio y cierre de sesión.
- Publicación, modificación y eliminación de tweets (hasta 140 caracteres).
- Búsqueda de tweets por texto.
- Persistencia de usuarios y tweets entre ejecuciones.

### Sprint 2

- Verificación de cuentas a través del servicio externo **FameCheck** (opción 0 del menú). La verificación se guarda con el usuario y no se pierde al reiniciar.
- Reporte a FameCheck de cada publicación de un usuario verificado. Si el servicio no responde, el tweet se publica igual y el fallo queda registrado en `logs.log`.
- Top 5 de usuarios verificados con más publicaciones (opción 6). Los empates se ordenan alfabéticamente.
- Feed con los tweets de cuentas verificadas primero y después el resto, cada grupo del más reciente al más viejo. Cada tweet muestra su fecha y hora, y la marca `[V]` si el autor está verificado.
- Recorrido del feed de a un tweet por vez: `D` pasa al siguiente y `A` vuelve al anterior.

Reglas a tener en cuenta:

- Sin sesión iniciada solo se puede registrarse, iniciar sesión o salir. Publicar, ver el feed y buscar requieren estar logueado.
- Solo el autor de un tweet puede modificarlo o eliminarlo.
- No hay un máximo de usuarios ni de tweets: el único límite es la memoria disponible.
- Nombre de usuario y contraseña admiten hasta 19 caracteres sin espacios.

## Estructura del proyecto

| Archivo | Responsabilidad |
|---------|-----------------|
| `lista.h` / `lista.c` | Lista doblemente enlazada genérica (`void *`), independiente de las entidades. |
| `user.h` / `user.c` | Entidad `User`: registro, inicio de sesión, carga y guardado de `users.bin`. |
| `tweet.h` / `tweet.c` | Entidad `Tweet`: publicar (y reportar a FameCheck), modificar, eliminar, feed ordenado, búsqueda, carga y guardado de `tweets.bin`. |
| `famecheck.h` / `famecheck.c` | Comunicación con FameCheck: verificar una cuenta, reportar una publicación, obtener el top 5 y registrar los fallos en `logs.log`. |
| `cJSON.h` / `cJSON.c` | Librería externa ([DaveGamble/cJSON](https://github.com/DaveGamble/cJSON)) para armar y leer JSON. |
| `main.c` | Interfaz de consola (menús), recorrido del feed y flujo general. |
| `config.ini` | Configuración del sistema operativo para la limpieza de pantalla y los colores. |
| `apikey.key` | Clave de API de FameCheck. No está en el repositorio: cada integrante crea la suya. |
| `test_users.c` / `test_tweets.c` / `test_famecheck.c` | Programas de prueba de cada módulo. |

Las dependencias van en un solo sentido: `main` → `user` / `tweet` / `famecheck` → `lista`. `tweet` usa `famecheck` para reportar cada publicación, y `famecheck` usa `curl` y `cJSON`.

## Compilación y ejecución

### Requisitos

- **libcurl** de 64 bits, para hablar con FameCheck. El proyecto la busca en `C:\libs\curl` (`include` y `lib`), y la carpeta `C:\libs\curl\bin` tiene que estar en el `PATH` para que el programa encuentre la DLL al ejecutarse.
- **`apikey.key`**: un archivo con la clave de API de FameCheck en una sola línea, en la carpeta desde donde se ejecuta el programa. Sin la clave, Cwitter funciona igual, pero las opciones que usan FameCheck devuelven error.

### Compilar

El proyecto incluye un archivo de proyecto de **Code::Blocks** (`Cwitter.cbp`), ya configurado con curl, que es la forma más simple de compilarlo.

Desde la terminal, con GCC (ajustar las rutas de curl según el entorno):

```bash
gcc -Wall -IC:/libs/curl/include main.c lista.c user.c tweet.c famecheck.c cJSON.c -LC:/libs/curl/lib -lcurl -o cwitter
./cwitter
```

Para compilar los tests de cada módulo (cada `test_*.c` tiene su propio `main`, por lo que se compilan por separado de `main.c`):

```bash
gcc -Wall test_users.c lista.c user.c -o test_users
gcc -Wall -IC:/libs/curl/include test_tweets.c lista.c user.c tweet.c famecheck.c cJSON.c -LC:/libs/curl/lib -lcurl -o test_tweets
gcc -Wall -IC:/libs/curl/include test_famecheck.c lista.c user.c tweet.c famecheck.c cJSON.c -LC:/libs/curl/lib -lcurl -o test_famecheck
```

### Configuración de `config.ini`

Para que la pantalla se limpie correctamente, `config.ini` debe estar en la carpeta desde donde se ejecuta el programa y contener en su primera línea:

```ini
linux=false
```

Cambiar a `linux=true` si se usa Linux (usa `clear` en lugar de `cls` y muestra los menús con colores). Si el archivo no existe, se asume `cls` (Windows).

## Persistencia

Los datos se guardan en archivos binarios, uno por entidad:

- `users.bin`
- `tweets.bin`

Se cargan al iniciar el programa y se guardan **al salir por el menú principal** (opción *Salir* del menú sin sesión). Si el programa se cierra de otra forma, se pierden los cambios de esa sesión.

Como en el Sprint 2 se agregaron campos a `User` y `Tweet`, los `.bin` generados con el Sprint 1 ya no se leen correctamente y hay que borrarlos.

Los reportes fallidos a FameCheck se agregan como una línea de texto a `logs.log` (fecha y hora, id del tweet, autor y motivo).

## Flujo de trabajo (Git)

Se usa un flujo basado en GitFlow:

- `main`: código estable.
- `develop`: rama de integración.
- `feature/*` y `fix/*`: ramas de trabajo, mergeadas a `develop`.
- `release/v1-fundamentos` y `release/v2-procesamiento`: ramas de release de cada sprint.
- Tags por sprint: `v1-fundamentos`, `v2-procesamiento`, `v3-final`.

## Documentación

- [`BITACORA.md`](BITACORA.md): decisiones de diseño, problemas encontrados, operaciones futuras y registro de uso de IA.
- [`TESTS.md`](TESTS.md): hoja de tests del Sprint 1 y el Sprint 2, con la salida obtenida de cada caso.

## Limitaciones conocidas

- El guardado ocurre solo al salir por el menú.
- Un tweet de más de 140 caracteres se trunca sin avisar.
- El registro de un usuario duplicado no muestra mensaje.
- Ingresar letras donde se espera un número en el menú sin sesión deja al programa en un bucle.
- La búsqueda distingue mayúsculas de minúsculas.
- Si alguien se verifica después de publicar, sus tweets anteriores no suben en el feed.
- Los reportes fallidos a FameCheck quedan en el log, pero no se reintentan.

El detalle y los planes de evolución están en la bitácora.
