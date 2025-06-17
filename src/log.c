#include "log.h"
#include <stdio.h>
#include <time.h>

void escribirLog(const char *nombre, int pid, double cpu, double ram)
{
    FILE *log = fopen(RUTA_LOG, "a");
    if (!log) 
    {
        perror("No se pudo abrir el archivo de log");
        return;
    }

    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    char fecha[64];
    strftime(fecha, sizeof(fecha), "%Y-%m-%d %H:%M:%S", tm_info);

    fprintf(log,
        "{ \"timestamp\": \"%s\", \"pid\": %d, \"proceso\": \"%s\", \"cpu\": %.2f, \"ram\": %.2f }\n",
        fecha, pid, nombre, cpu, ram);

    fclose(log);
}
