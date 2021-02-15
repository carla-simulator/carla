set(utils_gui_images_STAT_SRCS
   GUIIconSubSys.cpp
   GUIIconSubSys.h
   GUITextureSubSys.cpp
   GUITextureSubSys.h
   GUIIcons.h
   GUITexturesHelper.cpp
   GUITexturesHelper.h
   GUITextures.h
)

add_library(utils_gui_images STATIC ${utils_gui_images_STAT_SRCS})
set_property(TARGET utils_gui_images PROPERTY PROJECT_LABEL "z_utils_gui_images")
