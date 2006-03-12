kde4_header()

add_subdirectory( icons ) 

include_directories( ${CMAKE_SOURCE_DIR}/libkcddb ${CMAKE_SOURCE_DIR}/kscd ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kaudiocreator_SRCS 
   main.cpp 
   kaudiocreator.cpp 
   encoder.cpp 
   encoderconfigimp.cpp 
   job.cpp 
   jobqueimp.cpp 
   ripper.cpp 
   tracksimp.cpp 
   encodefileimp.cpp )

kde4_automoc(${kaudiocreator_SRCS})

kde4_add_ui_files(kaudiocreator_SRCS 
   encoderedit.ui 
   jobque.ui 
   tracks.ui 
   infodialog.ui 
   wizard.ui 
   general.ui 
   cdconfig.ui 
   ripconfig.ui 
   encoderoutput.ui 
   encoderconfig.ui 
   encodefile.ui )

kde4_add_kcfg_files(kaudiocreator_SRCS prefs.kcfgc encoder_prefs.kcfgc )

kde4_add_executable(kaudiocreator ${kaudiocreator_SRCS})

target_link_libraries(kaudiocreator  ${QT_AND_KDECORE_LIBS} kutils kio kcddb kcompactdisc )

install_targets(/bin kaudiocreator )


########### install files ###############

install_files( ${XDG_APPS_DIR} FILES kaudiocreator.desktop )
install_files( ${KCFG_INSTALL_DIR} FILES kaudiocreator.kcfg kaudiocreator_encoders.kcfg )
install_files( ${DATA_INSTALL_DIR}/konqueror/servicemenus FILES audiocd_extract.desktop )
install_files( ${DATA_INSTALL_DIR}/kaudiocreator FILES kaudiocreatorui.rc eventsrc )
install_files( ${DATA_INSTALL_DIR}/kconf_update FILES kaudiocreator-meta.upd kaudiocreator-libkcddb.upd )

kde4_install_icons( ${ICON_INSTALL_DIR}  hicolor )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/libkcddb \
#		   -I$(top_builddir)/libkcddb -I$(top_srcdir)/kscd $(all_includes)
#
#bin_PROGRAMS = kaudiocreator
#
#kaudiocreator_SOURCES = main.cpp kaudiocreator.cpp encoder.cpp \
#	encoderconfigimp.cpp job.cpp jobqueimp.cpp ripper.cpp tracksimp.cpp \
#	encoderedit.ui jobque.ui  tracks.ui infodialog.ui wizard.ui general.ui \
#	cdconfig.ui ripconfig.ui encoderoutput.ui encoderconfig.ui encodefile.ui \
#	prefs.kcfgc encoder_prefs.kcfgc encodefileimp.cpp
#
#EXTRA_DIST = encoder.h kaudiocreator.h kaudiocreator.desktop encoderconfigimp.h \
#	obqueimp.h ripper.h tracksimp.h encodefileimp.h
#
#kaudiocreator_LDFLAGS = $(all_libraries) $(KDE_RPATH) -lkutils
#
#METASOURCES = AUTO
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp *.h -o $(podir)/kaudiocreator.pot
#
#kaudiocreator_LDADD = $(LIB_KIO) $(top_builddir)/libkcddb/libkcddb.la \
#	$(top_builddir)/kscd/libkcompactdisc.la
#
#KDE_ICON = AUTO
#
#SUBDIRS = icons
#
#xdg_apps_DATA = kaudiocreator.desktop
#kde_kcfg_DATA = kaudiocreator.kcfg kaudiocreator_encoders.kcfg
#
#appdata_DATA = kaudiocreatorui.rc eventsrc
#appdatadir = $(kde_datadir)/kaudiocreator
#
#servicedata_DATA = audiocd_extract.desktop
#servicedatadir = $(kde_datadir)/konqueror/servicemenus
#
#updatedir = $(kde_datadir)/kconf_update
#update_DATA = kaudiocreator-meta.upd kaudiocreator-libkcddb.upd
#update_SCRIPTS = upgrade-kaudiocreator-metadata.sh
#
#encoderconfig.o: wizard.h
#kaudiocreator.o: ../libkcddb/configbase.h
#tracksimp.o: ../libkcddb/configbase.h ../libkcddb/cdinfodialogbase.h
