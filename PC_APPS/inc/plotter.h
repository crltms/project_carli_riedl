
#ifndef _plotter_
#define _plotter_

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <glib/gprintf.h>
#define X_MAX 390
#define Y_MAX 310

typedef struct {
	GtkApplication *app;
	GtkWidget *window;
	GtkWidget *headerbar;
	GtkWidget *statusbar;
	guint id;
	gchar msg[60];
	GtkWidget *label;
	gchar *filename;
	GtkWidget *image;
	GtkWidget *draw;
	GdkPixbuf *pixbuf;

	gint xsize;
	gint ysize;
	gint state;
	gint cmd_num;
	cairo_t* cr;
	int fd;

	GtkWidget  *progress_window;
	GtkWidget  *pbar;

} widgets;

#endif
