if (NOT APPLE)

project(kaudiocreator)

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


kde4_add_ui_files( kaudiocreator_SRCS
   encodefile.ui tracks.ui encoderedit.ui jobque.ui infodialog.ui general.ui
   wizard.ui encoderconfig.ui encoderoutput.ui )

kde4_add_ui3_files(kaudiocreator_SRCS
   cdconfig.ui
   ripconfig.ui )

kde4_add_kcfg_files(kaudiocreator_SRCS prefs.kcfgc encoder_prefs.kcfgc )

kde4_add_executable(kaudiocreator ${kaudiocreator_SRCS})

target_link_libraries(kaudiocreator  ${KDE4_KUTILS_LIBS} ${KDE4_KNOTIFYCONFIG_LIBS} kcddb kcompactdisc)

install(TARGETS kaudiocreator  DESTINATION ${BIN_INSTALL_DIR} )


########### install files ###############

install( FILES kaudiocreator.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )
install( FILES kaudiocreator.kcfg kaudiocreator_encoders.kcfg  DESTINATION  ${KCFG_INSTALL_DIR} )
install( FILES audiocd_extract.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/ServiceMenus )
install( FILES kaudiocreatorui.rc kaudiocreator.notifyrc DESTINATION  ${DATA_INSTALL_DIR}/kaudiocreator )

install( FILES kaudiocreator-libkcddb.upd  DESTINATION  ${KCONF_UPDATE_INSTALL_DIR} )

if(NOT WIN32)
    install( FILES kaudiocreator-meta.upd DESTINATION ${KCONF_UPDATE_INSTALL_DIR})
    install( PROGRAMS upgrade-kaudiocreator-metadata.sh DESTINATION  ${KCONF_UPDATE_INSTALL_DIR} )
endif(NOT WIN32)

kde4_install_icons( ${ICON_INSTALL_DIR}   )

endif (NOT APPLE)

