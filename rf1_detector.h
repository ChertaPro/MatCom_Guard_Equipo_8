#ifndef DETECTOR_H
#define DETECTOR_H

#define MAX_MOUNTS 100
#define MAX_PATH 512

typedef struct
{
    char dispositivo[MAX_PATH];
    char punto_montaje[MAX_PATH];
    char tipo_fs[64];
} Montaje;

int obtener_montajes(Montaje montajes[], int max_montajes);
Montaje *hay_nuevo_montaje(Montaje anteriores[], int cant_ant, Montaje actuales[], int cant_act);

#endif
