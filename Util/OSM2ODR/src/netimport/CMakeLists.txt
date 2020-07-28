add_subdirectory(vissim)

set(netimport_STAT_SRCS
   NIFrame.cpp
   NIFrame.h
   NILoader.cpp
   NILoader.h
   NITypeLoader.cpp
   NITypeLoader.h
   NIImporter_ArcView.cpp
   NIImporter_ArcView.h
   NIImporter_DlrNavteq.cpp
   NIImporter_DlrNavteq.h
   NIImporter_ITSUMO.cpp
   NIImporter_ITSUMO.h
   NIImporter_MATSim.cpp
   NIImporter_MATSim.h
   NIImporter_OpenDrive.cpp
   NIImporter_OpenDrive.h
   NIImporter_OpenStreetMap.cpp
   NIImporter_OpenStreetMap.h
   NIImporter_SUMO.cpp
   NIImporter_SUMO.h
   NIImporter_VISUM.cpp
   NIImporter_VISUM.h
   NIVisumTL.cpp
   NIVisumTL.h
   NIXMLConnectionsHandler.cpp
   NIXMLConnectionsHandler.h
   NIXMLEdgesHandler.cpp
   NIXMLEdgesHandler.h
   NIXMLNodesHandler.cpp
   NIXMLNodesHandler.h
   NIXMLTypesHandler.cpp
   NIXMLTypesHandler.h
   NIXMLPTHandler.cpp
   NIXMLPTHandler.h
   NIXMLShapeHandler.cpp
   NIXMLShapeHandler.h
   NIXMLTrafficLightsHandler.cpp
   NIXMLTrafficLightsHandler.h
   NINavTeqHelper.cpp
   NINavTeqHelper.h
   typemap.h
)

add_library(netimport STATIC ${netimport_STAT_SRCS})
set_property(TARGET netimport PROPERTY PROJECT_LABEL "z_netimport")
