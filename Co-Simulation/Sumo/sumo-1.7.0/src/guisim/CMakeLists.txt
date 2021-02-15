
########### next target ###############

SET(guisim_STAT_SRCS
   GUIBusStop.cpp
   GUIBusStop.h
   GUIContainer.cpp
   GUIContainer.h
   GUIContainerStop.cpp
   GUIContainerStop.h
   GUIParkingArea.cpp
   GUIParkingArea.h
   GUIDetectorWrapper.cpp
   GUIDetectorWrapper.h
   GUIE2Collector.cpp
   GUIE2Collector.h
   GUIE3Collector.cpp
   GUIE3Collector.h
   GUIEdge.cpp
   GUIEdge.h
   GUIEventControl.cpp
   GUIEventControl.h
   GUIInductLoop.cpp
   GUIInductLoop.h
   GUIInstantInductLoop.cpp
   GUIInstantInductLoop.h
   GUIJunctionWrapper.cpp
   GUIJunctionWrapper.h
   GUILane.cpp
   GUILane.h
   GUILaneSpeedTrigger.cpp
   GUILaneSpeedTrigger.h
   GUIPerson.cpp
   GUIPerson.h
   GUINet.cpp
   GUINet.h
   GUITrafficLightLogicWrapper.cpp
   GUITrafficLightLogicWrapper.h
   GUITriggeredRerouter.cpp
   GUITriggeredRerouter.h
   GUIOverheadWire.cpp
   GUIOverheadWire.h
   GUICalibrator.cpp
   GUICalibrator.h
   GUIChargingStation.cpp
   GUIChargingStation.h
   GUIBaseVehicle.cpp
   GUIBaseVehicle.h
   GUIVehicle.cpp
   GUIVehicle.h
   GUIVehicleControl.cpp
   GUIVehicleControl.h
   GUITransportableControl.cpp
   GUITransportableControl.h
)

add_library(guisim STATIC ${guisim_STAT_SRCS})
set_property(TARGET guisim PROPERTY PROJECT_LABEL "z_guisim")


########### install files ###############




#original Makefile.am contents follow:

#noinst_LIBRARIES = libguisim.a
#
#libguisim_a_SOURCES = GUIBusStop.cpp GUIBusStop.h \
#GUIContainer.cpp GUIContainer.h \
#GUIContainerStop.cpp GUIContainerStop.h \
#GUIParkingArea.cpp GUIParkingArea.h \
#GUIDetectorWrapper.cpp GUIDetectorWrapper.h \
#GUIE2Collector.cpp GUIE2Collector.h \
#GUIE3Collector.cpp GUIE3Collector.h GUIEdge.cpp GUIEdge.h \
#GUIEventControl.cpp GUIEventControl.h \
#GUIInductLoop.cpp GUIInductLoop.h \
#GUIInstantInductLoop.cpp GUIInstantInductLoop.h \
#GUIJunctionWrapper.cpp GUIJunctionWrapper.h \
#GUILane.cpp GUILane.h \
#GUILaneSpeedTrigger.cpp GUILaneSpeedTrigger.h \
#GUIPerson.cpp GUIPerson.h \
#GUINet.cpp GUINet.h \
#GUITrafficLightLogicWrapper.cpp GUITrafficLightLogicWrapper.h \
#GUITriggeredRerouter.cpp GUITriggeredRerouter.h \
#GUICalibrator.cpp GUICalibrator.h \
#GUIChargingStation.cpp GUIChargingStation.h \
#GUIBaseVehicle.cpp GUIBaseVehicle.h \
#GUIVehicle.cpp GUIVehicle.h GUIVehicleControl.cpp GUIVehicleControl.h \
#GUITransportableControl.cpp GUITransportableControl.h
#
#EXTRA_DIST = guisim64.ico
#
