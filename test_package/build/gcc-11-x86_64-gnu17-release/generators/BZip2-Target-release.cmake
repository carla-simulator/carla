# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(bzip2_FRAMEWORKS_FOUND_RELEASE "") # Will be filled later
conan_find_apple_frameworks(bzip2_FRAMEWORKS_FOUND_RELEASE "${bzip2_FRAMEWORKS_RELEASE}" "${bzip2_FRAMEWORK_DIRS_RELEASE}")

set(bzip2_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET bzip2_DEPS_TARGET)
    add_library(bzip2_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET bzip2_DEPS_TARGET
             APPEND PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Release>:${bzip2_FRAMEWORKS_FOUND_RELEASE}>
             $<$<CONFIG:Release>:${bzip2_SYSTEM_LIBS_RELEASE}>
             $<$<CONFIG:Release>:>)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### bzip2_DEPS_TARGET to all of them
conan_package_library_targets("${bzip2_LIBS_RELEASE}"    # libraries
                              "${bzip2_LIB_DIRS_RELEASE}" # package_libdir
                              "${bzip2_BIN_DIRS_RELEASE}" # package_bindir
                              "${bzip2_LIBRARY_TYPE_RELEASE}"
                              "${bzip2_IS_HOST_WINDOWS_RELEASE}"
                              bzip2_DEPS_TARGET
                              bzip2_LIBRARIES_TARGETS  # out_libraries_targets
                              "_RELEASE"
                              "bzip2"    # package_name
                              "${bzip2_NO_SONAME_MODE_RELEASE}")  # soname

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${bzip2_BUILD_DIRS_RELEASE} ${CMAKE_MODULE_PATH})

########## GLOBAL TARGET PROPERTIES Release ########################################
    set_property(TARGET BZip2::BZip2
                 APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                 $<$<CONFIG:Release>:${bzip2_OBJECTS_RELEASE}>
                 $<$<CONFIG:Release>:${bzip2_LIBRARIES_TARGETS}>
                 )

    if("${bzip2_LIBS_RELEASE}" STREQUAL "")
        # If the package is not declaring any "cpp_info.libs" the package deps, system libs,
        # frameworks etc are not linked to the imported targets and we need to do it to the
        # global target
        set_property(TARGET BZip2::BZip2
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     bzip2_DEPS_TARGET)
    endif()

    set_property(TARGET BZip2::BZip2
                 APPEND PROPERTY INTERFACE_LINK_OPTIONS
                 $<$<CONFIG:Release>:${bzip2_LINKER_FLAGS_RELEASE}>)
    set_property(TARGET BZip2::BZip2
                 APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                 $<$<CONFIG:Release>:${bzip2_INCLUDE_DIRS_RELEASE}>)
    # Necessary to find LINK shared libraries in Linux
    set_property(TARGET BZip2::BZip2
                 APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                 $<$<CONFIG:Release>:${bzip2_LIB_DIRS_RELEASE}>)
    set_property(TARGET BZip2::BZip2
                 APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                 $<$<CONFIG:Release>:${bzip2_COMPILE_DEFINITIONS_RELEASE}>)
    set_property(TARGET BZip2::BZip2
                 APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                 $<$<CONFIG:Release>:${bzip2_COMPILE_OPTIONS_RELEASE}>)

########## For the modules (FindXXX)
set(bzip2_LIBRARIES_RELEASE BZip2::BZip2)
