INCLUDES = $(all_includes) $(LAME_INCS) $(CDPARANOIA_INCS)

bin_PROGRAMS = kaudiocreator

kaudiocreator_SOURCES = main.cpp kaudiocreator.cpp cdconfigimp.cpp  encoderconfigimp.cpp  queconfigimp.cpp  ripconfigimp.cpp  tracksconfigimp.cpp cddb.cpp cdconfig.ui  encoderconfig.ui  queconfig.ui  ripconfig.ui  tracksconfig.ui id3tagdialog.ui

EXTRA_DIST = kaudiocreator.h kaudiocreator.desktop cdconfigimp.h  encoderconfigimp.h  queconfigimp.h  ripconfigimp.h  tracksconfigimp.h cddb.h

kaudiocreator_LDFLAGS = $(all_libraries) $(KDE_RPATH)

METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) rc.cpp $(kaudiocreator_SOURCES) -o $(podir)/kaudiocreator.pot

AM_CPPFLAGS =  -static -Wall -DDATADIR=\"$(kde_datadir)/kaudiocreator\"

kaudiocreator_LDADD = $(LIB_KIO) $(CDPARANOIA_LIBS) $(LAME_LIBS)

KDE_ICON = AUTO

SUBDIRS = icons

appsdir = $(kde_appsdir)/Multimedia
apps_DATA = kaudiocreator.desktop

data_DATA = kaudiocreatorui.rc
datadir = $(kde_datadir)/kaudiocreator

