########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

set(libbacktrace_COMPONENT_NAMES "")
if(DEFINED libbacktrace_FIND_DEPENDENCY_NAMES)
  list(APPEND libbacktrace_FIND_DEPENDENCY_NAMES )
  list(REMOVE_DUPLICATES libbacktrace_FIND_DEPENDENCY_NAMES)
else()
  set(libbacktrace_FIND_DEPENDENCY_NAMES )
endif()

########### VARIABLES #######################################################################
#############################################################################################
set(libbacktrace_PACKAGE_FOLDER_RELEASE "/home/masaya/.conan2/p/libbac86a5d99cc0ec/p")
set(libbacktrace_BUILD_MODULES_PATHS_RELEASE )


set(libbacktrace_INCLUDE_DIRS_RELEASE )
set(libbacktrace_RES_DIRS_RELEASE )
set(libbacktrace_DEFINITIONS_RELEASE )
set(libbacktrace_SHARED_LINK_FLAGS_RELEASE )
set(libbacktrace_EXE_LINK_FLAGS_RELEASE )
set(libbacktrace_OBJECTS_RELEASE )
set(libbacktrace_COMPILE_DEFINITIONS_RELEASE )
set(libbacktrace_COMPILE_OPTIONS_C_RELEASE )
set(libbacktrace_COMPILE_OPTIONS_CXX_RELEASE )
set(libbacktrace_LIB_DIRS_RELEASE "${libbacktrace_PACKAGE_FOLDER_RELEASE}/lib")
set(libbacktrace_BIN_DIRS_RELEASE )
set(libbacktrace_LIBRARY_TYPE_RELEASE STATIC)
set(libbacktrace_IS_HOST_WINDOWS_RELEASE 0)
set(libbacktrace_LIBS_RELEASE backtrace)
set(libbacktrace_SYSTEM_LIBS_RELEASE )
set(libbacktrace_FRAMEWORK_DIRS_RELEASE )
set(libbacktrace_FRAMEWORKS_RELEASE )
set(libbacktrace_BUILD_DIRS_RELEASE )
set(libbacktrace_NO_SONAME_MODE_RELEASE FALSE)


# COMPOUND VARIABLES
set(libbacktrace_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${libbacktrace_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${libbacktrace_COMPILE_OPTIONS_C_RELEASE}>")
set(libbacktrace_LINKER_FLAGS_RELEASE
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${libbacktrace_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${libbacktrace_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${libbacktrace_EXE_LINK_FLAGS_RELEASE}>")


set(libbacktrace_COMPONENTS_RELEASE )