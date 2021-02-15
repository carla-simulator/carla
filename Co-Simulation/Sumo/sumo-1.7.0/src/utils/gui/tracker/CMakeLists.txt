set(utils_gui_tracker_STAT_SRCS
   GUIParameterTracker.cpp
   GUIParameterTracker.h
   TrackerValueDesc.cpp
   TrackerValueDesc.h
)

add_library(utils_gui_tracker STATIC ${utils_gui_tracker_STAT_SRCS})
set_property(TARGET utils_gui_tracker PROPERTY PROJECT_LABEL "z_utils_gui_tracker")
