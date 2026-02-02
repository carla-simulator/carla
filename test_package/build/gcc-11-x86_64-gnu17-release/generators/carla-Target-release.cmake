# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(libcarla_FRAMEWORKS_FOUND_RELEASE "") # Will be filled later
conan_find_apple_frameworks(libcarla_FRAMEWORKS_FOUND_RELEASE "${libcarla_FRAMEWORKS_RELEASE}" "${libcarla_FRAMEWORK_DIRS_RELEASE}")

set(libcarla_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET libcarla_DEPS_TARGET)
    add_library(libcarla_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET libcarla_DEPS_TARGET
             APPEND PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Release>:${libcarla_FRAMEWORKS_FOUND_RELEASE}>
             $<$<CONFIG:Release>:${libcarla_SYSTEM_LIBS_RELEASE}>
             $<$<CONFIG:Release>:boost::boost;PNG::PNG;ZLIB::ZLIB>)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### libcarla_DEPS_TARGET to all of them
conan_package_library_targets("${libcarla_LIBS_RELEASE}"    # libraries
                              "${libcarla_LIB_DIRS_RELEASE}" # package_libdir
                              "${libcarla_BIN_DIRS_RELEASE}" # package_bindir
                              "${libcarla_LIBRARY_TYPE_RELEASE}"
                              "${libcarla_IS_HOST_WINDOWS_RELEASE}"
                              libcarla_DEPS_TARGET
                              libcarla_LIBRARIES_TARGETS  # out_libraries_targets
                              "_RELEASE"
                              "libcarla"    # package_name
                              "${libcarla_NO_SONAME_MODE_RELEASE}")  # soname

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${libcarla_BUILD_DIRS_RELEASE} ${CMAKE_MODULE_PATH})

########## COMPONENTS TARGET PROPERTIES Release ########################################

    ########## COMPONENT carla::carla-client #############

        set(libcarla_carla_carla-client_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(libcarla_carla_carla-client_FRAMEWORKS_FOUND_RELEASE "${libcarla_carla_carla-client_FRAMEWORKS_RELEASE}" "${libcarla_carla_carla-client_FRAMEWORK_DIRS_RELEASE}")

        set(libcarla_carla_carla-client_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET libcarla_carla_carla-client_DEPS_TARGET)
            add_library(libcarla_carla_carla-client_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET libcarla_carla_carla-client_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${libcarla_carla_carla-client_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${libcarla_carla_carla-client_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${libcarla_carla_carla-client_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'libcarla_carla_carla-client_DEPS_TARGET' to all of them
        conan_package_library_targets("${libcarla_carla_carla-client_LIBS_RELEASE}"
                              "${libcarla_carla_carla-client_LIB_DIRS_RELEASE}"
                              "${libcarla_carla_carla-client_BIN_DIRS_RELEASE}" # package_bindir
                              "${libcarla_carla_carla-client_LIBRARY_TYPE_RELEASE}"
                              "${libcarla_carla_carla-client_IS_HOST_WINDOWS_RELEASE}"
                              libcarla_carla_carla-client_DEPS_TARGET
                              libcarla_carla_carla-client_LIBRARIES_TARGETS
                              "_RELEASE"
                              "libcarla_carla_carla-client"
                              "${libcarla_carla_carla-client_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET carla::carla-client
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${libcarla_carla_carla-client_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${libcarla_carla_carla-client_LIBRARIES_TARGETS}>
                     )

        if("${libcarla_carla_carla-client_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET carla::carla-client
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         libcarla_carla_carla-client_DEPS_TARGET)
        endif()

        set_property(TARGET carla::carla-client APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${libcarla_carla_carla-client_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET carla::carla-client APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${libcarla_carla_carla-client_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET carla::carla-client APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${libcarla_carla_carla-client_LIB_DIRS_RELEASE}>)
        set_property(TARGET carla::carla-client APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${libcarla_carla_carla-client_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET carla::carla-client APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${libcarla_carla_carla-client_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT carla::carla-server #############

        set(libcarla_carla_carla-server_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(libcarla_carla_carla-server_FRAMEWORKS_FOUND_RELEASE "${libcarla_carla_carla-server_FRAMEWORKS_RELEASE}" "${libcarla_carla_carla-server_FRAMEWORK_DIRS_RELEASE}")

        set(libcarla_carla_carla-server_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET libcarla_carla_carla-server_DEPS_TARGET)
            add_library(libcarla_carla_carla-server_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET libcarla_carla_carla-server_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${libcarla_carla_carla-server_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${libcarla_carla_carla-server_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${libcarla_carla_carla-server_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'libcarla_carla_carla-server_DEPS_TARGET' to all of them
        conan_package_library_targets("${libcarla_carla_carla-server_LIBS_RELEASE}"
                              "${libcarla_carla_carla-server_LIB_DIRS_RELEASE}"
                              "${libcarla_carla_carla-server_BIN_DIRS_RELEASE}" # package_bindir
                              "${libcarla_carla_carla-server_LIBRARY_TYPE_RELEASE}"
                              "${libcarla_carla_carla-server_IS_HOST_WINDOWS_RELEASE}"
                              libcarla_carla_carla-server_DEPS_TARGET
                              libcarla_carla_carla-server_LIBRARIES_TARGETS
                              "_RELEASE"
                              "libcarla_carla_carla-server"
                              "${libcarla_carla_carla-server_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET carla::carla-server
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${libcarla_carla_carla-server_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${libcarla_carla_carla-server_LIBRARIES_TARGETS}>
                     )

        if("${libcarla_carla_carla-server_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET carla::carla-server
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         libcarla_carla_carla-server_DEPS_TARGET)
        endif()

        set_property(TARGET carla::carla-server APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${libcarla_carla_carla-server_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET carla::carla-server APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${libcarla_carla_carla-server_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET carla::carla-server APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${libcarla_carla_carla-server_LIB_DIRS_RELEASE}>)
        set_property(TARGET carla::carla-server APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${libcarla_carla_carla-server_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET carla::carla-server APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${libcarla_carla_carla-server_COMPILE_OPTIONS_RELEASE}>)


    ########## AGGREGATED GLOBAL TARGET WITH THE COMPONENTS #####################
    set_property(TARGET libcarla::libcarla APPEND PROPERTY INTERFACE_LINK_LIBRARIES carla::carla-client)
    set_property(TARGET libcarla::libcarla APPEND PROPERTY INTERFACE_LINK_LIBRARIES carla::carla-server)

########## For the modules (FindXXX)
set(libcarla_LIBRARIES_RELEASE libcarla::libcarla)
