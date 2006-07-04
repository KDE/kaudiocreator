project(kaudiocreator)

add_subdirectory( icons ) 

include_directories( 
		${CMAKE_SOURCE_DIR}/libkcddb 
		${CMAKE_BINARY_DIR}/libkcddb 
		${CMAKE_SOURCE_DIR}/kscd   
		${CMAKE_BINARY_DIR}/kscd
		)


########### next target ###############

set(kaudiocreator_SRCS ${libkcompactdisc_SRCS} ${libworkman_SRCS} 
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

kde4_add_ui_files( kaudiocreator_SRCS
   encodefile.ui )

kde4_add_ui3_files(kaudiocreator_SRCS 
   encoderedit.ui 
   jobque.ui 
   tracks.ui 
   infodialog.ui 
   wizard.ui 
   general.ui 
   cdconfig.ui 
   ripconfig.ui 
   encoderoutput.ui 
   encoderconfig.ui )

kde4_add_kcfg_files(kaudiocreator_SRCS prefs.kcfgc encoder_prefs.kcfgc )

kde4_add_executable(kaudiocreator ${kaudiocreator_SRCS})

target_link_libraries(kaudiocreator  ${KDE3SUPPORT} kcddb )

install(TARGETS kaudiocreator  DESTINATION ${BIN_INSTALL_DIR} )


########### install files ###############

install( FILES kaudiocreator.desktop  DESTINATION  ${XDG_APPS_DIR} )
install( FILES kaudiocreator.kcfg kaudiocreator_encoders.kcfg  DESTINATION  ${KCFG_INSTALL_DIR} )
install( FILES audiocd_extract.desktop  DESTINATION  ${DATA_INSTALL_DIR}/konqueror/servicemenus )
install( FILES kaudiocreatorui.rc eventsrc  DESTINATION  ${DATA_INSTALL_DIR}/kaudiocreator )
install( FILES kaudiocreator-meta.upd kaudiocreator-libkcddb.upd  DESTINATION  ${KCONF_UPDATE_INSTALL_DIR} )
install( FILES upgrade-kaudiocreator-metadata.sh DESTINATION  ${KCONF_UPDATE_INSTALL_DIR} )

kde4_install_icons( ${ICON_INSTALL_DIR}  hicolor )




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
