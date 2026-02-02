# Load the debug and release variables
file(GLOB DATA_FILES "${CMAKE_CURRENT_LIST_DIR}/module-BZip2-*-data.cmake")

foreach(f ${DATA_FILES})
    include(${f})
endforeach()

# Create the targets for all the components
foreach(_COMPONENT ${bzip2_COMPONENT_NAMES} )
    if(NOT TARGET ${_COMPONENT})
        add_library(${_COMPONENT} INTERFACE IMPORTED)
        message(${BZip2_MESSAGE_MODE} "Conan: Component target declared '${_COMPONENT}'")
    endif()
endforeach()

if(NOT TARGET BZip2::BZip2)
    add_library(BZip2::BZip2 INTERFACE IMPORTED)
    message(${BZip2_MESSAGE_MODE} "Conan: Target declared 'BZip2::BZip2'")
endif()
# Load the debug and release library finders
file(GLOB CONFIG_FILES "${CMAKE_CURRENT_LIST_DIR}/module-BZip2-Target-*.cmake")

foreach(f ${CONFIG_FILES})
    include(${f})
endforeach()