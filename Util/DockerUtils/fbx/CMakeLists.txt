# use: ./build/cmake .. -A x64
# Download from: https://www.autodesk.com/content/dam/autodesk/www/adn/fbx/2020-0-1/fbx202001_fbxsdk_linux.tar.gz
# Dependencies:
#	sudo apt-get install libxml2-dev

cmake_minimum_required(VERSION 2.8.9)
project(FBX2OBJ)

set(CMAKE_GENERATOR_PLATFORM x64)

#find_package(LibXml2 REQUIRED)

# include folders
include_directories(src)
include_directories(dependencies/include)

# library folders
link_directories(dependencies/lib/gcc/x64/release)

# sources
file(GLOB SOURCES "src/*.cpp")
add_executable(FBX2OBJ ${SOURCES})

# install location
install(TARGETS FBX2OBJ DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/../dist")

# libraries to link
target_link_libraries(FBX2OBJ fbxsdk)
target_link_libraries(FBX2OBJ dl)
target_link_libraries(FBX2OBJ z)
target_link_libraries(FBX2OBJ xml2)
