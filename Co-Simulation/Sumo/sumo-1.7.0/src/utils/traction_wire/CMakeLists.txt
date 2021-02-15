
set(utils_traction_wire_STAT_SRCS
   Circuit.cpp
   Circuit.h
   Element.cpp
   Element.h
# The following files are only used in standalone mode:
#   IO.cpp
#   IO.h
#   main.cpp
   Node.cpp
   Node.h
)

add_library(utils_traction_wire STATIC ${utils_traction_wire_STAT_SRCS})
set_property(TARGET utils_traction_wire PROPERTY PROJECT_LABEL "z_utils_traction_wire")
