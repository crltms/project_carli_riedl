#ifndef _menucallbacks_
#define _menucallbacks_

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include "bsp_parse_GCode.h"

void image_fail (gpointer data);
void quit_callback (GSimpleAction *action, GVariant *parameter, gpointer data);
void open_callback (GSimpleAction *action, GVariant *parameter, gpointer data);
void save_callback (GSimpleAction *action, GVariant *parameter, gpointer data);
GtkFileFilter *file_choose (const char* name, GtkWidget *chooser);
void check_len (int maxlen, int messagelen, gpointer data, int flag);
gchar* short_name (gchar* name, gchar sign);
void about_callback (GSimpleAction *action, GVariant *parameter, gpointer data);
void noimage (gpointer data);
void free_buf (guchar *pixels, gpointer data);
gboolean cd_configure_event (GtkWidget *widget, GdkEventConfigure *event, gpointer data);
void cd_draw_callback (GtkWidget *widget, GdkEvent *event, gpointer data);

#endif
