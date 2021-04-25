#! /bin/bash

cd ~/Downloads/ && \
wget https://www.blender.org/download/Blender2.92/blender-2.92.0-linux64.tar.xz/ && \
tar xfJv blender-2.92.0-linux64.tar.xz && \
echo "export PATH=\"$PATH:~/Downloads/blender-2.92.0-linux64\"" >> ~/.bashrc && \
source ~/.bashrc
