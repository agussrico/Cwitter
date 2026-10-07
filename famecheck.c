#include "famecheck.h"

#define URL_BASE                  "https://algoritmos-api.azurewebsites.net"
#define URL_USUARIO               (URL_BASE "/api/Cwitter/Usuario")
#define PREFIJO_AUTORIZACION      "Authorization: Bearer NoSeAdondeApunto"
#define TIMEOUT_PETICION_SEGUNDOS 10L
 
#define HTTP_OK      200
#define HTTP_CREADO  201
 
 
typedef struct
{
    char *datos;
    size_t tamanio;
} RespuestaHttp;


int famecheckIniciar(void)
{
    int estado;
 
    if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK)
    {
        return FAMECHECK_ERROR_RED;
    }
 
    estado = leerApiKeyDeConfig();
    if (estado != FAMECHECK_OK)
    {
        curl_global_cleanup();
    }
 
    return estado;
}
 
void famecheckFinalizar(void)
{
    free(apiKey);
    apiKey = NULL;
    curl_global_cleanup();
}
