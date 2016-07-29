if (NOT APPLE)

project(kaudiocreator)

cmake_minimum_required(VERSION 2.8.12)

# ECM setup
include(FeatureSummary)
find_package(ECM 5.24.0  NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://projects.kde.org/projects/kdesupport/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)
set(CMAKE_MODULE_PATH "${ECM_MODULE_PATH}" "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

find_package(Qt5 REQUIRED COMPONENTS
    Test
    Network
    Core
    Widgets
)
find_package(KF5 REQUIRED COMPONENTS
    CompactDisc
    KCMUtils
    Solid
    NotifyConfig
    ConfigWidgets
    Config
    Completion
    XmlGui
    Solid
    KDELibs4Support
    I18n
    KIO
)
find_package(KF5Cddb)
set_package_properties(KF5Cddb PROPERTIES
    DESCRIPTION "KF5 branch for CDDB library"
    URL "http://projects.kde.org/projects/kde/kdemultimedia/libkcddb"
    TYPE REQUIRED
    PURPOSE "libkcddb is used to retrieve audio CD meta data from the internet."
)
find_package(Phonon4Qt5 REQUIRED NO_MODULE)
find_package(DiscId REQUIRED)
find_package(Taglib)

include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)
include(ECMInstallIcons)
include(ECMMarkAsTest)

add_definitions(${TAGLIB_CFLAGS})
option(ENABLE_DEBUG "Enable debug" OFF)
if(ENABLE_DEBUG)
    add_definitions(-g -DDEBUG)
endif()

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
    ${PHONON_INCLUDE_DIR}
    ${TAGLIB_INCLUDE_DIR}
    ${DISCID_INCLUDE_DIR}
)

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
    wizard.cpp
)

qt5_wrap_ui(kaudiocreator_SRCS
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
    cdconfig.ui
)

kconfig_add_kcfg_files(kaudiocreator_SRCS prefs.kcfgc encoder_prefs.kcfgc)

add_executable(kaudiocreator ${kaudiocreator_SRCS})

target_link_libraries(kaudiocreator
    KF5::CddbWidgets
    KF5::CompactDisc
    KF5::KCMUtils
    KF5::Solid
    KF5::NotifyConfig
    KF5::KDELibs4Support
    KF5::Completion
    KF5::XmlGui
    KF5::Solid
    KF5::KIOWidgets
    Qt5::Core
    Qt5::Widgets
    ${PHONON_LIBRARIES}
    ${TAGLIB_LIBRARIES}
    ${DISCID_LIBRARIES}
)

install(TARGETS kaudiocreator  ${INSTALL_TARGETS_DEFAULT_ARGS})


########### install files ###############

install(PROGRAMS kaudiocreator.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR})
install(FILES kaudiocreator.appdata.xml DESTINATION  ${DATA_INSTALL_DIR}/appdata)
install(FILES kaudiocreator.kcfg kaudiocreator_encoders.kcfg  DESTINATION  ${KCFG_INSTALL_DIR})
install(FILES audiocd_extract.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/ServiceMenus)
install(FILES kaudiocreatorui.rc kaudiocreator.notifyrc DESTINATION  ${DATA_INSTALL_DIR}/kaudiocreator)

install(FILES kaudiocreator-libkcddb.upd  DESTINATION  ${KCONF_UPDATE_INSTALL_DIR})

if(NOT WIN32)
    install(FILES kaudiocreator-meta.upd DESTINATION ${KCONF_UPDATE_INSTALL_DIR})
    install(PROGRAMS upgrade-kaudiocreator-metadata.sh DESTINATION  ${KCONF_UPDATE_INSTALL_DIR})
endif(NOT WIN32)

ecm_install_icons(${ICON_INSTALL_DIR})

endif (NOT APPLE)

