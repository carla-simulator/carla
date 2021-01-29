
########### next target ###############

SET(mesosim_STAT_SRCS
   MEInductLoop.cpp
   MEInductLoop.h
   MELoop.cpp
   MELoop.h
   MESegment.cpp
   MESegment.h
   METriggeredCalibrator.cpp
   METriggeredCalibrator.h
   MEVehicle.cpp
   MEVehicle.h
   MEVehicleControl.cpp
   MEVehicleControl.h
)

add_library(mesosim STATIC ${mesosim_STAT_SRCS})
set_property(TARGET mesosim PROPERTY PROJECT_LABEL "z_mesosim")


########### install files ###############




#original Makefile.am contents follow:

#noinst_LIBRARIES = libmesosim.a
#
#libmesosim_a_SOURCES = MEInductLoop.cpp MEInductLoop.h \
#MELoop.cpp MELoop.h \
#MESegment.cpp MESegment.h \
#METriggeredCalibrator.cpp METriggeredCalibrator.h \
#MEVehicle.cpp MEVehicle.h \
#MEVehicleControl.cpp MEVehicleControl.h 
