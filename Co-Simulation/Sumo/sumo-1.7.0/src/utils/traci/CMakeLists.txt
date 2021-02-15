set(utils_traci_STAT_SRCS
    TraCIAPI.cpp
    TraCIAPI.h
)

add_library(utils_traci STATIC ${utils_traci_STAT_SRCS})
set_property(TARGET utils_traci PROPERTY PROJECT_LABEL "z_utils_traci")

if (SUMO_UTILS)
    add_library(traciclient SHARED ${utils_traci_STAT_SRCS})
    install(TARGETS traciclient DESTINATION lib)
    if (MSVC)
        target_link_libraries(traciclient foreign_tcpip ws2_32)
    elseif (APPLE)
        target_link_libraries(traciclient foreign_tcpip)
    else ()
        target_link_libraries(traciclient -Wl,--whole-archive foreign_tcpip -Wl,--no-whole-archive)
    endif()
endif()
