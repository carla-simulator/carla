set(utils_iodevices_STAT_SRCS
   OutputDevice.cpp
   OutputDevice.h
   OutputDevice_CERR.cpp
   OutputDevice_CERR.h
   OutputDevice_COUT.cpp
   OutputDevice_COUT.h
   OutputDevice_File.cpp
   OutputDevice_File.h
   OutputDevice_String.cpp
   OutputDevice_String.h
   OutputDevice_Network.cpp
   OutputDevice_Network.h
   OutputFormatter.h
   PlainXMLFormatter.cpp
   PlainXMLFormatter.h
)

add_library(utils_iodevices STATIC ${utils_iodevices_STAT_SRCS})
#add_dependencies(utils_iodevices generate-version-h)
set_property(TARGET utils_iodevices PROPERTY PROJECT_LABEL "z_utils_iodevices")
