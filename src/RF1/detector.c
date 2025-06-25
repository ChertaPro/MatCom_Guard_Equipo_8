#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "detector.h"

// Lee /proc/mounts y guarda los puntos de montaje encontrados
int obtener_montajes(Montaje montajes[], int max_montajes) {
    FILE *archivo = fopen("/proc/mounts", "r");
    if (!archivo) {
        perror("Error al abrir /proc/mounts");
        return -1;
    }

    char linea[512];
    int contador = 0;

    while (fgets(linea, sizeof(linea), archivo) && contador < max_montajes) {
        char dispositivo[MAX_PATH], punto_montaje[MAX_PATH], tipo_fs[64];
        if (sscanf(linea, "%s %s %s", dispositivo, punto_montaje, tipo_fs) == 3) {
            strcpy(montajes[contador].dispositivo, dispositivo);
            strcpy(montajes[contador].punto_montaje, punto_montaje);
            strcpy(montajes[contador].tipo_fs, tipo_fs);
            contador++;
        }
    }

    fclose(archivo);
    return contador;
}

// Devuelve puntero al nuevo montaje si lo encuentra, NULL si no.
Montaje* hay_nuevo_montaje(Montaje anteriores[], int cant_ant, Montaje actuales[], int cant_act) {
    for (int i = 0; i < cant_act; i++) {
        bool encontrado = false;

        for (int j = 0; j < cant_ant; j++) {
            if (strcmp(actuales[i].punto_montaje, anteriores[j].punto_montaje) == 0) {
                encontrado = true;
                break;
            }
        }

        if (!encontrado) {
            if (strcmp(actuales[i].tipo_fs, "vfat") == 0 ||
                strcmp(actuales[i].tipo_fs, "exfat") == 0 ||
                strcmp(actuales[i].tipo_fs, "ntfs")  == 0 ||
                strcmp(actuales[i].tipo_fs, "vboxsf") == 0) {
                return &actuales[i];
            }
        }
    }
    return NULL;
}
