set(netedit_frames_data_SRCS
   GNEGenericDataFrame.h
   GNEGenericDataFrame.cpp
   GNEEdgeDataFrame.h
   GNEEdgeDataFrame.cpp
   GNEEdgeRelDataFrame.h
   GNEEdgeRelDataFrame.cpp
   GNETAZRelDataFrame.h
   GNETAZRelDataFrame.cpp
   )

add_library(netedit_frames_data STATIC ${netedit_frames_data_SRCS})
set_property(TARGET netedit_frames_data PROPERTY PROJECT_LABEL "z_netedit_frames_data")