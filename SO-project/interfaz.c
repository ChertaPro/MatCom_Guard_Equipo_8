#include <gtk/gtk.h>
#include <stdlib.h>
#include "port_scanner.h"

// Funciones simuladas de detección (debes reemplazarlas con tus implementaciones reales)
int *obtener_puertos_abiertos(int *count)
{
    // Simulación: retorna puertos abiertos
    static int ports[] = {80, 443, 22, 8080};
    *count = 4;
    int *result = malloc(*count * sizeof(int));
    for (int i = 0; i < *count; i++)
    {
        result[i] = ports[i];
    }
    return result;
}

char **obtener_alertas(int *count)
{
    // Simulación: retorna alertas
    *count = 2;
    char **alerts = malloc(*count * sizeof(char *));
    alerts[0] = "Intento de acceso no autorizado en puerto 22";
    alerts[1] = "Tráfico inusual en puerto 80";
    return alerts;
}

// Estructura para pasar datos a las callbacks
typedef struct
{
    GtkWidget *parent_window;
} WindowData;

// Función para mostrar la ventana de alertas
void mostrar_alertas(GtkWidget *widget, gpointer user_data)
{
    WindowData *data = (WindowData *)user_data;

    // Crear ventana de alertas
    GtkWidget *alert_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(alert_window), "Alertas de Seguridad");
    gtk_window_set_default_size(GTK_WINDOW(alert_window), 400, 300);
    gtk_window_set_transient_for(GTK_WINDOW(alert_window), GTK_WINDOW(data->parent_window));
    gtk_window_set_modal(GTK_WINDOW(alert_window), TRUE);

    // Contenedor principal
    GtkWidget *content_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(alert_window), content_box);

    // Obtener alertas
    int alert_count;
    char **alerts = obtener_alertas(&alert_count);

    // Lista de alertas
    GtkWidget *alert_list = gtk_list_box_new();
    for (int i = 0; i < alert_count; i++)
    {
        GtkWidget *row = gtk_list_box_row_new();
        GtkWidget *label = gtk_label_new(alerts[i]);
        gtk_container_add(GTK_CONTAINER(row), label);
        gtk_container_add(GTK_CONTAINER(alert_list), row);
    }
    free(alerts);

    // Scroll para la lista
    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scrolled_window), alert_list);
    gtk_box_pack_start(GTK_BOX(content_box), scrolled_window, TRUE, TRUE, 0);

    gtk_widget_show_all(alert_window);
}

// Función para mostrar la ventana de puertos
void mostrar_puertos(GtkWidget *widget, gpointer user_data)
{
    WindowData *data = (WindowData *)user_data;

    // Crear ventana de puertos
    GtkWidget *port_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(port_window), "Puertos Abiertos");
    gtk_window_set_default_size(GTK_WINDOW(port_window), 300, 200);
    gtk_window_set_transient_for(GTK_WINDOW(port_window), GTK_WINDOW(data->parent_window));
    gtk_window_set_modal(GTK_WINDOW(port_window), TRUE);

    // Contenedor principal
    GtkWidget *content_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(port_window), content_box);

    // Obtener puertos abiertos
    int port_count;
    int *ports = obtener_puertos_abiertos(&port_count);

    // Lista de puertos
    GtkWidget *port_list = gtk_list_box_new();
    for (int i = 0; i < port_count; i++)
    {
        char port_str[16];
        snprintf(port_str, sizeof(port_str), "Puerto: %d", ports[i]);

        GtkWidget *row = gtk_list_box_row_new();
        GtkWidget *label = gtk_label_new(port_str);
        gtk_container_add(GTK_CONTAINER(row), label);
        gtk_container_add(GTK_CONTAINER(port_list), row);
    }
    free(ports);

    // Botón de alertas
    WindowData *alert_data = malloc(sizeof(WindowData));
    alert_data->parent_window = port_window;

    GtkWidget *alert_button = gtk_button_new_with_label("Ver Alertas");
    g_signal_connect(alert_button, "clicked", G_CALLBACK(mostrar_alertas), alert_data);

    // Agregar elementos al contenedor
    gtk_box_pack_start(GTK_BOX(content_box), port_list, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(content_box), alert_button, FALSE, FALSE, 0);

    gtk_widget_show_all(port_window);
}

int main(int argc, char *argv[])
{
    main_scanner();
    gtk_init(&argc, &argv);

    // Crear ventana principal
    GtkWidget *main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(main_window), "Monitor de Seguridad");
    gtk_window_set_default_size(GTK_WINDOW(main_window), 400, 300);
    g_signal_connect(main_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Botón para mostrar puertos
    GtkWidget *button = gtk_button_new_with_label("Mostrar Puertos");

    // Configurar datos para callback
    WindowData *data = malloc(sizeof(WindowData));
    data->parent_window = main_window;

    g_signal_connect(button, "clicked", G_CALLBACK(mostrar_puertos), data);

    // Contenedor principal
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(main_window), main_box);
    gtk_container_add(GTK_CONTAINER(main_box), button);

    gtk_widget_show_all(main_window);
    gtk_main();

    free(data);
    return 0;
}