# to build this image run the following command
# $ docker build -t sumo - < Dockerfile.ubuntu.git
# to use it run (GUI applications won't work)
# $ docker run -it sumo bash
# now you have a bash inside a docker container and can for instance run
# $ cd /opt/sumo; bin/sumo -c docs/examples/sumo/busses/test.sumocfg

FROM ubuntu:bionic

ENV SUMO_HOME=/opt/sumo
ENV PATH=/opt/sumo/bin:$PATH
ENV LD_LIBRARY_PATH=/opt/sumo/bin

RUN apt-get -y update
RUN apt-get -y install psmisc vim git cmake autoconf automake libtool libxerces-c-dev libfox-1.6-dev libgl1-mesa-dev libglu1-mesa-dev libgdal-dev libproj-dev python-pip libgl2ps-dev swig python3-dev libgtest-dev
RUN cd /usr/src/gtest; cmake CMakeLists.txt; make; cp *.a /usr/lib
RUN pip install texttest

RUN cd /opt; git clone --recursive https://github.com/eclipse/sumo
RUN cd /opt/sumo; mkdir build/cmake-build; cd build/cmake-build; cmake ../.. -DSUMO_UTILS=TRUE; make -j8 all examples
