#ifndef DEVICE_VIEWER_H
#define DEVICE_VIEWER_H

#include <gtk/gtk.h>
#include <string.h>
#include <pthread.h>
#include <libudev.h>
#include <libnotify/notify.h>
#include <unistd.h>
#include <stdlib.h>
#include "inotify_monitor.h"
#include "log_viewer.h"

void show_device_window(GtkWidget *widget, gpointer data);

#endif