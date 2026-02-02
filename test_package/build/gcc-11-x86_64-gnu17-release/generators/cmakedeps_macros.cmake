
macro(conan_find_apple_frameworks FRAMEWORKS_FOUND FRAMEWORKS FRAMEWORKS_DIRS)
    if(APPLE)
        foreach(_FRAMEWORK ${FRAMEWORKS})
            # https://cmake.org/pipermail/cmake-developers/2017-August/030199.html
            find_library(CONAN_FRAMEWORK_${_FRAMEWORK}_FOUND NAMES ${_FRAMEWORK} PATHS ${FRAMEWORKS_DIRS} CMAKE_FIND_ROOT_PATH_BOTH)
            if(CONAN_FRAMEWORK_${_FRAMEWORK}_FOUND)
                list(APPEND ${FRAMEWORKS_FOUND} ${CONAN_FRAMEWORK_${_FRAMEWORK}_FOUND})
                message(VERBOSE "Framework found! ${FRAMEWORKS_FOUND}")
            else()
                message(FATAL_ERROR "Framework library ${_FRAMEWORK} not found in paths: ${FRAMEWORKS_DIRS}")
            endif()
        endforeach()
    endif()
endmacro()

function(conan_package_library_targets libraries package_libdir package_bindir library_type
         is_host_windows deps_target out_libraries_target config_suffix package_name no_soname_mode)
    set(_out_libraries_target "")

    foreach(_LIBRARY_NAME ${libraries})
        if(CMAKE_SYSTEM_NAME MATCHES "Windows" AND NOT DEFINED MINGW AND CMAKE_VERSION VERSION_LESS "3.29")
            # Backport logic from https://github.com/Kitware/CMake/commit/c6efbd78d86798573654d1a791f76de0e71bd93f
            # which is only needed on versions older than 3.29
            # this allows finding static library files created by meson
            # We are not affected by PATH-derived folders, because we call find_library with NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH
            set(_original_find_library_suffixes "${CMAKE_FIND_LIBRARY_SUFFIXES}")
            set(_original_find_library_prefixes "${CMAKE_FIND_LIBRARY_PREFIXES}")
            set(CMAKE_FIND_LIBRARY_PREFIXES "" "lib")
            set(CMAKE_FIND_LIBRARY_SUFFIXES ".dll.lib" ".lib" ".a")
        endif()
        find_library(CONAN_FOUND_LIBRARY NAMES ${_LIBRARY_NAME} PATHS ${package_libdir}
                    NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
        if(DEFINED _original_find_library_suffixes)
            set(CMAKE_FIND_LIBRARY_SUFFIXES "${_original_find_library_suffixes}")
            set(CMAKE_FIND_LIBRARY_PREFIXES "${_original_find_library_prefixes}")
            unset(_original_find_library_suffixes)
            unset(_original_find_library_prefixes)
        endif()
        if(CONAN_FOUND_LIBRARY)
            message(VERBOSE "Conan: Library ${_LIBRARY_NAME} found ${CONAN_FOUND_LIBRARY}")

            # Create a micro-target for each lib/a found
            # Allow only some characters for the target name
            string(REGEX REPLACE "[^A-Za-z0-9.+_-]" "_" _LIBRARY_NAME ${_LIBRARY_NAME})
            set(_LIB_NAME CONAN_LIB::${package_name}_${_LIBRARY_NAME}${config_suffix})

            if(is_host_windows AND library_type STREQUAL "SHARED")
              # Store and reset the variable, so it doesn't leak
              set(_OLD_CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES})
              set(CMAKE_FIND_LIBRARY_SUFFIXES .dll ${CMAKE_FIND_LIBRARY_SUFFIXES})
              find_library(CONAN_SHARED_FOUND_LIBRARY NAMES ${_LIBRARY_NAME} PATHS ${package_bindir}
                           NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
              set(CMAKE_FIND_LIBRARY_SUFFIXES ${_OLD_CMAKE_FIND_LIBRARY_SUFFIXES})
              if(NOT CONAN_SHARED_FOUND_LIBRARY)
                message(DEBUG "DLL library not found, creating UNKNOWN IMPORTED target, searched for: ${_LIBRARY_NAME}")
                if(NOT TARGET ${_LIB_NAME})
                   add_library(${_LIB_NAME} UNKNOWN IMPORTED)
                endif()
                set_target_properties(${_LIB_NAME} PROPERTIES IMPORTED_LOCATION${config_suffix} ${CONAN_FOUND_LIBRARY})
              else()
                 if(NOT TARGET ${_LIB_NAME})
                   add_library(${_LIB_NAME} SHARED IMPORTED)
                 endif()
                 set_target_properties(${_LIB_NAME} PROPERTIES IMPORTED_LOCATION${config_suffix} ${CONAN_SHARED_FOUND_LIBRARY})
                 set_target_properties(${_LIB_NAME} PROPERTIES IMPORTED_IMPLIB${config_suffix} ${CONAN_FOUND_LIBRARY})
                 message(DEBUG "Found DLL and STATIC at ${CONAN_SHARED_FOUND_LIBRARY}, ${CONAN_FOUND_LIBRARY}")
              endif()
              unset(CONAN_SHARED_FOUND_LIBRARY CACHE)
            else()
              if(NOT TARGET ${_LIB_NAME})
                  # library_type can be STATIC, still UNKNOWN (if no package type available in the recipe) or SHARED (but no windows)
                  add_library(${_LIB_NAME} ${library_type} IMPORTED)
              endif()
              message(DEBUG "Created target ${_LIB_NAME} ${library_type} IMPORTED")
              set_target_properties(${_LIB_NAME} PROPERTIES IMPORTED_LOCATION${config_suffix} ${CONAN_FOUND_LIBRARY} IMPORTED_NO_SONAME ${no_soname_mode})
            endif()
            list(APPEND _out_libraries_target ${_LIB_NAME})
            message(VERBOSE "Conan: Found: ${CONAN_FOUND_LIBRARY}")
        else()
            message(FATAL_ERROR "Library '${_LIBRARY_NAME}' not found in package. If '${_LIBRARY_NAME}' is a system library, declare it with 'cpp_info.system_libs' property")
        endif()
        unset(CONAN_FOUND_LIBRARY CACHE)
    endforeach()

    # Add the dependencies target for all the imported libraries
    foreach(_T ${_out_libraries_target})
        set_property(TARGET ${_T} APPEND PROPERTY INTERFACE_LINK_LIBRARIES ${deps_target})
    endforeach()

    set(${out_libraries_target} ${_out_libraries_target} PARENT_SCOPE)
endfunction()

macro(check_build_type_defined)
    # Check that the -DCMAKE_BUILD_TYPE argument is always present
    get_property(isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
    if(NOT isMultiConfig AND NOT CMAKE_BUILD_TYPE)
        message(FATAL_ERROR "Please, set the CMAKE_BUILD_TYPE variable when calling to CMake "
                            "adding the '-DCMAKE_BUILD_TYPE=<build_type>' argument.")
    endif()
endmacro()
