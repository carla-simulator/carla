
########### next target ###############

SET(guinetload_STAT_SRCS
   GUIDetectorBuilder.cpp
   GUIDetectorBuilder.h
   GUIEdgeControlBuilder.cpp
   GUIEdgeControlBuilder.h
   GUITriggerBuilder.cpp
   GUITriggerBuilder.h
)

add_library(guinetload STATIC ${guinetload_STAT_SRCS})
set_property(TARGET guinetload PROPERTY PROJECT_LABEL "z_guinetload")


########### install files ###############




#original Makefile.am contents follow:

#noinst_LIBRARIES = libguinetload.a
#
#libguinetload_a_SOURCES = GUIDetectorBuilder.cpp GUIDetectorBuilder.h \
#GUIEdgeControlBuilder.cpp GUIEdgeControlBuilder.h \
#GUITriggerBuilder.cpp GUITriggerBuilder.h
