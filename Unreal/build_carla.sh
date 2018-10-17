#!/bin/bash

set -u

my_dir=$(dirname $(readlink -f "$0"))
parent_dir=$(dirname $my_dir)
unreal_branch="4.19"
image_tag="ue4:$unreal_branch"
watches="524288"

docker inspect $image_tag > /dev/null 2>&1
if [ "$?" != "0" ]; then
    echo "Please run ./build_ue4.sh before running $0"
    exit 1
fi

pushd $parent_dir > /dev/null
./Update.sh
popd > /dev/null

# This ensures that during an unreal build you do not run out of inotify
# watches; otherwise you get a bunch of errors like the following:
#
# ... (errno = 28, Out of inotify watches, increase user.max_inotify_watches)
for n in fs.inotify.max_user_watches user.max_inotify_watches; do
    n_val=$(sysctl -n $n)
    if [ $n_val -lt $watches ]; then
        sudo sysctl -w $n $watches
    fi
done

nvidia-docker run --rm \
                  -v $parent_dir:/carla:rw \
                  -u root \
                  -it \
                  -v /tmp/.X11-unix:/tmp/.X11-unix \
                  --device /dev/snd \
                  -e DISPLAY=unix$DISPLAY \
                  --workdir /carla \
                  $image_tag bash
