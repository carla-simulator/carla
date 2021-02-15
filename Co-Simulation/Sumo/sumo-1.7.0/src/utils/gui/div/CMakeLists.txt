set(utils_gui_div_STAT_SRCS
   GUIDialog_GLChosenEditor.cpp
   GUIDialog_GLChosenEditor.h
   GUIGlobalSelection.cpp
   GUIGlobalSelection.h
   GUIIOGlobals.cpp
   GUIIOGlobals.h
   GUIMessageWindow.cpp
   GUIMessageWindow.h
   GUIParam_PopupMenu.cpp
   GUIParam_PopupMenu.h
   GUIParameterTableItem.h
   GUIParameterTableWindow.cpp
   GUIParameterTableWindow.h
   GUISelectedStorage.cpp
   GUISelectedStorage.h
   GUIUserIO.cpp
   GUIUserIO.h
   GUIVideoEncoder.h
   GLHelper.cpp
   GLHelper.h
   GUIBaseVehicleHelper.cpp
   GUIBaseVehicleHelper.h
   GUIBasePersonHelper.cpp
   GUIBasePersonHelper.h
   GUIDesigns.h
   GLObjectValuePassConnector.h
)

add_library(utils_gui_div STATIC ${utils_gui_div_STAT_SRCS})
set_property(TARGET utils_gui_div PROPERTY PROJECT_LABEL "z_utils_gui_div")
