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
#include <pthread.h>

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
    int sockfd = socket(AF_INET, SOCK_STREAM, 0); // Create socket TCP
    if (sockfd < 0)
    {
        perror("socket");
        return -1;
    }
    struct sockaddr_in target_addr;
    memset(&target_addr, 0, sizeof(target_addr));
    target_addr.sin_family = AF_INET;
    target_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &target_addr.sin_addr) <= 0) // IP form char to binary
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
    if (select(sockfd + 1, NULL, &fdset, NULL, &tv) > 0)
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

typedef struct
{
    const char *ip;
    unsigned int timeout_sec;
    pthread_mutex_t mutex;
    unsigned int next_port;
    unsigned int end_port;
    int open_ports;
} ThreadControl;

void *scan_worker(void *arg)
{
    ThreadControl *control = (ThreadControl *)arg;
    while (1)
    {
        unsigned int port_to_scan;
        pthread_mutex_lock(&control->mutex);
        if (control->next_port > control->end_port)
        {
            pthread_mutex_unlock(&control->mutex);
            break;
        }
        port_to_scan = control->next_port++;
        pthread_mutex_unlock(&control->mutex);
        int status = test_port(control->ip, port_to_scan, control->timeout_sec);
        if (status == 1)
        {
            pthread_mutex_lock(&control->mutex);
            printf("\033[32m[+] Puerto %d abierto\033[0m\n", port_to_scan);
            control->open_ports++;
            pthread_mutex_unlock(&control->mutex);
        }
        else if (status == -1)
            printf("\033[31m  [!] Error al escanear puerto %d\033[0m\n", port_to_scan);
    }
}

void threaded_port_scanner(PortScanConfig config)
{
    printf("\nEscaneando puertos TCP en %s (puertos %d-%d)...\n", config.target_ip, config.start_port, config.end_port);
    ThreadControl control = {
        .ip = config.target_ip,
        .timeout_sec = config.timeout_sec,
        .next_port = config.start_port,
        .end_port = config.end_port,
        .open_ports = 0};
    pthread_mutex_init(&control.mutex, NULL);
    pthread_t threads[32];
    for (int i = 0; i < 32; i++)
        pthread_create(&threads[i], NULL, scan_worker, &control);
    for (int i = 0; i < 32; i++)
        pthread_join(threads[i], NULL);
    printf("\nEscaneo completado. Puertos abiertos: %d/%d\n", control.open_ports, config.end_port - config.start_port + 1);
    pthread_mutex_destroy(&control.mutex);
}

void main_scanner()
{
    PortScanConfig config = {
        .target_ip = "127.0.0.1",
        .start_port = 1,
        .end_port = 1024,
        .timeout_sec = 1};
    while (1)
    {
        system("clear");
        threaded_port_scanner(config);
        sleep(3);
    }
}

int main()
{
    main_scanner();

    return 0;
}