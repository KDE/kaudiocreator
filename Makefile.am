INCLUDES = $(LAME_INCS) $(CDPARANOIA_INCS) -I$(top_srcdir) -I$(top_srcdir)/libkcddb $(all_includes)

bin_PROGRAMS = kaudiocreator

kaudiocreator_SOURCES = main.cpp kaudiocreator.cpp encoder.cpp encoderconfigimp.cpp  job.cpp  jobqueimp.cpp  ripper.cpp  tracksimp.cpp encoderedit.ui jobque.ui  tracks.ui infodialog.ui wizard.ui \
general.ui cdconfig.ui ripconfig.ui encoderoutput.ui encoderconfig.ui prefs.kcfgc encoder_prefs.kcfgc

EXTRA_DIST = encoder.h kaudiocreator.h kaudiocreator.desktop encoderconfigimp.h  jobqueimp.h  ripper.h  tracksimp.h 

kaudiocreator_LDFLAGS = $(all_libraries) $(KDE_RPATH) -lkutils

METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/kaudiocreator.pot

AM_CPPFLAGS = -static -Wall

kaudiocreator_LDADD = $(LIB_KIO) $(top_builddir)/libkcddb/libkcddb.la $(top_builddir)/kscd/libwm/libworkman.la

updatedir = $(kde_datadir)/kconf_update
update_DATA = kaudiocreator-libkcddb.upd

KDE_ICON = AUTO

SUBDIRS = icons

xdg_apps_DATA = kaudiocreator.desktop
kde_kcfg_DATA = kaudiocreator.kcfg kaudiocreator_encoders.kcfg

appdata_DATA = kaudiocreatorui.rc eventsrc
appdatadir = $(kde_datadir)/kaudiocreator

conf_DATA = kaudiocreatorrc
confdir = $(kde_confdir)
