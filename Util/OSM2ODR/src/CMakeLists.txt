set(osm2odr_sources "OSM2ODR.cpp")

configure_file(config.h.cmake config.h)

if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../.git")
    set(VERSION_DEP "${CMAKE_CURRENT_SOURCE_DIR}/../.git")
endif()

file(GLOB netbuild_sources "netbuild/*.cpp" "netbuild/*.h")
set(osm2odr_sources "${osm2odr_sources};${netbuild_sources}")

file(GLOB netimport_sources "netimport/*.cpp" "netimport/*.h")
set(osm2odr_sources "${osm2odr_sources};${netimport_sources}")

file(GLOB netimport_vissim_sources "netimport/vissim/*.cpp" "netimport/vissim/*.h")
set(osm2odr_sources "${osm2odr_sources};${netimport_vissim_sources}")

file(GLOB netimport_vissim_tempstructs_sources "netimport/vissim/tempstructs/*.cpp" "netimport/vissim/tempstructs/*.h")
set(osm2odr_sources "${osm2odr_sources};${netimport_vissim_tempstructs_sources}")

file(GLOB netimport_vissim_typeloader_sources "netimport/vissim/typeloader/*.cpp" "netimport/vissim/typeloader/*.h")
set(osm2odr_sources "${osm2odr_sources};${netimport_vissim_typeloader_sources}")

file(GLOB netwrite_sources "netwrite/*.cpp" "netwrite/*.h")
set(osm2odr_sources "${osm2odr_sources};${netwrite_sources}")

file(GLOB foreign_eulerspiral_sources "foreign/eulerspiral/*.cpp" "foreign/eulerspiral/*.h")
set(osm2odr_sources "${osm2odr_sources};${foreign_eulerspiral_sources}")

file(GLOB foreign_tcpip_sources "foreign/tcpip/*.cpp" "foreign/tcpip/*.h")
set(osm2odr_sources "${osm2odr_sources};${foreign_tcpip_sources}")

file(GLOB utils_common_sources "utils/common/*.cpp" "utils/common/*.h")
set(osm2odr_sources "${osm2odr_sources};${utils_common_sources}")

file(GLOB utils_distribution_sources "utils/distribution/*.cpp" "utils/distribution/*.h")
set(osm2odr_sources "${osm2odr_sources};${utils_distribution_sources}")

file(GLOB utils_shapes_sources "utils/shapes/*.cpp" "utils/shapes/*.h")
set(osm2odr_sources "${osm2odr_sources};${utils_shapes_sources}")

file(GLOB utils_options_sources "utils/options/*.cpp" "utils/options/*.h")
set(osm2odr_sources "${osm2odr_sources};${utils_options_sources}")

file(GLOB utils_xml_sources "utils/xml/*.cpp" "utils/xml/*.h")
set(osm2odr_sources "${osm2odr_sources};${utils_xml_sources}")

file(GLOB utils_geom_sources "utils/geom/*.cpp" "utils/geom/*.h")
set(osm2odr_sources "${osm2odr_sources};${utils_geom_sources}")

file(GLOB utils_importio_sources "utils/importio/*.cpp" "utils/importio/*.h")
set(osm2odr_sources "${osm2odr_sources};${utils_importio_sources}")

file(GLOB utils_iodevices_sources "utils/iodevices/*.cpp" "utils/iodevices/*.h")
set(osm2odr_sources "${osm2odr_sources};${utils_iodevices_sources}")

file(GLOB utils_traction_wire_sources "utils/traction_wire/*.cpp" "utils/traction_wire/*.h")
set(osm2odr_sources "${osm2odr_sources};${utils_traction_wire_sources}")

add_library(osm2odr STATIC ${osm2odr_sources})
target_link_libraries(osm2odr ${XercesC_LIBRARIES} ${ZLIB_LIBRARIES} ${PROJ_LIBRARY})

target_compile_definitions(osm2odr PUBLIC "XERCES_STATIC_LIBRARY")

install(TARGETS osm2odr ARCHIVE DESTINATION lib)
install(FILES OSM2ODR.h DESTINATION include)
