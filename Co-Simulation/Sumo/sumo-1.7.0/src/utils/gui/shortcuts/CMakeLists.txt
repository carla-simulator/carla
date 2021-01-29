set(utils_gui_shortcuts_STAT_SRCS
   GUIShortcutsSubSys.cpp
   GUIShortcutsSubSys.h
   GUIShortcuts.h
)

add_library(utils_gui_shortcuts STATIC ${utils_gui_shortcuts_STAT_SRCS})
set_property(TARGET utils_gui_shortcuts PROPERTY PROJECT_LABEL "z_utils_gui_shortcuts")
