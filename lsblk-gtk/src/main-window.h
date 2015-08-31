#ifndef _MAIN_WINDOW_H_
#define _MAIN_WINDOW_H_

#include <gtk/gtk.h>

#include "partitions-provider.h"

class MainWindow
{
private:
	GtkWidget*		m_pMainWidget;
	GtkTreeStore*   m_pTreeStore;
	
public:
	MainWindow();
	
	void showWindow();
	void run();
	
protected:

private:
	void initWindow();
	void initTreeWidget(GtkWidget* pLayout);
	void createAndFillStore();

	void addTreeNodes(std::list<PtrPartitionInfo> partitions, GtkTreeIter* parent);
	
	// Handlers
	static void closeWindowHandler ();
	static void refreshClickHandler(GtkWidget* sender, gpointer* data);

	// Actions
	void refreshAction();
};

#endif // _MAIN_WINDOW_H_

