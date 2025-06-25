#include <gtk/gtk.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <stdio.h>

/*
 * GTK Scanner Interface
 * Cuatro botones: Escanear dispositivos, Escanear procesos, Escanear puertos, Escanear todo.
 * Medio: GtkTextView para info en tiempo real.
 * Derecha: GtkTextView para alertas.
 * Tú implementas la lógica interna y usas append_info()/append_alert().
 */

/* Prototipos de tus funciones */
void do_scan_devices(GtkTextBuffer *info_buffer, GtkTextBuffer *alert_buffer);
void do_scan_processes(GtkTextBuffer *info_buffer, GtkTextBuffer *alert_buffer);
void do_scan_ports(GtkTextBuffer *info_buffer, GtkTextBuffer *alert_buffer);
void do_scan_all(GtkTextBuffer *info_buffer, GtkTextBuffer *alert_buffer);

/* Manejadores de botones */
static void on_scan_devices_clicked(GtkButton *button, gpointer data) {
    GtkTextBuffer **b = (GtkTextBuffer **)data;
    do_scan_devices(b[0], b[1]);
}
static void on_scan_processes_clicked(GtkButton *button, gpointer data) {
    GtkTextBuffer **b = (GtkTextBuffer **)data;
    do_scan_processes(b[0], b[1]);
}
static void on_scan_ports_clicked(GtkButton *button, gpointer data) {
    GtkTextBuffer **b = (GtkTextBuffer **)data;
    do_scan_ports(b[0], b[1]);
}
static void on_scan_all_clicked(GtkButton *button, gpointer data) {
    GtkTextBuffer **b = (GtkTextBuffer **)data;
    do_scan_all(b[0], b[1]);
}

/* Global buffers */
GtkTextBuffer *info_buffer;
GtkTextBuffer *alert_buffer;

/* Implementación de ejemplo: Escaneo de puertos */
void do_scan_ports(GtkTextBuffer *info_buffer, GtkTextBuffer *alert_buffer) {
    struct sockaddr_in addr;
    char msg[256];

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    for (int port = 1; port <= 1024; ++port) {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) continue;
        addr.sin_port = htons(port);
        fcntl(sock, F_SETFL, O_NONBLOCK);
        connect(sock, (struct sockaddr*)&addr, sizeof(addr));
        struct timeval tv = {0, 100000};
        fd_set wfds;
        FD_ZERO(&wfds);
        FD_SET(sock, &wfds);
        if (select(sock+1, NULL, &wfds, NULL, &tv) > 0) {
            int err = 0; socklen_t len = sizeof(err);
            if (getsockopt(sock, SOL_SOCKET, SO_ERROR, &err, &len)==0 && err==0) {
                snprintf(msg, sizeof(msg), "Puerto %d abierto\n", port);
                GtkTextIter it;
                gtk_text_buffer_get_end_iter(info_buffer, &it);
                gtk_text_buffer_insert(info_buffer, &it, msg, -1);
                if (port==22||port==23||port==3389) {
                    snprintf(msg, sizeof(msg), "Alerta: puerto %d abierto (critico)\n", port);
                    gtk_text_buffer_get_end_iter(alert_buffer, &it);
                    gtk_text_buffer_insert(alert_buffer, &it, msg, -1);
                }
            }
        }
        close(sock);
    }
}

/* Implementación de ejemplo: Escaneo de dispositivos USB */
void do_scan_devices(GtkTextBuffer *info_buffer, GtkTextBuffer *alert_buffer) {
    DIR *d = opendir("/sys/bus/usb/devices");
    struct dirent *entry;
    char msg[256];
    if (!d) {
        snprintf(msg, sizeof(msg), "Error: no se pudo abrir /sys/bus/usb/devices\n");
        GtkTextIter it;
        gtk_text_buffer_get_end_iter(alert_buffer, &it);
        gtk_text_buffer_insert(alert_buffer, &it, msg, -1);
        return;
    }
    while ((entry = readdir(d)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        snprintf(msg, sizeof(msg), "USB: %s conectado\n", entry->d_name);
        GtkTextIter it;
        gtk_text_buffer_get_end_iter(info_buffer, &it);
        gtk_text_buffer_insert(info_buffer, &it, msg, -1);
        // Ejemplo de alerta si encuentro un dispositivo sospechoso
        if (strstr(entry->d_name, "1-1")) {
            snprintf(msg, sizeof(msg), "Alerta: dispositivo USB sospechoso %s\n", entry->d_name);
            gtk_text_buffer_get_end_iter(alert_buffer, &it);
            gtk_text_buffer_insert(alert_buffer, &it, msg, -1);
        }
    }
    closedir(d);
}

/* Implementación de ejemplo: Escaneo de procesos en /proc */
void do_scan_processes(GtkTextBuffer *info_buffer, GtkTextBuffer *alert_buffer) {
    DIR *d = opendir("/proc");
    struct dirent *entry;
    char path[256], name[128], msg[256];
    if (!d) {
        snprintf(msg, sizeof(msg), "Error: no se pudo abrir /proc\n");
        GtkTextIter it;
        gtk_text_buffer_get_end_iter(alert_buffer, &it);
        gtk_text_buffer_insert(alert_buffer, &it, msg, -1);
        return;
    }
    while ((entry = readdir(d)) != NULL) {
        if (!isdigit(entry->d_name[0])) continue;
        snprintf(path, sizeof(path), "/proc/%s/comm", entry->d_name);
        FILE *f = fopen(path, "r");
        if (!f) continue;
        if (fgets(name, sizeof(name), f)) {
            name[strcspn(name, "\n")] = '\0';
            GtkTextIter it;
            snprintf(msg, sizeof(msg), "PID %s: %s\n", entry->d_name, name);
            gtk_text_buffer_get_end_iter(info_buffer, &it);
            gtk_text_buffer_insert(info_buffer, &it, msg, -1);
            // Alerta para procesos comunes de red
            if (strcmp(name, "nc") == 0 || strcmp(name, "telnet") == 0) {
                snprintf(msg, sizeof(msg), "Alerta: proceso sospechoso %s (PID %s)\n", name, entry->d_name);
                gtk_text_buffer_get_end_iter(alert_buffer, &it);
                gtk_text_buffer_insert(alert_buffer, &it, msg, -1);
            }
        }
        fclose(f);
    }
    closedir(d);
}

/* Escaneo completo: llama a las tres */
void do_scan_all(GtkTextBuffer *info_buffer, GtkTextBuffer *alert_buffer) {
    do_scan_devices(info_buffer, alert_buffer);
    do_scan_processes(info_buffer, alert_buffer);
    do_scan_ports(info_buffer, alert_buffer);
}

/* Manejadores */
static void on_scan(GtkButton *btn, gpointer data) {
    GtkTextBuffer **b = (GtkTextBuffer **)data;
    const char *label = gtk_button_get_label(btn);
    if (g_strcmp0(label, "Escanear dispositivos") == 0)
        do_scan_devices(b[0], b[1]);
    else if (g_strcmp0(label, "Escanear procesos") == 0)
        do_scan_processes(b[0], b[1]);
    else if (g_strcmp0(label, "Escanear puertos") == 0)
        do_scan_ports(b[0], b[1]);
    else if (g_strcmp0(label, "Escanear todo") == 0)
        do_scan_all(b[0], b[1]);
}

GtkTextBuffer *info_buffer;
GtkTextBuffer *alert_buffer;

/* Ejemplos de implementaciones omitidas para brevedad... */

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget *win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(win), "GTK Scanner");
    gtk_window_set_default_size(GTK_WINDOW(win), 900, 600);
    g_signal_connect(win, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Pane principal: izquierda botones, derecha otro pane para info/alert
    GtkWidget *pane_main = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_container_add(GTK_CONTAINER(win), pane_main);

    // Izquierda: botones
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_margin_start(vbox, 10);
    gtk_widget_set_margin_top(vbox, 10);
    gtk_widget_set_margin_bottom(vbox, 10);
    gtk_paned_pack1(GTK_PANED(pane_main), vbox, FALSE, FALSE);

    const char *labels[] = {"Escanear dispositivos", "Escanear procesos", "Escanear puertos", "Escanear todo"};
    for (int i = 0; i < 4; i++) {
        GtkWidget *btn = gtk_button_new_with_label(labels[i]);
        gtk_box_pack_start(GTK_BOX(vbox), btn, FALSE, FALSE, 5);
    }

    // Pane secundario (derecha): info | alertas
    GtkWidget *pane_info_alert = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_paned_pack2(GTK_PANED(pane_main), pane_info_alert, TRUE, FALSE);

    // Zona info (ajustable)
    GtkWidget *sw_info = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_hexpand(sw_info, TRUE);
    gtk_widget_set_vexpand(sw_info, TRUE);
    GtkWidget *tv_info = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(tv_info), FALSE);
    info_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(tv_info));
    gtk_container_add(GTK_CONTAINER(sw_info), tv_info);
    gtk_paned_pack1(GTK_PANED(pane_info_alert), sw_info, TRUE, FALSE);

    // Zona alertas
    GtkWidget *sw_alert = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_size_request(sw_alert, 875, -1);
    gtk_widget_set_vexpand(sw_alert, TRUE);
    GtkWidget *tv_alert = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(tv_alert), FALSE);
    alert_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(tv_alert));
    gtk_container_add(GTK_CONTAINER(sw_alert), tv_alert);
    gtk_paned_pack2(GTK_PANED(pane_info_alert), sw_alert, FALSE, FALSE);

    // Conectar señales
    GtkTextBuffer *buffers[2] = {info_buffer, alert_buffer};
    GList *children = gtk_container_get_children(GTK_CONTAINER(vbox));
    for (GList *l = children; l; l = l->next) {
        GtkWidget *btn = GTK_WIDGET(l->data);
        g_signal_connect(btn, "clicked", G_CALLBACK(on_scan), buffers);
    }
    g_list_free(children);

    gtk_widget_show_all(win);
    gtk_main();
    return 0;
}
