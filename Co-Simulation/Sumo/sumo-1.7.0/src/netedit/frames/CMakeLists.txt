add_subdirectory(common)
add_subdirectory(network)
add_subdirectory(demand)
add_subdirectory(data)

set(netedit_frames_SRCS
   GNEFrame.h
   GNEFrame.cpp
   GNEFrameModuls.h
   GNEFrameModuls.cpp
   GNEFrameAttributesModuls.h
   GNEFrameAttributesModuls.cpp
   )

add_library(netedit_frames STATIC ${netedit_frames_SRCS})
set_property(TARGET netedit_frames PROPERTY PROJECT_LABEL "z_netedit_frames")