#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <pthread.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <libnotify/notify.h>

#define EVENT_SIZE (sizeof(struct inotify_event))
#define EVENT_BUF_LEN (1024 * (EVENT_SIZE + 16))
#define MAX_WATCHES 4096

// Estructura global
static int inotify_fd;
static char log_path[512];
static pthread_t watcher_thread;

// Mapa entre watch descriptor y path
typedef struct
{
    int wd;
    char path[512];
} WatchPath;

static WatchPath watch_paths[MAX_WATCHES];
static int watch_count = 0;

// Registrar evento en el log
void write_to_log(const char *event, const char *filepath)
{
    FILE *f = fopen(log_path, "a");
    if (!f)
        return;
    time_t now = time(NULL);
    fprintf(f, "[%s] %s: %s\n", ctime(&now), event, filepath);
    fclose(f);
}

// Notificación visual
void show_alert(const char *event, const char *filepath)
{
    char msg[512];
    snprintf(msg, sizeof(msg), "%s: %s", event, filepath);
    NotifyNotification *n = notify_notification_new("⚠️ Alerta de Archivo", msg, NULL);
    notify_notification_show(n, NULL);
    g_object_unref(G_OBJECT(n));
}

// Asignar watch a un directorio
void add_watch_recursive(const char *path)
{
    struct stat st;
    if (stat(path, &st) < 0 || !S_ISDIR(st.st_mode))
        return;

    int wd = inotify_add_watch(inotify_fd, path, IN_CREATE | IN_DELETE | IN_MODIFY | IN_DELETE_SELF);
    if (wd < 0)
        return;

    if (watch_count < MAX_WATCHES)
    {
        watch_paths[watch_count].wd = wd;
        strncpy(watch_paths[watch_count].path, path, sizeof(watch_paths[watch_count].path));
        watch_count++;
    }

    DIR *dir = opendir(path);
    if (!dir)
        return;

    struct dirent *entry;
    while ((entry = readdir(dir)))
    {
        if (entry->d_name[0] == '.')
            continue;

        char subpath[512];
        snprintf(subpath, sizeof(subpath), "%s/%s", path, entry->d_name);

        if (stat(subpath, &st) == 0 && S_ISDIR(st.st_mode))
            add_watch_recursive(subpath); // Recursión
    }

    closedir(dir);
}

// Buscar path por descriptor
const char *get_path_by_wd(int wd)
{
    for (int i = 0; i < watch_count; ++i)
        if (watch_paths[i].wd == wd)
            return watch_paths[i].path;
    return "???";
}

// Hilo principal de monitoreo
void *watch_loop(void *arg)
{
    char buffer[EVENT_BUF_LEN];
    while (1)
    {
        int length = read(inotify_fd, buffer, EVENT_BUF_LEN);
        if (length < 0)
            continue;

        int i = 0;
        while (i < length)
        {
            struct inotify_event *event = (struct inotify_event *)&buffer[i];
            if (event->len)
            {
                const char *parent = get_path_by_wd(event->wd);
                char fullpath[1024];
                snprintf(fullpath, sizeof(fullpath), "%s/%s", parent, event->name);

                if (event->mask & IN_CREATE)
                {
                    show_alert("Archivo creado", fullpath);
                    write_to_log("CREADO", fullpath);

                    // Si es directorio, agregar watch recursivo
                    struct stat st;
                    if (stat(fullpath, &st) == 0 && S_ISDIR(st.st_mode))
                        add_watch_recursive(fullpath);
                }
                else if (event->mask & IN_DELETE)
                {
                    show_alert("Archivo eliminado", fullpath);
                    write_to_log("ELIMINADO", fullpath);
                }
                else if (event->mask & IN_MODIFY)
                {
                    show_alert("Archivo modificado", fullpath);
                    write_to_log("MODIFICADO", fullpath);
                }
            }

            i += EVENT_SIZE + event->len;
        }
    }

    return NULL;
}

// Iniciar monitoreo en una lista de rutas
void start_recursive_monitor()
{
    const char *home = getenv("HOME");
    if (!home)
    {
        fprintf(stderr, "No se pudo obtener $HOME\n");
        exit(EXIT_FAILURE);
    }
    snprintf(log_path, sizeof(log_path), "%s/.fileguardian.log", home);

    notify_init("FileGuardian");

    inotify_fd = inotify_init();
    if (inotify_fd < 0)
    {
        perror("inotify_init");
        return;
    }

    const char *paths[] = {
        "/media", // Dispositivos externos montados
        "/home",  // Ruta interna
    };
    const int count = 2;
    
    for (int i = 0; i < count; ++i)
        add_watch_recursive(paths[i]);

    pthread_create(&watcher_thread, NULL, watch_loop, NULL);
}
