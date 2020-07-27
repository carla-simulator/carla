set(netconvertlibs
        netwrite netimport netbuild foreign_eulerspiral ${GDAL_LIBRARY} netimport_vissim netimport_vissim_typeloader netimport_vissim_tempstructs ${commonlibs})

#add_subdirectory(foreign)
#add_subdirectory(netbuild)
#add_subdirectory(netimport)
#add_subdirectory(netwrite)
#add_subdirectory(utils)

#add_executable(netconvert netconvert_main.cpp)
#set_target_properties(netconvert PROPERTIES OUTPUT_NAME netconvert${BINARY_SUFFIX})
#set_target_properties(netconvert PROPERTIES OUTPUT_NAME_DEBUG netconvert${BINARY_SUFFIX}D)
#target_link_libraries(netconvert ${netconvertlibs})
#add_dependencies(netconvert generate-version-h)

#install(TARGETS netconvert RUNTIME DESTINATION bin)

set(converter_sources "Converter.cpp")
#add_library(converter STATIC ${converter_sources})
#target_link_libraries(converter ${netconvertlibs})
#install(TARGETS converter ARCHIVE DESTINATION lib)

configure_file(config.h.cmake config.h)

if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../.git")
    set(VERSION_DEP "${CMAKE_CURRENT_SOURCE_DIR}/../.git")
endif()

file(GLOB netbuild_sources "netbuild/*.cpp" "netbuild/*.h")
set(converter_sources "${converter_sources};${netbuild_sources}")

file(GLOB netimport_sources "netimport/*.cpp" "netimport/*.h")
set(converter_sources "${converter_sources};${netimport_sources}")

file(GLOB netimport_vissim_sources "netimport/vissim/*.cpp" "netimport/vissim/*.h")
set(converter_sources "${converter_sources};${netimport_vissim_sources}")

file(GLOB netimport_vissim_tempstructs_sources "netimport/vissim/tempstructs/*.cpp" "netimport/vissim/tempstructs/*.h")
set(converter_sources "${converter_sources};${netimport_vissim_tempstructs_sources}")

file(GLOB netimport_vissim_typeloader_sources "netimport/vissim/typeloader/*.cpp" "netimport/vissim/typeloader/*.h")
set(converter_sources "${converter_sources};${netimport_vissim_typeloader_sources}")

file(GLOB netwrite_sources "netwrite/*.cpp" "netwrite/*.h")
set(converter_sources "${converter_sources};${netwrite_sources}")

file(GLOB foreign_eulerspiral_sources "foreign/eulerspiral/*.cpp" "foreign/eulerspiral/*.h")
set(converter_sources "${converter_sources};${foreign_eulerspiral_sources}")

file(GLOB foreign_tcpip_sources "foreign/tcpip/*.cpp" "foreign/tcpip/*.h")
set(converter_sources "${converter_sources};${foreign_tcpip_sources}")

file(GLOB utils_common_sources "utils/common/*.cpp" "utils/common/*.h")
set(converter_sources "${converter_sources};${utils_common_sources}")

file(GLOB utils_distribution_sources "utils/distribution/*.cpp" "utils/distribution/*.h")
set(converter_sources "${converter_sources};${utils_distribution_sources}")

file(GLOB utils_shapes_sources "utils/shapes/*.cpp" "utils/shapes/*.h")
set(converter_sources "${converter_sources};${utils_shapes_sources}")

file(GLOB utils_options_sources "utils/options/*.cpp" "utils/options/*.h")
set(converter_sources "${converter_sources};${utils_options_sources}")

file(GLOB utils_xml_sources "utils/xml/*.cpp" "utils/xml/*.h")
set(converter_sources "${converter_sources};${utils_xml_sources}")

file(GLOB utils_geom_sources "utils/geom/*.cpp" "utils/geom/*.h")
set(converter_sources "${converter_sources};${utils_geom_sources}")

file(GLOB utils_importio_sources "utils/importio/*.cpp" "utils/importio/*.h")
set(converter_sources "${converter_sources};${utils_importio_sources}")

file(GLOB utils_iodevices_sources "utils/iodevices/*.cpp" "utils/iodevices/*.h")
set(converter_sources "${converter_sources};${utils_iodevices_sources}")

file(GLOB utils_traction_wire_sources "utils/traction_wire/*.cpp" "utils/traction_wire/*.h")
set(converter_sources "${converter_sources};${utils_traction_wire_sources}")

add_library(converter STATIC ${converter_sources})
target_link_libraries(converter ${XercesC_LIBRARIES} ${ZLIB_LIBRARIES} ${PROJ_LIBRARY})
if (WIN32)
    target_compile_definitions(converter PUBLIC "XERCES_STATIC_LIBRARY")
endif ()
install(TARGETS converter ARCHIVE DESTINATION lib)
install(FILES Converter.h DESTINATION include)