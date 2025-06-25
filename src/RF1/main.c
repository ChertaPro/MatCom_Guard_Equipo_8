#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include "detector.h"
#include "archivos.h"

#define PATH_BASELINE "baseline.txt"

int main() {
    Montaje montajes_ant[MAX_MOUNTS];
    Montaje montajes_act[MAX_MOUNTS];

    int cant_ant = obtener_montajes(montajes_ant, MAX_MOUNTS);
    if (cant_ant < 0) {
        fprintf(stderr, "Error al leer montajes.\n");
        return 1;
    }

    printf("Monitoreando nuevos dispositivos USB, loop y carpetas compartidas...\n");

    ListaArchivos baseline;
    lista_inicializar(&baseline);

    ListaArchivos actual;
    lista_inicializar(&actual);

    Montaje *montado = NULL;

    while (1) {
        sleep(5);
        int cant_act = obtener_montajes(montajes_act, MAX_MOUNTS);

        if (!montado) {
            montado = hay_nuevo_montaje(montajes_ant, cant_ant, montajes_act, cant_act);
            if (montado != NULL) {
                printf("\n>> Nuevo dispositivo detectado:\n");
                printf("   Dispositivo: %s\n", montado->dispositivo);
                printf("   Punto de montaje: %s\n", montado->punto_montaje);
                printf("   Tipo de FS: %s\n", montado->tipo_fs);

                printf(">> Escaneando y calculando hashes...\n");
                recorrer_directorio_con_hash(montado->punto_montaje, &baseline);
            }
        } else {
            // Verificar si el dispositivo sigue montado
            bool sigue_montado = false;
            for (int i = 0; i < cant_act; i++) {
                if (strcmp(montado->punto_montaje, montajes_act[i].punto_montaje) == 0) {
                    sigue_montado = true;
                    break;
                }
            }

            if (!sigue_montado) {
                printf("\n>> Dispositivo desmontado: %s\n", montado->punto_montaje);
                lista_liberar(&baseline);
                lista_inicializar(&baseline);
                montado = NULL;
            } else {
                printf("\n>> Escaneo periódico en %s...\n", montado->punto_montaje);

                lista_liberar(&actual);
                lista_inicializar(&actual);
                recorrer_directorio_con_hash(montado->punto_montaje, &actual);

                printf(">> Comparando con baseline...\n");
                int cambios = 0;
                char *reporte = comparar_listas_y_reportar(&baseline, &actual, &cambios);
                printf("%s", reporte);
                free(reporte);

                // No actualizamos baseline para conservar cambios
                lista_liberar(&actual);
                lista_inicializar(&actual);
            }
        }

        // Actualizar montajes previos
        cant_ant = cant_act;
        for (int i = 0; i < cant_act; i++) {
            montajes_ant[i] = montajes_act[i];
        }
    }

    lista_liberar(&baseline);
    lista_liberar(&actual);

    return 0;
}
