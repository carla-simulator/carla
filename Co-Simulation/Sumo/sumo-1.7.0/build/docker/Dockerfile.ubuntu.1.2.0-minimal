# This docker file does not work yet due to a bug in the 1.2.0 release! It is just a template for future versions.
# to build this image run the following command
# $ docker build -t sumo:$SUMO_VERSION-minimal - < Dockerfile.ubuntu.$SUMO_VERSION-minimal
# to use it run (GUI applications won't work)
# $ docker run -it sumo:$SUMO_VERSION-minimal bash
# now you have a bash inside a docker container and can for instance run
# $ cd /opt/sumo; bin/sumo -c docs/examples/sumo/busses/test.sumocfg

FROM ubuntu:bionic

ENV SUMO_HOME=/opt/sumo
ENV SUMO_VERSION=1.2.0
ENV PATH=/opt/sumo/bin:$PATH

RUN apt-get -y update
RUN apt-get -y install wget cmake libxerces-c-dev

RUN cd /opt; wget -q http://downloads.sourceforge.net/project/sumo/sumo/version%20$SUMO_VERSION/sumo-src-$SUMO_VERSION.tar.gz; tar xzf sumo-src-$SUMO_VERSION.tar.gz; mv sumo-$SUMO_VERSION sumo
RUN cd /opt/sumo; mkdir build/cmake-build; cd build/cmake-build; cmake ../.. -DSUMO_UTILS=TRUE; make -j8 all examples
