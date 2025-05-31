#include <stdio.h>
#include <stdlib.h>
#include <dirent.h> //para recorrer el directorio /proc
#include <string.h> // para trabajar con strings 
#include <unistd.h> //para obtener sysconf(_SC_CLK_TCK), los ticks de reloj del sistema.


int Is_digit(int c) {
    return (c >= '0' && c <= '9');
}

int main()
{
    DIR *proc_dir;
    struct dirent *entry;
    long ticks = sysconf(_SC_CLK_TCK);

    proc_dir = opendir("/proc");
    if (proc_dir == NULL) {
        perror("Error al abrir el directorio /proc");
        exit(1);
    }

    while (entry = readdir(proc_dir)) {
        if (Is_digit(entry->d_name[0])) { 
            char path[256];
            char proces_name[256];
            FILE *archive;
            printf("---------------------------------------------------\n");
            snprintf(path, sizeof(path), "/proc/%s/comm", entry->d_name);
            archive = fopen(path, "r"); 
        
            if (archive != NULL) {
                if(fgets(proces_name, sizeof(proces_name),archive) != NULL) {
                    proces_name[strcspn(proces_name, "\n")] = '\0';
                    printf("PID: %s | Nombre : %s",entry->d_name,proces_name);
                }
                fclose(archive);
            }

            snprintf(path, sizeof(path),"/proc/%s/status",entry->d_name);
            archive = fopen(path, "r");

            if(archive != NULL){
                char line[512];
                while (fgets(line, sizeof(line), archive))
                {
                    if(strncmp(line,"VmRRS",6) == 0){
                        int ram_kb;
                        
                        sscanf(line, "VmRSS: %d kB", &ram_kb);

                        printf("  RAM: %d KB\n", ram_kb);
                        break;
                    }
                }
                fclose(archive);
            }

            snprintf(path,sizeof(path),"/proc/%s/stat",entry->d_name);
            archive = fopen(path,"r");

            if(archive != NULL) {
                long usertime, systime;
                char buffer[4096];
                
                if(fgets(buffer,sizeof(buffer),archive)){
                    char *token;
                    int i = 1;

                    token = strtok(buffer, " ");
                    while (token != NULL) {
                        if (i == 14) usertime = atol(token);
                        if (i == 15){
                            systime = atol(token);
                            break;
                        } 
                        i++;
                    }

                    double cpu_use = (usertime + systime)/(double)ticks;
                    printf("  CPU (segundos de CPU totales): %.2f s\n", cpu_use);
                }
                fclose(archive);      
            }
            else{
                fclose(archive);
            }
        }
    }

    closedir(proc_dir);
    return 0;
}