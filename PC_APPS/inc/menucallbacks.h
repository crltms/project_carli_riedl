#ifndef _menucallbacks_
#define _menucallbacks_

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include "bsp_parse_GCode.h"

void quit_callback (GSimpleAction *action, GVariant *parameter, gpointer data);
void open_callback (GSimpleAction *action, GVariant *parameter, gpointer data);
void draw_callback (GSimpleAction *action, GVariant *parameter, gpointer data);
void stop_callback (GSimpleAction *action, GVariant *parameter, gpointer data);
void save_callback (GSimpleAction *action, GVariant *parameter, gpointer data);
void check_len (int maxlen, int messagelen, gpointer data, int flag);
gchar* short_name (gchar* name, gchar sign);
void about_callback (GSimpleAction *action, GVariant *parameter, gpointer data);
void error (gpointer data,gchar *message);
void free_buf (guchar *pixels, gpointer data);
gboolean cd_configure_event (GtkWidget *widget, GdkEventConfigure *event, gpointer data);
void cd_draw_callback (GtkWidget *widget, GdkEvent *event, gpointer data);
gpointer worker (gpointer data);
gboolean worker_finish_in_idle (gpointer data);

#endif
