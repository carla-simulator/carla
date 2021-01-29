set(od_STAT_SRCS
   ODDistrict.cpp
   ODDistrict.h
   ODDistrictCont.cpp
   ODDistrictCont.h
   ODAmitranHandler.cpp
   ODAmitranHandler.h
   ODDistrictHandler.cpp
   ODDistrictHandler.h
   ODCell.h
   ODMatrix.h
   ODMatrix.cpp
)

add_library(od STATIC ${od_STAT_SRCS})
set_property(TARGET od PROPERTY PROJECT_LABEL "z_od")
