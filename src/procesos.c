#include "procesos.h"
#include "config.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>

int Is_digit(int c) {
    return (c >= '0' && c <= '9');
}

Proceso *leerProcesos(int *num_procesos, long ticks)
{
    
    DIR *proc_dir;
    struct dirent *entry;
    Proceso *procesos = NULL;
    *num_procesos = 0;

    proc_dir = opendir("/proc");
    if (proc_dir == NULL) {
        perror("Error al abrir el directorio /proc");
        exit(1);
    }

    while (entry = readdir(proc_dir)) 
    {
        if (Is_digit(entry->d_name[0])) 
        { 
            char path[256];
            char proces_name[256];
            FILE *archive;
            Proceso proc;
            
            //Obteniendo nombre y PID
            proc.pid = atoi(entry->d_name);
            proc.tiempo_sobre_umbral = 0;

            snprintf(path, sizeof(path), "/proc/%s/comm", entry->d_name);
            archive = fopen(path, "r"); 
        
            if (archive != NULL) 
            {
                if(fgets(proces_name, sizeof(proces_name),archive) != NULL) 
                {
                    proces_name[strcspn(proces_name, "\n")] = '\0';
                    strncpy(proc.nombre,proces_name,sizeof(proc.nombre));
                }
                else
                {
                    strncpy(proc.nombre,"Desconocido",sizeof(proc.nombre));
                }
                fclose(archive);
            }
            else
            {
                strncpy(proc.nombre,"Desconocido",sizeof(proc.nombre));
            }

            //Leyendo RAM
            snprintf(path, sizeof(path),"/proc/%s/status",entry->d_name);
            if (access(path, R_OK) != 0) continue;
            archive = fopen(path, "r");
            proc.ram_kb = -1;

            if(archive != NULL)
            {
                char line[512];

                while (fgets(line, sizeof(line), archive))
                {

                    if(strncmp(line,"VmRSS",5) == 0)
                    {
                        
                        sscanf(line, "VmRSS: %d kB", &proc.ram_kb);

                        // printf("  RAM: %d KB\n", ram_kb);
                        break;
                    }
                }
                fclose(archive);
            }

            snprintf(path,sizeof(path),"/proc/%s/stat",entry->d_name);
            archive = fopen(path,"r");
            proc.cpu_s = 0.0;

            if(archive != NULL)
            {
                long usertime, systime;
                char buffer[4096];
                
                if(fgets(buffer,sizeof(buffer),archive))
                {
                    char *token;
                    int i = 1;

                    token = strtok(buffer, " ");
                    while (token != NULL) 
                    {
                        if (i == 14) usertime = atol(token);
                        if (i == 15)
                        {
                            systime = atol(token);
                            break;
                        } 
                        token = strtok(NULL," ");
                        i++;
                    }

                    proc.cpu_s = (usertime + systime)/(double)ticks;
                }
                fclose(archive);      
            }
            
            // printf("PID: %d | Nombre: %s | RAM: %d KB | CPU: %.2f s\n", 
            // proc.pid, proc.nombre, proc.ram_kb, proc.cpu_s);

            Proceso *tmp = (Proceso *)realloc(procesos, (*num_procesos + 1)*sizeof(Proceso));
            if(!tmp)
            {
                perror("Fallo reservando memoria para procesos");
                free(procesos);
                exit(1);
            }
            procesos = tmp;
            procesos [*num_procesos] = proc;
            (*num_procesos)++;
        }
    }

    closedir(proc_dir);
    return procesos;
}

void compararProcesos(Proceso *anteriores, int num_anteriores, Proceso *actuales, int num_actuales,long num_cpus)
{
    for (int i = 0; i < num_actuales; i++)
    {
        Proceso * actual = &actuales[i];

        int encontrado = 0;
        for (int j = 0; j < num_anteriores; j++)
        {
            if(anteriores[j].pid == actual->pid)
            {
                double delta_cpu = actual->cpu_s - anteriores[j].cpu_s;
                double porcentaje_cpu = (delta_cpu / 5.0) * 100.0 / num_cpus;

                double porcentaje_ram = (actual->ram_kb != -1 && TOTAL_RAM_KB != 0) ? 
                    ((double)actual->ram_kb / (double)TOTAL_RAM_KB) * 100.0 : 0.0;
                
                if ((porcentaje_cpu > UMBRAL_CPU) || (porcentaje_ram > UMBRAL_RAM))
                {
                    actuales[i].tiempo_sobre_umbral =  anteriores[j].tiempo_sobre_umbral + 1;
                }
                else
                {
                    actuales[i].tiempo_sobre_umbral = 0;
                }

                if (actuales[i].tiempo_sobre_umbral >= TIEMPO_UMBRAL) 
                {
                    if (!estaEnWhitelist(actual->nombre)) {
                        printf("⚠️  ALERTA: Proceso '%s' (PID %d) excedió umbral.\n", actual->nombre, actual->pid);
                        printf("  CPU delta: %.2f %% | RAM: %.2f %% (%d KB)\n", porcentaje_cpu, porcentaje_ram, actual->ram_kb);
                        
                        escribirLog(actual->nombre, actual->pid, porcentaje_cpu, porcentaje_ram);
                    }
                }
                else
                {
                    printf("Proceso '%s' (PID %d) CPU delta: %.2f %% | RAM: %.2f %% (%d KB)\n",
                                actual->nombre, actual->pid, porcentaje_cpu, porcentaje_ram, actual->ram_kb);
                }
                encontrado = 1;
                break;
            }
        }

        if(!encontrado)
        {
            actuales[i].tiempo_sobre_umbral = 0;
        }
    }
}