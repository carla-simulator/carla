set(activitygen_activities_STAT_SRCS
   AGActivities.cpp
   AGActivities.h
   AGTrip.cpp
   AGTrip.h
   AGActivity.cpp
   AGActivity.h
   AGWorkAndSchool.cpp
   AGWorkAndSchool.h
   AGFreeTime.cpp
   AGFreeTime.h
)

add_library(activitygen_activities STATIC ${activitygen_activities_STAT_SRCS})
set_property(TARGET activitygen_activities PROPERTY PROJECT_LABEL "z_activitygen_activities")
