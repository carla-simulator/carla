set(netedit_frames_common_SRCS
   GNEInspectorFrame.h
   GNEInspectorFrame.cpp
   GNESelectorFrame.h
   GNESelectorFrame.cpp
   GNEDeleteFrame.h
   GNEDeleteFrame.cpp
   GNEMoveFrame.h
   GNEMoveFrame.cpp
   )

add_library(netedit_frames_common STATIC ${netedit_frames_common_SRCS})
set_property(TARGET netedit_frames_common PROPERTY PROJECT_LABEL "z_netedit_frames_common")