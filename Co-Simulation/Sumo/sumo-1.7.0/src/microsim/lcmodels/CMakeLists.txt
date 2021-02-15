set(microsim_lcmodels_STAT_SRCS
   MSAbstractLaneChangeModel.cpp
   MSAbstractLaneChangeModel.h
   MSLCHelper.h
   MSLCHelper.cpp
   MSLCM_DK2008.cpp
   MSLCM_DK2008.h
   MSLCM_LC2013.cpp
   MSLCM_LC2013.h
   MSLCM_SL2015.cpp
   MSLCM_SL2015.h
)

add_library(microsim_lcmodels STATIC ${microsim_lcmodels_STAT_SRCS})
set_property(TARGET microsim_lcmodels PROPERTY PROJECT_LABEL "z_microsim_lcmodels")
