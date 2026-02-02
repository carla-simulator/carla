# Load the debug and release variables
file(GLOB DATA_FILES "${CMAKE_CURRENT_LIST_DIR}/module-PNG-*-data.cmake")

foreach(f ${DATA_FILES})
    include(${f})
endforeach()

# Create the targets for all the components
foreach(_COMPONENT ${libpng_COMPONENT_NAMES} )
    if(NOT TARGET ${_COMPONENT})
        add_library(${_COMPONENT} INTERFACE IMPORTED)
        message(${PNG_MESSAGE_MODE} "Conan: Component target declared '${_COMPONENT}'")
    endif()
endforeach()

if(NOT TARGET PNG::PNG)
    add_library(PNG::PNG INTERFACE IMPORTED)
    message(${PNG_MESSAGE_MODE} "Conan: Target declared 'PNG::PNG'")
endif()
# Load the debug and release library finders
file(GLOB CONFIG_FILES "${CMAKE_CURRENT_LIST_DIR}/module-PNG-Target-*.cmake")

foreach(f ${CONFIG_FILES})
    include(${f})
endforeach()