
#include "plotter.h"
#include "menucallbacks.h"
#include "serial.h"

/******************************************************************** GLOBALS */

// map menu actions to callbacks
const GActionEntry app_actions[] = {
	{ "open", open_callback, NULL, NULL, NULL, {0, 0, 0} },
	{ "draw", draw_callback, NULL, NULL, NULL, {0, 0, 0} },
	{ "stop", stop_callback, NULL, NULL, NULL, {0, 0, 0} },
	{ "save", save_callback, NULL, NULL, NULL, {0, 0, 0} },
	{ "uart", dialog_cb, NULL, NULL, NULL, {0, 0, 0} },
	{ "quit", quit_callback, NULL, NULL, NULL, {0, 0, 0} },
	{ "about", about_callback, NULL, NULL, NULL, {0, 0, 0} }
};

/****************************************************** FILE LOCAL PROTOTYPES */
static void
construct_menu (GtkApplication *app, GtkWidget *box, gpointer data, GApplicationCommandLine *cmdline);


/*********************************************************************** Menu */
static void
construct_menu (GtkApplication *app, GtkWidget *box, gpointer data, GApplicationCommandLine *cmdline)
{
	GMenu *editmenu;
	GtkWidget *openbutton, *startbutton, *stopbutton;
	GMenu *gearmenu;
	GtkWidget *gearmenubutton;
	GtkWidget *gearicon;
	GtkWidget *hbox;

	widgets *a = (widgets *) data;
//commandline arguments
	gchar **args1;
	gchar **argv1;
	gint argc1;
	gint i;

	args1 = g_application_command_line_get_arguments (cmdline, &argc1);

	argv1 = g_new (gchar*, argc1 + 1);
	for (i = 0; i <= argc1; i++) {
		argv1[i] = args1[i];
	}
	a->filename = argv1[1];
	a->cmd_num = 0;
	a->progress_state = 0;
	initial_uart_config (a);

// define keyboard accelerators
	const gchar *open_accels[2] = { "<Ctrl>O", NULL };
	const gchar *stop_accels[2] = { "<Ctrl>F", NULL };
	const gchar *draw_accels[2] = { "<Ctrl>D", NULL };
	const gchar *save_accels[2] = { "<Ctrl>S", NULL };
	const gchar *quit_accels[2] = { "<Ctrl>Q", NULL };
	const gchar *about_accels[2] = { "<Ctrl>A", NULL };
	const gchar *uart_accels[2] = { "<Ctrl>U", NULL };
// headerbar
	a->headerbar = gtk_header_bar_new ();
	gtk_widget_show (a->headerbar);
	gtk_header_bar_set_title (GTK_HEADER_BAR (a->headerbar), "Plotter");
	gtk_header_bar_set_subtitle (GTK_HEADER_BAR (a->headerbar), "Riedl & Carli");
	gtk_header_bar_set_show_close_button (GTK_HEADER_BAR (a->headerbar), TRUE);
	gtk_window_set_titlebar (GTK_WINDOW (a->window), a->headerbar);
	gtk_window_set_resizable (GTK_WINDOW (a->window), FALSE);
// OPEN button
	openbutton = gtk_button_new_with_label ("Open");
	gtk_header_bar_pack_start (GTK_HEADER_BAR (a->headerbar), openbutton);
	gtk_actionable_set_action_name (GTK_ACTIONABLE (openbutton), "app.open");
// Draw button
	startbutton = gtk_button_new_with_label ("Start");
	gtk_header_bar_pack_start (GTK_HEADER_BAR (a->headerbar), startbutton);
	gtk_actionable_set_action_name (GTK_ACTIONABLE (startbutton), "app.draw");
// Draw button
	stopbutton = gtk_button_new_with_label ("Stop");
	gtk_header_bar_pack_start (GTK_HEADER_BAR (a->headerbar), stopbutton);
	gtk_actionable_set_action_name (GTK_ACTIONABLE (stopbutton), "app.stop");

//gear menu button
	gearmenubutton = gtk_menu_button_new();
	gearicon = gtk_image_new_from_icon_name ("emblem-system-symbolic",
						 GTK_ICON_SIZE_SMALL_TOOLBAR);
	gtk_button_set_image (GTK_BUTTON (gearmenubutton), gearicon);
	gtk_header_bar_pack_end (GTK_HEADER_BAR (a->headerbar), gearmenubutton);
// gearmenu
	gearmenu = g_menu_new();
	g_menu_append (gearmenu, "_Save As ...", "app.save");
	g_menu_append (gearmenu, "_Configure UART", "app.uart");
	editmenu = g_menu_new();
	g_menu_append (editmenu, "_About", "app.about");
	g_menu_append_section (gearmenu, NULL, G_MENU_MODEL (editmenu));
	gtk_menu_button_set_menu_model (GTK_MENU_BUTTON (gearmenubutton),
					G_MENU_MODEL (gearmenu));
//------------------------------------------------------------------------------
//statusbar and boxes
//------------------------------------------------------------------------------
	box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 20);
	gtk_container_add (GTK_CONTAINER (a->window), box);
//drawing area
	a->draw = gtk_drawing_area_new();
	gtk_widget_set_size_request (a->draw, X_MAX, Y_MAX);
	gtk_box_pack_start (GTK_BOX (box), a->draw, TRUE, TRUE, 20);
//Statusbar
	hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_pack_start (GTK_BOX (box), hbox, FALSE, FALSE, 0);
	a->statusbar = gtk_statusbar_new();
	gtk_widget_set_size_request (a->statusbar, X_MAX, 5);
	gtk_box_pack_start (GTK_BOX (hbox), a->statusbar, FALSE, FALSE, 0);
	a->id = gtk_statusbar_get_context_id (GTK_STATUSBAR (a->statusbar), "demo");
	a->label = gtk_label_new (" ");
	gtk_box_pack_start (GTK_BOX (hbox), a->label, TRUE, TRUE, 0);
	gtk_widget_set_opacity (a->label, 0.8);

//------------------------------------------------------------------------------
//commandline handling
//------------------------------------------------------------------------------
	if (a->filename != NULL) {
		a->cmd_num = 0;
		a->cmd_num = parse_GCode (a->filename);;
		g_sprintf (a->msg, "Opened %s from commandline", a->filename);
		gtk_statusbar_push (GTK_STATUSBAR (a->statusbar), a->id, a->msg);
		gtk_header_bar_set_subtitle (GTK_HEADER_BAR (a->headerbar), a->filename);
		// }
	}

	// Invoke the cd_draw_callback() whenever a "draw" request signal is emitted.
	// Note: "draw" signals are emitted whenever the focus of a window changes
	//       or when, e.g., gtk_widget_queue_draw() is invoked.
	g_signal_connect (a->draw, "draw", G_CALLBACK (cd_draw_callback), (gpointer) a);
	// The configure event is emitted once after start and whenever the window is
	// resized. The callback creates a drawing surface.
	g_signal_connect (a->draw, "configure_event", G_CALLBACK (cd_configure_event), (gpointer) a);


	g_object_unref (editmenu);
	g_object_unref (gearmenu);
// connect keyboard accelerators
	gtk_application_set_accels_for_action (GTK_APPLICATION (app),
					       "app.open", open_accels);
	gtk_application_set_accels_for_action (GTK_APPLICATION (app),
					       "app.stop", stop_accels);
	gtk_application_set_accels_for_action (GTK_APPLICATION (app),
					       "app.uart", uart_accels);
	gtk_application_set_accels_for_action (GTK_APPLICATION (app),
					       "app.draw", draw_accels);
	gtk_application_set_accels_for_action (GTK_APPLICATION (app),
					       "app.save", save_accels);
	gtk_application_set_accels_for_action (GTK_APPLICATION (app),
					       "app.quit", quit_accels);
	gtk_application_set_accels_for_action (GTK_APPLICATION (app),
					       "app.about", about_accels);
	g_free (argv1);
	g_strfreev (args1);
	g_object_unref (cmdline);
}

/*********************************************************** STARTUP CALLBACK */
static void
startup (GApplication *app, gpointer data)
{
	widgets *a = (widgets *) data;
// connect actions with callbacks
	g_action_map_add_action_entries (G_ACTION_MAP (app), app_actions,
					 G_N_ELEMENTS (app_actions), (gpointer) a);
}
static gint
commandline (GtkApplication *app, GApplicationCommandLine *cmdline, gpointer data)
{
	widgets *a = (widgets *) data;
//  window
	a->window = gtk_application_window_new (app);
	gtk_window_set_application (GTK_WINDOW (a->window), GTK_APPLICATION (app));
	gtk_window_set_title (GTK_WINDOW (a->window), "Plotter");
	gtk_window_set_position (GTK_WINDOW (a->window), GTK_WIN_POS_CENTER);
	gtk_window_set_default_size (GTK_WINDOW (a->window), X_MAX, Y_MAX);
	gtk_window_set_default_icon_from_file ("icon.png", NULL);

	construct_menu (app, NULL, (gpointer) a, cmdline);
	gtk_widget_show_all (GTK_WIDGET (a->window));
	g_object_ref (cmdline);
	return 0;
}
/*********************************************************************** main */
int
main (int argc, char **argv)
{

	int status;
	// *************** Serial ***************
	char *portname = "/dev/ttyUSB0";
	int fd;

	fd = open (portname, O_RDWR | O_NOCTTY | O_SYNC);
	if (fd < 0) {
		printf ("Error opening %s: %s\n", portname, strerror (errno));
		return -1;
	}
	/*baudrate 9600, 8 bits, no parity, 1 stop bit */
	set_interface_attribs (fd, 9600, 0, 8, 1, 0, 0);

	gtk_init (&argc, &argv);
	widgets *a = g_malloc (sizeof (widgets));
	a->fd = fd;
	a->app = gtk_application_new (NULL, G_APPLICATION_HANDLES_COMMAND_LINE);

	g_signal_connect (a->app, "command-line", G_CALLBACK (commandline), (gpointer) a);
	g_signal_connect (a->app, "startup", G_CALLBACK (startup), (gpointer) a);
	status = g_application_run (G_APPLICATION (a->app), argc, argv);
	g_object_unref (a->app);

	g_free (a);
	return status;
}
