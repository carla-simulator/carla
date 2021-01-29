add_subdirectory(network)
add_subdirectory(additional)
add_subdirectory(demand)
add_subdirectory(data)

set(netedit_elements_SRCS
   GNEAttributeProperties.h
   GNEAttributeProperties.cpp
   GNETagProperties.h
   GNETagProperties.cpp
   GNEAttributeCarrier.h
   GNEAttributeCarrier.cpp
   GNEHierarchicalContainer.h
   GNEHierarchicalContainer.cpp
   GNEHierarchicalElement.h
   GNEHierarchicalElement.cpp
   GNEPathElements.h
   GNEPathElements.cpp
   GNECandidateElement.h
   GNECandidateElement.cpp
   )

add_library(netedit_elements STATIC ${netedit_elements_SRCS})
set_property(TARGET netedit_elements PROPERTY PROJECT_LABEL "z_netedit_elements")
