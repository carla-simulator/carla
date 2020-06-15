
########### next target ###############

SET(netload_STAT_SRCS
   NLDetectorBuilder.h
   NLDetectorBuilder.cpp
   NLDiscreteEventBuilder.h
   NLDiscreteEventBuilder.cpp
   NLEdgeControlBuilder.h
   NLEdgeControlBuilder.cpp
   NLJunctionControlBuilder.h
   NLJunctionControlBuilder.cpp
   NLBuilder.h
   NLBuilder.cpp
   NLHandler.h
   NLHandler.cpp
   NLTriggerBuilder.h
   NLTriggerBuilder.cpp
)

add_library(netload STATIC ${netload_STAT_SRCS})
set_property(TARGET netload PROPERTY PROJECT_LABEL "z_netload")


########### install files ###############




#original Makefile.am contents follow:

#noinst_LIBRARIES = libnetload.a
#
#libnetload_a_SOURCES = NLDetectorBuilder.h NLDetectorBuilder.cpp \
#NLDiscreteEventBuilder.h NLDiscreteEventBuilder.cpp \
#NLEdgeControlBuilder.h NLEdgeControlBuilder.cpp \
#NLJunctionControlBuilder.h NLJunctionControlBuilder.cpp \
#NLBuilder.h NLBuilder.cpp \
#NLHandler.h NLHandler.cpp \
#NLTriggerBuilder.h NLTriggerBuilder.cpp
