SCRIPT_RELATIVE_PATH=$(basename $PWD)
pushd ..
docker build -t carla-ubuntu-22.04 --build-arg LOCALE_LANG=$LANG -f $SCRIPT_RELATIVE_PATH/Dockerfile .
popd
