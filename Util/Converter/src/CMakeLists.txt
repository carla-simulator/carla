set(netconvertlibs
        netwrite netimport netbuild foreign_eulerspiral ${GDAL_LIBRARY} netimport_vissim netimport_vissim_typeloader netimport_vissim_tempstructs ${commonlibs})

add_subdirectory(foreign)
add_subdirectory(netbuild)
add_subdirectory(netgen)
add_subdirectory(netimport)
add_subdirectory(netload)
add_subdirectory(netwrite)
add_subdirectory(polyconvert)
add_subdirectory(utils)

add_executable(netconvert netconvert_main.cpp)
set_target_properties(netconvert PROPERTIES OUTPUT_NAME netconvert${BINARY_SUFFIX})
set_target_properties(netconvert PROPERTIES OUTPUT_NAME_DEBUG netconvert${BINARY_SUFFIX}D)
target_link_libraries(netconvert ${netconvertlibs})
#add_dependencies(netconvert generate-version-h)

install(TARGETS netconvert RUNTIME DESTINATION bin)

configure_file(config.h.cmake config.h)

if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../.git")
    set(VERSION_DEP "${CMAKE_CURRENT_SOURCE_DIR}/../.git")
endif()
#add_custom_command(OUTPUT version.h
#                   COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../tools/build/version.py ${CMAKE_CURRENT_BINARY_DIR}
#                   DEPENDS ${VERSION_DEP}
#                  )
#add_custom_target(generate-version-h DEPENDS version.h)
#set_property(TARGET generate-version-h PROPERTY FOLDER "CMake")
