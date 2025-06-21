#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

typedef struct
{
    const char *target_ip;    // IP a escanear
    unsigned int start_port;  // Puerto inicial
    unsigned int end_port;    // Puerto final
    unsigned int timeout_sec; // Tiempo de espera
} PortScanConfig;

// Para que el socket no se bloquee en operaciones E/S
void set_nonblocking(int sockfd)
{
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags == -1)
    {
        perror("fcntl F_GETFL");
        return;
    }
    if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1)
        perror("fcntl F_SETFL");
}

int test_port(const char *ip, unsigned int port, unsigned int timeout_sec)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0); // Crear socket TCP
    if (sockfd < 0)
    {
        perror("socket");
        return -1;
    }
    struct sockaddr_in target_addr;
    memset(&target_addr, 0, sizeof(target_addr));
    target_addr.sin_family = AF_INET;
    target_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &target_addr.sin_addr) <= 0) // Convierte IP (char to binary)
    {
        perror("inet_pton");
        close(sockfd);
        return -1;
    }
    set_nonblocking(sockfd);
    int connect_result = connect(sockfd, (struct sockaddr *)&target_addr, sizeof(target_addr));
    if (!connect_result)
    {
        close(sockfd);
        return 1;
    }
    if (errno != EINPROGRESS)
    {
        close(sockfd);
        return 0;
    }
    fd_set fdset;
    FD_ZERO(&fdset);
    FD_SET(sockfd, &fdset);
    struct timeval tv;
    tv.tv_sec = timeout_sec;
    tv.tv_usec = 0;
    int result = 0;
    if (select(sockfd + 1, NULL, &fdset, NULL, &tv) > 0) // Evita tiempo de espera infinito intentando conectarse
    {
        int so_error;
        socklen_t len = sizeof(so_error);
        getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &so_error, &len);
        if (so_error == 0)
            result = 1;
    }
    close(sockfd);
    return result;
}

// Función principal de escaneo
void port_scanner(PortScanConfig config)
{
    printf("\nEscaneando puertos TCP en %s (puertos %d-%d)...\n", config.target_ip, config.start_port, config.end_port);
    unsigned int open_ports = 0;
    for (unsigned int port = config.start_port; port <= config.end_port; port++)
    {
        int status = test_port(config.target_ip, port, config.timeout_sec);
        if (status == 1)
            printf("  [+] Puerto %d abierto\n", port), open_ports++;
        else if (status == -1)
            printf("  [!] Error al escanear puerto %d\n", port);
    }
    printf("\nEscaneo completado. Puertos abiertos: %d/%d\n", open_ports, config.end_port - config.start_port + 1);
}

void main_scanner()
{
    PortScanConfig config = {
        .target_ip = "127.0.0.1",
        .start_port = 1,
        .end_port = 1024,
        .timeout_sec = 1};
    port_scanner(config);
}

int main()
{
    main_scanner();

    return 0;
}