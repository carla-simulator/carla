FROM ubuntu:22.04

RUN apt update -y
RUN apt install build-essential -y
RUN apt install make
RUN apt install ninja-build
RUN apt install libvulkan1 -y
RUN apt install python3
RUN apt install python3-dev -y
RUN apt install python3-pip -y
RUN apt install libpng-dev -y
RUN apt install libtiff5-dev -y
RUN apt install libjpeg-dev
RUN DEBIAN_FRONTEND=noninteractive apt install tzdata
RUN apt install sed
RUN apt install curl -y
RUN apt install libtool -y
RUN apt install rsync -y
RUN apt install libxml2-dev -y
RUN apt install git -y
RUN apt install git-lfs

RUN curl -L -O https://github.com/Kitware/CMake/releases/download/v3.28.3/cmake-3.28.3-linux-x86_64.tar.gz
RUN mkdir -p /opt
RUN tar -xzf cmake-3.28.3-linux-x86_64.tar.gz -C /opt
RUN echo -e '\n#CARLA CMake 3.28.3\nPATH=/opt/cmake-3.28.3-linux-x86_64/bin:$PATH' >> ~/.bashrc
RUN export PATH=/opt/cmake-3.28.3-linux-x86_64/bin:$PATH
RUN rm -rf cmake-3.28.3-linux-x86_64.tar.gz

#RUN pip3 install --upgrade pip
#COPY ../requirements.txt .
#RUN pip3 install -r requirements.txt
