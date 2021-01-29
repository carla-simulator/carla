set(utils_gui_cursors_STAT_SRCS
   GUICursorSubSys.cpp
   GUICursorSubSys.h
   GUICursors.h
)

add_library(utils_gui_cursors STATIC ${utils_gui_cursors_STAT_SRCS})
set_property(TARGET utils_gui_cursors PROPERTY PROJECT_LABEL "z_utils_gui_cursors")
