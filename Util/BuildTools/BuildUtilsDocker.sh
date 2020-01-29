echo "Building FBX2OBJ"
echo "----------------"

FBXSDK_URL=https://www.autodesk.com/content/dam/autodesk/www/adn/fbx/2020-0-1/fbx202001_fbxsdk_linux.tar.gz

cd Util/DockerUtils
mkdir dist
cp . dist/ -r
cd dist
cd fbx

echo "Download FBX SDK 2020"
wget -c ${FBXSDK_URL} -O fbx202001_fbxsdk_linux.tar.gz

echo "Unpacking"
tar -xvzf fbx202001_fbxsdk_linux.tar.gz
rm fbx202001_fbxsdk_linux.tar.gz

echo "Installing"
printf "y\nyes\nn\n" | ./fbx202001_fbxsdk_linux
rm fbx202001_fbxsdk_linux

echo "Compiling FBX2OBJ..."
mkdir build
cd build
cmake ..
make

echo "Copy binary FBX2OBJ"
mv FBX2OBJ ../..
cd ../..
rm -Rf build
