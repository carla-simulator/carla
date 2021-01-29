set(activitygen_city_STAT_SRCS
   AGTime.cpp
   AGTime.h
   AGCity.cpp
   AGCity.h
   AGStreet.cpp
   AGStreet.h
   AGPosition.cpp
   AGPosition.h
   AGSchool.cpp
   AGSchool.h
   AGBusLine.cpp
   AGBusLine.h
   AGBus.cpp
   AGBus.h
   AGHousehold.cpp
   AGHousehold.h
   AGWorkPosition.cpp
   AGWorkPosition.h
   AGPerson.cpp
   AGPerson.h
   AGAdult.cpp
   AGAdult.h
   AGChild.cpp
   AGChild.h
   AGDataAndStatistics.cpp
   AGDataAndStatistics.h
   AGCar.cpp
   AGCar.h
)

add_library(activitygen_city STATIC ${activitygen_city_STAT_SRCS})
set_property(TARGET activitygen_city PROPERTY PROJECT_LABEL "z_activitygen_city")
