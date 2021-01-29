
########### next target ###############

SET(mesogui_STAT_SRCS
   GUIMEInductLoop.cpp
   GUIMEInductLoop.h
   GUIMEVehicle.cpp
   GUIMEVehicle.h
   GUIMEVehicleControl.cpp
   GUIMEVehicleControl.h
)

add_library(mesogui STATIC ${mesogui_STAT_SRCS})
set_property(TARGET mesogui PROPERTY PROJECT_LABEL "z_mesogui")


########### install files ###############




#original Makefile.am contents follow:

#noinst_LIBRARIES = libmesogui.a
#
#libmesogui_a_SOURCES = GUIMEInductLoop.cpp GUIMEInductLoop.h \
#GUIMEVehicle.cpp GUIMEVehicle.h \
#GUIMEVehicleControl.cpp GUIMEVehicleControl.h
