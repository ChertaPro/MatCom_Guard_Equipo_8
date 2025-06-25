// gui.c
#include <gtk/gtk.h>
#include "pdf.h"
#include "main.h"
#include "gui.h"

GtkTextBuffer *info_buffer = NULL;
GtkTextBuffer *alert_buffer = NULL;

static const char *btn_labels[4] = {
    "Escanear dispositivos",
    "Escanear procesos",
    "Escanear puertos",
    "Escanear todo"};

static const char *pdf_filenames[4] = {
    "dispositivos.pdf",
    "procesos.pdf",
    "puertos.pdf",
    "todo.pdf"};

static void on_button_clicked(GtkButton *btn, gpointer data)
{
    int idx = GPOINTER_TO_INT(data);
    char *label = malloc(strlen(Alert_Devices.data) + strlen(Alert_Process.data) + strlen(Alert_Ports.data));
    char *data1 = Alert_Devices.data;
    char *data2 = Alert_Process.data;
    char *data3 = Alert_Ports.data;
    if (idx == 0)
        label = Alert_Devices.data;
    else if (idx == 1)
        label = Alert_Process.data;
    else if (idx == 2)
        label = Alert_Ports.data;
    else
        strcpy(label, data1), strcat(label, data2), strcat(label, data3);
    const char *file = pdf_filenames[idx];
    generate_pdf(label, file);
}

void clear_info(void)
{
    gtk_text_buffer_set_text(info_buffer, "", -1);
}

void clear_alert(void)
{
    gtk_text_buffer_set_text(alert_buffer, "", -1);
}

void gui_init(int *argc, char ***argv)
{
    gtk_init(argc, argv);
    GtkWidget *win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(win), "GTK Scanner con PDF");
    gtk_window_set_default_size(GTK_WINDOW(win), 900, 600);
    g_signal_connect(win, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Pane principal horizontal: botones | panes derecho
    GtkWidget *pane_main = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_container_add(GTK_CONTAINER(win), pane_main);

    // Botonera
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_paned_pack1(GTK_PANED(pane_main), vbox, FALSE, FALSE);
    for (int i = 0; i < 4; ++i)
    {
        GtkWidget *btn = gtk_button_new_with_label(btn_labels[i]);
        gtk_box_pack_start(GTK_BOX(vbox), btn, FALSE, FALSE, 5);
        g_signal_connect(btn, "clicked", G_CALLBACK(on_button_clicked), GINT_TO_POINTER(i));
    }

    // Pane derecho horizontal: info | alertas
    GtkWidget *pane_right = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_paned_pack2(GTK_PANED(pane_main), pane_right, TRUE, FALSE);

    // Área info
    GtkWidget *sw_info = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_hexpand(sw_info, TRUE);
    gtk_widget_set_vexpand(sw_info, TRUE);
    GtkWidget *tv_info = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(tv_info), FALSE);
    info_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(tv_info));
    gtk_container_add(GTK_CONTAINER(sw_info), tv_info);
    gtk_paned_pack1(GTK_PANED(pane_right), sw_info, TRUE, TRUE);

    // Área alertas
    GtkWidget *sw_alert = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_hexpand(sw_alert, TRUE);
    gtk_widget_set_vexpand(sw_alert, TRUE);
    gtk_widget_set_size_request(sw_alert, 850, -1);
    GtkWidget *tv_alert = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(tv_alert), FALSE);
    alert_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(tv_alert));
    gtk_container_add(GTK_CONTAINER(sw_alert), tv_alert);
    gtk_paned_pack2(GTK_PANED(pane_right), sw_alert, FALSE, TRUE);

    gtk_widget_show_all(win);
}

void gui_run(void)
{
    gtk_main();
}

void append_info(const char *msg)
{
    GtkTextIter it;
    gtk_text_buffer_get_end_iter(info_buffer, &it);
    gtk_text_buffer_insert(info_buffer, &it, msg, -1);
}

void append_alert(const char *msg)
{
    GtkTextIter it;
    gtk_text_buffer_get_end_iter(alert_buffer, &it);
    gtk_text_buffer_insert(alert_buffer, &it, msg, -1);
}