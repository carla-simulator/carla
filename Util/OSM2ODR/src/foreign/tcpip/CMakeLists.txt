set(foreign_tcpip_STAT_SRCS
   socket.h
   socket.cpp
   storage.h
   storage.cpp
)

add_library(foreign_tcpip STATIC ${foreign_tcpip_STAT_SRCS})
set_property(TARGET foreign_tcpip PROPERTY PROJECT_LABEL "z_foreign_tcpip")
if (SUMO_UTILS)
    install(TARGETS foreign_tcpip DESTINATION lib)
endif ()
