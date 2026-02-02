########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

set(bzip2_COMPONENT_NAMES "")
if(DEFINED bzip2_FIND_DEPENDENCY_NAMES)
  list(APPEND bzip2_FIND_DEPENDENCY_NAMES )
  list(REMOVE_DUPLICATES bzip2_FIND_DEPENDENCY_NAMES)
else()
  set(bzip2_FIND_DEPENDENCY_NAMES )
endif()

########### VARIABLES #######################################################################
#############################################################################################
set(bzip2_PACKAGE_FOLDER_RELEASE "/home/masaya/.conan2/p/bzip2f7e4c2f440dc6/p")
set(bzip2_BUILD_MODULES_PATHS_RELEASE "${bzip2_PACKAGE_FOLDER_RELEASE}/lib/cmake/conan-official-bzip2-variables.cmake")


set(bzip2_INCLUDE_DIRS_RELEASE )
set(bzip2_RES_DIRS_RELEASE )
set(bzip2_DEFINITIONS_RELEASE )
set(bzip2_SHARED_LINK_FLAGS_RELEASE )
set(bzip2_EXE_LINK_FLAGS_RELEASE )
set(bzip2_OBJECTS_RELEASE )
set(bzip2_COMPILE_DEFINITIONS_RELEASE )
set(bzip2_COMPILE_OPTIONS_C_RELEASE )
set(bzip2_COMPILE_OPTIONS_CXX_RELEASE )
set(bzip2_LIB_DIRS_RELEASE "${bzip2_PACKAGE_FOLDER_RELEASE}/lib")
set(bzip2_BIN_DIRS_RELEASE )
set(bzip2_LIBRARY_TYPE_RELEASE STATIC)
set(bzip2_IS_HOST_WINDOWS_RELEASE 0)
set(bzip2_LIBS_RELEASE bz2)
set(bzip2_SYSTEM_LIBS_RELEASE )
set(bzip2_FRAMEWORK_DIRS_RELEASE )
set(bzip2_FRAMEWORKS_RELEASE )
set(bzip2_BUILD_DIRS_RELEASE )
set(bzip2_NO_SONAME_MODE_RELEASE FALSE)


# COMPOUND VARIABLES
set(bzip2_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${bzip2_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${bzip2_COMPILE_OPTIONS_C_RELEASE}>")
set(bzip2_LINKER_FLAGS_RELEASE
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${bzip2_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${bzip2_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${bzip2_EXE_LINK_FLAGS_RELEASE}>")


set(bzip2_COMPONENTS_RELEASE )