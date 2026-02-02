# Load the debug and release variables
file(GLOB DATA_FILES "${CMAKE_CURRENT_LIST_DIR}/carla-*-data.cmake")

foreach(f ${DATA_FILES})
    include(${f})
endforeach()

# Create the targets for all the components
foreach(_COMPONENT ${libcarla_COMPONENT_NAMES} )
    if(NOT TARGET ${_COMPONENT})
        add_library(${_COMPONENT} INTERFACE IMPORTED)
        message(${carla_MESSAGE_MODE} "Conan: Component target declared '${_COMPONENT}'")
    endif()
endforeach()

if(NOT TARGET libcarla::libcarla)
    add_library(libcarla::libcarla INTERFACE IMPORTED)
    message(${carla_MESSAGE_MODE} "Conan: Target declared 'libcarla::libcarla'")
endif()
# Load the debug and release library finders
file(GLOB CONFIG_FILES "${CMAKE_CURRENT_LIST_DIR}/carla-Target-*.cmake")

foreach(f ${CONFIG_FILES})
    include(${f})
endforeach()