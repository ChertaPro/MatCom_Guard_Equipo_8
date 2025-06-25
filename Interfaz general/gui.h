// gui.h
#ifndef GUI_H
#define GUI_H

#include <gtk/gtk.h>

/**
 * Inicializa la interfaz gráfica. Debe llamarse antes de usar append_info/alert.
 * scan_handlers es un array de cuatro punteros:
 *  [0] dispositivos, [1] procesos, [2] puertos, [3] todo
 */
void gui_init(int *argc, char ***argv);

/**
 * Lanza el bucle GTK. Bloqueante.
 */
void gui_run(void);

/**
 * Añade texto al buffer de info.
 */
void append_info(const char *msg);

/**
 * Añade texto al buffer de alertas.
 */
void append_alert(const char *msg);

void clear_info(void);
void clear_alert(void);

#endif // GUI_H