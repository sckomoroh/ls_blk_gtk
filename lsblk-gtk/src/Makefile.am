## Process this file with automake to produce Makefile.in

## Created by Anjuta


uidir = $(pkgdatadir)/ui
ui_DATA = lsblk_gtk.ui


AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\" \
	$(LSBLK_GTK_CFLAGS)

AM_CFLAGS =\
	 -Wall\
	 -g

bin_PROGRAMS = lsblk_gtk


lsblk_gtk_SOURCES =  \
	main.cpp \
	main-window.cpp \
	main-window.h \
	partitions-provider.cpp \
	partitions-provider.h

lsblk_gtk_LDFLAGS = \
	-Wl,--export-dynamic \
	-ludev

lsblk_gtk_LDADD = $(LSBLK_GTK_LIBS)




EXTRA_DIST = $(ui_DATA)

# Remove ui directory on uninstall
uninstall-local:
	-rm -r $(uidir)
	-rm -r $(pkgdatadir)

