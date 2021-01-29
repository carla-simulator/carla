set(gui_dialogs_STAT_SRCS
   GUIDialog_AboutSUMO.cpp
   GUIDialog_AboutSUMO.h
   GUIDialog_AppSettings.cpp
   GUIDialog_AppSettings.h
   GUIDialog_Breakpoints.cpp
   GUIDialog_Breakpoints.h
)

add_library(gui_dialogs STATIC ${gui_dialogs_STAT_SRCS})
set_property(TARGET gui_dialogs PROPERTY PROJECT_LABEL "z_gui_dialogs")
add_dependencies(gui_dialogs generate-version-h)
