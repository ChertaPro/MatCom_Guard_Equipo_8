#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

double UMBRAL_CPU = 0.0;
double UMBRAL_RAM = 0.0;
int TIEMPO_UMBRAL = 0;
long TOTAL_RAM_KB = 0;
int MODO_SERVICIO = 0;

char **whitelist = NULL;
int num_whitelist = 0;

void leerConfiguracion()
{
    FILE *archive = fopen("/etc/matcomguard.conf", "r");
    if (archive == NULL)
    {
        printf("No se pudo abrir /etc/matcomguard.conf, usando valores por defecto. \n");
    }

    char line[256];
    while (fgets(line, sizeof(line),archive))
    {
        char key[64];
        char value[64];

        if (line[0] == '#' || line[0] == '\n')
        {
            continue;
        }

        if (sscanf(line, "%[^=]=%s", key, value) == 2) 
        {
            if (strcmp(key, "UMBRAL_CPU") == 0) 
            {
                UMBRAL_CPU = atof(value);
            } 
            else if (strcmp(key, "UMBRAL_RAM") == 0) 
            {
                UMBRAL_RAM = atof(value);
            } else if (strcmp(key, "TIEMPO_UMBRAL") == 0) {
                TIEMPO_UMBRAL = atoi(value);
            }
            else if (strcmp(key, "MODO_SERVICIO") == 0)
                MODO_SERVICIO = atoi(value);

        }
    }

    fclose(archive);
    printf("Configuración cargada:\n");
    printf("  UMBRAL_CPU: %.2f\n", UMBRAL_CPU);
    printf("  UMBRAL_RAM: %.2f\n", UMBRAL_RAM);
    printf("  TIEMPO_UMBRAL: %d\n", TIEMPO_UMBRAL);
}

void leerMemTotal()
{
    FILE *archivo = fopen("/proc/meminfo", "r");
    if (archivo == NULL) 
    {
        perror("No se pudo abrir /proc/meminfo");
        exit(1);
    }

    char linea[256];
    while (fgets(linea, sizeof(linea), archivo)) 
    {
        if (strncmp(linea, "MemTotal:", 9) == 0) 
        {
            sscanf(linea, "MemTotal: %ld kB", &TOTAL_RAM_KB);
            break;
        }
    }
    if (TOTAL_RAM_KB == 0) 
    {
        fprintf(stderr, "No se pudo obtener MemTotal. Abortando.\n");
        exit(1);
    }

    fclose(archivo);
    printf("Memoria total del sistema: %ld KB\n", TOTAL_RAM_KB);
}

void leerWhitelist()
{
    FILE *archive = fopen("/etc/matcomguard_whitelist.conf","r");
    if (archive == NULL)
    {
        printf("No se pudo abrir /etc/matcomguard_whitelist.conf. Sin whitelist.\n");
        return;
    }

    char line [256];
    while (fgets(line,sizeof(line),archive))
    {
        if(line[0] == '#' || line[0] == '\n')
        {
            continue;
        }

        line[strcspn(line,"\n")] = '\0';

        whitelist = (char **)realloc(whitelist, (num_whitelist + 1)*sizeof(char*));
        if (whitelist == NULL) {
            perror("Fallo reservando memoria para whitelist");
            exit(1);
        }

        whitelist[num_whitelist] = strdup(line);
        num_whitelist++;
    }
    fclose(archive);

    // Mostrar whitelist cargada
    printf("Whitelist cargada (%d procesos):\n", num_whitelist);
    for (int i = 0; i < num_whitelist; i++) {
        printf("  - %s\n", whitelist[i]);
    }
    
}

int estaEnWhitelist(const char *nombre) {
    for (int i = 0; i < num_whitelist; i++)
        if (strcmp(nombre, whitelist[i]) == 0) return 1;
    return 0;
}