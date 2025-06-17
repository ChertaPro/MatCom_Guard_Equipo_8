#ifndef PROCESOS_H
#define PROCESOS_H

typedef struct {
    int pid;
    char nombre[256];
    int ram_kb;
    double cpu_s;
    int tiempo_sobre_umbral;
} Proceso;

Proceso* leerProcesos(int *num_procesos, long ticks);
void compararProcesos(Proceso *anteriores, int num_anteriores, Proceso *actuales, int num_actuales);
int Is_digit(int c);

#endif
