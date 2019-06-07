#include <uart_dialog.h>

static void set_new_status (gpointer user_data);
static void config_uart (gpointer user_data);
static void create_device_list (GtkComboBoxText *combo);
static void create_baudrate_list (GtkComboBoxText *combo);

#define DEVLEN 15

gchar dev[][DEVLEN] = {"/dev/ttyUSB0",
		       "/dev/ttyUSB1",
		       "/dev/ttyS0",
		       "/dev/ttyS1",
		       "/dev/ttyACM0",
		       "/dev/ttyACM1"
		      };

gint br[] = {  9600,
	       19200,
	       38400,
	       57600,
	       115200
	    };

/* Initialize some default values for the UART configuration.
 */
void
initial_uart_config (gpointer user_data)
{
  widgets *mw = (widgets *) user_data;
	// initialize default settings for the UART IF
	mw->status_device = "/dev/ttyUSB0";
	mw->status_parity = NONE;
	mw->status_baudrate = 9600;
	mw->status_hwcheck = HWCHECKOFF;
	mw->status_swcheck = SWCHECKOFF;
	mw->status_smcheck = SMCHECKOFF;
	mw->status_databits = 8;
	mw->status_stopbits = 1;
	// inital device settings
	mw->idd = 0;
	// initial baudrate settings
	mw->idb = 0;
}

/* Callback function in which reacts to the "response" signal from the user in
 * the dialog window.
 */
void
on_response (GtkDialog *dialog, gint response_id, gpointer user_data)
{
	widgets *mw = (widgets *) user_data;

	// If the button clicked gives response OK (response_id being -5)
	if (response_id == GTK_RESPONSE_OK) {
		set_new_status (mw);
		config_uart (mw);
	}
	// If the button clicked gives response CANCEL (response_id being -6)
	else if (response_id == GTK_RESPONSE_CANCEL)
		config_uart (mw);
	// If the message dialog is destroyed (for example by pressing escape)
	else if (response_id == GTK_RESPONSE_DELETE_EVENT)
		config_uart (mw);

	// Destroy the dialog after one of the above actions have taken place
	gtk_widget_destroy (GTK_WIDGET (mw->dialog));
}

/*
 * UART Configuration Dialog
 */
// void
// dialog_cb (GtkWidget *widget, gpointer user_data)
void dialog_cb (GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
	GtkWidget *grid;
	GtkWidget *device_label, *baudrate_label, *parity_label;
	GtkWidget *databits_label, *stopbits_label, *hs_label;
	GtkWidget *add_button, *cancel_button;
	GtkStyleContext *context;

	GtkWidget *content_area;
	widgets *mw = (widgets *) user_data;

	/* Create a new dialog, and set the parameters as follows:
	 * Dialog Flags - make the constructed dialog modal
	 * (modal windows prevent interaction with other windows in the application)
	 * Buttons Type - use the ok and cancel buttons
	 */
	mw->dialog = gtk_dialog_new_with_buttons ("UART Configuration",
						  GTK_WINDOW (mw->window),
						  GTK_DIALOG_MODAL |
						  GTK_DIALOG_DESTROY_WITH_PARENT |
						  GTK_DIALOG_USE_HEADER_BAR,
						  (const gchar *) GTK_BUTTONS_NONE,
						  NULL);
	add_button = gtk_dialog_add_button (GTK_DIALOG (mw->dialog), "_OK",
					    GTK_RESPONSE_OK);
	cancel_button = gtk_dialog_add_button (GTK_DIALOG (mw->dialog), "_Cancel",
					       GTK_RESPONSE_CANCEL);
	context = gtk_widget_get_style_context (add_button);
	gtk_style_context_add_class (context, "text-button");
	gtk_style_context_add_class (context, "suggested-action");
	context = gtk_widget_get_style_context (cancel_button);
	gtk_style_context_add_class (context, "text-button");
	gtk_style_context_add_class (context, "destructive-action");
	gtk_window_set_default_size (GTK_WINDOW (mw->dialog), 350, 100);

	// Create a custom dialog for a UART configuration.
	content_area = gtk_dialog_get_content_area (GTK_DIALOG (mw->dialog));
	// we use a grid and pack it into the content area of the dialog
	grid = gtk_grid_new();
	gtk_grid_set_row_spacing (GTK_GRID (grid), 10);
	gtk_grid_set_column_spacing (GTK_GRID (grid), 10);
	gtk_container_add (GTK_CONTAINER (content_area), grid);
	gtk_container_set_border_width (GTK_CONTAINER (content_area), 10);

	// device label and combo + entry box - mw->idd is set as the active one
	device_label = gtk_label_new ("Device");
	gtk_grid_attach (GTK_GRID (grid), device_label, 0, 0, 1, 1);
	mw->device_combo = gtk_combo_box_text_new_with_entry ();
	create_device_list (GTK_COMBO_BOX_TEXT (mw->device_combo));
	gtk_combo_box_set_active (GTK_COMBO_BOX (mw->device_combo), mw->idd);
	gtk_grid_attach (GTK_GRID (grid), mw->device_combo, 1, 0, 1, 1);
	// baudrate label and combo + entry box - mw-idb is set as the active one
	baudrate_label = gtk_label_new ("Baud Rate");
	gtk_grid_attach (GTK_GRID (grid), baudrate_label, 0, 1, 1, 1);
	mw->baudrate_combo = gtk_combo_box_text_new_with_entry ();
	create_baudrate_list (GTK_COMBO_BOX_TEXT (mw->baudrate_combo));
	gtk_combo_box_set_active (GTK_COMBO_BOX (mw->baudrate_combo), mw->idb);
	gtk_grid_attach (GTK_GRID (grid), mw->baudrate_combo, 1, 1, 1, 1);
	// parity label and combo box
	parity_label = gtk_label_new ("Parity");
	gtk_grid_attach (GTK_GRID (grid), parity_label, 0, 2, 1, 1);
	mw->parity_combo = gtk_combo_box_text_new ();
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (mw->parity_combo), "NONE");
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (mw->parity_combo), "EVEN");
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (mw->parity_combo), "ODD");
	if (mw->status_parity == NONE)
		gtk_combo_box_set_active (GTK_COMBO_BOX (mw->parity_combo), 0);
	if (mw->status_parity == EVEN)
		gtk_combo_box_set_active (GTK_COMBO_BOX (mw->parity_combo), 1);
	if (mw->status_parity == ODD)
		gtk_combo_box_set_active (GTK_COMBO_BOX (mw->parity_combo), 2);
	gtk_grid_attach (GTK_GRID (grid), mw->parity_combo, 1, 2, 1, 1);
	// label and spin buttons for the # of data bits
	GtkAdjustment *databits_adjustment = gtk_adjustment_new (8, 5, 8, 1, 1, 0);
	databits_label = gtk_label_new ("# of Data Bits");
	gtk_grid_attach (GTK_GRID (grid), databits_label, 2, 0, 1, 1);
	mw->databits_spinb = gtk_spin_button_new (databits_adjustment, 1, 0);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (mw->databits_spinb),
				   mw->status_databits);
	gtk_grid_attach (GTK_GRID (grid), mw->databits_spinb, 3, 0, 2, 1);
	// label and spin button for the # of stop bits
	GtkAdjustment *stopbits_ajustment = gtk_adjustment_new (1, 1, 2, 1, 1, 0);
	stopbits_label = gtk_label_new ("# of Stop Bits");
	gtk_grid_attach (GTK_GRID (grid), stopbits_label, 2, 1, 1, 1);
	mw->stopbits_spinb = gtk_spin_button_new (stopbits_ajustment, 1, 0);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (mw->stopbits_spinb),
				   mw->status_stopbits);
	gtk_grid_attach (GTK_GRID (grid), mw->stopbits_spinb, 3, 1, 2, 1);
	// label and checkboxes for HW and SW handshake
	hs_label = gtk_label_new ("Handshake");
	gtk_grid_attach (GTK_GRID (grid), hs_label, 2, 2, 1, 1);
	// HW handshake
	mw->hw_check = gtk_check_button_new_with_label ("HW");
	if (mw->status_hwcheck == HWCHECKON)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (mw->hw_check), TRUE);
	if (mw->status_hwcheck == HWCHECKOFF)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (mw->hw_check), FALSE);
	gtk_grid_attach (GTK_GRID (grid), mw->hw_check, 3, 2, 1, 1);
	// SW handshake
	mw->sw_check = gtk_check_button_new_with_label ("SW");
	if (mw->status_swcheck == SWCHECKON)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (mw->sw_check), TRUE);
	if (mw->status_swcheck == SWCHECKOFF)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (mw->sw_check), FALSE);
	gtk_grid_attach (GTK_GRID (grid), mw->sw_check, 4, 2, 1, 1);
	// label and checkboxes for HW and SW handshake
	hs_label = gtk_label_new ("SafeMode");
	gtk_grid_attach (GTK_GRID (grid), hs_label, 2, 3, 1, 1);
	// SafeMode
	mw->sm_check = gtk_check_button_new_with_label ("SM");
	if (mw->status_smcheck == SMCHECKON)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (mw->sm_check), TRUE);
	if (mw->status_smcheck == SMCHECKOFF)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (mw->sm_check), FALSE);
	gtk_grid_attach (GTK_GRID (grid), mw->sm_check, 3, 3, 1, 1);
	gtk_widget_set_tooltip_text(mw->sm_check, "Sets timeout of 25.5s for read()\nATTENTION: Errors can occur when disabled!");

	// finally show the dialog and connect a callback
	gtk_widget_show_all (mw->dialog);
	g_signal_connect (GTK_DIALOG (mw->dialog), "response",
			  G_CALLBACK (on_response), mw);
}

/* construct the elements for the device combo box
 */
static void
create_device_list (GtkComboBoxText *combo)
{
	gchar devstr[20];

	for (gint i = 0; i < sizeof (dev) / 15 * sizeof (gchar); i++) {
		g_sprintf (devstr, "%s", dev[i]);
		gtk_combo_box_text_append_text (combo, devstr);
	}
}

/* construct the elements for the baudrate combo box
 */
static void
create_baudrate_list (GtkComboBoxText *combo)
{
	gchar brstr[10];

	for (gint i = 0; i < sizeof (br) / sizeof (gint); i++) {
		g_sprintf (brstr, "%d", br[i]);
		gtk_combo_box_text_append_text (combo, brstr);
	}
}

/* Set the new status stored in the structure by reading the settings from
 * the dialog.
 */
static void
set_new_status (gpointer user_data)
{
	gchar *parity_value;
	gchar *baudrate_value;
	gchar *endptr;
  widgets *mw = (widgets *) user_data;

	// read the selected device from the DEVICE combo box
	mw->status_device = gtk_combo_box_text_get_active_text (
				    GTK_COMBO_BOX_TEXT (mw->device_combo));
	// iterate through our devices to determine the selected index
	for (gint i = 0; i < sizeof (dev) / DEVLEN * sizeof (gchar); i++) {
		if (g_strcmp0 (mw->status_device, dev[i]) == 0)
			mw->idd = i;
	}
	// read the selected baudrate from the BAUDRATE combo box
	baudrate_value = gtk_combo_box_text_get_active_text (
				 GTK_COMBO_BOX_TEXT (mw->baudrate_combo));
	mw->status_baudrate = g_ascii_strtoll (baudrate_value, &endptr, 10);
	// iterate through our baudrate options to determine the selected index
	for (gint i = 0; i < sizeof (br) / sizeof (gint); i++) {
		if (mw->status_baudrate == br[i])
			mw->idb = i;
	}
	// read the selected partity from the PARITY combo box
	parity_value = gtk_combo_box_text_get_active_text (
			       GTK_COMBO_BOX_TEXT (mw->parity_combo));
	if (g_strcmp0 (parity_value, "ODD") == 0)
		mw->status_parity = ODD;
	if (g_strcmp0 (parity_value, "EVEN") == 0)
		mw->status_parity = EVEN;
	if (g_strcmp0 (parity_value, "NONE") == 0)
		mw->status_parity = NONE;
	// read the # of data bits from the spin button
	mw->status_databits = gtk_spin_button_get_value_as_int (
				      GTK_SPIN_BUTTON (mw->databits_spinb));
	// read the # of stop bits from the spin button
	mw->status_stopbits = gtk_spin_button_get_value_as_int (
				      GTK_SPIN_BUTTON (mw->stopbits_spinb));
	// read the HW checkbutton
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (mw->hw_check)))
		mw->status_hwcheck = HWCHECKON;
	else
		mw->status_hwcheck = HWCHECKOFF;
	// read the SW checkbutton
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (mw->sw_check)))
		mw->status_swcheck = SWCHECKON;
	else
		mw->status_swcheck = SWCHECKOFF;
	// read the SM checkbutton
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (mw->sm_check)))
		mw->status_smcheck = SMCHECKON;
	else
		mw->status_smcheck = SMCHECKOFF;
}

/* based on the new settings construct a string to be displayed on a label in
 * the parent window
 */
static void
config_uart (gpointer user_data)
{
  widgets *mw = (widgets *) user_data;

  printf("device %s\n",mw->status_device);
  printf("baudrate %d\n",mw->status_baudrate);
  printf("parity %d\n",mw->status_parity);
  printf("databits %d\n",mw->status_databits);
  printf("stopbits %d\n",mw->status_stopbits);
  printf("hwcheck %d\n",mw->status_hwcheck);
  printf("swcheck %d\n",mw->status_swcheck);
	printf("smcheck %d\n",mw->status_smcheck);

  close(mw->fd);
  if (mw->fd < 0) {
      printf("Error closing device %s\n", strerror(errno));
  }
  mw->fd=0;
  mw->fd = open(mw->status_device, O_RDWR | O_NOCTTY | O_SYNC);
  if (mw->fd < 0) {
      printf("Error opening %s: %s\n", mw->status_device, strerror(errno));
  }
  set_interface_attribs (mw->fd, mw->status_baudrate, mw->status_parity, mw->status_databits, mw->status_stopbits, mw->status_hwcheck, mw->status_swcheck,mw->status_smcheck);
}

/** EOF */
