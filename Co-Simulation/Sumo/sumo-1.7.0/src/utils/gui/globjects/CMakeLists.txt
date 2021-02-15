set(utils_gui_globjects_STAT_SRCS
   GLIncludes.h
   GUIGLObjectPopupMenu.cpp
   GUIGLObjectPopupMenu.h
   GUIGlObject.cpp
   GUIGlObject.h
   GUIGlObjectStorage.cpp
   GUIGlObjectStorage.h
   GUIGlObjectTypes.h
   GUIGlObject_AbstractAdd.cpp
   GUIGlObject_AbstractAdd.h
   GUIShapeContainer.cpp
   GUIShapeContainer.h
   GUIPointOfInterest.cpp
   GUIPointOfInterest.h
   GUIPolygon.cpp
   GUIPolygon.h
)

add_library(utils_gui_globjects STATIC ${utils_gui_globjects_STAT_SRCS})
set_property(TARGET utils_gui_globjects PROPERTY PROJECT_LABEL "z_utils_gui_globjects")
