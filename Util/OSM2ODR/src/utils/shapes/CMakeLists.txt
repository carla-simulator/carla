set(utils_shapes_STAT_SRCS
   PointOfInterest.h
   Shape.cpp
   Shape.h
   SUMOPolygon.cpp
   SUMOPolygon.h
   PolygonDynamics.cpp
   PolygonDynamics.h
   ShapeHandler.cpp
   ShapeHandler.h
   ShapeContainer.cpp
   ShapeContainer.h
)

add_library(utils_shapes STATIC ${utils_shapes_STAT_SRCS})
set_property(TARGET utils_shapes PROPERTY PROJECT_LABEL "z_utils_shapes")
