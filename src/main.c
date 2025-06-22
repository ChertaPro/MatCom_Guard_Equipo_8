#include "procesos.h"
#include "config.h"
#include "log.h"
#include <unistd.h>

//Compilar make
//Limpiar make clean
//Ejecutar ./MatComGuard


int main()
{
    long ticks = sysconf(_SC_CLK_TCK);
    Proceso *procesos_anteriores = NULL;
    int num_anteriores = 0;
    long num_cpus = sysconf(_SC_NPROCESSORS_ONLN);

    leerConfiguracion();
    leerMemTotal();
    leerWhitelist();

    int iteracion = 1;
    
    do {
        system("clear");
        printf("\n===== Iteración %d =====\n", iteracion++);
        int num_actuales = 0;
        Proceso *procesos_actuales = leerProcesos(&num_actuales, ticks);

        if (iteracion > 1)
            compararProcesos(procesos_anteriores, num_anteriores, procesos_actuales, num_actuales,num_cpus);

        free(procesos_anteriores);
        procesos_anteriores = procesos_actuales;
        num_anteriores = num_actuales;

        sleep(5);

    } while (MODO_SERVICIO || iteracion <= 5);

    free(procesos_anteriores);
    return 0;
}