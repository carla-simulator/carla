# Source: https://github.com/qgis/QGIS/blob/3b3f6748f1e5e8f77fa87477add360ec0203fb26/cmake/FindProj.cmake
# Changes: included BSD license text
 
# Find Proj
# ~~~~~~~~~
# Copyright (c) 2007, Martin Dobias <wonder.sk at gmail.com>
# Redistribution and use is allowed according to the terms of the BSD license.
#
#Redistribution and use in source and binary forms, with or without
#modification, are permitted provided that the following conditions
#are met:
#
#1. Redistributions of source code must retain the copyright
#   notice, this list of conditions and the following disclaimer.
#2. Redistributions in binary form must reproduce the copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
#3. The name of the author may not be used to endorse or promote products 
#   derived from this software without specific prior written permission.
#
#THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
#IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
#OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
#IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
#INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
#NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
#DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
#THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
#THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# CMake module to search for Proj library
#
# If it's found it sets PROJ_FOUND to TRUE
# and following variables are set:
#    PROJ_INCLUDE_DIR
#    PROJ_API_FILE
#    PROJ_LIBRARY

# FIND_PATH and FIND_LIBRARY normally search standard locations
# before the specified paths. To search non-standard paths first,
# FIND_* is invoked first with specified paths and NO_DEFAULT_PATH
# and then again with no specified paths to search the default
# locations. When an earlier FIND_* succeeds, subsequent FIND_*s
# searching for the same item do nothing. 

# try to use framework on mac
# want clean framework path, not unix compatibility path
IF (APPLE)
  IF (CMAKE_FIND_FRAMEWORK MATCHES "FIRST"
      OR CMAKE_FRAMEWORK_PATH MATCHES "ONLY"
      OR NOT CMAKE_FIND_FRAMEWORK)
    SET (CMAKE_FIND_FRAMEWORK_save ${CMAKE_FIND_FRAMEWORK} CACHE STRING "" FORCE)
    SET (CMAKE_FIND_FRAMEWORK "ONLY" CACHE STRING "" FORCE)
    #FIND_PATH(PROJ_INCLUDE_DIR PROJ/proj_api.h)
    FIND_LIBRARY(PROJ_LIBRARY PROJ)
    IF (PROJ_LIBRARY)
      # FIND_PATH doesn't add "Headers" for a framework
      SET (PROJ_INCLUDE_DIR ${PROJ_LIBRARY}/Headers CACHE PATH "Path to a file.")
    ENDIF (PROJ_LIBRARY)
    SET (CMAKE_FIND_FRAMEWORK ${CMAKE_FIND_FRAMEWORK_save} CACHE STRING "" FORCE)
  ENDIF ()
ENDIF (APPLE)

FIND_PATH(PROJ_INCLUDE_DIR NAMES proj.h proj_api.h PATHS
  "$ENV{INCLUDE}"
  "$ENV{LIB_DIR}/include"
  "$ENV{GDAL_DIR}/include"
  )

FIND_LIBRARY(PROJ_LIBRARY NAMES proj_i proj proj_6_1 PATHS
  "$ENV{LIB}"
  "$ENV{LIB_DIR}/lib"
  "$ENV{GDAL_DIR}/lib"
  )

IF (PROJ_INCLUDE_DIR AND PROJ_LIBRARY)
  SET(PROJ_FOUND TRUE)
  IF (EXISTS "${PROJ_INCLUDE_DIR}/proj.h")
    SET(PROJ_API_FILE "proj.h")
  ELSE ()
    SET(PROJ_API_FILE "proj_api.h")
  ENDIF ()
  IF (NOT PROJ_FIND_QUIETLY)
    MESSAGE(STATUS "Found Proj: ${PROJ_LIBRARY}")
  ENDIF (NOT PROJ_FIND_QUIETLY)
ELSE ()
  SET(PROJ_FOUND FALSE)
  SET(PROJ_LIBRARY "")
  IF (PROJ_FIND_REQUIRED)
    MESSAGE(FATAL_ERROR "Could not find Proj")
  ELSE ()
    IF (NOT PROJ_FIND_QUIETLY)
      MESSAGE(STATUS "Could NOT find Proj")
    ENDIF (NOT PROJ_FIND_QUIETLY)
  ENDIF (PROJ_FIND_REQUIRED)
ENDIF ()
