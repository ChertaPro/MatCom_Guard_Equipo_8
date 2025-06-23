#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cairo.h>
#include <cairo-pdf.h>
#include <time.h>

GtkWidget *log_window = NULL;
GtkTextView *text_view = NULL;
GtkComboBoxText *filter_combo = NULL;
GtkComboBoxText *sort_combo = NULL;

// Devuelve la dirección del archivo de log
char *get_log_path()
{
    const char *home = getenv("HOME");
    char *log_path = malloc(512);
    snprintf(log_path, 512, "%s/.fileguardian.log", home);
    return log_path;
}

// Comparador de líneas para ordenamiento
int compare_lines(const void *a, const void *b, void *sort_key_ptr)
{
    const char *key = sort_key_ptr;
    const char *line1 = *(const char **)a;
    const char *line2 = *(const char **)b;

    if (strcmp(key, "Tipo") == 0)
    {
        return strcasecmp(strstr(line1, "]") + 2, strstr(line2, "]") + 2);
    }
    else if (strcmp(key, "Nombre") == 0)
    {
        const char *colon1 = strrchr(line1, ':');
        const char *colon2 = strrchr(line2, ':');
        return strcasecmp(colon1 ? colon1 + 1 : "", colon2 ? colon2 + 1 : "");
    }
    else
    { // Fecha (default)
        return strcasecmp(line1, line2);
    }
}

// Filtra y ordena las líneas según el tipo seleccionado
char *filtered_sorted_log(const char *filter_type, const char *sort_key)
{
    char *log_path = get_log_path();
    FILE *f = fopen(log_path, "r");
    free(log_path);
    if (!f)
        return g_strdup("No se encontró el archivo de log.");

    // Leer y filtrar líneas
    char **lines = malloc(2048 * sizeof(char *));
    int count = 0;
    char line[1024];
    while (fgets(line, sizeof(line), f))
    {
        if (strcmp(filter_type, "TODOS") == 0 || strstr(line, filter_type))
        {
            lines[count] = strdup(line);
            count++;
        }
    }
    fclose(f);

    // Ordenar
    qsort_r(lines, count, sizeof(char *), compare_lines, (void *)sort_key);

    // Concatenar
    char *content = malloc(count * 1024);
    content[0] = '\0';
    for (int i = 0; i < count; ++i)
    {
        strcat(content, lines[i]);
        free(lines[i]);
    }
    free(lines);
    return content;
}

// Refresca el TextView con el filtro seleccionado
void refresh_log_view()
{
    const char *filter = gtk_combo_box_text_get_active_text(filter_combo);
    const char *sort = gtk_combo_box_text_get_active_text(sort_combo);
    if (!filter || !sort)
        return;

    char *filtered_sorted_log = read_filtered_sorted_log(filter, sort);
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(text_view);
    gtk_text_buffer_set_text(buffer, filtered_sorted_log, -1);
    free(filtered_sorted_log);
}

// Exporta el contenido a PDF
void export_log_to_pdf(const char *content)
{
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char filename[128];
    strftime(filename, sizeof(filename), "log_export_%Y%m%d_%H%M%S.pdf", t);

    cairo_surface_t *surface = cairo_pdf_surface_create(filename, 595, 842); // A4
    cairo_t *cr = cairo_create(surface);

    cairo_select_font_face(cr, "Courier", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 12);

    double x = 40, y = 40, line_height = 16;
    const char *ptr = content;
    while (*ptr)
    {
        const char *newline = strchr(ptr, '\n');
        int len = newline ? (newline - ptr) : strlen(ptr);

        char line[1024];
        strncpy(line, ptr, len);
        line[len] = '\0';

        cairo_move_to(cr, x, y);
        cairo_show_text(cr, line);
        y += line_height;

        if (y > 800)
        {
            cairo_show_page(cr);
            y = 40;
        }

        ptr = newline ? newline + 1 : ptr + len;
    }

    cairo_destroy(cr);
    cairo_surface_destroy(surface);

    // Notificación
    char msg[256];
    snprintf(msg, sizeof(msg), "Log exportado como %s", filename);
    NotifyNotification *n = notify_notification_new("✅ Exportación exitosa", msg, NULL);
    notify_notification_show(n, NULL);
    g_object_unref(n);
}

// Función callback para el botón
void on_export_button_clicked(GtkButton *button, gpointer user_data)
{
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(text_view);
    GtkTextIter start, end;
    gtk_text_buffer_get_start_iter(buffer, &start);
    gtk_text_buffer_get_end_iter(buffer, &end);
    char *content = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);

    export_log_to_pdf(content);
    g_free(content);
}

// Muestra ventana de log con filtro
void show_log_window(GtkWidget *widget, gpointer data)
{
    if (log_window)
    {
        gtk_window_present(GTK_WINDOW(log_window));
        return;
    }

    log_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(log_window), "Log de Actividad");
    gtk_window_set_default_size(GTK_WINDOW(log_window), 600, 400);
    g_signal_connect(log_window, "destroy", G_CALLBACK(gtk_widget_destroyed), &log_window);

    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);

    // ComboBox de filtro
    filter_combo = GTK_COMBO_BOX_TEXT(gtk_combo_box_text_new());
    gtk_combo_box_text_append(filter_combo, NULL, "TODOS");
    gtk_combo_box_text_append(filter_combo, NULL, "CREADO");
    gtk_combo_box_text_append(filter_combo, NULL, "ELIMINADO");
    gtk_combo_box_text_append(filter_combo, NULL, "MODIFICADO");
    gtk_combo_box_set_active(GTK_COMBO_BOX(filter_combo), 0);
    g_signal_connect(filter_combo, "changed", G_CALLBACK(refresh_log_view), NULL);
    gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(filter_combo), FALSE, FALSE, 5);

    // ComboBox de orden
    sort_combo = GTK_COMBO_BOX_TEXT(gtk_combo_box_text_new());
    gtk_combo_box_text_append(sort_combo, NULL, "Fecha");
    gtk_combo_box_text_append(sort_combo, NULL, "Tipo");
    gtk_combo_box_text_append(sort_combo, NULL, "Nombre");
    gtk_combo_box_set_active(GTK_COMBO_BOX(sort_combo), 0);
    g_signal_connect(sort_combo, "changed", G_CALLBACK(refresh_log_view), NULL);
    gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(sort_combo), FALSE, FALSE, 5);

    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);

    GtkWidget *export_button = gtk_button_new_with_label("📄 Exportar a PDF");
    g_signal_connect(export_button, "clicked", G_CALLBACK(on_export_button_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(hbox), export_button, FALSE, FALSE, 5);

    // Área de texto
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    text_view = GTK_TEXT_VIEW(gtk_text_view_new());
    gtk_text_view_set_editable(text_view, FALSE);
    gtk_text_view_set_wrap_mode(text_view, GTK_WRAP_WORD_CHAR);
    gtk_container_add(GTK_CONTAINER(scroll), GTK_WIDGET(text_view));
    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);

    // Inicializar con contenido
    refresh_log_view();

    gtk_container_add(GTK_CONTAINER(log_window), vbox);
    gtk_widget_show_all(log_window);
}
