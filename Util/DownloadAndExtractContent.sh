#! /bin/bash
CONTENT_URL=$1
CONTENT_ARCHIVE_PATH=$2
CONTENT_PATH=$3
set -e
echo "Downloading Carla Content from "$CONTENT_URL" to "$CONTENT_ARCHIVE_PATH"."
wget -c $CONTENT_LINK -O $CONTENT_ARCHIVE_PATH
echo "Extracting Carla Content from "$CONTENT_ARCHIVE_PATH" to "$CONTENT_PATH"."
tar -xzf $CONTENT_ARCHIVE_PATH -C $CONTENT_PATH
