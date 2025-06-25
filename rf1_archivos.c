#include "rf1_archivos.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <stdbool.h>
#include <openssl/sha.h> // Para SHA256

// Inicializar lista dinámica
void lista_inicializar(ListaArchivos *lista)
{
    lista->cantidad = 0;
    lista->capacidad = 100;
    lista->datos = malloc(sizeof(ArchivoHash) * lista->capacidad);
    if (!lista->datos)
    {
        perror("malloc");
        exit(1);
    }
}

// Agregar elemento (aumenta capacidad si es necesario)
void lista_agregar(ListaArchivos *lista, const char *ruta, unsigned char hash[32])
{
    if (lista->cantidad == lista->capacidad)
    {
        lista->capacidad *= 2;
        lista->datos = realloc(lista->datos, sizeof(ArchivoHash) * lista->capacidad);
        if (!lista->datos)
        {
            perror("realloc");
            exit(1);
        }
    }
    strncpy(lista->datos[lista->cantidad].ruta, ruta, MAX_PATH - 1);
    lista->datos[lista->cantidad].ruta[MAX_PATH - 1] = '\0';
    memcpy(lista->datos[lista->cantidad].hash, hash, 32);
    lista->cantidad++;
}

// Liberar memoria
void lista_liberar(ListaArchivos *lista)
{
    if (lista->datos)
    {
        free(lista->datos);
        lista->datos = NULL;
    }
    lista->cantidad = 0;
    lista->capacidad = 0;
}

// Calcular SHA256 de un archivo
int calcular_sha256(const char *ruta, unsigned char salida_hash[32])
{
    FILE *f = fopen(ruta, "rb");
    if (!f)
    {
        perror("fopen");
        return -1;
    }
    SHA256_CTX sha256;
    SHA256_Init(&sha256);

    unsigned char buffer[8192];
    size_t bytes_leidos;
    while ((bytes_leidos = fread(buffer, 1, sizeof(buffer), f)) != 0)
    {
        SHA256_Update(&sha256, buffer, bytes_leidos);
    }
    SHA256_Final(salida_hash, &sha256);
    fclose(f);
    return 0;
}

// Recorrer directorio recursivamente y agregar archivos con hash a la lista
void recorrer_directorio_con_hash_aux(const char *base_path, const char *actual_path, ListaArchivos *lista)
{
    char ruta_completa[MAX_PATH];
    if (strlen(actual_path) == 0)
        snprintf(ruta_completa, sizeof(ruta_completa), "%s", base_path);
    else
        snprintf(ruta_completa, sizeof(ruta_completa), "%s/%s", base_path, actual_path);

    DIR *dir = opendir(ruta_completa);
    if (!dir)
    {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        // Ignorar "." y ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char ruta_relativa[MAX_PATH];
        if (strlen(actual_path) == 0)
            snprintf(ruta_relativa, sizeof(ruta_relativa), "%s", entry->d_name);
        else
            snprintf(ruta_relativa, sizeof(ruta_relativa), "%s/%s", actual_path, entry->d_name);

        char ruta_abs[MAX_PATH];
        snprintf(ruta_abs, sizeof(ruta_abs), "%s/%s", base_path, ruta_relativa);

        struct stat st;
        if (stat(ruta_abs, &st) == 0)
        {
            if (S_ISDIR(st.st_mode))
            {
                recorrer_directorio_con_hash_aux(base_path, ruta_relativa, lista);
            }
            else if (S_ISREG(st.st_mode))
            {
                unsigned char hash[32];
                if (calcular_sha256(ruta_abs, hash) == 0)
                {
                    lista_agregar(lista, ruta_relativa, hash);
                }
                else
                {
                    fprintf(stderr, "Error calculando SHA256: %s\n", ruta_abs);
                }
            }
        }
    }
    closedir(dir);
}

void recorrer_directorio_con_hash(const char *path, ListaArchivos *lista)
{
    recorrer_directorio_con_hash_aux(path, "", lista);
}

// Guardar baseline en archivo
int guardar_baseline(const char *ruta_archivo, ListaArchivos *lista)
{
    FILE *f = fopen(ruta_archivo, "w");
    if (!f)
    {
        perror("fopen guardar baseline");
        return -1;
    }
    for (int i = 0; i < lista->cantidad; i++)
    {
        fprintf(f, "%s ", lista->datos[i].ruta);
        for (int j = 0; j < 32; j++)
        {
            fprintf(f, "%02x", lista->datos[i].hash[j]);
        }
        fprintf(f, "\n");
    }
    fclose(f);
    return 0;
}

// Leer baseline de archivo
int leer_baseline(const char *ruta_archivo, ListaArchivos *lista)
{
    FILE *f = fopen(ruta_archivo, "r");
    if (!f)
    {
        return -1; // No perror aquí para evitar ruido si no existe baseline aún
    }
    char linea[1024];
    while (fgets(linea, sizeof(linea), f))
    {
        char ruta[MAX_PATH];
        char hash_hex[65];
        if (sscanf(linea, "%s %64s", ruta, hash_hex) == 2)
        {
            unsigned char hash_bin[32];
            for (int i = 0; i < 32; i++)
            {
                sscanf(hash_hex + 2 * i, "%2hhx", &hash_bin[i]);
            }
            lista_agregar(lista, ruta, hash_bin);
        }
    }
    fclose(f);
    return 0;
}

// Comparar listas e imprimir diferencias. Indica si hay o no cambios.
int comparar_listas(ListaArchivos *baseline, ListaArchivos *actual)
{
    int cambios = 0;

    // Archivos eliminados o modificados
    for (int i = 0; i < baseline->cantidad; i++)
    {
        ArchivoHash *base = &baseline->datos[i];
        bool encontrado = false;
        for (int j = 0; j < actual->cantidad; j++)
        {
            if (strcmp(base->ruta, actual->datos[j].ruta) == 0)
            {
                encontrado = true;
                // Comparar hash
                if (memcmp(base->hash, actual->datos[j].hash, 32) != 0)
                {
                    printf("Archivo MODIFICADO: %s\n", base->ruta);
                    cambios++;
                }
                break;
            }
        }
        if (!encontrado)
        {
            printf("Archivo ELIMINADO: %s\n", base->ruta);
            cambios++;
        }
    }

    // Archivos nuevos
    for (int i = 0; i < actual->cantidad; i++)
    {
        ArchivoHash *act = &actual->datos[i];
        bool encontrado = false;
        for (int j = 0; j < baseline->cantidad; j++)
        {
            if (strcmp(act->ruta, baseline->datos[j].ruta) == 0)
            {
                encontrado = true;
                break;
            }
        }
        if (!encontrado)
        {
            printf("Archivo NUEVO: %s\n", act->ruta);
            cambios++;
        }
    }

    return cambios;
}

void registrar_evento(const char *mensaje)
{
    FILE *f = fopen("log.txt", "a");
    if (!f)
    {
        perror("Error abriendo log.txt");
        return;
    }

    time_t ahora = time(NULL);
    struct tm *tm_info = localtime(&ahora);
    char tiempo_str[26];
    strftime(tiempo_str, sizeof(tiempo_str), "%Y-%m-%d %H:%M:%S", tm_info);

    fprintf(f, "[%s] %s\n", tiempo_str, mensaje);
    fclose(f);
}

char *comparar_listas_y_reportar(ListaArchivos *baseline, ListaArchivos *actual, int *hubo_cambios)
{
    size_t buffer_size = 8192;
    char *buffer = malloc(buffer_size);
    if (!buffer)
    {
        perror("malloc");
        exit(1);
    }
    buffer[0] = '\0';
    *hubo_cambios = 0;

    // Archivos eliminados o modificados
    for (int i = 0; i < baseline->cantidad; i++)
    {
        ArchivoHash *base = &baseline->datos[i];
        bool encontrado = false;
        for (int j = 0; j < actual->cantidad; j++)
        {
            if (strcmp(base->ruta, actual->datos[j].ruta) == 0)
            {
                encontrado = true;
                if (memcmp(base->hash, actual->datos[j].hash, 32) != 0)
                {
                    snprintf(buffer + strlen(buffer), buffer_size - strlen(buffer),
                             "Archivo MODIFICADO: %s\n", base->ruta);
                    *hubo_cambios = 1;
                }
                break;
            }
        }
        if (!encontrado)
        {
            snprintf(buffer + strlen(buffer), buffer_size - strlen(buffer),
                     "Archivo ELIMINADO: %s\n", base->ruta);
            *hubo_cambios = 1;
        }
    }
    // Archivos nuevos
    for (int i = 0; i < actual->cantidad; i++)
    {
        ArchivoHash *act = &actual->datos[i];
        bool encontrado = false;
        for (int j = 0; j < baseline->cantidad; j++)
        {
            if (strcmp(act->ruta, baseline->datos[j].ruta) == 0)
            {
                encontrado = true;
                break;
            }
        }
        if (!encontrado)
        {
            snprintf(buffer + strlen(buffer), buffer_size - strlen(buffer),
                     "[ALERTA] Se detectó un archivo NUEVO: %s\n", act->ruta);
            *hubo_cambios = 1;
        }
    }
    return buffer;
}
