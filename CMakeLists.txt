project(kaudiocreator)

include_directories(
		${CMAKE_SOURCE_DIR}/libkcddb
		${CMAKE_BINARY_DIR}/libkcddb
		${CMAKE_SOURCE_DIR}/libkcompactdisc
		${CMAKE_BINARY_DIR}/libkcompactdisc
		)

add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

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
   encodefileimp.cpp general.cpp wizard.cpp )

kde4_automoc(${kaudiocreator_SRCS})

kde4_add_ui_files( kaudiocreator_SRCS
   encodefile.ui tracks.ui encoderedit.ui jobque.ui infodialog.ui general.ui wizard.ui)

kde4_add_ui3_files(kaudiocreator_SRCS
   cdconfig.ui
   ripconfig.ui
   encoderoutput.ui
   encoderconfig.ui )

kde4_add_kcfg_files(kaudiocreator_SRCS prefs.kcfgc encoder_prefs.kcfgc )

kde4_add_executable(kaudiocreator ${kaudiocreator_SRCS})

target_link_libraries(kaudiocreator  ${KDE4_KUTILS_LIBS} ${KDE4_KNOTIFYCONFIG_LIBS} kcddb kcompactdisc)

install(TARGETS kaudiocreator  DESTINATION ${BIN_INSTALL_DIR} )


########### install files ###############

install( FILES kaudiocreator.desktop  DESTINATION  ${XDG_APPS_DIR} )
install( FILES kaudiocreator.kcfg kaudiocreator_encoders.kcfg  DESTINATION  ${KCFG_INSTALL_DIR} )
install( FILES audiocd_extract.desktop  DESTINATION  ${DATA_INSTALL_DIR}/konqueror/servicemenus )
install( FILES kaudiocreatorui.rc kaudiocreator.notifyrc DESTINATION  ${DATA_INSTALL_DIR}/kaudiocreator )
install( FILES kaudiocreator-meta.upd kaudiocreator-libkcddb.upd  DESTINATION  ${KCONF_UPDATE_INSTALL_DIR} )
install( PROGRAMS upgrade-kaudiocreator-metadata.sh DESTINATION  ${KCONF_UPDATE_INSTALL_DIR} )

kde4_install_icons( ${ICON_INSTALL_DIR}   )

