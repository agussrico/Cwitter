#ifndef FAMECHECK_H_INCLUDED
#define FAMECHECK_H_INCLUDED

#define FAMECHECK_OK                 0
#define FAMECHECK_ERROR_RED          (-1)  /* no se pudo contactar al servicio */
#define FAMECHECK_ERROR_RESPUESTA    (-2)  /* respondio algo inesperado (400, 401, JSON invalido) */
#define FAMECHECK_ERROR_MEMORIA      (-3)
#define FAMECHECK_ERROR_CONFIG       (-4)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "cJSON.h"
#include "user.h"
#include "lista.h"
#include "tweet.h"

int famecheckIniciar(void);
void famecheckFinalizar(void);
int famecheckVerificarCuenta(const char *nombre, int *verificado);

#endif // FAMECHECK_H_INCLUDED