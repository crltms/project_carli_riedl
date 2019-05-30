
#include "plotter.h"
#include "menucallbacks.h"
#include <string.h>
#include "serial.h"

#define EoF_RES 10     // \n
#define EoF 36         //$
#define SoF 35         //#
const char DONE[] = "G01\n";
const char ERR[] = "ERR\n";
#define STEPS_PER_MM 91

void
quit_callback (GSimpleAction *action, GVariant *parameter, gpointer data)
{
	widgets *a = (widgets *) data;
	g_application_quit (G_APPLICATION (a->app));
}
void
draw_callback (GSimpleAction *action, GVariant *parameter, gpointer data)
{
	widgets *wd = (widgets *) data;
	printf ("draw_callback\n");
	GThread    *thread;

	if (GTK_IS_WIDGET (wd->progress_window) == FALSE) {
		if (wd->cmd_num > 0) {
			printf ("create window\n");
			wd->progress_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
			gtk_window_set_title (GTK_WINDOW (wd->progress_window), "Drawing Progress");

			wd->pbar = gtk_progress_bar_new ();
			gtk_container_add (GTK_CONTAINER (wd->progress_window), wd->pbar);
			gtk_container_set_border_width (GTK_CONTAINER (wd->progress_window), 30);
			gtk_window_set_default_size (GTK_WINDOW (wd->progress_window), 500, 200);
			gtk_widget_show_all (wd->progress_window);
			printf ("create thread\n");
			/* run the time-consuming operation in a separate thread */
			thread = g_thread_new ("worker", worker, wd);
			g_thread_unref (thread);
		} else {
			error (wd,"no image");
		}
	}
}
gpointer
worker (gpointer data)
{
	widgets *wd = (widgets *) data;
	gchar response[STRING_SIZE]={0};
	gchar string[STRING_SIZE] = {0};
	struct gcode *gcode = get_gcode_ptr();
	int i = 0;
//---------------------------------------------------------------------------
//--ADD MAXVAL
//---------------------------------------------------------------------------
	while (i <= wd->cmd_num) {
		if(gcode[i].cmd[1] == '2'){
			g_snprintf (string, STRING_SIZE, "#%s$", gcode[i].cmd);
		}
		else{
			g_snprintf (string, STRING_SIZE, "#%s %.2f %.2f$", gcode[i].cmd,gcode[i].x_val, gcode[i].y_val);
		}
		g_print ("%s\n", string);
		send_cmd (wd->fd, string);
		memset (&response, 0, STRING_SIZE);
		while (strcmp (response, DONE) != 0) {
			get_cmd (wd->fd, response);
		}
		g_print ("%s", response);
		response[0]='E';
		response[1]='R';
		response[2]='R';
		if((response[0]=='E')&&(response[1]=='R')&&(response[2]=='R')){
			error(wd,"help");
			break;
		}
		if (GTK_IS_PROGRESS_BAR (wd->pbar) == 0) {
			return FALSE;
		}
		gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (wd->pbar), (float) i / wd->cmd_num);
		i++;
	}
	/* we finished working, do something back in the main thread */
	g_idle_add (worker_finish_in_idle, wd);

	return NULL;
}

gboolean
worker_finish_in_idle (gpointer data)
{
	widgets *wd = (widgets *) data;

	/* destroy everything */
	gtk_widget_destroy (wd->progress_window);

	return FALSE; /* stop running */
}
void
stop_callback (GSimpleAction *action, GVariant *parameter, gpointer data)
{
	widgets *wd = (widgets *) data;
	printf ("stop_callback\n");

	if (GTK_IS_WIDGET (wd->progress_window) == TRUE) {
		g_idle_add (worker_finish_in_idle, wd);
	}
}

void
open_callback (GSimpleAction *action, GVariant *parameter, gpointer data)
{
	widgets *a = (widgets *) data;

	GtkWidget *chooser;
	chooser = gtk_file_chooser_dialog_new ("Open File ...",
					       GTK_WINDOW (a ->window),
					       GTK_FILE_CHOOSER_ACTION_OPEN,
					       "_Cancel", GTK_RESPONSE_CANCEL,
					       "_Open", GTK_RESPONSE_ACCEPT,
					       NULL);
	gtk_window_set_default_size (GTK_WINDOW (chooser), 300, 300);

	if (gtk_dialog_run (GTK_DIALOG (chooser)) == GTK_RESPONSE_ACCEPT) {

		a->filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (chooser));
		a->cmd_num = parse_GCode (a->filename);

		check_len ( (sizeof (a->msg) - strlen ("opened from ")), (sizeof (a->msg) - strlen ("Opened from ...")), a, 1);
		gtk_statusbar_push (GTK_STATUSBAR (a->statusbar), a->id, a->msg);
		a->filename = short_name (a->filename, '/');
		gtk_header_bar_set_subtitle (GTK_HEADER_BAR (a->headerbar), a->filename);
	}
	gtk_widget_destroy (chooser);
}

void
save_callback (GSimpleAction *action, GVariant *parameter, gpointer data)
{
	widgets *a = (widgets *) data;
	// if (a->pixbuf != NULL) {
	GtkWidget *chooser;
	GtkFileFilter *filter;
	const gchar * filtername = "jpeg";
	gchar *fn = NULL;

	chooser = gtk_file_chooser_dialog_new ("File Save ...",
					       GTK_WINDOW (a ->window),
					       GTK_FILE_CHOOSER_ACTION_SAVE,
					       "_Cancel", GTK_RESPONSE_CANCEL,
					       "_Save", GTK_RESPONSE_ACCEPT,
					       NULL);
	gtk_window_set_default_size (GTK_WINDOW (chooser), 300, 300);
	filter = file_choose ("jpeg", chooser);
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (chooser), filter);
	filter = file_choose ("png", chooser);
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (chooser), filter);
	filter = file_choose ("ico", chooser);
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (chooser), filter);
	filter = file_choose ("bmp", chooser);
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (chooser), filter);

	gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (chooser), a->filename);
	if (gtk_dialog_run (GTK_DIALOG (chooser)) == GTK_RESPONSE_ACCEPT) {
		a->filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (chooser));
		fn = short_name (a->filename, '.');
		if (strcmp (fn, "jpeg") == 0 || strcmp (fn, "bmp") == 0 || strcmp (fn, "png") == 0 || strcmp (fn, "ico") == 0) {
			filtername = fn;
		} else {
			filter = gtk_file_chooser_get_filter (GTK_FILE_CHOOSER (chooser));
			filtername = gtk_file_filter_get_name (filter);
		}
		gdk_pixbuf_save (a->pixbuf, a->filename, filtername, NULL, NULL, NULL, NULL);

		check_len ( (sizeof (a->msg) - strlen ("saved to ")), (sizeof (a->msg) - strlen ("saved to ...")), a, 0);
		gtk_statusbar_push (GTK_STATUSBAR (a->statusbar), a->id, a->msg);
		a->filename = short_name (a->filename, '/');
		gtk_header_bar_set_subtitle (GTK_HEADER_BAR (a->headerbar), a->filename);
	}
	gtk_widget_destroy (chooser);
}
void check_len (int maxlen, int messagelen, gpointer data, int flag)
{

	widgets *a = (widgets *) data;
	int len = strlen (a->filename);
	char *text;
	if (flag == 0) {
		text = "Saved to";
	} else {
		text = "Opened from";
	}
	if (len > maxlen) {
		char filename_copy[messagelen];
		for (int i = 0; i < messagelen; i++) {
			filename_copy[i] = a->filename[ (len - (messagelen - 1) + i)];
		}
		filename_copy[sizeof (filename_copy)] = '\0';
		g_sprintf (a->msg, "%s ...%s", text, filename_copy);
	} else {
		g_sprintf (a->msg, "%s %s", text, a->filename);

	}
}
gchar* short_name (gchar* name, gchar sign)
{
	gchar* fn = NULL;
	if (name[ (g_utf8_strlen (name, 256) - 1)] == sign) {
		name[ (g_utf8_strlen (name, 256) - 1)] = '\0';
	}
	fn = g_utf8_strrchr (name, g_utf8_strlen (name, 256), sign);
	if (fn != NULL) {
		fn++;
	}

	else {
		fn = name;
	}

	return fn;
}
GtkFileFilter *file_choose (const char* name, GtkWidget *chooser)
{
	char format[11];
	GtkFileFilter *filter;
	filter = gtk_file_filter_new();
	gtk_file_filter_set_name (filter, name);
	sprintf (format, "image/%2s", name);
	format[sizeof (format) - 1] = '\0';
	gtk_file_filter_add_mime_type (filter, (const char*) format);

	return filter;
}
void free_buf (guchar *pixels, gpointer data)
{
	g_free (pixels);
}

void
about_callback (GSimpleAction *action, GVariant *parameter, gpointer data)
{
	widgets *a = (widgets *) data;

	GdkPixbuf *pixbuf;
	GtkWidget *about_dialog;
	const gchar *authors[] = {"Riedl and Carli", NULL};
	const gchar *documenters[] = {"Riedl and Carli", NULL};

	about_dialog = gtk_about_dialog_new ();
	pixbuf = gdk_pixbuf_new_from_file ("gnome.png", NULL);
	gtk_show_about_dialog (GTK_WINDOW (a->window),
			       "program-name", "Plotter",
			       "version", "0.1",
			       "copyright", "Copyright \xc2\xa9 Riedl & Carli",
			       "license-type", GTK_LICENSE_LGPL_3_0,
			       "website", "http://developer.gnome.org",
			       "comments", "A simple GTK+3 Plotter",
			       "authors", authors,
			       "documenters", documenters,
			       "logo", pixbuf,
			       "title", "About: Plotter",
			       NULL);
	g_signal_connect (about_dialog, "response", G_CALLBACK (gtk_widget_destroy), NULL);
	g_object_unref (pixbuf);
}
void
cd_draw_callback (GtkWidget *widget, GdkEvent *event, gpointer data)
{
	widgets *w = (widgets*) data;
	struct gcode *gcode = get_gcode_ptr();
	int i = 0;

	// obtain the size of the drawing area
	w->xsize = gtk_widget_get_allocated_width (w->draw);
	w->ysize = gtk_widget_get_allocated_height (w->draw);
	// printf ("x %i y %i \n", w->xsize, w->ysize);

	cairo_set_line_width (w->cr, 0.005);
	// draw a white filled rectangle
	cairo_set_source_rgb (w->cr, 1.0, 1.0, 1.0);
	cairo_rectangle (w->cr, 0, 0, w->xsize, w->ysize);
	cairo_fill (w->cr);
	// set up a transform so that (0,0) to (1,1) maps to (0, 0) to (width, height)
	cairo_translate (w->cr, 0, 0);
	cairo_scale (w->cr, w->xsize, w->ysize);

	cairo_set_source_rgb (w->cr, 0.0, 0.0, 0.0);

	if (w->filename != NULL) {
		// printf ("filename %s\n", w->filename);
		// printf ("cmd_num %i\n", w->cmd_num);

		while (i <= w->cmd_num) {
			// printf ("%i %s %f %f\n", gcode[i].ID, gcode[i].cmd, gcode[i].x_val, -gcode[i].y_val / w->ysize + 1);
			if ( (gcode[i].x_val < X_MAX) && (gcode[i].y_val < Y_MAX)) {
				if (gcode[i].cmd[2] == '0') {
					cairo_move_to (w->cr, gcode[i].x_val / w->xsize, -gcode[i].y_val / w->ysize + 1);
				} else if (gcode[i].cmd[2] == '1') {
					cairo_line_to (w->cr, gcode[i].x_val / w->xsize, -gcode[i].y_val / w->ysize + 1);
				}
			 else if (gcode[i].cmd[1] == '2') {
				cairo_move_to (w->cr, 0, 1);
			}
			}
			i++;
		}
		if (w->cmd_num == 0) {
			error (data,"Could not load gcode");
			w->filename = NULL;
		}
		cairo_stroke (w->cr);
		// cairo_move_to (w->cr, 0.0, 0.0);
		// cairo_line_to (w->cr, 1,1);
		// // cairo_line_to (w->cr, 1.0, 1.0);
		// // cairo_line_to (w->cr, 0.0, 1.0);
		// // cairo_line_to (w->cr, 0.0, 0.0);
		// // cairo_move_to (w->cr, 0.0, 0.0);
		// cairo_stroke (w->cr);
	}

}
gboolean
cd_configure_event (GtkWidget *widget, GdkEventConfigure *event, gpointer data)
{
	GtkAllocation allocation;
	static cairo_surface_t *surface = NULL;
	widgets *w = (widgets*) data;

	gtk_widget_get_allocation (widget, &allocation);
	surface = gdk_window_create_similar_surface (gtk_widget_get_window (widget),
						     CAIRO_CONTENT_COLOR,
						     allocation.width,
						     allocation.height);
	w->cr = cairo_create (surface);
	cairo_destroy (w->cr);
	return TRUE;
}

void error (gpointer data,gchar *message)
{
	widgets *a = (widgets *) data;
	GtkWidget *dialog;
	dialog = gtk_message_dialog_new (GTK_WINDOW (a->window),
					 GTK_DIALOG_DESTROY_WITH_PARENT,
					 GTK_MESSAGE_INFO,
					 GTK_BUTTONS_CLOSE,
					 "%s",message);
	g_signal_connect (dialog, "response",
			  G_CALLBACK (gtk_widget_destroy), NULL);
	gtk_widget_show (dialog);
}
