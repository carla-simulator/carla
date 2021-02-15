set(netedit_frames_network_SRCS
   GNEConnectorFrame.h
   GNEConnectorFrame.cpp
   GNETLSEditorFrame.h
   GNETLSEditorFrame.cpp
   GNEAdditionalFrame.h
   GNEAdditionalFrame.cpp
   GNECrossingFrame.h
   GNECrossingFrame.cpp
   GNETAZFrame.h
   GNETAZFrame.cpp
   GNEPolygonFrame.h
   GNEPolygonFrame.cpp
   GNECreateEdgeFrame.h
   GNECreateEdgeFrame.cpp
   GNEProhibitionFrame.h
   GNEProhibitionFrame.cpp
   )

add_library(netedit_frames_network STATIC ${netedit_frames_network_SRCS})
set_property(TARGET netedit_frames_network PROPERTY PROJECT_LABEL "z_netedit_frames_network")