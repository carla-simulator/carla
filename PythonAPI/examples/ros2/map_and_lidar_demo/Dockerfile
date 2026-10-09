ARG BASE_IMAGE=carla-rviz-humble-fastdds
FROM ${BASE_IMAGE}

# PEP 668: allow installing into the system python on newer bases (jazzy)
ENV PIP_BREAK_SYSTEM_PACKAGES=1

RUN apt-get update && \
    apt-get install -y --no-install-recommends python3-pip && \
    rm -rf /var/lib/apt/lists/*

COPY carla-*.whl /tmp/
RUN python3 -m pip install --no-cache-dir /tmp/carla-*.whl && \
    rm -f /tmp/carla-*.whl

COPY ros2_native.py stack.json map_to_markers.py cleanup.py launcher.sh /opt/carla/
RUN chmod +x /opt/carla/launcher.sh

CMD ["/opt/carla/launcher.sh"]
