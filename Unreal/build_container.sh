#!/bin/bash

set -e
set -u

build_script=$(readlink -f "$0")
build_dir=$(dirname $build_script)
unreal_branch="4.19"
target_dir="$build_dir/UE4"
clone_depth=1
image_tag="ue4:$unreal_branch"

if [ ! -d "$target_dir" ]; then
    # This requires that you have signed up and gotten access to the repo...
    # which though not hard, does require some work.
    git clone -b $unreal_branch --depth $clone_depth git@github.com:EpicGames/UnrealEngine.git $target_dir
fi

# Seems better to run this outside (vs all the time inside the image).
pushd $target_dir > /dev/null
bash Engine/Build/BatchFiles/Linux/GitDependencies.sh
popd > /dev/null

nvidia-docker build -t "$image_tag" \
                       --label "ue4.version=$unreal_branch" \
                       -f Dockerfile \
                       $build_dir
