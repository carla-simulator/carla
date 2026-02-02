########## MACROS ###########################################################################
#############################################################################################

# Requires CMake > 3.15
if(${CMAKE_VERSION} VERSION_LESS "3.15")
    message(FATAL_ERROR "The 'CMakeDeps' generator only works with CMake >= 3.15")
endif()

if(carla_FIND_QUIETLY)
    set(carla_MESSAGE_MODE VERBOSE)
else()
    set(carla_MESSAGE_MODE STATUS)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/cmakedeps_macros.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/carlaTargets.cmake)
include(CMakeFindDependencyMacro)

check_build_type_defined()

foreach(_DEPENDENCY ${libcarla_FIND_DEPENDENCY_NAMES} )
    # Check that we have not already called a find_package with the transitive dependency
    if(NOT ${_DEPENDENCY}_FOUND)
        find_dependency(${_DEPENDENCY} REQUIRED ${${_DEPENDENCY}_FIND_MODE})
    endif()
endforeach()

set(carla_VERSION_STRING "0.10.0")
set(carla_INCLUDE_DIRS ${libcarla_INCLUDE_DIRS_RELEASE} )
set(carla_INCLUDE_DIR ${libcarla_INCLUDE_DIRS_RELEASE} )
set(carla_LIBRARIES ${libcarla_LIBRARIES_RELEASE} )
set(carla_DEFINITIONS ${libcarla_DEFINITIONS_RELEASE} )


# Definition of extra CMake variables from cmake_extra_variables


# Only the last installed configuration BUILD_MODULES are included to avoid the collision
foreach(_BUILD_MODULE ${libcarla_BUILD_MODULES_PATHS_RELEASE} )
    message(${carla_MESSAGE_MODE} "Conan: Including build module from '${_BUILD_MODULE}'")
    include(${_BUILD_MODULE})
endforeach()


