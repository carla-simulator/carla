# to build this image run the following command
# $ docker build -t sumo:latest - < Dockerfile.ubuntu.latest
# to use it run (GUI applications won't work)
# $ docker run -it sumo:latest bash
# now you have a bash inside a docker container and can for instance run
# $ sumo -c $SUMO_HOME/docs/examples/sumo/busses/test.sumocfg

FROM ubuntu:bionic

ENV SUMO_HOME=/usr/share/sumo

RUN apt-get -y update; apt-get -y install software-properties-common; add-apt-repository ppa:sumo/stable
RUN apt-get -y update; apt-get -y install sumo sumo-tools sumo-doc
