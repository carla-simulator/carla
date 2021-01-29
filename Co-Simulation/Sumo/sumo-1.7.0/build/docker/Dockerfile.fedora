FROM fedora:rawhide

RUN dnf -y upgrade
RUN dnf -y install proj-devel xerces-c-devel git cmake make gcc-c++
RUN dnf -y install fox-devel freetype-devel fontconfig-devel libXft-devel libXcursor-devel libXrandr-devel libXi-devel libjpeg-devel libtiff-devel libGLU-devel
# optional libs
RUN dnf -y install gdal-devel swig python-devel java-devel gl2ps-devel OpenSceneGraph-devel gtest-devel
RUN git clone https://github.com/eclipse/sumo
RUN cd sumo; mkdir cmake-build; cd cmake-build; cmake .. && make -j && make install

