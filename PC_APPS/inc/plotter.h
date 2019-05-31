
#ifndef _plotter_
#define _plotter_

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <glib/gprintf.h>
#include "uart_dialog.h"
#define X_MAX 390
#define Y_MAX 310
#define TIMEOUT 3000
enum parity {NONE, EVEN, ODD};
enum hwcheck {HWCHECKON, HWCHECKOFF};
enum swcheck {SWCHECKON, SWCHECKOFF};

typedef struct {
	GtkApplication *app;
	GtkWidget *window;
	GtkWidget *headerbar;
	GtkWidget *statusbar;
	guint id;
	gchar msg[60];
	GtkWidget *label;
	gchar *filename;
	GtkWidget *draw;
//cairo
	gint xsize;
	gint ysize;
	gint state;
	gint cmd_num;
	cairo_t* cr;
	int fd;
//drawing progress
	GtkWidget  *progress_window;
	GtkWidget  *pbar;
	guint progress_state;
	guint error_flag;
//uart configuration
	GtkWidget *dialog;
	GtkWidget *device_combo;
	GtkWidget *baudrate_combo;
	GtkWidget *parity_combo;
	GtkWidget *databits_spinb;
	GtkWidget *stopbits_spinb;
	GtkWidget *hw_check;
	GtkWidget *sw_check;
	gchar *status_device;
	gint status_baudrate;
	enum  parity status_parity;
	gint status_databits;
	gint status_stopbits;
	enum  hwcheck status_hwcheck;
	enum  swcheck status_swcheck;
	gboolean initial;
	gint idd;
	gint idb;

} widgets;

#endif
