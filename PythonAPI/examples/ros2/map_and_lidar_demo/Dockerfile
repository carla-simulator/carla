ARG BASE_IMAGE=carla-rviz-humble-fastdds
FROM ${BASE_IMAGE}
ARG CARLA_VERSION=0.10.0

# PEP 668: allow installing into the system python on newer bases (jazzy)
ENV PIP_BREAK_SYSTEM_PACKAGES=1

RUN apt-get update && \
    apt-get install -y --no-install-recommends python3-pip && \
    rm -rf /var/lib/apt/lists/*

COPY carla-*.whl /tmp/
# rclpy's path is supplied by the ROS image entrypoint at container run time.
# Validate the ABI, package version, and the APIs this demo relies on before
# tagging the image. This rejects a host-glibc wheel and an older CARLA wheel
# that merely happens to import successfully.
RUN python3 -m pip install --no-cache-dir /tmp/carla-*.whl && \
    CARLA_VERSION="${CARLA_VERSION}" python3 -c 'import importlib.metadata as metadata; import os; import carla; expected=os.environ["CARLA_VERSION"]; actual=metadata.version("carla"); assert actual == expected, f"expected CARLA {expected}, got {actual}"; assert hasattr(carla.World, "get_publish_tf"), "CARLA wheel lacks World.get_publish_tf"' && \
    rm -f /tmp/carla-*.whl

COPY ros2_native.py stack.json map_to_markers.py ego_tf_broadcaster.py cleanup.py launcher.sh /opt/carla/
RUN chmod +x /opt/carla/launcher.sh

CMD ["/opt/carla/launcher.sh"]
