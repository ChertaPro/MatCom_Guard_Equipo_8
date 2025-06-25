#ifndef PROCESOS_H
#define PROCESOS_H

// Estructura de un proceso
typedef struct {
    int pid;
    char nombre[256];
    int ram_kb;
    double cpu_s;
    int tiempo_sobre_umbral;
} Proceso;

int Is_digit(int c);

Proceso* leerProcesos(int *num_procesos, long ticks);

void compararProcesos(Proceso *anteriores, int num_anteriores, Proceso *actuales, int num_actuales, long num_cpus);

void inicializarBuffers();

char* obtenerProcesosFormateados();

char* obtenerAlertasFormateadas();

#endif
