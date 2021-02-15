set(utils_gui_settings_STAT_SRCS
   GUICompleteSchemeStorage.cpp
   GUICompleteSchemeStorage.h
   GUIPropertyScheme.h
   GUIPropertySchemeStorage.h
   GUISettingsHandler.cpp
   GUISettingsHandler.h
   GUIVisualizationSettings.cpp
   GUIVisualizationSettings.h
)

add_library(utils_gui_settings STATIC ${utils_gui_settings_STAT_SRCS})
set_property(TARGET utils_gui_settings PROPERTY PROJECT_LABEL "z_utils_gui_settings")
