#ifndef LOG_H
#define LOG_H

#define RUTA_LOG "/var/log/matcomguard.log"

void escribirLog(const char *nombre, int pid, double cpu, double ram);

#endif
