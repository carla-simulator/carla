FROM carlasim/carla:0.9.13
USER root
RUN apt-get update
RUN apt-get install -y libjpeg-dev libtiff-dev
RUN apt-get install -y python3-pip
RUN apt-get install -y git
RUN apt-get install -y xdg-user-dirs xdg-utils
RUN apt-get install -y fontconfig
RUN apt-get install -y zip
RUN apt-get clean
USER carla
RUN python3 -m pip install Cython
RUN python3 -m pip install --upgrade pip
RUN python3 -m pip install -r /home/carla/PythonAPI/examples/requirements.txt
COPY requirements.txt /tmp/requirements.txt
RUN python3 -m pip install -r /tmp/requirements.txt
ENV PYTHONPATH "${PYTHONPATH}:/home/carla/PythonAPI/carla/dist/carla-0.9.13-py3.7-linux-x86_64.egg"
WORKDIR /home/carla
