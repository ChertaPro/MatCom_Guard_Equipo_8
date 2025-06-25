#include "gui.h"
#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include "list.h"
#include "rf2_procesos.h"
#include "rf2_config.h"
#include <stdlib.h>
#include <stdbool.h>
#include "rf1_detector.h"
#include "rf1_archivos.h"
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include "rf3_port_db.h"
#include <netdb.h>
#include "rf3_port_scanner.h"

// sed -i 's/\r$//' fakeusb.sh
// gcc main.c list.c gui.c pdf.c rf2_config.c rf2_procesos.c rf1_detector.c rf1_archivos.c rf3_port_db.c rf3_port_scanner.c -o code `pkg-config --cflags --libs gtk+-3.0 cairo` -lcrypto -lcjson

#define PATH_BASELINE "baseline.txt"

int CONT = 0;
MsgList Alert_Devices, Alert_Process, Alert_Ports;

void do_append_info(char *message)
{
    append_info(message);
    // while (gtk_events_pending())
    //     gtk_main_iteration();
}

void do_append_alert(char *message, int id) // id 0:devices 1:process 2:ports
{
    CONT = (CONT + 1) % 5;
    if (CONT == 4)
    {
        if (id == 0)
            msglist_add(&Alert_Devices, message);
        else if (id == 1)
            msglist_add(&Alert_Process, message);
        else
            msglist_add(&Alert_Ports, message);
    }
    append_alert(message);
    // while (gtk_events_pending())
    //     gtk_main_iteration();
}

void general_append_info(char *dev, char *proc, char *port)
{
    char *buffer = malloc(strlen(dev) + strlen(proc) + strlen(port));
    strcpy(buffer, dev);
    strcat(buffer, proc);
    strcat(buffer, port);
    clear_info();
    do_append_info(buffer);
}

void init_list()
{
    msglist_init(&Alert_Devices);
    msglist_init(&Alert_Process);
    msglist_init(&Alert_Ports);
}

void free_list()
{
    msglist_free(&Alert_Devices);
    msglist_free(&Alert_Process);
    msglist_free(&Alert_Ports);
}

long ticks;
Proceso *procesos_anteriores = NULL;
int num_anteriores = 0;
long num_cpus;
int counter = 1;
Montaje montajes_ant[MAX_MOUNTS];
Montaje montajes_act[MAX_MOUNTS];
ListaArchivos baseline;
ListaArchivos actual;
Montaje *montado = NULL;
int cant_ant;
PortScanConfig config = {
    .target_ip = "127.0.0.1",
    .start_port = 1,
    .end_port = 65535,
    .timeout_sec = 1};

gboolean update_ui(gpointer user_data)
{
    clear_info();

    int num_actuales = 0;
    Proceso *procesos_actuales = leerProcesos(&num_actuales, ticks);
    inicializarBuffers();

    if (counter > 1)
        compararProcesos(procesos_anteriores, num_anteriores, procesos_actuales, num_actuales, num_cpus);
    counter++;

    char *procesos_str = obtenerProcesosFormateados();
    do_append_info(procesos_str);

    free(procesos_str);

    char *alertas_str = obtenerAlertasFormateadas();
    do_append_alert(alertas_str, 1);
    free(alertas_str);

    free(procesos_anteriores);
    procesos_anteriores = procesos_actuales;
    num_anteriores = num_actuales;

    // rf111111111111111111111111111111111111

    int cant_act = obtener_montajes(montajes_act, MAX_MOUNTS);

    if (!montado)
    {
        montado = hay_nuevo_montaje(montajes_ant, cant_ant, montajes_act, cant_act);
        if (montado != NULL)
        {

            char *mensaje = NULL;

            // Usamos asprintf para concatenar todo en un solo paso (evitando problemas de tamaño)
            asprintf(&mensaje,
                     "\n>> Nuevo dispositivo detectado:\n"
                     "   Dispositivo: %s\n"
                     "   Punto de montaje: %s\n"
                     "   Tipo de FS: %s\n",
                     montado->dispositivo, montado->punto_montaje, montado->tipo_fs);
            do_append_alert(mensaje, 0);
            recorrer_directorio_con_hash(montado->punto_montaje, &baseline);
        }
    }
    else
    {
        // Verificar si el dispositivo sigue montado
        bool sigue_montado = false;
        for (int i = 0; i < cant_act; i++)
        {
            if (strcmp(montado->punto_montaje, montajes_act[i].punto_montaje) == 0)
            {
                sigue_montado = true;
                break;
            }
        }

        if (!sigue_montado)
        {
            char *mensaje = NULL;
            asprintf(&mensaje, "\n>> Dispositivo desmontado: %s\n", montado->punto_montaje);
            do_append_alert(mensaje, 0);
            lista_liberar(&baseline);
            lista_inicializar(&baseline);
            montado = NULL;
        }
        else
        {
            char *mensaje = NULL;

            lista_liberar(&actual);
            lista_inicializar(&actual);
            recorrer_directorio_con_hash(montado->punto_montaje, &actual);

            int cambios = 0;
            char *reporte = comparar_listas_y_reportar(&baseline, &actual, &cambios);
            baseline = actual;
            do_append_alert(reporte, 0);

            free(reporte);

            // No actualizamos baseline para conservar cambios
            lista_liberar(&actual);
            lista_inicializar(&actual);
        }
    }

    // Actualizar montajes previos
    cant_ant = cant_act;
    for (int i = 0; i < cant_act; i++)
    {
        montajes_ant[i] = montajes_act[i];
    }

    // rf33333333333333333

    threaded_port_scanner(config);

    do_append_alert("\n", 2);

    return TRUE;
}

int main(int argc, char *argv[])
{
    init_list();
    gui_init(&argc, &argv);

    // code
    load_json("config.json");

    ticks = sysconf(_SC_CLK_TCK);
    num_cpus = sysconf(_SC_NPROCESSORS_ONLN);

    cant_ant = obtener_montajes(montajes_ant, MAX_MOUNTS);
    if (cant_ant < 0)
    {
        fprintf(stderr, "Error al leer montajes.\n");
        return 1;
    }

    lista_inicializar(&baseline);

    lista_inicializar(&actual);

    leerConfiguracion();
    leerMemTotal();
    leerWhitelist();

    g_timeout_add(5000, update_ui, NULL);

    gui_run();
    liberar_memoria();

    free_list();

    return 0;
}