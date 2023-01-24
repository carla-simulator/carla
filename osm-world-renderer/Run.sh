BUILD_DIR=build/

if [ -d $BUILD_DIR ];
then
    echo "Deleting existing ${BUILD_DIR} directory." 
    rm -r $BUILD_DIR
fi

echo "Creating ${BUILD_DIR} directory."
mkdir $BUILD_DIR
cd $BUILD_DIR

cmake ..
make

./osm-world-renderer