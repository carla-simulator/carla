BUILD_DIR=build/

if [ -d $BUILD_DIR ];
then
    echo "Deleting existing ${BUILD_DIR} directory." 
    rm -r $BUILD_DIR
fi

echo "Creating ${BUILD_DIR} directory."
mkdir $BUILD_DIR
cd $BUILD_DIR

LIB_LIBOSMSCOUT_INCLUDE=/home/aollero/Downloads/libosmcout/libosmscout-master/libosmscout-map-svg/include/
LIB_LIBOSMSCOUT_LIBS=/home/aollero/Downloads/libosmcout/libosmscout-master/build/libosmscout-map-svg


cmake -DCMAKE_CXX_FLAGS="-std=c++17 -g -Og" ..
make

./osm-world-renderer