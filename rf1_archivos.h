#ifndef ARCHIVOS_H
#define ARCHIVOS_H

#include <stddef.h>

#define MAX_PATH 512

typedef struct
{
    char ruta[MAX_PATH];
    unsigned char hash[32]; // SHA256 de 32 bytes
} ArchivoHash;

typedef struct
{
    ArchivoHash *datos;
    int cantidad;
    int capacidad;
} ListaArchivos;

// Funciones principales
void recorrer_directorio_con_hash(const char *path, ListaArchivos *lista);

int calcular_sha256(const char *ruta, unsigned char salida_hash[32]);

void lista_inicializar(ListaArchivos *lista);

void lista_agregar(ListaArchivos *lista, const char *ruta, unsigned char hash[32]);

void lista_liberar(ListaArchivos *lista);

int guardar_baseline(const char *ruta_archivo, ListaArchivos *lista);

int leer_baseline(const char *ruta_archivo, ListaArchivos *lista);

int comparar_listas(ListaArchivos *baseline, ListaArchivos *actual);

void registrar_evento(const char *mensaje);

char *comparar_listas_y_reportar(ListaArchivos *baseline, ListaArchivos *actual, int *hubo_cambios);

#endif
