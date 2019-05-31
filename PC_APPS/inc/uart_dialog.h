#ifndef _uart_dialog_
#define _uart_dialog_

#include <gtk/gtk.h>
#include <glib.h>
#include <glib/gprintf.h>
#include "plotter.h"
#include "serial.h"

void initial_uart_config (gpointer user_data);
void on_response (GtkDialog *dialog, gint response_id, gpointer user_data);
void dialog_cb (GSimpleAction *action, GVariant *parameter, gpointer user_data);

#endif
