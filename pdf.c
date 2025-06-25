#include <cairo.h>
#include <cairo-pdf.h>
#include <string.h>
#include <stdlib.h>

void draw_text_with_newlines(cairo_t *cr, double page_width, double page_height,
                             double x, double *y_ptr, const char *text, double line_height)
{
    const char *line_start = text;
    const char *p = text;
    double y = *y_ptr;

    while (*p)
    {
        if (*p == '\n')
        {
            char line[1024];
            int len = p - line_start;
            if (len > 1023)
                len = 1023;
            strncpy(line, line_start, len);
            line[len] = '\0';

            if (y + line_height > page_height)
            {
                cairo_show_page(cr);
                y = 50; // reset vertical position for new page
            }

            cairo_text_extents_t ext;
            cairo_text_extents(cr, line, &ext);
            double centered_x = x + (page_width - ext.width) / 2;

            cairo_move_to(cr, centered_x, y);
            cairo_show_text(cr, line);

            y += line_height;
            line_start = p + 1;
        }
        p++;
    }

    // Última línea (si no termina en '\n')
    if (line_start != p)
    {
        if (y + line_height > page_height)
        {
            cairo_show_page(cr);
            y = 50;
        }

        cairo_text_extents_t ext;
        cairo_text_extents(cr, line_start, &ext);
        double centered_x = x + (page_width - ext.width) / 2;

        cairo_move_to(cr, centered_x, y);
        cairo_show_text(cr, line_start);

        y += line_height;
    }

    *y_ptr = y; // devolver posición final
}

void generate_pdf(const char *text, const char *filename)
{
    const double width = 595, height = 842;
    cairo_surface_t *surface = cairo_pdf_surface_create(filename, width, height);
    cairo_t *cr = cairo_create(surface);

    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 12);
    cairo_set_source_rgb(cr, 0, 0, 0);

    double margin = 10;
    double y = 50;
    double line_height = 20;

    draw_text_with_newlines(cr, width - 2 * margin, height - margin, margin, &y, text, line_height);

    cairo_show_page(cr); // asegúrate de cerrar la última página
    cairo_destroy(cr);
    cairo_surface_destroy(surface);
}
