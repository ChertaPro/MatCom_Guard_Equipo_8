#ifndef PORT_SCANNER_H
#define PORT_SCANNER_H

typedef struct
{
    const char *target_ip;    // IP a escanear
    unsigned int start_port;  // Puerto inicial
    unsigned int end_port;    // Puerto final
    unsigned int timeout_sec; // Tiempo de espera
} PortScanConfig;

void main_scanner();

#endif