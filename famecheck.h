#ifndef FAMECHECK_H_INCLUDED
#define FAMECHECK_H_INCLUDED

#define FAMECHECK_OK                 0
#define FAMECHECK_ERROR_RED          (-1)  /* no se pudo contactar al servicio */
#define FAMECHECK_ERROR_RESPUESTA    (-2)  /* respondio algo inesperado (400, 401, JSON invalido) */
#define FAMECHECK_ERROR_MEMORIA      (-3)
#define FAMECHECK_ERROR_CONFIG       (-4)

#define TOP_CANT 5

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <time.h>
#include "cJSON.h"
#include "user.h"
#include "lista.h"
#include "tweet.h"


//estructura para bajar la actividad de los usuarios y sacar el top 5
typedef struct {
    char nombre[20];
    int cantidadCweets;
} ActividadFame;



int famecheckIniciar(void);
void famecheckFinalizar(void);
int famecheckVerificarCuenta(const char *, int *);
int famecheckReportarPublicacion(const char *, const char *, int *);
int famecheckObtenerTop5(tLista *top);
void famecheckMostrarUsuarioTop(void *data);
int famecheckRegistrarLog(char* user, int tweetId, char* error);

#endif // FAMECHECK_H_INCLUDED