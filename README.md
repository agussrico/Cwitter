# Cwitter

Red social de consola escrita en **ANSI C**, desarrollada como Trabajo PrÃ¡ctico de *Algoritmo y Estructura de Datos* (3640 â€“ ComisiÃ³n 90-3900) en la Universidad Nacional de La Matanza (UNLaM), Departamento de IngenierÃ­a e Investigaciones TecnolÃ³gicas, Centro Regional Universitario de ItuzaingÃ³.

**Grupo:** Puntero
**Estado actual:** Sprint 2 â€“ `v2-procesamiento`

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
- Inicio y cierre de sesiÃ³n.
- PublicaciÃ³n, modificaciÃ³n y eliminaciÃ³n de tweets (hasta 140 caracteres).
- BÃºsqueda de tweets por texto.
- Persistencia de usuarios y tweets entre ejecuciones.

### Sprint 2

- VerificaciÃ³n de cuentas a travÃ©s del servicio externo **FameCheck** (opciÃ³n 0 del menÃº). La verificaciÃ³n se guarda con el usuario y no se pierde al reiniciar.
- Reporte a FameCheck de cada publicaciÃ³n de un usuario verificado. Si el servicio no responde, el tweet se publica igual y el fallo queda registrado en `logs.log`.
- Top 5 de usuarios verificados con mÃ¡s publicaciones (opciÃ³n 6). Los empates se ordenan alfabÃ©ticamente.
- Feed con los tweets de cuentas verificadas primero y despuÃ©s el resto, cada grupo del mÃ¡s reciente al mÃ¡s viejo. Cada tweet muestra su fecha y hora, y la marca `[V]` si el autor estÃ¡ verificado.
- Recorrido del feed de a un tweet por vez: `D` pasa al siguiente y `A` vuelve al anterior.

Reglas a tener en cuenta:

- Sin sesiÃ³n iniciada solo se puede registrarse, iniciar sesiÃ³n o salir. Publicar, ver el feed y buscar requieren estar logueado.
- Solo el autor de un tweet puede modificarlo o eliminarlo.
- No hay un mÃ¡ximo de usuarios ni de tweets: el Ãºnico lÃ­mite es la memoria disponible.
- Nombre de usuario y contraseÃ±a admiten hasta 19 caracteres sin espacios.

## Estructura del proyecto

| Archivo | Responsabilidad |
|---------|-----------------|
| `include/lista.h` / `src/lista.c` | Lista doblemente enlazada genÃ©rica (`void *`), independiente de las entidades. |
| `include/user.h` / `src/user.c` | Entidad `User`: registro, inicio de sesiÃ³n, carga y guardado de `users.bin`. |
| `include/tweet.h` / `src/tweet.c` | Entidad `Tweet`: publicar (y reportar a FameCheck), modificar, eliminar, feed ordenado, bÃºsqueda, carga y guardado de `tweets.bin`. |
| `include/famecheck.h` / `src/famecheck.c` | ComunicaciÃ³n con FameCheck: verificar una cuenta, reportar una publicaciÃ³n, obtener el top 5 y registrar los fallos en `logs.log`. |
| `include/cJSON.h` / `src/cJSON.c` | LibrerÃ­a externa ([DaveGamble/cJSON](https://github.com/DaveGamble/cJSON)) para armar y leer JSON. |
| `src/main.c` | Interfaz de consola (menÃºs), recorrido del feed y flujo general. |
| `config.ini` | ConfiguraciÃ³n del sistema operativo para la limpieza de pantalla y los colores. |
| `apikey.key` | Clave de API de FameCheck. No estÃ¡ en el repositorio: cada integrante crea la suya. |
| `tests/test_users.c` / `tests/test_tweets.c` / `tests/test_famecheck.c` | Programas de prueba de cada mÃ³dulo. |

Las dependencias van en un solo sentido: `main` â†’ `user` / `tweet` / `famecheck` â†’ `lista`. `tweet` usa `famecheck` para reportar cada publicaciÃ³n, y `famecheck` usa `curl` y `cJSON`.

## CompilaciÃ³n y ejecuciÃ³n

### Requisitos

- **libcurl** de 64 bits, para hablar con FameCheck. El proyecto la busca en `C:\libs\curl` (`include` y `lib`), y la carpeta `C:\libs\curl\bin` tiene que estar en el `PATH` para que el programa encuentre la DLL al ejecutarse.
- **`apikey.key`**: un archivo con la clave de API de FameCheck en una sola lÃ­nea, en la carpeta desde donde se ejecuta el programa. Sin la clave, Cwitter funciona igual, pero las opciones que usan FameCheck devuelven error.

### Compilar

El proyecto incluye un archivo de proyecto de **Code::Blocks** (`Cwitter.cbp`), ya configurado con curl, que es la forma mÃ¡s simple de compilarlo.

Desde la terminal, con GCC (ajustar las rutas de curl segÃºn el entorno):

```bash
gcc -Wall -Iinclude -IC:/libs/curl/include src/main.c src/lista.c src/user.c src/tweet.c src/famecheck.c src/cJSON.c -LC:/libs/curl/lib -lcurl -o cwitter
./cwitter
```

Para compilar los tests de cada mÃ³dulo (cada `test_*.c` tiene su propio `main`, por lo que se compilan por separado de `main.c`):

```bash
gcc -Wall -Iinclude tests/test_users.c src/lista.c src/user.c -o test_users
gcc -Wall -Iinclude -IC:/libs/curl/include tests/test_tweets.c src/lista.c src/user.c src/tweet.c src/famecheck.c src/cJSON.c -LC:/libs/curl/lib -lcurl -o test_tweets
gcc -Wall -Iinclude -IC:/libs/curl/include tests/test_famecheck.c src/lista.c src/user.c src/tweet.c src/famecheck.c src/cJSON.c -LC:/libs/curl/lib -lcurl -o test_famecheck
```

### ConfiguraciÃ³n de `config.ini`

Para que la pantalla se limpie correctamente, `config.ini` debe estar en la carpeta desde donde se ejecuta el programa y contener en su primera lÃ­nea:

```ini
linux=false
```

Cambiar a `linux=true` si se usa Linux (usa `clear` en lugar de `cls` y muestra los menÃºs con colores). Si el archivo no existe, se asume `cls` (Windows).

## Persistencia

Los datos se guardan en archivos binarios, uno por entidad:

- `users.bin`
- `tweets.bin`

Se cargan al iniciar el programa y se guardan **al salir por el menÃº principal** (opciÃ³n *Salir* del menÃº sin sesiÃ³n). Si el programa se cierra de otra forma, se pierden los cambios de esa sesiÃ³n.

Como en el Sprint 2 se agregaron campos a `User` y `Tweet`, los `.bin` generados con el Sprint 1 ya no se leen correctamente y hay que borrarlos.

Los reportes fallidos a FameCheck se agregan como una lÃ­nea de texto a `logs.log` (fecha y hora, id del tweet, autor y motivo).

## Flujo de trabajo (Git)

Se usa un flujo basado en GitFlow:

- `main`: cÃ³digo estable.
- `develop`: rama de integraciÃ³n.
- `feature/*` y `fix/*`: ramas de trabajo, mergeadas a `develop`.
- `release/v1-fundamentos` y `release/v2-procesamiento`: ramas de release de cada sprint.
- Tags por sprint: `v1-fundamentos`, `v2-procesamiento`, `v3-final`.

## DocumentaciÃ³n

- [`BITACORA.md`](BITACORA.md): decisiones de diseÃ±o, problemas encontrados, operaciones futuras y registro de uso de IA.
- [`TESTS.md`](TESTS.md): hoja de tests del Sprint 1 y el Sprint 2, con la salida obtenida de cada caso.

## Limitaciones conocidas

- El guardado ocurre solo al salir por el menÃº.
- Un tweet de mÃ¡s de 140 caracteres se trunca sin avisar.
- El registro de un usuario duplicado no muestra mensaje.
- Ingresar letras donde se espera un nÃºmero en el menÃº sin sesiÃ³n deja al programa en un bucle.
- La bÃºsqueda distingue mayÃºsculas de minÃºsculas.
- Si alguien se verifica despuÃ©s de publicar, sus tweets anteriores no suben en el feed.
- Los reportes fallidos a FameCheck quedan en el log, pero no se reintentan.

El detalle y los planes de evoluciÃ³n estÃ¡n en la bitÃ¡cora.
