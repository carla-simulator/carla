set(netedit_elements_data_SRCS
   GNEDataHandler.h
   GNEDataHandler.cpp
   GNEDataSet.h
   GNEDataSet.cpp
   GNEDataInterval.h
   GNEDataInterval.cpp
   GNEGenericData.h
   GNEGenericData.cpp
   GNEEdgeData.h
   GNEEdgeData.cpp
   GNEEdgeRelData.h
   GNEEdgeRelData.cpp
   GNETAZRelData.h
   GNETAZRelData.cpp
   )

add_library(netedit_elements_data STATIC ${netedit_elements_data_SRCS})
set_property(TARGET netedit_elements_data PROPERTY PROJECT_LABEL "z_netedit_elements_data")
