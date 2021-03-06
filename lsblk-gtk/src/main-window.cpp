#include "main-window.h"



MainWindow::MainWindow()
{
	initWindow ();
}

void MainWindow::showWindow ()
{
	gtk_widget_show_all (m_pMainWidget);
}

void MainWindow::run()
{
	gtk_main();
}

void MainWindow::initWindow()
{
	m_pMainWidget = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW(m_pMainWidget), "LSBLK GTK");
	gtk_container_set_border_width (GTK_CONTAINER(m_pMainWidget), 10);

	// Init vertical box
	GtkWidget* pVBox;
	pVBox = gtk_box_new (GTK_ORIENTATION_VERTICAL, FALSE);
	gtk_box_set_spacing (GTK_BOX(pVBox), 5);
	gtk_container_add (GTK_CONTAINER(m_pMainWidget), pVBox);

	// Init refresh button
	GtkWidget* pRefreshButton;
	pRefreshButton = gtk_button_new_with_label ("Refresh");
	gtk_widget_set_halign (GTK_WIDGET(pRefreshButton), GTK_ALIGN_END);
	gtk_box_pack_start (GTK_BOX(pVBox),
	                    GTK_WIDGET(pRefreshButton),
	                    FALSE,
	                    FALSE,
	                    0);

	initTreeWidget(pVBox);

	g_signal_connect(m_pMainWidget,
                 "destroy",
                 G_CALLBACK(MainWindow::closeWindowHandler),
                 NULL);

	g_signal_connect(pRefreshButton,
             "clicked",
             G_CALLBACK(MainWindow::refreshClickHandler),
             this);

}

void MainWindow::initTreeWidget (GtkWidget* pLayout)
{
	createAndFillStore();
	
	GtkWidget* pTreeView;
	GtkWidget* pScrolledWindow;
		
	GtkTreeViewColumn*  pColumnDevName;
	GtkTreeViewColumn*  pColumnFsType;
	GtkTreeViewColumn*  pColumnMountPoint;
	GtkCellRenderer*	pDevNameRenderer;
	GtkCellRenderer*	pFsTypeRenderer;
	GtkCellRenderer*	pMountPointRenderer;

	pTreeView = gtk_tree_view_new_with_model (GTK_TREE_MODEL(m_pTreeStore));
	
	pColumnDevName = gtk_tree_view_column_new ();
    pColumnFsType = gtk_tree_view_column_new ();
    pColumnMountPoint = gtk_tree_view_column_new ();

	pDevNameRenderer = gtk_cell_renderer_text_new ();
	pFsTypeRenderer = gtk_cell_renderer_text_new ();
	pMountPointRenderer = gtk_cell_renderer_text_new ();

	gtk_tree_view_column_set_resizable (pColumnDevName, TRUE);
	gtk_tree_view_column_set_resizable (pColumnFsType, TRUE);
	gtk_tree_view_column_set_resizable (pColumnMountPoint, TRUE);

	gtk_tree_view_column_set_title (GTK_TREE_VIEW_COLUMN(pColumnDevName), "Device name");
	gtk_tree_view_append_column (GTK_TREE_VIEW(pTreeView), 
	                             GTK_TREE_VIEW_COLUMN(pColumnDevName));
  
	gtk_tree_view_column_set_title (GTK_TREE_VIEW_COLUMN(pColumnFsType), "FS type");
	gtk_tree_view_append_column (GTK_TREE_VIEW(pTreeView), 
	                             GTK_TREE_VIEW_COLUMN(pColumnFsType));

	gtk_tree_view_column_set_title (GTK_TREE_VIEW_COLUMN(pColumnMountPoint), "Mount point");
	gtk_tree_view_append_column (GTK_TREE_VIEW(pTreeView), 
	                             GTK_TREE_VIEW_COLUMN(pColumnMountPoint));

	gtk_tree_view_column_pack_start (pColumnDevName, pDevNameRenderer, TRUE);
	gtk_tree_view_column_pack_start (pColumnFsType, pFsTypeRenderer, TRUE);
	gtk_tree_view_column_pack_start (pColumnMountPoint, pMountPointRenderer, TRUE);

	gtk_tree_view_column_add_attribute (pColumnDevName,
	                                    pDevNameRenderer,
	                                    "text",
	                                    0);

	gtk_tree_view_column_add_attribute (pColumnFsType,
	                                    pFsTypeRenderer,
	                                    "text",
	                                    1);

	gtk_tree_view_column_add_attribute (pColumnMountPoint,
	                                    pMountPointRenderer,
	                                    "text",
	                                    2);

	gtk_widget_set_vexpand_set (GTK_WIDGET(pTreeView), TRUE);
	gtk_widget_set_hexpand_set (GTK_WIDGET(pTreeView), TRUE);

	gtk_widget_set_vexpand (GTK_WIDGET(pTreeView), TRUE);
	gtk_widget_set_hexpand (GTK_WIDGET(pTreeView), TRUE);

	pScrolledWindow = gtk_scrolled_window_new (NULL, NULL);

	gtk_container_add (GTK_CONTAINER(pScrolledWindow), pTreeView);
	gtk_container_add (GTK_CONTAINER(pLayout), pScrolledWindow);
}

void MainWindow::createAndFillStore()
{
	GtkTreeIter   iter;
	int i;

	m_pTreeStore = gtk_tree_store_new (3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

	/*PartitionsProvider partProvider;
	partProvider.init ();

	std::list<PtrPartitionInfo> partitions = partProvider.partitions ();
	addTreeNodes(partitions, NULL);*/
}

void MainWindow::addTreeNodes(std::list<PtrPartitionInfo> partitions, GtkTreeIter* parent)
{
	GtkTreeIter   treeIter;
	std::list<PtrPartitionInfo>::iterator iter = partitions.begin();
	for (; iter != partitions.end(); iter++)
	{
		PtrPartitionInfo data = *iter;

		std::string deviceNameFull = data->deviceName;
		if (data->mappedTo.length() > 0)
		{
			deviceNameFull += "(" + data->mappedTo + ")"; 
		}
		
		gtk_tree_store_append (m_pTreeStore, &treeIter, parent);
		gtk_tree_store_set (m_pTreeStore,
		                    &treeIter,
		                    0, deviceNameFull.c_str(),
		                    1, data->fileSystemType.c_str(),
		                    2, data->mountPoint.c_str(),
		                    -1);

		if (data->holders.size() > 0)
		{
			addTreeNodes (data->holders, &treeIter);
		}
	}
}

void MainWindow::closeWindowHandler ()
{
	gtk_main_quit ();
}

void MainWindow::refreshClickHandler(GtkWidget* sender, gpointer* data)
{
	MainWindow* pThis = (MainWindow*)data;
	pThis->refreshAction ();
}

void MainWindow::refreshAction ()
{
	gtk_tree_store_clear (m_pTreeStore);

	PartitionsProvider partProvider;
	partProvider.init ();

	std::list<PtrPartitionInfo> partitions = partProvider.partitions ();
	addTreeNodes(partitions, NULL);
}