# FindFFMPEG
# ----------
#
# Find the native FFMPEG includes and library
#
# This module defines::
#
#  FFMPEG_INCLUDE_DIR, where to find avcodec.h, avformat.h ...
#  FFMPEG_LIBRARIES, the libraries to link against to use FFMPEG.
#  FFMPEG_FOUND, If false, do not try to use FFMPEG.
#
# also defined, but not for general use are::
#
#   FFMPEG_avformat_LIBRARY, where to find the FFMPEG avformat library.
#   FFMPEG_avcodec_LIBRARY, where to find the FFMPEG avcodec library.
#
# This is useful to do it this way so that we can always add more libraries
# if needed to ``FFMPEG_LIBRARIES`` if ffmpeg ever changes...

#=============================================================================
#YCM - Extra CMake Modules for YARP and friends
#Copyright 2013-2014 iCub Facility, Istituto Italiano di Tecnologia
#All rights reserved.
#
#Redistribution and use in source and binary forms, with or without
#modification, are permitted provided that the following conditions
#are met:
#
#* Redistributions of source code must retain the above copyright
#  notice, this list of conditions and the following disclaimer.
#
#* Redistributions in binary form must reproduce the above copyright
#  notice, this list of conditions and the following disclaimer in the
#  documentation and/or other materials provided with the distribution.
#
#* Neither the names of iCub Facility, Istituto Italiano di Tecnologia,
#  nor the names of their contributors may be used to endorse or promote
#  products derived from this software without specific prior written
#  permission.
#
#THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
#LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
#A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
#HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
#SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
#LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
#DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
#THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
#OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#------------------------------------------------------------------------------
#
#The above copyright and license notice applies to distributions of
#YCM in source and binary form.  Some source files contain additional
#notices of original copyright by their contributors; see each source
#for details.  Third-party software packages supplied with YCM under
#compatible licenses provide their own copyright notices documented in
#corresponding subdirectories.
#
#------------------------------------------------------------------------------
#
#YCM is being developed by the iCub Facility, Istituto Italiano di
#Tecnologia.
#
#Additional support to YCM was received from the FP7 EU project
#WALK-MAN (http://walk-man.eu/)
#=============================================================================

# Originally from VTK project

# note: _FFMPEG_z_LIBRARY_ was disabled because is already included in SUMO


find_path(FFMPEG_INCLUDE_DIR1 libavformat/avformat.h
  $ENV{FFMPEG_DIR}
  $ENV{FFMPEG_DIR}/ffmpeg
  $ENV{FFMPEG_DIR}/libavformat
  $ENV{FFMPEG_DIR}/include/libavformat
  $ENV{FFMPEG_DIR}/include/ffmpeg
  /usr/local/include/ffmpeg
  /usr/include/ffmpeg
  /usr/include/libavformat
  /usr/include/ffmpeg/libavformat
  /usr/include/${CMAKE_LIBRARY_ARCHITECTURE}/libavformat
  /usr/local/include/libavformat
)

find_path(FFMPEG_INCLUDE_DIR2 libavutil/avutil.h
  $ENV{FFMPEG_DIR}
  $ENV{FFMPEG_DIR}/ffmpeg
  $ENV{FFMPEG_DIR}/libavutil
  $ENV{FFMPEG_DIR}/include/libavutil
  $ENV{FFMPEG_DIR}/include/ffmpeg
  /usr/local/include/ffmpeg
  /usr/include/ffmpeg
  /usr/include/libavutil
  /usr/include/ffmpeg/libavutil
  /usr/include/${CMAKE_LIBRARY_ARCHITECTURE}/libavutil
  /usr/local/include/libavutil
)

find_path(FFMPEG_INCLUDE_DIR3 libavcodec/avcodec.h
  $ENV{FFMPEG_DIR}
  $ENV{FFMPEG_DIR}/ffmpeg
  $ENV{FFMPEG_DIR}/libavcodec
  $ENV{FFMPEG_DIR}/include/libavcodec
  $ENV{FFMPEG_DIR}/include/ffmpeg
  /usr/local/include/ffmpeg
  /usr/include/ffmpeg
  /usr/include/libavcodec
  /usr/include/ffmpeg/libavcodec
  /usr/include/${CMAKE_LIBRARY_ARCHITECTURE}/libavcodec
  /usr/local/include/libavcodec
)

find_path(FFMPEG_INCLUDE_DIR4 libswscale/swscale.h
  $ENV{FFMPEG_DIR}
  $ENV{FFMPEG_DIR}/ffmpeg
  $ENV{FFMPEG_DIR}/libswscale
  $ENV{FFMPEG_DIR}/include/libswscale
  $ENV{FFMPEG_DIR}/include/ffmpeg
  /usr/local/include/ffmpeg
  /usr/include/ffmpeg
  /usr/include/libswscale
  /usr/include/ffmpeg/libswscale
  /usr/include/${CMAKE_LIBRARY_ARCHITECTURE}/libswscale
  /usr/local/include/libswscale
)

find_path(FFMPEG_INCLUDE_DIR5 libavdevice/avdevice.h
  $ENV{FFMPEG_DIR}
  $ENV{FFMPEG_DIR}/ffmpeg
  $ENV{FFMPEG_DIR}/libavdevice
  $ENV{FFMPEG_DIR}/include/libavdevice
  $ENV{FFMPEG_DIR}/include/ffmpeg
  /usr/local/include/ffmpeg
  /usr/include/ffmpeg
  /usr/include/libavdevice
  /usr/include/ffmpeg/libavdevice
  /usr/include/${CMAKE_LIBRARY_ARCHITECTURE}/libavdevice
  /usr/local/include/libavdevice
)

if(FFMPEG_INCLUDE_DIR1)
  if(FFMPEG_INCLUDE_DIR2)
    if(FFMPEG_INCLUDE_DIR3)
      set(FFMPEG_INCLUDE_DIR ${FFMPEG_INCLUDE_DIR1}
                             ${FFMPEG_INCLUDE_DIR2}
                             ${FFMPEG_INCLUDE_DIR3})
    endif()
  endif()
endif()

if(FFMPEG_INCLUDE_DIR4)
  set(FFMPEG_INCLUDE_DIR ${FFMPEG_INCLUDE_DIR}
                         ${FFMPEG_INCLUDE_DIR4})
endif()

if(FFMPEG_INCLUDE_DIR5)
  set(FFMPEG_INCLUDE_DIR ${FFMPEG_INCLUDE_DIR}
                         ${FFMPEG_INCLUDE_DIR5})
endif()

find_library(FFMPEG_avformat_LIBRARY avformat
  $ENV{FFMPEG_DIR}
  $ENV{FFMPEG_DIR}/lib
  $ENV{FFMPEG_DIR}/libavformat
  /usr/local/lib
  /usr/lib
)

find_library(FFMPEG_avcodec_LIBRARY avcodec
  $ENV{FFMPEG_DIR}
  $ENV{FFMPEG_DIR}/lib
  $ENV{FFMPEG_DIR}/libavcodec
  /usr/local/lib
  /usr/lib
)

find_library(FFMPEG_avutil_LIBRARY avutil
  $ENV{FFMPEG_DIR}
  $ENV{FFMPEG_DIR}/lib
  $ENV{FFMPEG_DIR}/libavutil
  /usr/local/lib
  /usr/lib
)

if(NOT DISABLE_SWSCALE)
  find_library(FFMPEG_swscale_LIBRARY swscale
    $ENV{FFMPEG_DIR}
    $ENV{FFMPEG_DIR}/lib
    $ENV{FFMPEG_DIR}/libswscale
    /usr/local/lib
    /usr/lib
  )
endif(NOT DISABLE_SWSCALE)

find_library(FFMPEG_avdevice_LIBRARY avdevice
  $ENV{FFMPEG_DIR}
  $ENV{FFMPEG_DIR}/lib
  $ENV{FFMPEG_DIR}/libavdevice
  /usr/local/lib
  /usr/lib
)


#find_library(_FFMPEG_z_LIBRARY_ z
#  $ENV{FFMPEG_DIR}
#  $ENV{FFMPEG_DIR}/lib
#  /usr/local/lib
#  /usr/lib
#)



if(FFMPEG_INCLUDE_DIR AND EXISTS ${FFMPEG_INCLUDE_DIR2}/libavutil/ffversion.h)
  if(FFMPEG_avformat_LIBRARY)
    if(FFMPEG_avcodec_LIBRARY)
      if(FFMPEG_avutil_LIBRARY)
        file(STRINGS ${FFMPEG_INCLUDE_DIR2}/libavutil/ffversion.h _version_line REGEX "^[ \t]*#define FFMPEG_VERSION.*")
        if(_version_line)
          string(REGEX REPLACE ".*#define FFMPEG_VERSION[ \t]+\"n?([0-9\.]+).*" "\\1" FFMPEG_VERSION "${_version_line}")
          if("${FFMPEG_VERSION}" VERSION_LESS "3.4")
            message(STATUS "Unsuitable FFmpeg version found ${FFMPEG_VERSION}")
          else()
            set(FFMPEG_FOUND "YES")
            set(FFMPEG_LIBRARIES ${FFMPEG_avformat_LIBRARY}
                                 ${FFMPEG_avcodec_LIBRARY}
                                 ${FFMPEG_avutil_LIBRARY}
              )
            if(FFMPEG_swscale_LIBRARY)
              set(FFMPEG_LIBRARIES ${FFMPEG_LIBRARIES}
                                   ${FFMPEG_swscale_LIBRARY}
              )
            endif()
            if(FFMPEG_avdevice_LIBRARY)
              set(FFMPEG_LIBRARIES ${FFMPEG_LIBRARIES}
                                   ${FFMPEG_avdevice_LIBRARY}
              )
            endif()
            #if(_FFMPEG_z_LIBRARY_)
            #  set( FFMPEG_LIBRARIES ${FFMPEG_LIBRARIES}
            #                        ${_FFMPEG_z_LIBRARY_}
            #  )
            #endif()
          endif()
        endif()
      endif()
    endif()
  endif()
endif()
if (NOT FFMPEG_FOUND)
  message(STATUS "Could NOT find suitable FFmpeg.")
endif()

mark_as_advanced(
  FFMPEG_INCLUDE_DIR
  FFMPEG_INCLUDE_DIR1
  FFMPEG_INCLUDE_DIR2
  FFMPEG_INCLUDE_DIR3
  FFMPEG_INCLUDE_DIR4
  FFMPEG_INCLUDE_DIR5
  FFMPEG_avformat_LIBRARY
  FFMPEG_avcodec_LIBRARY
  FFMPEG_avutil_LIBRARY
  FFMPEG_swscale_LIBRARY
  FFMPEG_avdevice_LIBRARY
  #_FFMPEG_z_LIBRARY_
  )

# Set package properties if FeatureSummary was included
if(COMMAND set_package_properties)
  set_package_properties(FFMPEG PROPERTIES DESCRIPTION "A complete, cross-platform solution to record, convert and stream audio and video")
  set_package_properties(FFMPEG PROPERTIES URL "http://ffmpeg.org/")
endif()
