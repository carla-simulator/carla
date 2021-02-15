# - Find GL2PS library
# Find the native GL2PS includes and library
# This module defines
#  GL2PS_INCLUDE_DIR, where to find tiff.h, etc.
#  GL2PS_LIBRARIES, libraries to link against to use GL2PS.
#  GL2PS_FOUND, If false, do not try to use GL2PS.
# also defined, but not for general use are
#  GL2PS_LIBRARY, where to find the GL2PS library.

#=============================================================================
# Copyright 2009 Kitware, Inc.
# Copyright 2009 Mathieu Malaterre <mathieu.malaterre@gmail.com>
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distributed this file outside of CMake, substitute the full
#  License text for the above reference.)

FIND_PATH(GL2PS_INCLUDE_DIR gl2ps.h)

FIND_LIBRARY(GL2PS_LIBRARY NAMES gl2ps)

# handle the QUIETLY and REQUIRED arguments and set GL2PS_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GL2PS DEFAULT_MSG GL2PS_LIBRARY GL2PS_INCLUDE_DIR)

IF(GL2PS_FOUND)
  SET( GL2PS_LIBRARIES ${GL2PS_LIBRARY} )
ENDIF()

MARK_AS_ADVANCED(GL2PS_INCLUDE_DIR GL2PS_LIBRARY)

