set(foreign_phemlight_STAT_SRCS
   CEP.cpp
   CEP.h
   CEPHandler.cpp
   CEPHandler.h
   Constants.cpp
   Constants.h
   Helpers.cpp
   Helpers.h
)

add_library(foreign_phemlight STATIC ${foreign_phemlight_STAT_SRCS})
set_property(TARGET foreign_phemlight PROPERTY PROJECT_LABEL "z_foreign_phemlight")
