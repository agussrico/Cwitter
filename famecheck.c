#include "famecheck.h"
 
#define URL_USUARIO               "https://algoritmos-api.azurewebsites.net/api/Cwitter/Usuario"
#define URL_CWEET                 "https://algoritmos-api.azurewebsites.net/api/Cwitter/Cweet"
#define PREFIJO_AUTORIZACION      "Authorization: Bearer "
#define TIMEOUT_PETICION_SEGUNDOS 10L
 
#define ARCHIVO_API_KEY           "apikey.key"
#define TAM_LINEA_API_KEY         256
 
typedef struct
{
    char *datos;
    size_t tam;
} RespuestaHttp;
 
/// Header completo "Authorization: Bearer <clave>", se arma al leer apikey.txt 
char headerAuth[TAM_LINEA_API_KEY + 32] = "";
 

/// Leer api key del archivo .key
int leerApiKeyDeArchivo(void)
{
    FILE *archivo;
    char linea[TAM_LINEA_API_KEY];
    int estado = FAMECHECK_ERROR_CONFIG;
 
    archivo = fopen(ARCHIVO_API_KEY, "r");
    if (archivo == NULL)
    {
        return FAMECHECK_ERROR_CONFIG;
    }
 
    if (fgets(linea, TAM_LINEA_API_KEY, archivo) != NULL)
    {
        linea[strcspn(linea, " \t\r\n")] = '\0'; /* saca el salto de linea y espacios */
 
        if (linea[0] != '\0')
        {
            strcpy(headerAuth, PREFIJO_AUTORIZACION);
            strcat(headerAuth, linea);
            estado = FAMECHECK_OK;
        }
    }
 
    fclose(archivo);
 
    return estado;
}

/// Funcion que llama Curl para guardar la respuesta.
size_t escribirRespuesta(void *contenido, size_t tamElemento, size_t cantidadElementos, void *contexto)
{
    size_t tamNuevo = tamElemento * cantidadElementos;
    RespuestaHttp *respuesta = (RespuestaHttp *) contexto;
    char *bufferAgrandado = realloc(respuesta->datos, respuesta->tam + tamNuevo + 1);
 
    if (bufferAgrandado == NULL)
    {
        return 0;
    }
 
    respuesta->datos = bufferAgrandado;
    memcpy(&(respuesta->datos[respuesta->tam]), contenido, tamNuevo);
    respuesta->tam += tamNuevo;
    respuesta->datos[respuesta->tam] = '\0';
 
    return tamNuevo;
}

/// Funcion que envia un POST
int enviarPeticion(const char *url, const char *body, RespuestaHttp *respuesta, long *codigoHttp)
{
    CURL *curl;
    CURLcode resultado;
    struct curl_slist *headers;
 
    respuesta->datos = NULL;
    respuesta->tam = 0;
 
    curl = curl_easy_init();
    if (curl == NULL)
    {
        return FAMECHECK_ERROR_RED;
    }
 
    headers = curl_slist_append(NULL, "Content-Type: application/json");
    headers = curl_slist_append(headers, headerAuth);
 
    ///hace la request y llama a la funcion escribir respuesta
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    //HACE QUE LA FUNCION SIRVE PARA POST Y GET 
    if (body != NULL)
    {
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);
    }
    //--------
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, escribirRespuesta);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) respuesta);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, TIMEOUT_PETICION_SEGUNDOS);

    ///El resultado de la request se guarda en el struct tipo RespuestaHttp
 
    resultado = curl_easy_perform(curl);
    if (resultado == CURLE_OK)
    {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, codigoHttp);
    }
 
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
 
    if (resultado != CURLE_OK)
    {
        free(respuesta->datos);
        return FAMECHECK_ERROR_RED;
    }
 
    return FAMECHECK_OK;
}

/// Inicio CURL 

int famecheckIniciar(void)
{
    int estado;
 
    if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK)
    {
        return FAMECHECK_ERROR_RED;
    }
 
    estado = leerApiKeyDeArchivo();
    if (estado != FAMECHECK_OK)
    {
        curl_global_cleanup();
    }
 
    return estado;
}

/// Cierro CURL

void famecheckFinalizar(void)
{
    curl_global_cleanup();
}


/// Verifico una cuenta (MODIFICA DIRECTAMENTE EL VALOR DE VERIFICADO QUE LE PASO)

int famecheckVerificarCuenta(const char *nombre, int *verificado)
{
    RespuestaHttp respuesta;
    long codigoHttp = 0;
    cJSON *json;
    cJSON *campo;
    char *body;
    int estado;
 
    if (headerAuth[0] == '\0')
    {
        return FAMECHECK_ERROR_CONFIG; /* no se llamo a famecheckIniciar o fallo */
    }
 
    /* Body: {"Nombre": "<nombre>"} */
    json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "Nombre", nombre);
    body = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);
    if (body == NULL)
    {
        return FAMECHECK_ERROR_MEMORIA;
    }
 
    estado = enviarPeticion(URL_USUARIO, body, &respuesta, &codigoHttp);
    ///Envio post
    cJSON_free(body);
    if (estado != FAMECHECK_OK)
    {
        return estado;
    }
    ///Si post salio bien chequeo la respuesta segun el formato dado
    ///Respuesta: {"nombre": "...", "verificado": true/false} 
    estado = FAMECHECK_ERROR_RESPUESTA;
    if (codigoHttp == 200 || codigoHttp == 201)
    {
        json = cJSON_Parse(respuesta.datos);
        campo = cJSON_GetObjectItemCaseSensitive(json, "verificado");
        if (cJSON_IsBool(campo))
        {
            ///Si el resultado de verificado es true lo configuro como true en verificado
            *verificado = cJSON_IsTrue(campo);
            estado = FAMECHECK_OK;
        }
        cJSON_Delete(json);
    }
 
    free(respuesta.datos);
    
    ///Retorno FAMECEHCK_OK si todo funciono OK
    return estado;
}


int famecheckReportarPublicacion(const char *nombre, const char *mensaje, int *reportadoAFamecheck)
{
    RespuestaHttp respuesta;
    long codigoHttp = 0;
    cJSON *json;
    char *body;
    int estado;

    *reportadoAFamecheck = 0;

    if (headerAuth[0] == '\0')
    {
        return FAMECHECK_ERROR_CONFIG;
    }

    /* Body: {"NombreUsuario": "<nombre>", "Mensaje": "<mensaje>"} */
    json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "NombreUsuario", nombre);
    cJSON_AddStringToObject(json, "Mensaje", mensaje);
    body = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);
    if (body == NULL)
    {
        return FAMECHECK_ERROR_MEMORIA;
    }

    estado = enviarPeticion(URL_CWEET, body, &respuesta, &codigoHttp);
    cJSON_free(body);
    if (estado != FAMECHECK_OK)
    {
        return estado;
    }

    free(respuesta.datos);

    if (codigoHttp == 204) //La doc de la api esta mal. Dice q la respuesta es 201 pero enrealidad es 204 porque NO TIENE BODY
    {
        *reportadoAFamecheck = 1;
        return FAMECHECK_OK;
    }

    return FAMECHECK_ERROR_RESPUESTA;
}

//devuelve > 0 si user1 va despues de user2 (orden descendente por cantidad de cweets)
int famecheckCmpActividad(void *user1, void *user2){
    ActividadFame *aUser1 = (ActividadFame *)user1;
    ActividadFame *aUser2 = (ActividadFame *)user2;

    if(aUser1->cantidadCweets < aUser2->cantidadCweets){
        return 1;
    }
    else if(aUser1->cantidadCweets > aUser2->cantidadCweets){
        return -1;
    }
    else{
        //si son iguales desempato por nombre
        if(strcmp(aUser1->nombre, aUser2->nombre) > 0){
            return 1;
        }
        else{
            return -1;
        }
    }
}

//Guarda en una lista el TOP 5 de usuarios chequeados con mas actividad chequeando en la api
int famecheckObtenerTop5(tLista *top)
{
    RespuestaHttp respuesta;
    long codigoHttp = 0;
    cJSON *json;
    cJSON *item;
    cJSON *nombre;
    cJSON *cweets;
    ActividadFame nuevo;
    int estado;

    if (headerAuth[0] == '\0')
    {
        return FAMECHECK_ERROR_CONFIG;
    }

    estado = enviarPeticion(URL_USUARIO, NULL, &respuesta, &codigoHttp); // body NULL = GET 
    if (estado != FAMECHECK_OK)
    {
        return estado;
    }

    estado = FAMECHECK_ERROR_RESPUESTA;
    if (codigoHttp == 200)
    {
        json = cJSON_Parse(respuesta.datos);
        if (cJSON_IsArray(json))
        {
            estado = FAMECHECK_OK;
            cJSON_ArrayForEach(item, json)
            {
                nombre = cJSON_GetObjectItemCaseSensitive(item, "nombre");
                cweets = cJSON_GetObjectItemCaseSensitive(item, "cantidadCweets");
                if (cJSON_IsString(nombre) && cJSON_IsNumber(cweets))
                {
                    memset(&nuevo, 0, sizeof(ActividadFame));
                    strncpy(nuevo.nombre, nombre->valuestring, sizeof(nuevo.nombre) - 1);
                    nuevo.cantidadCweets = cweets->valueint;
                    ///cada elemento que bajo lo guardo en un struct y luego lo inserto ordenado
                    if (listaInsertarOrdenado(top, &nuevo, sizeof(ActividadFame), famecheckCmpActividad) != TODO_OK)
                    {
                        estado = FAMECHECK_ERROR_MEMORIA;
                    }
                    if (top->cant > TOP_CANT)
                    {
                        listaEliminarUltimo(top); /* el sexto sale */
                    }
                }
            }
        }
        cJSON_Delete(json);
    }

    free(respuesta.datos);

    return estado;
}

void famecheckMostrarUsuarioTop(void *data){
    ActividadFame *f = (ActividadFame *)data;
    printf("  Usuario: %s, Cantidad Tweets %d\n", f->nombre, f->cantidadCweets);
}