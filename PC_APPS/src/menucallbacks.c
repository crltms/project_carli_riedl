
#include "plotter.h"
#include "menucallbacks.h"
#include <string.h>
#include "serial.h"

void
quit_callback (GSimpleAction *action, GVariant *parameter, gpointer data)
{
	widgets *a = (widgets *) data;
	g_application_quit (G_APPLICATION (a->app));
}

void
open_callback (GSimpleAction *action, GVariant *parameter, gpointer data)
{
	widgets *a = (widgets *) data;

	GtkWidget *chooser;
	// GtkFileFilter *filter;
	chooser = gtk_file_chooser_dialog_new ("Open File ...",
					       GTK_WINDOW (a ->window),
					       GTK_FILE_CHOOSER_ACTION_OPEN,
					       "_Cancel", GTK_RESPONSE_CANCEL,
					       "_Open", GTK_RESPONSE_ACCEPT,
					       NULL);
	gtk_window_set_default_size (GTK_WINDOW (chooser), 300, 300);
	//filter = gtk_file_filter_new();
	//gtk_file_filter_add_pixbuf_formats (filter);
	//gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (chooser), filter);

	if (gtk_dialog_run (GTK_DIALOG (chooser)) == GTK_RESPONSE_ACCEPT) {

		a->filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (chooser));
		parse_GCode(a->filename);
		// if (a->pixbuf != NULL) {
		// 	a->pixbuf = NULL;
		// }

		// a->pixbuf = gdk_pixbuf_new_from_file (a->filename, NULL);
		// if (a->pixbuf == NULL) {
		// 	image_fail (data);
		// } else {
		// 	gtk_image_set_from_pixbuf (GTK_IMAGE (a->image), a->pixbuf);

			check_len ( (sizeof (a->msg) - strlen ("opened from ")), (sizeof (a->msg) - strlen ("Opened from ...")), a, 1);
			gtk_statusbar_push (GTK_STATUSBAR (a->statusbar), a->id, a->msg);
			a->filename = short_name (a->filename, '/');
			gtk_header_bar_set_subtitle (GTK_HEADER_BAR (a->headerbar), a->filename);
			// g_object_unref (a->pixbuf);
		// }
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
	// } else {
	// 	noimage (data);
	// }
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

	cairo_t* cr = w->cr;

	for (int i = 0; i < 5; i++) {
		cairo_set_source_rgb (cr, g_random_double(),
				      g_random_double(),
				      g_random_double());
		cairo_move_to (cr, g_random_double(), g_random_double());
		cairo_line_to (cr, g_random_double(), g_random_double());
		cairo_stroke (cr);

		cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
		cairo_move_to (cr, 0.18, 0.6);
		cairo_line_to (cr, 0.3,0.2);
		cairo_stroke (cr);
	}
	// invoke a drawing function depending on the value of the state variable
	// switch (w->state) {
	// case 0:
	// 	cd_draw_happyface (w);
	// 	break;
	// case 1:
	// 	cd_draw_lines (w);
	// 	break;
	// case 2:
	// 	cd_draw_rectangles (w);
	// 	break;
	// case 3:
	// 	cd_draw_arcs (w);
	// 	break;
	// default:
	// 	break;
	// }
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


void
image_fail (gpointer data)
{
	GtkWidget *dialog;
	widgets *a = (widgets *) data;
	dialog = gtk_message_dialog_new (GTK_WINDOW (a->window),
					 GTK_DIALOG_DESTROY_WITH_PARENT,
					 GTK_MESSAGE_ERROR,
					 GTK_BUTTONS_CLOSE,
					 "Could not load image");
	g_signal_connect (dialog, "response",
			  G_CALLBACK (gtk_widget_destroy), NULL);
	gtk_widget_show (dialog);
}
void noimage (gpointer data)
{
	widgets *a = (widgets *) data;
	GtkWidget *dialog;
	dialog = gtk_message_dialog_new (GTK_WINDOW (a->window),
					 GTK_DIALOG_DESTROY_WITH_PARENT,
					 GTK_MESSAGE_INFO,
					 GTK_BUTTONS_CLOSE,
					 "No image");
	g_signal_connect (dialog, "response",
			  G_CALLBACK (gtk_widget_destroy), NULL);
	gtk_widget_show (dialog);
}
