if (NOT APPLE)

project(kaudiocreator)

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/modules)

find_package(KDE4 REQUIRED)
find_package(KdeMultimedia REQUIRED)
find_package(DiscId REQUIRED)
find_package(Taglib)
include (KDE4Defaults)
include (MacroLibrary)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS} ${TAGLIB_CFLAGS})

include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES} ${KDEMULTIMEDIA_INCLUDE_DIR} ${TAGLIB_INCLUDE_DIR} ${DISCID_INCLUDE_DIR})

CONFIGURE_FILE(config-kaudiocreator.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kaudiocreator.h)

########### next target ###############

set(kaudiocreator_SRCS
    main.cpp
    kaudiocreator.cpp
    encoder.cpp
    encoderconfigimp.cpp
    job.cpp
    jobdelegate.cpp
    jobqueimp.cpp
    ripper.cpp
    tracksimp.cpp
    encodefilemodel.cpp
    encodefiledelegate.cpp
    encodefileimp.cpp
    general.cpp
    audiocd.cpp
    wizard.cpp)


kde4_add_ui_files(kaudiocreator_SRCS
    encodefile.ui
    tracks.ui
    encoderedit.ui
    jobque.ui
    infodialog.ui
    general.ui
    wizard.ui
    encoderconfig.ui
    encoderoutput.ui
    ripconfig.ui
    cdconfig.ui)

kde4_add_kcfg_files(kaudiocreator_SRCS prefs.kcfgc encoder_prefs.kcfgc )

kde4_add_executable(kaudiocreator ${kaudiocreator_SRCS})

target_link_libraries(kaudiocreator
    ${KDE4_KUTILS_LIBS} ${KDE4_KNOTIFYCONFIG_LIBS} ${KDE4_KIO_LIBS} ${KDE4_PHONON_LIBS} ${KDE4_SOLID_LIBS}
    ${KDEMULTIMEDIA_LIBRARIES} ${TAGLIB_LIBRARIES} ${DISCID_LIBRARIES})

install(TARGETS kaudiocreator  ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### install files ###############

install( PROGRAMS kaudiocreator.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )
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

