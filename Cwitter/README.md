# Cwitter

Red social de consola escrita en **ANSI C**, desarrollada como Trabajo Práctico de *Algoritmo y Estructura de Datos* (3640 – Comisión 90-3900) en la Universidad Nacional de La Matanza (UNLaM), Departamento de Ingeniería e Investigaciones Tecnológicas, Centro Regional Universitario de Ituzaingó.

**Grupo:** Puntero
**Estado actual:** Sprint 1 – `v1-fundamentos`

## Integrantes

| Nombre | Usuario de GitHub |
|--------|-------------------|
| Agustina Rico | [@agussrico](https://github.com/agussrico) |
| Franco Lago | [@LagooFran](https://github.com/LagooFran) |
| Mateo Feiertag | [@mateofeiertag](https://github.com/mateofeiertag) |
| Francisco Garbaccio | [@Garbaaa](https://github.com/Garbaaa) |

## Funcionalidades (Sprint 1)

- Registro de nuevos usuarios.
- Inicio y cierre de sesión.
- Publicación, modificación y eliminación de tweets (hasta 140 caracteres).
- Consulta del feed (tweets de todos los usuarios, del más antiguo al más nuevo).
- Búsqueda de tweets por texto.
- Persistencia de usuarios y tweets entre ejecuciones.

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
| `tweet.h` / `tweet.c` | Entidad `Tweet`: publicar, modificar, eliminar, feed, búsqueda, carga y guardado de `tweets.bin`. |
| `main.c` | Interfaz de consola (menús) y flujo general. |
| `config.ini` | Configuración del sistema operativo para la limpieza de pantalla. |
| `test_users.c` / `test_tweets.c` | Programas de prueba de cada módulo. |

Las dependencias van en un solo sentido: `main` → `user` / `tweet` → `lista`.

## Compilación y ejecución

El proyecto incluye un archivo de proyecto de **Code::Blocks** (`Cwitter.cbp`), que es la forma más simple de compilarlo.

Desde la terminal, con GCC (ajustar según el entorno):

```bash
gcc -ansi -Wall -Wextra main.c lista.c user.c tweet.c -o cwitter
./cwitter
```

Para compilar los tests de cada módulo (cada `test_*.c` tiene su propio `main`, por lo que se compilan por separado de `main.c`):

```bash
gcc -ansi -Wall -Wextra test_users.c lista.c user.c -o test_users
gcc -ansi -Wall -Wextra test_tweets.c lista.c tweet.c -o test_tweets
```

### Configuración de `config.ini`

Para que la pantalla se limpie correctamente, `config.ini` debe estar en la carpeta desde donde se ejecuta el programa y contener en su primera línea:

```ini
linux=false
```

Cambiar a `linux=true` si se usa Linux (usa `clear` en lugar de `cls`). Si el archivo no existe, se asume `cls` (Windows).

## Persistencia

Los datos se guardan en archivos binarios, uno por entidad:

- `users.bin`
- `tweets.bin`

Se cargan al iniciar el programa y se guardan **al salir por el menú principal** (opción *Salir* del menú sin sesión). Si el programa se cierra de otra forma, se pierden los cambios de esa sesión.

## Flujo de trabajo (Git)

Se usa un flujo basado en GitFlow:

- `main`: código estable.
- `develop`: rama de integración.
- `feature/*` y `fix/*`: ramas de trabajo, mergeadas a `develop`.
- `release/v1-fundamentos`: rama de release del Sprint 1.
- Tags por sprint: `v1-fundamentos`, `v2-procesamiento`, `v3-final`.

## Documentación

- [`BITACORA.md`](BITACORA.md): decisiones de diseño, problemas encontrados, operaciones futuras y registro de uso de IA.
- [`HojaDeTests.md`](HojaDeTests.md): casos de prueba del Sprint 1 (usuarios y tweets).

## Limitaciones conocidas

- El guardado ocurre solo al salir por el menú.
- Un tweet de más de 140 caracteres se trunca sin avisar.
- El registro de un usuario duplicado no muestra mensaje.
- Ingresar letras donde se espera un número en el menú deja al programa en un bucle.
- La búsqueda distingue mayúsculas de minúsculas.

El detalle y los planes de evolución están en la bitácora.
