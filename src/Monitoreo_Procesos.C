#include <stdio.h>
#include <stdlib.h>
#include <dirent.h> //para recorrer el directorio /proc
#include <string.h> // para trabajar con strings 
#include <unistd.h> //para obtener sysconf(_SC_CLK_TCK), los ticks de reloj del sistema.

//matcomguard.conf
double UMBRAL_CPU = 0.0;      
double UMBRAL_RAM = 0.0;      
int TIEMPO_UMBRAL = 0;        
long TOTAL_RAM_KB = 0;  

//Whitelist
char **whitelist = NULL;
int num_whitelist = 0;

typedef struct {
    int pid;
    char nombre[256];
    int ram_kb;
    double cpu_s;
    int tiempo_sobre_umbral;
} Proceso;

// Prototipos de funciones
Proceso* leerProcesos(int *num_procesos, long ticks);
void compararProcesos(Proceso *anteriores, int num_anteriores, Proceso *actuales, int num_actuales);
void leerConfiguracion();
void leerMemTotal();
void leerWhitelist();
int estaEnWhitelist(const char *nombre);

int Is_digit(int c) {
    return (c >= '0' && c <= '9');
}

int main()
{
    long ticks = sysconf(_SC_CLK_TCK);
    Proceso *procesos_anteriores = NULL;
    int num_anteriores = 0;

    leerConfiguracion();
    leerMemTotal();
    leerWhitelist();


    for (int i = 0; i < 5; i++)
    {
        printf("\n===== Iteracion %d =====\n",i+1);
        int num_actuales = 0;
        Proceso *procesos_actuales = leerProcesos(&num_actuales,ticks);

        if ( i > 0)
        {
            compararProcesos(procesos_anteriores,num_anteriores,procesos_actuales,num_actuales);
        }
        free(procesos_anteriores);
        procesos_anteriores = procesos_actuales;
        num_anteriores = num_actuales;
        sleep(1);

    }
    free(procesos_anteriores);
    return 0;
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

            procesos = (Proceso *)realloc(procesos, (*num_procesos + 1)*sizeof(Proceso));
            if(procesos == NULL)
            {
                perror("Fallo reservando memoria para procesos");
                exit(1);
            }

            procesos [*num_procesos] = proc;
            (*num_procesos)++;
        }
    }

    closedir(proc_dir);
    return procesos;
}

void compararProcesos(Proceso *anteriores, int num_anteriores, Proceso *actuales, int num_actuales)
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
                double porcentaje_cpu = delta_cpu*100;

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
                    }
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
    if (archivo == NULL) {
        perror("No se pudo abrir /proc/meminfo");
        exit(1);
    }

    char linea[256];
    while (fgets(linea, sizeof(linea), archivo)) {
        if (strncmp(linea, "MemTotal:", 9) == 0) {
            sscanf(linea, "MemTotal: %ld kB", &TOTAL_RAM_KB);
            break;
        }
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