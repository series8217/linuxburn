#ifndef LINUXBURN_GUI_INCLUDED
#define LINUXBURN_GUI_INCLUDED

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

#include <gtk/gtk.h>

//  linuxburn GUI function prototypes 
// ==================================================================

int linuxburn_gui(int argc, char* argv[]);
// runs the linuxburn GTK+ graphical user interface 

static gboolean linuxburn_gui_quit( GtkWidget *widget, gpointer data);
// called on delete_event, closes the connection (if there is one)
// and call gtk_main_quit(). if a transfer is currently in progress,
// should pop up a dialogue to confirm quitting.

static void linuxburn_gui_burn( GtkWidget *widget, gpointer data);
static void linuxburn_gui_verify( GtkWidget *widget, gpointer data);
static void linuxburn_gui_read( GtkWidget *widget, gpointer data);
static void linuxburn_gui_loadbin( GtkWidget *widget, GtkFileSelection *fs);
static void linuxburn_gui_savebin( GtkWidget *widget, GtkFileSelection *fs);

// calls gtk_widget_show on the widget specified in the data argument 
static void linuxburn_widgetshow(GtkWidget *widget, gpointer data);

// calls gtk_widget_hide on the widget specified in the data argument 
static void linuxburn_widgethide(GtkWidget *widget, gpointer data);



void quick_alert(gchar *message);
// pops up an alert with message in the body and an ok button.
// the alert will block access to other windows until the button is clicked.
// background activity will continue.
#endif
