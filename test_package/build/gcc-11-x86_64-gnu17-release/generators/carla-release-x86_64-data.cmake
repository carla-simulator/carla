########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

list(APPEND libcarla_COMPONENT_NAMES carla::carla-server carla::carla-client)
list(REMOVE_DUPLICATES libcarla_COMPONENT_NAMES)
if(DEFINED libcarla_FIND_DEPENDENCY_NAMES)
  list(APPEND libcarla_FIND_DEPENDENCY_NAMES Boost PNG ZLIB)
  list(REMOVE_DUPLICATES libcarla_FIND_DEPENDENCY_NAMES)
else()
  set(libcarla_FIND_DEPENDENCY_NAMES Boost PNG ZLIB)
endif()
set(Boost_FIND_MODE "NO_MODULE")
set(PNG_FIND_MODE "NO_MODULE")
set(ZLIB_FIND_MODE "NO_MODULE")

########### VARIABLES #######################################################################
#############################################################################################
set(libcarla_PACKAGE_FOLDER_RELEASE "/home/masaya/.conan2/p/b/libca7d10afd76ca31/p")
set(libcarla_BUILD_MODULES_PATHS_RELEASE )


set(libcarla_INCLUDE_DIRS_RELEASE "${libcarla_PACKAGE_FOLDER_RELEASE}/include")
set(libcarla_RES_DIRS_RELEASE )
set(libcarla_DEFINITIONS_RELEASE )
set(libcarla_SHARED_LINK_FLAGS_RELEASE )
set(libcarla_EXE_LINK_FLAGS_RELEASE )
set(libcarla_OBJECTS_RELEASE )
set(libcarla_COMPILE_DEFINITIONS_RELEASE )
set(libcarla_COMPILE_OPTIONS_C_RELEASE )
set(libcarla_COMPILE_OPTIONS_CXX_RELEASE )
set(libcarla_LIB_DIRS_RELEASE "${libcarla_PACKAGE_FOLDER_RELEASE}/lib")
set(libcarla_BIN_DIRS_RELEASE )
set(libcarla_LIBRARY_TYPE_RELEASE STATIC)
set(libcarla_IS_HOST_WINDOWS_RELEASE 0)
set(libcarla_LIBS_RELEASE carla-client carla-server)
set(libcarla_SYSTEM_LIBS_RELEASE )
set(libcarla_FRAMEWORK_DIRS_RELEASE )
set(libcarla_FRAMEWORKS_RELEASE )
set(libcarla_BUILD_DIRS_RELEASE )
set(libcarla_NO_SONAME_MODE_RELEASE FALSE)


# COMPOUND VARIABLES
set(libcarla_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${libcarla_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${libcarla_COMPILE_OPTIONS_C_RELEASE}>")
set(libcarla_LINKER_FLAGS_RELEASE
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${libcarla_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${libcarla_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${libcarla_EXE_LINK_FLAGS_RELEASE}>")


set(libcarla_COMPONENTS_RELEASE carla::carla-server carla::carla-client)
########### COMPONENT carla::carla-client VARIABLES ############################################

set(libcarla_carla_carla-client_INCLUDE_DIRS_RELEASE "${libcarla_PACKAGE_FOLDER_RELEASE}/include")
set(libcarla_carla_carla-client_LIB_DIRS_RELEASE "${libcarla_PACKAGE_FOLDER_RELEASE}/lib")
set(libcarla_carla_carla-client_BIN_DIRS_RELEASE )
set(libcarla_carla_carla-client_LIBRARY_TYPE_RELEASE STATIC)
set(libcarla_carla_carla-client_IS_HOST_WINDOWS_RELEASE 0)
set(libcarla_carla_carla-client_RES_DIRS_RELEASE )
set(libcarla_carla_carla-client_DEFINITIONS_RELEASE )
set(libcarla_carla_carla-client_OBJECTS_RELEASE )
set(libcarla_carla_carla-client_COMPILE_DEFINITIONS_RELEASE )
set(libcarla_carla_carla-client_COMPILE_OPTIONS_C_RELEASE "")
set(libcarla_carla_carla-client_COMPILE_OPTIONS_CXX_RELEASE "")
set(libcarla_carla_carla-client_LIBS_RELEASE carla-client)
set(libcarla_carla_carla-client_SYSTEM_LIBS_RELEASE )
set(libcarla_carla_carla-client_FRAMEWORK_DIRS_RELEASE )
set(libcarla_carla_carla-client_FRAMEWORKS_RELEASE )
set(libcarla_carla_carla-client_DEPENDENCIES_RELEASE )
set(libcarla_carla_carla-client_SHARED_LINK_FLAGS_RELEASE )
set(libcarla_carla_carla-client_EXE_LINK_FLAGS_RELEASE )
set(libcarla_carla_carla-client_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(libcarla_carla_carla-client_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${libcarla_carla_carla-client_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${libcarla_carla_carla-client_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${libcarla_carla_carla-client_EXE_LINK_FLAGS_RELEASE}>
)
set(libcarla_carla_carla-client_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${libcarla_carla_carla-client_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${libcarla_carla_carla-client_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT carla::carla-server VARIABLES ############################################

set(libcarla_carla_carla-server_INCLUDE_DIRS_RELEASE "${libcarla_PACKAGE_FOLDER_RELEASE}/include")
set(libcarla_carla_carla-server_LIB_DIRS_RELEASE "${libcarla_PACKAGE_FOLDER_RELEASE}/lib")
set(libcarla_carla_carla-server_BIN_DIRS_RELEASE )
set(libcarla_carla_carla-server_LIBRARY_TYPE_RELEASE STATIC)
set(libcarla_carla_carla-server_IS_HOST_WINDOWS_RELEASE 0)
set(libcarla_carla_carla-server_RES_DIRS_RELEASE )
set(libcarla_carla_carla-server_DEFINITIONS_RELEASE )
set(libcarla_carla_carla-server_OBJECTS_RELEASE )
set(libcarla_carla_carla-server_COMPILE_DEFINITIONS_RELEASE )
set(libcarla_carla_carla-server_COMPILE_OPTIONS_C_RELEASE "")
set(libcarla_carla_carla-server_COMPILE_OPTIONS_CXX_RELEASE "")
set(libcarla_carla_carla-server_LIBS_RELEASE carla-server)
set(libcarla_carla_carla-server_SYSTEM_LIBS_RELEASE )
set(libcarla_carla_carla-server_FRAMEWORK_DIRS_RELEASE )
set(libcarla_carla_carla-server_FRAMEWORKS_RELEASE )
set(libcarla_carla_carla-server_DEPENDENCIES_RELEASE )
set(libcarla_carla_carla-server_SHARED_LINK_FLAGS_RELEASE )
set(libcarla_carla_carla-server_EXE_LINK_FLAGS_RELEASE )
set(libcarla_carla_carla-server_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(libcarla_carla_carla-server_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${libcarla_carla_carla-server_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${libcarla_carla_carla-server_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${libcarla_carla_carla-server_EXE_LINK_FLAGS_RELEASE}>
)
set(libcarla_carla_carla-server_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${libcarla_carla_carla-server_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${libcarla_carla_carla-server_COMPILE_OPTIONS_C_RELEASE}>")