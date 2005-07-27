INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/libkcddb \
		   -I$(top_builddir)/libkcddb -I$(top_srcdir)/kscd $(all_includes)

bin_PROGRAMS = kaudiocreator

kaudiocreator_SOURCES = main.cpp kaudiocreator.cpp encoder.cpp \
	encoderconfigimp.cpp job.cpp jobqueimp.cpp ripper.cpp tracksimp.cpp \
	encoderedit.ui jobque.ui  tracks.ui infodialog.ui wizard.ui general.ui \
	cdconfig.ui ripconfig.ui encoderoutput.ui encoderconfig.ui encodefile.ui \
	prefs.kcfgc encoder_prefs.kcfgc encodefileimp.cpp

EXTRA_DIST = encoder.h kaudiocreator.h kaudiocreator.desktop encoderconfigimp.h \
	obqueimp.h ripper.h tracksimp.h encodefileimp.h

kaudiocreator_LDFLAGS = $(all_libraries) $(KDE_RPATH) -lkutils

METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/kaudiocreator.pot

kaudiocreator_LDADD = $(LIB_KIO) $(top_builddir)/libkcddb/libkcddb.la \
	$(top_builddir)/kscd/libkcompactdisc.la

KDE_ICON = AUTO

SUBDIRS = icons

xdg_apps_DATA = kaudiocreator.desktop
kde_kcfg_DATA = kaudiocreator.kcfg kaudiocreator_encoders.kcfg

appdata_DATA = kaudiocreatorui.rc eventsrc
appdatadir = $(kde_datadir)/kaudiocreator

servicedata_DATA = audiocd_extract.desktop
servicedatadir = $(kde_datadir)/konqueror/servicemenus

updatedir = $(kde_datadir)/kconf_update
update_DATA = kaudiocreator-meta.upd kaudiocreator-libkcddb.upd
update_SCRIPTS = upgrade-kaudiocreator-metadata.sh

encoderconfig.o: wizard.h
kaudiocreator.o: ../libkcddb/configbase.h
tracksimp.o: ../libkcddb/configbase.h ../libkcddb/cdinfodialogbase.h
