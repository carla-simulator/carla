FROM ubuntu:24.04 AS dependencies

USER root
SHELL ["/bin/bash", "-c"]

ARG CARLA_API_PATH=/opt/carla/PythonAPI

# Install essentials for running the CARLA PythonAPI client.
RUN apt-get update && \
        apt-get install -y \
            fontconfig \
            x11-apps \
            python3-pip \
            libomp5 \
            libjpeg-turbo8 \
            libtiff6 \
            libgeos-dev \
            git \
        && rm -rf /var/lib/apt/lists/*

# Use system pip (PEP 668): don't upgrade Debian-managed pip to avoid uninstall errors

# Move over PythonAPI
COPY ./artifacts/PythonAPI ${CARLA_API_PATH}
RUN rm -f ${CARLA_API_PATH}/carla/__init__.py

# Recursively install PythonAPI requirements, keep version of first occurrence
RUN cat $(find ${CARLA_API_PATH} -type f -name "requirements.txt") > /tmp/requirements_raw.txt \
    && awk -F '==' '{print $1}' /tmp/requirements_raw.txt | awk '!visited[$1]++' > /tmp/requirements.txt \
    && python3 -m pip install --break-system-packages -r /tmp/requirements.txt

# Install CARLA PythonAPI wheel
RUN set -e; \
    pyver=$(python3 -c "import sys; print(f'{sys.version_info.major}{sys.version_info.minor}')"); \
    wheel=$(echo ${CARLA_API_PATH}/carla/dist/*${pyver}*.whl); \
    python3 -m pip install --no-cache-dir --break-system-packages "$wheel"

# Create script that adds API to pythonpath and make .bashrc source it
RUN echo "export PYTHONPATH=\$PYTHONPATH:$CARLA_API_PATH/carla/agents" >> /setup_carla_env.sh; \
    echo "export PYTHONPATH=\$PYTHONPATH:$CARLA_API_PATH/carla" >> /setup_carla_env.sh; \
    echo "export CARLA_API_PATH=$CARLA_API_PATH" >> /setup_carla_env.sh; \
    echo "source /setup_carla_env.sh" >> ~/.bashrc

# Default file cache for CARLA client-side map files
ENV CARLA_CACHE_DIR=/tmp/carlaCache
RUN mkdir -p ${CARLA_CACHE_DIR} \
    && chmod 777 ${CARLA_CACHE_DIR}

# Needed for (pygame based) scripts that have a GUI
ENV SDL_VIDEODRIVER=x11

USER root
SHELL ["/bin/bash", "-c"]

ARG WORKSPACE=${CARLA_API_PATH}
ENV WORKSPACE=${WORKSPACE}
WORKDIR ${WORKSPACE}

# Set entrypoint
COPY ./Util/Docker/client/entrypoint.sh /
ENTRYPOINT [ "/entrypoint.sh" ]

CMD [ "python3", "util/test_connection.py" ]
