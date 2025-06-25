#include <stdio.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include "message.h"

const char *get_service_name(int port)
{
    struct servent *service;
    service = getservbyport(htons(port), "tcp");
    if (service != NULL)
        return service->s_name;
    else
        return "Desconocido";
}

void check_port(int port)
{
    const char *service = get_service_name(port);
    char msg[256];
    snprintf(msg, sizeof(msg), "Puerto %d (%s): ABIERTO", port, service);
    add_message(msg, 0);
    // printf("Puerto %d (%s): \033[36mABIERTO\033[0m\n", port, service);
    if (port > 1024 && strcmp(service, "Desconocido") != 0)
    {
        // printf("\033[33m[!] Alerta: Servicio %s en puerto no estándar %d\033[0m\n", service, port);
        snprintf(msg, sizeof(msg),
                 "[!] Alerta: Servicio %s en puerto no estándar %d",
                 service, port);
        add_message(msg, 1);
    }
}