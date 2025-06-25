#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cjson/cJSON.h>
#include <stdbool.h>

typedef struct
{
    int puerto;
    char nombre[50];
    char protocolo[10];
} ServicioComun;

typedef struct
{
    int puerto;
    char descripcion[50];
    int riesgo;
} Anomalia;

ServicioComun *servicios_comunes = NULL;
Anomalia *anomalias = NULL;
int total_servicios = 0;
int total_anomalias = 0;

void load_json(const char *filename)
{
    FILE *f = fopen(filename, "r");
    if (!f)
    {
        perror("Error al abrir el archivo JSON");
        return;
    }
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *data = (char *)malloc(len + 1);
    fread(data, 1, len, f);
    fclose(f);
    data[len] = '\0';
    cJSON *json = cJSON_Parse(data);
    if (!json)
    {
        printf("Error al parsear JSON: %s\n", cJSON_GetErrorPtr());
        free(data);
        return;
    }
    cJSON *servicios_json = cJSON_GetObjectItemCaseSensitive(json, "servicios_comunes");
    total_servicios = cJSON_GetArraySize(servicios_json);
    servicios_comunes = malloc(total_servicios * sizeof(ServicioComun));
    cJSON *item;
    int i = 0;
    cJSON_ArrayForEach(item, servicios_json)
    {
        servicios_comunes[i].puerto = cJSON_GetObjectItem(item, "puerto")->valueint;
        strncpy(servicios_comunes[i].nombre, cJSON_GetObjectItem(item, "nombre")->valuestring, 49);
        strncpy(servicios_comunes[i].protocolo, cJSON_GetObjectItem(item, "protocolo")->valuestring, 9);
        i++;
    }
    cJSON *anomalias_json = cJSON_GetObjectItemCaseSensitive(json, "anomalias");
    total_anomalias = cJSON_GetArraySize(anomalias_json);
    anomalias = malloc(total_anomalias * sizeof(Anomalia));
    i = 0;
    cJSON_ArrayForEach(item, anomalias_json)
    {
        anomalias[i].puerto = cJSON_GetObjectItem(item, "puerto")->valueint;
        strncpy(anomalias[i].descripcion, cJSON_GetObjectItem(item, "descripcion")->valuestring, 99);
        anomalias[i].riesgo = cJSON_GetObjectItem(item, "riesgo")->valueint;
        i++;
    }
    cJSON_Delete(json);
    free(data);
}

int es_servicio_comun(int puerto)
{
    for (int i = 0; i < total_servicios; i++)
        if (servicios_comunes[i].puerto == puerto)
            return i;
    return -1;
}

int es_anomalia(int puerto)
{
    for (int i = 0; i < total_anomalias; i++)
        if (anomalias[i].puerto == puerto)
            return i;
    return -1;
}

const char *clasificar_puerto_desconocido(int puerto)
{
    if (puerto <= 1024)
        return "\033[37m[ALERTA] Puerto privilegiado no registrado\033[0m";
    else if (puerto > 1024 && puerto <= 49151)
        return "\033[36m[INFO] Puerto registrado no catalogado\033[0m";
    else
        return "\033[35m[OBSERVADO] Puerto efímero inusual\033[0m";
}

char *verificar_puerto(int puerto)
{
    int i_comun = es_servicio_comun(puerto);
    int i_anomalia = es_anomalia(puerto);
    if (i_comun != -1)
        printf("Puerto %d: \033[32mServicio común (%s)\033[0m\n", puerto, servicios_comunes[i_comun].nombre);
    else if (i_anomalia != -1)
        printf("Puerto %d: \033[31m%s\033[0m\n", puerto, anomalias[i_anomalia].descripcion);
    else
        printf("Puerto %d: %s\n", puerto, clasificar_puerto_desconocido(puerto));
}

void liberar_memoria()
{
    free(servicios_comunes);
    free(anomalias);
    total_servicios = 0;
    total_anomalias = 0;
}