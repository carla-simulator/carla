set(utils_distribution_STAT_SRCS
   Distribution.h
   Distribution_Parameterized.cpp
   Distribution_Parameterized.h
   Distribution_Points.cpp
   Distribution_Points.h
   DistributionCont.cpp
   DistributionCont.h
   RandomDistributor.h
)

add_library(utils_distribution STATIC ${utils_distribution_STAT_SRCS})
set_property(TARGET utils_distribution PROPERTY PROJECT_LABEL "z_utils_distribution")
