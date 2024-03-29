
#include "plotter.h"
#include "menucallbacks.h"
#include <string.h>
#include "serial.h"

const char EoF = 36;  //$
const char SoF = 35;  //#
const char DONE[] = "DONE\n";
const char ERR[] = "ERR\n";
#define STEPS_PER_MM 91

void
quit_callback (GSimpleAction *action, GVariant *parameter, gpointer data)
{
	widgets *a = (widgets *) data;
	close (a->fd);
	if (a->fd < 0) {
		printf ("Error closing device %s\n", strerror (errno));
	}
	g_application_quit (G_APPLICATION (a->app));
}
void
draw_callback (GSimpleAction *action, GVariant *parameter, gpointer data)
{
	widgets *wd = (widgets *) data;
	GThread    *thread;

	if (wd->progress_state < 1) {
		if (wd->cmd_num > 0) {
			wd->progress_state = 1;
			wd->progress_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
			gtk_window_set_title (GTK_WINDOW (wd->progress_window), "Drawing Progress");

			wd->pbar = gtk_progress_bar_new ();
			gtk_container_add (GTK_CONTAINER (wd->progress_window), wd->pbar);
			gtk_container_set_border_width (GTK_CONTAINER (wd->progress_window), 30);
			gtk_window_set_default_size (GTK_WINDOW (wd->progress_window), 500, 200);
			gtk_window_set_deletable(GTK_WINDOW (wd->progress_window), FALSE);
			gtk_widget_show_all (wd->progress_window);
			/* run the time-consuming operation in a separate thread */
			thread = g_thread_new ("worker", worker, wd);
			g_thread_unref (thread);
		} else {
			error (wd, "no image");
		}
	}
}
gpointer
worker (gpointer data)
{
	widgets *wd = (widgets *) data;
	gchar response[STRING_SIZE] = {0};
	gchar string[STRING_SIZE] = {0};
	struct gcode gcode_copy[wd->cmd_num+1];
	memcpy(gcode_copy,wd->gcode,sizeof( gcode_copy));
  int i = 0;
	int timeout = 0;
	int ret_send = 0;
	int ret_get = 0;
	int cmd_num = wd->cmd_num;
	wd->error_flag = 0;

	while (i <= cmd_num) {
		if ( (gcode_copy[i].x_val < X_MAX) && (gcode_copy[i].y_val < Y_MAX)) {
			if (gcode_copy[i].cmd[1] == '2') {
				g_snprintf (string, STRING_SIZE, "%c%s%c",SoF, gcode_copy[i].cmd,EoF);
			} else {
				g_snprintf (string, STRING_SIZE, "%c%s %.2f %.2f%c", SoF,gcode_copy[i].cmd, gcode_copy[i].x_val * STEPS_PER_MM, gcode_copy[i].y_val * STEPS_PER_MM,EoF);
			}

			ret_send = send_cmd (wd->fd, string);
			g_print ("%s\n", string);
			memset (&response, 0, STRING_SIZE);
			while ((!*response) && (timeout < TIMEOUT)) {
				ret_get = get_cmd (wd->fd, response);
				if (ret_get > 0) {
					wd->error_flag = 1;
					break;
				}
				timeout++;
				g_usleep (5000);
			}
			if ( (ret_get > 0) || (ret_send > 0)) {
				wd->error_flag = 1;
				break;
			}
			g_print ("%s", response);
			// response[0]='E';
			// response[1]='R';
			// response[2]='R';
			if ((strcmp (response, ERR) == 0) || timeout >= TIMEOUT || (strcmp (response, DONE) != 0)) {
				wd->error_flag = 1;
				break;
			}
		}
		if (GTK_IS_PROGRESS_BAR (wd->pbar) == 0) {
			wd->progress_state = 0;
			return FALSE;
		}
		i++;
		gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (wd->pbar), (float) i / cmd_num);
	}
	/* we finished working, do something back in the main thread */
	g_idle_add (worker_finish_in_idle, wd);
	g_thread_exit (NULL);

	return NULL;
}

gboolean
worker_finish_in_idle (gpointer data)
{
	widgets *wd = (widgets *) data;
	/* destroy everything */
	gtk_widget_destroy (wd->progress_window);
	wd->progress_state = 0;
	if (wd->error_flag > 0) {
		error (wd, "Error when sending data");
	}

	return FALSE; /* stop running */
}
void
stop_callback (GSimpleAction *action, GVariant *parameter, gpointer data)
{
	widgets *wd = (widgets *) data;

	if (wd->progress_state > 0) {
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
		a->cmd_num = parse_GCode (a->filename,a->gcode);

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
	if (a->filename != NULL) {
		GtkWidget *chooser;
		FILE *pF;
		gchar filename_old[STRINGLENGTH];
		int ret = 0;
		int i = 0;
		strcpy (filename_old, a->filename);

		chooser = gtk_file_chooser_dialog_new ("File Save ...",
						       GTK_WINDOW (a ->window),
						       GTK_FILE_CHOOSER_ACTION_SAVE,
						       "_Cancel", GTK_RESPONSE_CANCEL,
						       "_Save", GTK_RESPONSE_ACCEPT,
						       NULL);

		gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (chooser), "output.txt");
		if (gtk_dialog_run (GTK_DIALOG (chooser)) == GTK_RESPONSE_ACCEPT) {
			a->filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (chooser));

			pF = fopen (a->filename, "w");
			if (pF == NULL) {
				printf ("%s can't be opened\n", a->filename);
			}
			if (a->gcode != NULL) {
				while (i <= a->cmd_num) {
					ret = fprintf (pF, "%i %s %.2f %.2f\n", a->gcode[i].ID, a->gcode[i].cmd, a->gcode[i].x_val, a->gcode[i].y_val);
					if (ret < 0) {
						printf ("Error when writing data\n");
						break;
					}
					i++;
				}
			}
			ret = fclose (pF);
			if (ret != 0) {
				printf ("%s wasn't closed correctly\n", a->filename);
			}

			check_len ( (sizeof (a->msg) - strlen ("saved to ")), (sizeof (a->msg) - strlen ("saved to ...")), a, 0);
			gtk_statusbar_push (GTK_STATUSBAR (a->statusbar), a->id, a->msg);
		}
		gtk_widget_destroy (chooser);
		strcpy (a->filename, filename_old);
	} else {
		error (a, "Nothing to safe");
	}
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

	int i = 0;

	// obtain the size of the drawing area
	w->xsize = gtk_widget_get_allocated_width (w->draw);
	w->ysize = gtk_widget_get_allocated_height (w->draw);

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

		while (i <= w->cmd_num) {
			if ( (w->gcode[i].x_val < X_MAX) && (w->gcode[i].y_val < Y_MAX)) {

				if (w->gcode[i].cmd[2] == '0') {
					cairo_move_to (w->cr, w->gcode[i].x_val / w->xsize, -w->gcode[i].y_val / w->ysize + 1);
				} else if (w->gcode[i].cmd[2] == '1') {
					cairo_line_to (w->cr, w->gcode[i].x_val / w->xsize, -w->gcode[i].y_val / w->ysize + 1);
				} else if (w->gcode[i].cmd[1] == '2') {
					cairo_move_to (w->cr, 0, 1);
				}
			}
			i++;
		}
		if (w->cmd_num == 0) {
			error (data, "Could not load gcode");
			w->filename = NULL;
		}
		cairo_stroke (w->cr);
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

void error (gpointer data, gchar *message)
{
	widgets *a = (widgets *) data;
	GtkWidget *dialog;
	dialog = gtk_message_dialog_new (GTK_WINDOW (a->window),
					 GTK_DIALOG_DESTROY_WITH_PARENT,
					 GTK_MESSAGE_INFO,
					 GTK_BUTTONS_CLOSE,
					 "%s", message);
	g_signal_connect (dialog, "response",
			  G_CALLBACK (gtk_widget_destroy), NULL);
	gtk_widget_show (dialog);
}
