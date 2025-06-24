#ifndef LOG_VIEWER_H
#define LOG_VIEWER_H

#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cairo.h>
#include <cairo-pdf.h>
#include <libnotify/notify.h>
#include <time.h>

void show_log_window(GtkWidget *widget, gpointer data);

#endif