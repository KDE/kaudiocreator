INCLUDES = $(LAME_INCS) $(CDPARANOIA_INCS) -I$(top_srcdir)/libkcddb $(all_includes)

bin_PROGRAMS = kaudiocreator

kaudiocreator_SOURCES = options.cpp main.cpp kaudiocreator.cpp cdconfigimp.cpp  encoderconfigimp.cpp  job.cpp  queconfigimp.cpp  ripconfigimp.cpp  tracksconfigimp.cpp  cdconfig.ui  encoderconfig.ui  queconfig.ui  ripconfig.ui  tracksconfig.ui id3tagdialog.ui wizard.ui

EXTRA_DIST = options.h kaudiocreator.h kaudiocreator.desktop cdconfigimp.h  encoderconfigimp.h  queconfigimp.h  ripconfigimp.h  tracksconfigimp.h 

kaudiocreator_LDFLAGS = $(all_libraries) $(KDE_RPATH)

METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/kaudiocreator.pot

AM_CPPFLAGS = -static -Wall

kaudiocreator_LDADD = $(LIB_KIO) $(CDPARANOIA_LIBS) $(LAME_LIBS) -L$(top_builddir)/libkcddb -lkcddb

updatedir = $(kde_datadir)/kconf_update
update_DATA = kaudiocreator-libkcddb.upd

KDE_ICON = AUTO

SUBDIRS = icons

xdg_apps_DATA = kaudiocreator.desktop

appdata_DATA = kaudiocreatorui.rc
appdatadir = $(kde_datadir)/kaudiocreator
