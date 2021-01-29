SET(osgview_STAT_SRCS
   GUIOSGBuilder.cpp
   GUIOSGBuilder.h
   GUIOSGView.cpp
   GUIOSGView.h
)

add_library(osgview STATIC ${osgview_STAT_SRCS})
set_property(TARGET osgview PROPERTY PROJECT_LABEL "z_osgview")
