#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/inotify.h>
#include <libnotify/notify.h>
#include "recursive_watcher.h"

#define MONITOR_DIR "/media"
#define MAX_DEVICES 64
#define EVENT_SIZE (sizeof(struct inotify_event))
#define EVENT_BUF_LEN (1024 * (EVENT_SIZE + 16))

// Lista global de dispositivos montados
char mounted_devices[MAX_DEVICES][256];
int device_count = 0;
pthread_mutex_t device_mutex = PTHREAD_MUTEX_INITIALIZER;

// Función para enviar notificaciones
void send_notification(const char *title, const char *message)
{
    NotifyNotification *n = notify_notification_new(title, message, "dialog-information");
    notify_notification_show(n, NULL);
    g_object_unref(G_OBJECT(n));
}

// Función para actualizar lista interna
void scan_mounted_devices()
{
    pthread_mutex_lock(&device_mutex);
    device_count = 0;
    DIR *dir = opendir(MONITOR_DIR);
    if (!dir)
    {
        perror("No se pudo abrir /media");
        pthread_mutex_unlock(&device_mutex);
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL && device_count < MAX_DEVICES)
    {
        if (entry->d_name[0] == '.')
            continue; // ignorar "." y ".."
        snprintf(mounted_devices[device_count], 256, "%s", entry->d_name);
        device_count++;
    }

    closedir(dir);
    pthread_mutex_unlock(&device_mutex);
}

// Hilo de monitoreo con inotify
void *inotify_thread(void *arg)
{
    int fd = inotify_init();
    if (fd < 0)
    {
        perror("inotify_init");
        return NULL;
    }
    int wd = inotify_add_watch(fd, MONITOR_DIR, IN_CREATE | IN_DELETE);
    if (wd == -1)
    {
        perror("inotify_add_watch");
        close(fd);
        return NULL;
    }
    char buffer[EVENT_BUF_LEN];

    while (1)
    {
        int length = read(fd, buffer, EVENT_BUF_LEN); // Lee todos los eventos pendientes del descriptor de inotify
        if (length < 0)
        {
            perror("read");
            break;
        }
        int i = 0;
        // Recorremos el buffer que contiene la lista de eventos. Cada uno se interpreta como un inotify_event
        while (i < length)
        {
            struct inotify_event *event = (struct inotify_event *)&buffer[i];
            if (event->len)
            {
                // Se creó un nuevo archivo/directorio (un nuevo dispositivo insertado)
                if (event->mask & IN_CREATE)
                    send_notification("Dispositivo montado", event->name);
                // Se eliminó un archivo/directorio (dispositivo expulsado)
                else if (event->mask & IN_DELETE)
                    send_notification("Dispositivo desmontado", event->name);
                scan_mounted_devices(); // actualizar lista interna
            }
            i += EVENT_SIZE + event->len;
        }
    }

    inotify_rm_watch(fd, wd);
    close(fd);
    return NULL;
}

// Inicializa notificaciones y lanza el hilo
void start_inotify_monitor()
{
    notify_init("Monitor de Dispositivos");

    scan_mounted_devices(); // inicializa la lista al arrancar
    pthread_t tid;
    pthread_create(&tid, NULL, inotify_thread, NULL);
    pthread_detach(tid);

    start_recursive_monitor();
}
