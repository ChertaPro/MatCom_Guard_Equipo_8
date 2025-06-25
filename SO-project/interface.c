#include <gtk/gtk.h>
#include <cairo-pdf.h>
#include <time.h>

/* Prototipos de tus funciones */
gchar* Reporte(void);
gchar* EscaneoDispositivos(void);
gchar* EscaneoProcesos(void);
gchar* EscaneoPuertos(void);
gchar* EscaneoGeneral(void);
void GenerarPDF(const gchar *texto, const char *filename);
gchar* ObtenerAlertas(void);

/* Buffers globales */
static GtkTextBuffer *buffer_reporte;
static GtkTextBuffer *buffer_alertas;

/* Refresca el área de Reportes */
static gboolean
actualizar_reporte_cb (gpointer user_data)
{
    gchar *texto = Reporte();
    gtk_text_buffer_set_text(buffer_reporte, texto, -1);
    g_free(texto);
    return TRUE;
}

/* Refresca el área de Alertas */
static gboolean
actualizar_alertas_cb (gpointer user_data)
{
    gchar *texto = ObtenerAlertas();
    gtk_text_buffer_set_text(buffer_alertas, texto, -1);
    g_free(texto);
    return TRUE;
}

/* Callback genérico para botones */
static void
on_button_clicked (GtkButton *button, gpointer user_data)
{
    gchar* (*func)() = user_data;
    gchar *resultado = func();

    /* Generar PDF */
    GenerarPDF(resultado, "salida.pdf");

    /* Actualizar vista de reportes */
    gtk_text_buffer_set_text(buffer_reporte, resultado, -1);
    g_free(resultado);
}

int main (int argc, char *argv[])
{
    gtk_init(&argc, &argv);

    /* Ventana principal */
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 1000, 500);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    /* Contenedor horizontal principal */
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_container_add(GTK_CONTAINER(window), hbox);

    /* 1) VBox de botones */
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 6);

    struct {
        const char *label;
        gchar* (*func)();
    } botones[] = {
        { "Escaneo de Dispositivos", EscaneoDispositivos },
        { "Escaneo de Procesos",    EscaneoProcesos    },
        { "Escaneo de Puertos",     EscaneoPuertos     },
        { "Escaneo General",        EscaneoGeneral    },
    };

    for (int i = 0; i < G_N_ELEMENTS(botones); i++) {
        GtkWidget *btn = gtk_button_new_with_label(botones[i].label);
        gtk_box_pack_start(GTK_BOX(vbox), btn, FALSE, FALSE, 2);
        g_signal_connect(btn, "clicked",
                         G_CALLBACK(on_button_clicked),
                         botones[i].func);
    }

    /* 2) Panel divisorio para Reportes y Alertas */
    GtkWidget *paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(hbox), paned, TRUE, TRUE, 6);
    /* Posición inicial: reportes 700px, alertas ajustable */
    gtk_paned_set_position(GTK_PANED(paned), 700);

    /* Área de Reportes */
    GtkWidget *scrolled_reporte = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_reporte),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);
    gtk_paned_add1(GTK_PANED(paned), scrolled_reporte);

    GtkWidget *textview_reporte = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textview_reporte), GTK_WRAP_WORD);
    gtk_container_add(GTK_CONTAINER(scrolled_reporte), textview_reporte);
    buffer_reporte = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview_reporte));

    /* Área de Alertas */
    GtkWidget *scrolled_alertas = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_alertas),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);
    gtk_paned_add2(GTK_PANED(paned), scrolled_alertas);

    GtkWidget *textview_alertas = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textview_alertas), GTK_WRAP_WORD);
    gtk_container_add(GTK_CONTAINER(scrolled_alertas), textview_alertas);
    buffer_alertas = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview_alertas));

    /* Timers para refrescar automáticamente */
    g_timeout_add_seconds(1, actualizar_reporte_cb, NULL);
    g_timeout_add_seconds(2, actualizar_alertas_cb, NULL);

    gtk_widget_show_all(window);
    gtk_main();
    return 0;
}

/* === Implementaciones de ejemplo === */

gchar* Reporte(void) {
    return g_strdup_printf("[%ld] Reporte en curso...\n", time(NULL));
}

gchar* ObtenerAlertas(void) {
    return g_strdup_printf("[%ld] ALERTA: Umbral excedido!\n", time(NULL));
}

gchar* EscaneoDispositivos(void) {
    return g_strdup("Dispositivos encontrados: A, B, C...\n");
}

gchar* EscaneoProcesos(void) { return g_strdup("Procesos: 1234, 5678...\n"); }

gchar* EscaneoPuertos(void)  { return g_strdup("Puertos abiertos: 22, 80...\n"); }

gchar* EscaneoGeneral(void) { return g_strdup("Escaneo completo realizado.\n"); }

void GenerarPDF(const gchar *texto, const char *filename) {
    cairo_surface_t *surface = cairo_pdf_surface_create(filename, 595, 842);
    cairo_t *cr = cairo_create(surface);
    PangoLayout *layout = pango_cairo_create_layout(cr);
    PangoFontDescription *font = pango_font_description_from_string("Monospace 10");

    pango_layout_set_font_description(layout, font);
    pango_layout_set_width(layout, PANGO_SCALE * 550);
    pango_layout_set_text(layout, texto, -1);
    pango_cairo_update_layout(cr, layout);
    pango_cairo_show_layout(cr, layout);

    g_object_unref(layout);
    pango_font_description_free(font);
    cairo_destroy(cr);
    cairo_surface_destroy(surface);
}
