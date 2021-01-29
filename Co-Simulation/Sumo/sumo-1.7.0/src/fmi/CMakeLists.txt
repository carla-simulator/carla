
#########################################################
# Building of the libsumo-FMI2 library 
set (libsumofmi2_SRCS
  fmi2Functions.c 
  fmi2main.c 
  libsumocpp2c.cpp 
)

add_library(libsumofmi2 SHARED ${libsumofmi2_SRCS})
set_target_properties(libsumofmi2 PROPERTIES PREFIX "")
target_link_libraries(libsumofmi2 libsumocpp)

#########################################################
# Building of the ZIP Archive with the model description
# and the static library for the FMU

# The Library needs to go to a specific folder
# (binaries/win32 or binaries/win64 or ...)
# We need to determine the name for this folder

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(BITS "64")
else()
    set(BITS "32")
endif() 

if(WIN32) 
  set(PLATFORM_FOLDER_NAME "win${BITS}")
endif()

if(UNIX AND NOT APPLE)
  set(PLATFORM_FOLDER_NAME "linux${BITS}")
endif()
  
if(APPLE)
  set(PLATFORM_FOLDER_NAME "darwin${BITS}")
endif()

set(libsumofmi2_model_description modelDescription.xml)
set(libsumofmi2_lib_name "sumo-fmi2-${PLATFORM_FOLDER_NAME}.fmu")

# Set the temporary folder name to collect the contents for the ZIP archive
set(ZIP_BUILD_FOLDER_NAME "sumo-fmi2")

add_custom_target(create_fmi2_zip 
  # Remove the old directory (if exists) and create a new directory
  COMMAND ${CMAKE_COMMAND} -E rm -rf "${ZIP_BUILD_FOLDER_NAME}"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${ZIP_BUILD_FOLDER_NAME}"
  # Copy the XML file with the model description
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/${libsumofmi2_model_description}" "${ZIP_BUILD_FOLDER_NAME}"
  # Create the folder for the binaries and copy the library
  COMMAND ${CMAKE_COMMAND} -E make_directory "${ZIP_BUILD_FOLDER_NAME}/binaries/${PLATFORM_FOLDER_NAME}/"
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/bin/libsumofmi2${CMAKE_SHARED_LIBRARY_SUFFIX}" "${ZIP_BUILD_FOLDER_NAME}/binaries/${PLATFORM_FOLDER_NAME}/"
  # Build the ZIP archive
  COMMAND ${CMAKE_COMMAND} -E chdir "${ZIP_BUILD_FOLDER_NAME}" tar "cf" "${CMAKE_SOURCE_DIR}/bin/${libsumofmi2_lib_name}" --format=zip "."
)

add_dependencies(create_fmi2_zip libsumofmi2)

###########################################################
# Define the custom target FMI to have all of these files 
# built (also as part of ALL - but of course the happens
# only when FMI is enabled)
add_custom_target(fmi ALL)
add_dependencies(fmi libsumofmi2 create_fmi2_zip)
