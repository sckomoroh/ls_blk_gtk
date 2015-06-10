#include <iostream>
#include <gtk/gtk.h>

#include "main-window.h"
#include "partitions-provider.h"

int main(int argc, char *argv[])
{
	PartitionsProvider provider;
	provider.init ();
	
	gtk_init (&argc, &argv);

	MainWindow mainWindow;
	mainWindow.showWindow ();
	mainWindow.run ();
	
	return 0;
}
