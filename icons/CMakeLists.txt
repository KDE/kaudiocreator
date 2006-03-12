kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kaudiocreator/pics FILES check.png )

kde4_footer()



#original Makefile.am contents follow:

#
#icondir = $(kde_datadir)/kaudiocreator/pics
#icon_DATA = check.png
#
