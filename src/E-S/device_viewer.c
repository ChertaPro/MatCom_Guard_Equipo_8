#include <gtk/gtk.h>
#include <string.h>
#include <pthread.h>
#include <libudev.h>
#include <libnotify/notify.h>
#include <unistd.h>
#include <stdlib.h>
#include "inotify_monitor.h"
#include "log_viewer.h"

// Referencia a la ventana principal
GtkWidget *device_window = NULL;
GtkListBox *device_list = NULL;
static pthread_t udev_thread;

// Determina ícono por tipo de nombre
const char *get_device_icon(const char *name)
{
    if (strstr(name, "usb") || strstr(name, "USB"))
        return "📀";
    if (strstr(name, "disk") || strstr(name, "HDD"))
        return "🖴";
    if (strstr(name, "printer"))
        return "🖨️";
    if (strstr(name, "keyboard"))
        return "⌨️";
    if (strstr(name, "mouse"))
        return "🖱️";
    return "❓";
}

// Muestra lista actual de dispositivos
void refresh_device_list()
{
    if (!device_list)
        return;

    gtk_list_box_invalidate_filter(device_list);

    // Limpiar
    GList *children = gtk_container_get_children(GTK_CONTAINER(device_list));
    for (GList *iter = children; iter != NULL; iter = iter->next)
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    g_list_free(children);

    // Añadir elementos
    pthread_mutex_lock(&device_mutex);
    for (int i = 0; i < device_count; ++i)
    {
        char entry[512];
        const char *icon = get_device_icon(mounted_devices[i]);
        snprintf(entry, sizeof(entry), "%s  %s", icon, mounted_devices[i]);
        GtkWidget *label = gtk_label_new(entry);
        gtk_list_box_insert(device_list, label, -1);
    }
    pthread_mutex_unlock(&device_mutex);

    gtk_widget_show_all(GTK_WIDGET(device_list));
    // ad_join(udev_thread, NULL);
    device_window = NULL;
}

// Hilo que escucha eventos de udev
void *udev_monitor_thread(void *data)
{
    struct udev *udev = udev_new();
    struct udev_monitor *mon = udev_monitor_new_from_netlink(udev, "udev");
    udev_monitor_filter_add_match_subsystem_devtype(mon, "block", NULL);
    udev_monitor_enable_receiving(mon);
    int fd = udev_monitor_get_fd(mon);

    while (1)
    {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        select(fd + 1, &fds, NULL, NULL, NULL);
        if (FD_ISSET(fd, &fds))
        {
            struct udev_device *dev = udev_monitor_receive_device(mon);
            if (dev)
            {
                const char *action = udev_device_get_action(dev);
                const char *devnode = udev_device_get_devnode(dev);
                if (action && devnode)
                    g_idle_add((GSourceFunc)refresh_device_list, NULL); // Llama en hilo GTK
                udev_device_unref(dev);
            }
        }
    }

    udev_unref(udev);
    return NULL;
}

// Función para cuando la ventana se cierra
static void on_device_window_destroy(GtkWidget *widget, gpointer data)
{
    pthread_join(udev_thread, NULL);
    device_window = NULL;
}

// Función que crea o muestra la ventana
void show_device_window(GtkWidget *widget, gpointer data)
{
    if (device_window)
    {
        gtk_window_present(GTK_WINDOW(device_window));
        return;
    }

    device_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(device_window), "Dispositivos Conectados");
    gtk_window_set_default_size(GTK_WINDOW(device_window), 400, 300);
    g_signal_connect(device_window, "destroy", G_CALLBACK(on_device_window_destroy), NULL);

    GtkWidget *btn = gtk_button_new_with_label("Ver Log de Eventos");
    g_signal_connect(btn, "clicked", G_CALLBACK(show_log_window), NULL);
    gtk_container_add(GTK_CONTAINER(device_window), btn);

    // start_udev_if_needed(); // Activar udev cuando se abre

    device_list = GTK_LIST_BOX(gtk_list_box_new());
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroll), GTK_WIDGET(device_list));
    gtk_container_add(GTK_CONTAINER(device_window), scroll);

    pthread_create(&udev_thread, NULL, udev_monitor_thread, NULL);

    refresh_device_list();
    gtk_widget_show_all(device_window);
}
