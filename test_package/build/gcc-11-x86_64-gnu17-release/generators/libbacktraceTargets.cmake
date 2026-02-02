# Load the debug and release variables
file(GLOB DATA_FILES "${CMAKE_CURRENT_LIST_DIR}/libbacktrace-*-data.cmake")

foreach(f ${DATA_FILES})
    include(${f})
endforeach()

# Create the targets for all the components
foreach(_COMPONENT ${libbacktrace_COMPONENT_NAMES} )
    if(NOT TARGET ${_COMPONENT})
        add_library(${_COMPONENT} INTERFACE IMPORTED)
        message(${libbacktrace_MESSAGE_MODE} "Conan: Component target declared '${_COMPONENT}'")
    endif()
endforeach()

if(NOT TARGET libbacktrace::libbacktrace)
    add_library(libbacktrace::libbacktrace INTERFACE IMPORTED)
    message(${libbacktrace_MESSAGE_MODE} "Conan: Target declared 'libbacktrace::libbacktrace'")
endif()
# Load the debug and release library finders
file(GLOB CONFIG_FILES "${CMAKE_CURRENT_LIST_DIR}/libbacktrace-Target-*.cmake")

foreach(f ${CONFIG_FILES})
    include(${f})
endforeach()