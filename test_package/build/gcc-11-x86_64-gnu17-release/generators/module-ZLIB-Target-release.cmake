# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(zlib_FRAMEWORKS_FOUND_RELEASE "") # Will be filled later
conan_find_apple_frameworks(zlib_FRAMEWORKS_FOUND_RELEASE "${zlib_FRAMEWORKS_RELEASE}" "${zlib_FRAMEWORK_DIRS_RELEASE}")

set(zlib_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET zlib_DEPS_TARGET)
    add_library(zlib_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET zlib_DEPS_TARGET
             APPEND PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Release>:${zlib_FRAMEWORKS_FOUND_RELEASE}>
             $<$<CONFIG:Release>:${zlib_SYSTEM_LIBS_RELEASE}>
             $<$<CONFIG:Release>:>)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### zlib_DEPS_TARGET to all of them
conan_package_library_targets("${zlib_LIBS_RELEASE}"    # libraries
                              "${zlib_LIB_DIRS_RELEASE}" # package_libdir
                              "${zlib_BIN_DIRS_RELEASE}" # package_bindir
                              "${zlib_LIBRARY_TYPE_RELEASE}"
                              "${zlib_IS_HOST_WINDOWS_RELEASE}"
                              zlib_DEPS_TARGET
                              zlib_LIBRARIES_TARGETS  # out_libraries_targets
                              "_RELEASE"
                              "zlib"    # package_name
                              "${zlib_NO_SONAME_MODE_RELEASE}")  # soname

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${zlib_BUILD_DIRS_RELEASE} ${CMAKE_MODULE_PATH})

########## GLOBAL TARGET PROPERTIES Release ########################################
    set_property(TARGET ZLIB::ZLIB
                 APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                 $<$<CONFIG:Release>:${zlib_OBJECTS_RELEASE}>
                 $<$<CONFIG:Release>:${zlib_LIBRARIES_TARGETS}>
                 )

    if("${zlib_LIBS_RELEASE}" STREQUAL "")
        # If the package is not declaring any "cpp_info.libs" the package deps, system libs,
        # frameworks etc are not linked to the imported targets and we need to do it to the
        # global target
        set_property(TARGET ZLIB::ZLIB
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     zlib_DEPS_TARGET)
    endif()

    set_property(TARGET ZLIB::ZLIB
                 APPEND PROPERTY INTERFACE_LINK_OPTIONS
                 $<$<CONFIG:Release>:${zlib_LINKER_FLAGS_RELEASE}>)
    set_property(TARGET ZLIB::ZLIB
                 APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                 $<$<CONFIG:Release>:${zlib_INCLUDE_DIRS_RELEASE}>)
    # Necessary to find LINK shared libraries in Linux
    set_property(TARGET ZLIB::ZLIB
                 APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                 $<$<CONFIG:Release>:${zlib_LIB_DIRS_RELEASE}>)
    set_property(TARGET ZLIB::ZLIB
                 APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                 $<$<CONFIG:Release>:${zlib_COMPILE_DEFINITIONS_RELEASE}>)
    set_property(TARGET ZLIB::ZLIB
                 APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                 $<$<CONFIG:Release>:${zlib_COMPILE_OPTIONS_RELEASE}>)

########## For the modules (FindXXX)
set(zlib_LIBRARIES_RELEASE ZLIB::ZLIB)
