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
    Core
    Network
    Test
    Widgets
)
find_package(KF5 REQUIRED COMPONENTS
    Completion
    Config
    ConfigWidgets
    DocTools
    I18n
    KCMUtils
    KDELibs4Support
    KIO
    NotifyConfig
    Solid
    XmlGui
)
find_package(KF5Cddb 5.0.0) # >= 16.12.0
set_package_properties(KF5Cddb PROPERTIES
    DESCRIPTION "KDE CDDB library"
    URL "https://commits.kde.org/libkcddb"
    TYPE REQUIRED
    PURPOSE "libkcddb is used to retrieve audio CD meta data from the internet."
)
find_package(KF5CompactDisc 5.0.0) # >= 16.12.0
set_package_properties(KF5CompactDisc PROPERTIES
    DESCRIPTION "KCompactDisc library"
    URL "https://commits.kde.org/libkcompactdisc"
    TYPE REQUIRED
    PURPOSE "libkcompactdisc is used to access CD drives."
)
find_package(Phonon4Qt5 REQUIRED NO_MODULE)
find_package(DiscId REQUIRED)
find_package(Taglib)

include(KDEInstallDirs)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)
include(ECMInstallIcons)
include(ECMMarkAsTest)
include(ECMEnableSanitizers)

add_definitions(${TAGLIB_CFLAGS})
option(ENABLE_DEBUG "Enable debug" OFF)
if(ENABLE_DEBUG)
    add_definitions(-g -DDEBUG)
endif()

if(KF5DocTools_VERSION)
    add_subdirectory(doc)
endif(KF5DocTools_VERSION)

if(BUILD_TESTING)
    find_package(Qt5Test REQUIRED)
    add_subdirectory(tests)
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

ki18n_wrap_ui(kaudiocreator_SRCS
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

install(PROGRAMS org.kde.kaudiocreator.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR})
install(FILES org.kde.kaudiocreator.appdata.xml DESTINATION  ${KDE_INSTALL_METAINFODIR})
install(FILES kaudiocreator.kcfg kaudiocreator_encoders.kcfg  DESTINATION  ${KCFG_INSTALL_DIR})
install(FILES audiocd_extract.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/ServiceMenus)
install(FILES kaudiocreatorui.rc DESTINATION  ${KXMLGUI_INSTALL_DIR}/kaudiocreator)
install(FILES kaudiocreator.notifyrc DESTINATION  ${KDE_INSTALL_KNOTIFY5RCDIR} )

install(FILES kaudiocreator-libkcddb.upd  DESTINATION  ${KCONF_UPDATE_INSTALL_DIR})

if(NOT WIN32)
    install(FILES kaudiocreator-meta.upd DESTINATION ${KCONF_UPDATE_INSTALL_DIR})
    install(PROGRAMS upgrade-kaudiocreator-metadata.sh DESTINATION  ${KCONF_UPDATE_INSTALL_DIR})
endif(NOT WIN32)

ecm_install_icons(ICONS
    16-apps-kaudiocreator.png
    32-apps-kaudiocreator.png
    DESTINATION ${KDE_INSTALL_ICONDIR}
    THEME hicolor
)

endif (NOT APPLE)

