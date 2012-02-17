/*(C) copyright 2008, Steven Snyder, All Rights Reserved

Steven T. Snyder, <stsnyder@ucla.edu> http://www.steventsnyder.com

LICENSING INFORMATION:
 This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include "linuxburn.h"
#include "linuxburn_gui.h"


extern linuxburn_settings burn_settings;

// linuxburn GUI mode function definitions
// ===================================================================


// runs the linuxburn GTK+ graphical user interface
int linuxburn_gui(int argc, char* argv[])
{
	//				GTK+ GUI APPLICATION MODE 
	// ================================================================
	GtkWidget *window;
	GtkWidget *button;
	GtkWidget *vbox_main;
	GtkWidget *hbox_settings;
	GtkWidget *vbox_settings_left;
	GtkWidget *bbox_cmds;
	GtkWidget *frame_cmds;
	GtkWidget *lfilew; // load file selection dialogue 
	GtkWidget *sfilew; // save file selection dialogue 
	char* binfilename;
	
	gtk_init(&argc, &argv);
	
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW (window), "Linuxburn - GUI Mode");
	g_signal_connect(G_OBJECT(window),"delete_event", G_CALLBACK (linuxburn_gui_quit), NULL);
	g_signal_connect(G_OBJECT(window),"destroy", G_CALLBACK (gtk_main_quit), NULL);

	// main vbox organizes settings seperately from progress bar and command buttons
	vbox_main = gtk_vbox_new(FALSE,0);
	gtk_widget_show(vbox_main);	
	gtk_container_add(GTK_CONTAINER (window), vbox_main);
	
	// hbox to seperate chip selection and bin load/save from addressing information entry forms
	hbox_settings = gtk_hbox_new(FALSE,0);
	gtk_widget_show(hbox_settings);
	
	// vbox for dropdown box and buttons on left side of settings area 
	vbox_settings_left = gtk_vbox_new(FALSE, 0);

	// Load File selection dialogue
	// =======================
	lfilew = gtk_file_selection_new("Load File");
	
	g_signal_connect(G_OBJECT (lfilew), "destroy", G_CALLBACK (linuxburn_widgethide), (gpointer) lfilew);
	g_signal_connect(G_OBJECT (GTK_FILE_SELECTION (lfilew)->ok_button), "clicked", 
										G_CALLBACK (linuxburn_gui_loadbin), (gpointer) lfilew);
	g_signal_connect(G_OBJECT (GTK_FILE_SELECTION (lfilew)->ok_button), "clicked", 
										G_CALLBACK (linuxburn_widgethide), (gpointer) lfilew);
	g_signal_connect(G_OBJECT (GTK_FILE_SELECTION (lfilew)->cancel_button), "clicked",
										G_CALLBACK (linuxburn_widgethide), (gpointer) lfilew);
										
	// Save File selection dialogue
	// =======================
	sfilew = gtk_file_selection_new("Save File");
	
	g_signal_connect(G_OBJECT (sfilew), "destroy", G_CALLBACK (linuxburn_widgethide), (gpointer) sfilew);
	g_signal_connect(G_OBJECT (GTK_FILE_SELECTION (sfilew)->ok_button), "clicked", 
										G_CALLBACK (linuxburn_gui_savebin), (gpointer) sfilew);
	g_signal_connect(G_OBJECT (GTK_FILE_SELECTION (sfilew)->ok_button), "clicked", 
										G_CALLBACK (linuxburn_widgethide), (gpointer) sfilew);
	g_signal_connect(G_OBJECT (GTK_FILE_SELECTION (sfilew)->cancel_button), "clicked",
										G_CALLBACK (linuxburn_widgethide), (gpointer) sfilew);
	
	
	// Commands (burn, verify, read, exit)
	// ===================================
		
	// button box for commands
	bbox_cmds = gtk_hbutton_box_new ();
	gtk_button_box_set_layout(GTK_BUTTON_BOX (bbox_cmds), GTK_BUTTONBOX_SPREAD);
	gtk_widget_show(bbox_cmds);	
	
	// frame for commands
	frame_cmds = gtk_frame_new("Commands");	
	gtk_container_add(GTK_CONTAINER (frame_cmds), bbox_cmds);
	gtk_widget_show(frame_cmds);
	
	// burn button
	button = gtk_button_new_with_label("Burn");
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(linuxburn_gui_burn), (gpointer) &burn_settings);
	gtk_container_add(GTK_CONTAINER (bbox_cmds), button);
	gtk_widget_show(button);	
	
	// verify button
	button = gtk_button_new_with_label("Verify");
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(linuxburn_gui_verify), (gpointer) &burn_settings);
	gtk_container_add(GTK_CONTAINER (bbox_cmds), button);
	gtk_widget_show(button);	
	
	// read button
	button = gtk_button_new_with_label("Read");
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(linuxburn_gui_read), (gpointer) &burn_settings);
	gtk_container_add(GTK_CONTAINER (bbox_cmds), button);
	gtk_widget_show(button);	
	
	// exit button
	button = gtk_button_new_with_label("Exit");
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(linuxburn_gui_quit), NULL);
	gtk_container_add(GTK_CONTAINER (bbox_cmds), button);
	gtk_widget_show(button);
	
	gtk_box_pack_end (GTK_BOX (vbox_main), frame_cmds, FALSE, FALSE, 0);
	



	// Settings (load .bin, save .bin, select chip, start and end addresses, etc.)
	// ===========================================================================
	gtk_box_pack_start (GTK_BOX (vbox_main), hbox_settings, FALSE, FALSE, 0);
	gtk_widget_show(hbox_settings);
	
	// left side of settings area 
	gtk_box_pack_start (GTK_BOX (hbox_settings), vbox_settings_left, FALSE, FALSE, 0);
	gtk_widget_show(vbox_settings_left);
	
	// load .bin file button
	button = gtk_button_new_with_label("Load .bin to buffer");
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(linuxburn_widgetshow), (gpointer) lfilew);
	gtk_box_pack_start (GTK_BOX (vbox_settings_left), button, FALSE, FALSE, 0);
	gtk_widget_show(button);
	
	// save .bin file button
	button = gtk_button_new_with_label("Save buffer to .bin");
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(linuxburn_widgetshow), (gpointer) sfilew);
	gtk_box_pack_start (GTK_BOX (vbox_settings_left), button, FALSE, FALSE, 0);
	gtk_widget_show(button);
	
	

	
	gtk_widget_show(window);
	
	gtk_main();
	
	return 0;
} 

// calls gtk_widget_show on the widget specified in the data argument
static void linuxburn_widgetshow(GtkWidget *widget, gpointer data)
{
	gtk_widget_show(GTK_WIDGET (data));
}

// calls gtk_widget_hide on the widget specified in the data argument
static void linuxburn_widgethide(GtkWidget *widget, gpointer data)
{
	gtk_widget_hide(GTK_WIDGET (data));
}

// this function is called as the result of the "ok" button being
// selected in the load bin file selection dialogue. it loads the file 
// into the burn buffer, up to burn_settings.chip_size bytes max. 
static void linuxburn_gui_loadbin( GtkWidget *widget, GtkFileSelection *fs){
	int res;
	unsigned int total_bytes=0;
	unsigned char inputbuffer[256]; // XXX need to dynamically allocate this
	const char* binfilename = gtk_file_selection_get_filename(fs);
	
	// Open the .bin file for reading
	// ------------------------------
	burn_settings.fbinfile = open(binfilename,O_RDONLY);
	if (burn_settings.fbinfile == -1)
	{
		printf("Unable to open/create %s for reading.\n",binfilename);
		return;	}
	
	while(1) // read from file until no more bytes left to read.
	{
		res = read(burn_settings.fbinfile,inputbuffer,256);
		if (res==0 || total_bytes>= burn_settings.chip_size) // EOF, or bin buffer full 
		{
			break;
		}
		else if (res<0)
		{
			printf("Error reading from file.\n");
			close (burn_settings.fbinfile);
			return;
		}
		else{
			// if the entire input buffer will fit in the bin buffer, copy it all and increment total_bytes 
			if (total_bytes+res <= burn_settings.chip_size){
				memcpy(burn_settings.binbuffer+total_bytes, inputbuffer, res);
				total_bytes+=res;
			}
			// else if the input buffer wont all fit, copy all that will fit and increment total_bytes to 
			// max size and break. 
			else if (total_bytes+res > burn_settings.chip_size){
				memcpy( burn_settings.binbuffer+total_bytes,
						inputbuffer,
						burn_settings.chip_size-total_bytes);
				total_bytes = burn_settings.chip_size;
				break;
			}
		}
			
		if (total_bytes == 0) break; // if end of file
	}
	printf("Read %d bytes from %s into buffer\n", total_bytes, binfilename);
	
	// if the file doesnt fill up the bin buffer, fill the rest of the buffer with zeros
	if (total_bytes<burn_settings.chip_size)
	{
		for (; total_bytes<burn_settings.chip_size; total_bytes++)
			burn_settings.binbuffer[total_bytes] = 0;
	}
	//burn_settings.chip_size = total_bytes;	
	
	close(burn_settings.fbinfile);
	return;
}

// this function is called as the result of the "ok" button being
// selected in the save bin file selection dialogue. it saves 
// the current buffer into the selected file.
static void linuxburn_gui_savebin( GtkWidget *widget, GtkFileSelection *fs){
	int res;
	unsigned int bytes_written=0;
	unsigned char outputbuffer[256]; // XXX need to dynamically allocate this
	const char* binfilename = gtk_file_selection_get_filename(fs);
	
	// Open the .bin file for write, create if it doesnt exist
	// ------------------------------------------------------------
	burn_settings.fbinfile = open(binfilename,O_RDWR | O_CREAT, 0666);
	if (burn_settings.fbinfile == -1)
	{
		printf("Unable to open/create %s for writing.\n",binfilename);
		return;	}
		
	do	{
		res = write ( burn_settings.fbinfile,
							burn_settings.binbuffer+bytes_written,
							burn_settings.chip_size-bytes_written);
		bytes_written += res;
		
		
	} while (res>0 && bytes_written<burn_settings.chip_size);
	
	if (res < 0)
		printf("%d couldn't write complete buffer to %s.\n",res,binfilename);
	else	printf("%d byte buffer written to %s.\n",res,binfilename);
	close(burn_settings.fbinfile);
}

// called on delete_event, closes the connection (if there is one)
// and call gtk_main_quit(). if a transfer is currently in progress,
// should pop up a dialogue to confirm quitting.
static gboolean linuxburn_gui_quit( GtkWidget *widget, gpointer data)
{
	gtk_main_quit();
	return FALSE;
}

// burn the buffer to the chip starting at start_addr on the chip
// and stopping at the block starting at an address higher than end_addr
// XXX need to avoid going over the end of binbuffer!!
static void linuxburn_gui_burn( GtkWidget *widget, gpointer data)
{
	int res;
	unsigned int burned_bytes = 0;
	unsigned int max_bytes = burn_settings.end_addr - burn_settings.start_addr;
	unsigned int addr = burn_settings.start_addr;	
	unsigned int block_size = burn_settings.burn_block_size;

	while (burned_bytes < max_bytes)
	{
		res = writeblock(addr, burn_settings.binbuffer+burned_bytes, block_size);
		if (res<0 || res!=block_size)
		{
			printf("Block burn failure.\n");
			return;
		}
		burned_bytes+=block_size;
		addr+=block_size;
	}
	printf("Burned %d bytes to chip.\n", burned_bytes);
	return;
}

static void linuxburn_gui_verify( GtkWidget *widget, gpointer data)
{
	unsigned char* cmpbuffer;
	int res;
	unsigned int total_bytes=0, block_attempts = 0;
	int addr = 0;
	unsigned int block_size = burn_settings.read_block_size;

	cmpbuffer = g_malloc(burn_settings.chip_size);

	while(addr<burn_settings.end_addr)
	{
		res = readblock(addr,cmpbuffer+total_bytes,block_size);
		
		/* if entire block not read, or error occured */
		if ((unsigned)res != block_size)
		{
			/* retry if not at max # of attempts for block */ 
			if (block_attempts < MAX_BLOCK_ATTEMPTS)
			{
				printf("Error receiving block. Retrying.\n");
				block_attempts++;
				continue;
			}
			else
			{
				printf("Failed to receive block after %d retries. Aborting.\n",
						MAX_BLOCK_ATTEMPTS);
				g_free(cmpbuffer);
				return;
			}
		}
		
		total_bytes+=res; // increment total # of bytes by amount read in last read
		addr+=0x100; // increment start address by 256 bytes
		block_attempts = 0; // reset number of attempts for the block

	}
	res = strncmp((char*)cmpbuffer,(char*)burn_settings.binbuffer,total_bytes);
	if (res!=0)
	{
		printf("Bytes read from chip: %d.\n",total_bytes);
		printf("Bytes in buffer: %d.\n", burn_settings.chip_size);
		printf("ERROR: *** Chip does not match first %d of buffer. ***\n",total_bytes);
	}
	else 
	{
		// XXX somehow control reaches here sometimes when the chip does
		// NOT match the buffer. need to do some testing.
		printf("Bytes read from chip: %d.\n",total_bytes);
		printf("Bytes in buffer: %d.\n", burn_settings.chip_size);
		printf("Chip matches first %d of buffer.\n",total_bytes);
	}
	
	g_free(cmpbuffer);
																				
	return;
}

static void linuxburn_gui_read( GtkWidget *widget, gpointer data)
{
	unsigned char* readbuffer;
	int res;
	unsigned int total_bytes=0, block_attempts = 1;
	int addr = 0;
	unsigned int block_size = burn_settings.read_block_size;

	readbuffer = g_malloc(block_size);

	while(addr<burn_settings.end_addr)
	{
		res = readblock(addr,readbuffer,block_size);
		
		/* if entire block not read, or error occured */
		if ((unsigned)res != block_size)
		{
			/* retry if not at max # of attempts for block */ 
			if (block_attempts < MAX_BLOCK_ATTEMPTS)
			{
				printf("Error receiving block. Retrying.\n");
				block_attempts++;
				continue;
			}
			else
			{
				quick_alert("Failed to receive block after multiple retries. Aborting.\n");
				g_free(readbuffer);
				return;
			}
		}
		// copy the data into the bin buffer		
		memcpy(burn_settings.binbuffer+total_bytes,readbuffer,block_size); 
		burn_settings.chip_size=total_bytes;
		total_bytes+=res; // increment total # of bytes by amount read in last read
		addr+=0x100; // increment address by 256 bytes
		block_attempts = 0; // reset number of attempts for the block

	}
	burn_settings.chip_size=total_bytes;
	printf("Read %d bytes from chip.\n",total_bytes);

	g_free(readbuffer);

	return;
}

// ==================================
//  Alert popup
// ==================================

// pops up an alert with message in the body and an ok button.
// the alert will block access to other windows until the button is clicked.
// background activity will continue.
void quick_alert(gchar *message)
{
		GtkWidget *label, *content_area;
		GtkWidget *dialog = gtk_dialog_new_with_buttons("Alert",NULL,
						GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT |GTK_DIALOG_NO_SEPARATOR,
						GTK_STOCK_OK, GTK_RESPONSE_DELETE_EVENT, NULL);
		g_signal_connect_swapped(dialog, "response", G_CALLBACK (gtk_widget_destroy), dialog);
		content_area = GTK_DIALOG(dialog)->vbox;
		label = gtk_label_new(message);
		gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_CENTER);
		gtk_box_pack_start(GTK_BOX(content_area),label,FALSE,FALSE,10);
		gtk_widget_show_all(dialog);
}
