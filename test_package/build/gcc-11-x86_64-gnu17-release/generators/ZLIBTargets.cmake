# Load the debug and release variables
file(GLOB DATA_FILES "${CMAKE_CURRENT_LIST_DIR}/ZLIB-*-data.cmake")

foreach(f ${DATA_FILES})
    include(${f})
endforeach()

# Create the targets for all the components
foreach(_COMPONENT ${zlib_COMPONENT_NAMES} )
    if(NOT TARGET ${_COMPONENT})
        add_library(${_COMPONENT} INTERFACE IMPORTED)
        message(${ZLIB_MESSAGE_MODE} "Conan: Component target declared '${_COMPONENT}'")
    endif()
endforeach()

if(NOT TARGET ZLIB::ZLIB)
    add_library(ZLIB::ZLIB INTERFACE IMPORTED)
    message(${ZLIB_MESSAGE_MODE} "Conan: Target declared 'ZLIB::ZLIB'")
endif()
# Load the debug and release library finders
file(GLOB CONFIG_FILES "${CMAKE_CURRENT_LIST_DIR}/ZLIB-Target-*.cmake")

foreach(f ${CONFIG_FILES})
    include(${f})
endforeach()