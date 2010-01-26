# Module to find the discid library
# It can be found at http://musicbrainz.org/doc/libdiscid
#
# It defines
#  DISCID_INCLUDE_DIR - the include dir
#  DISCID_LIBRARIES - the required libraries
#  DISCID_FOUND - true if both of the above have been found

# Copyright (c) 2006,2007 Laurent Montel, <montel@kde.org>
# Copyright (c) 2010 Gerd Fleischer
#
# Redistribution and use is allowed according to the terms of the BSD license.

FIND_PATH(DISCID_INCLUDE_DIR discid/discid.h)

FIND_LIBRARY(DISCID_LIBRARIES NAMES discid)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args( DiscId DEFAULT_MSG
                                   DISCID_INCLUDE_DIR DISCID_LIBRARIES)
