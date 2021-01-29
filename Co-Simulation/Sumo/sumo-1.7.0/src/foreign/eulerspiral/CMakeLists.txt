set(foreign_eulerspiral_STAT_SRCS
    odrSpiral.cpp
    odrSpiral.h
)

add_library(foreign_eulerspiral STATIC ${foreign_eulerspiral_STAT_SRCS})
set_property(TARGET foreign_eulerspiral PROPERTY PROJECT_LABEL "z_foreign_eulerspiral")
