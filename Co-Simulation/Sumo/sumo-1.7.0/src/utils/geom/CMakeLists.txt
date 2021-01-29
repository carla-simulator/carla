set(utils_geom_STAT_SRCS
   AbstractPoly.h
   Boundary.cpp
   Boundary.h
   Bresenham.cpp
   Bresenham.h
   GeomConvHelper.cpp
   GeomConvHelper.h
   GeoConvHelper.cpp
   GeoConvHelper.h
   GeomHelper.cpp
   GeomHelper.h
   Position.cpp
   Position.h
   PositionVector.cpp
   PositionVector.h
)

add_library(utils_geom STATIC ${utils_geom_STAT_SRCS})
set_property(TARGET utils_geom PROPERTY PROJECT_LABEL "z_utils_geom")
