#include <gtk/gtk.h>
#include "inotify_monitor.h"
#include "device_viewer.h"

// Añadir las funciones que muestran las ventanas de los procesos y las redes. Provisionales:
void show_process_window(GtkWidget *widget, gpointer data){}
void show_network_window(GtkWidget *widget, gpointer data){}

// GTK app
void app_init() // Se puede colocar en un archivo para la interfaz
{
    // Creando la ventana del menú principal
    GtkWidget *main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(main_window), "MatCom Guard");
    gtk_window_set_default_size(GTK_WINDOW(main_window), 200, 100);

    // Creando el contenedor con los botones para dispositivos, procesos y redes
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    GtkWidget *btn_devices = gtk_button_new_with_label("Ver Dispositivos");
    GtkWidget *btn_processes = gtk_button_new_with_label("Ver Procesos");
    GtkWidget *btn_networks = gtk_button_new_with_label("Ver Redes");

    g_signal_connect(btn_devices, "clicked", G_CALLBACK(show_device_window), NULL);
    g_signal_connect(btn_processes, "clicked", G_CALLBACK(show_process_window), NULL);
    g_signal_connect(btn_networks, "clicked", G_CALLBACK(show_network_window), NULL);

    gtk_box_pack_start(GTK_BOX(vbox), btn_devices, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), btn_processes, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), btn_networks, TRUE, TRUE, 0);

    gtk_container_add(GTK_CONTAINER(main_window), vbox);

    gtk_widget_show_all(main_window);
}

int main(int argc, char *argv[])
{
    gtk_init(&argc, &argv);

    // Iniciar inotify en segundo plano
    start_inotify_monitor();

    app_init();
    gtk_main();

    while (1)
        sleep(60); // mantener el programa corriendo

    return 0;
}
