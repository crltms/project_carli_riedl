#ifndef _gnome_lib_
#define _gnome_lib_

#include <gtk/gtk.h>
#include <glib/gprintf.h>
#include <glib.h>
#include "serial.h"

struct my_widgets{
  GtkApplication *app;
  GtkWidget *window;
  GtkWidget *dc_slider;
  GtkWidget *entrybox_min;
  GtkWidget *entrybox_sek;
  GtkWidget *entrybox_msek;
  GtkWidget *entrybox_usek;
  GtkWidget *resetbutton;
  GtkWidget *periodbutton;
  int dutycycle;
  int fd;
};

#endif
