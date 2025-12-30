ARG UBUNTU_DISTRO="20.04"

FROM carla-base:ue4-${UBUNTU_DISTRO}

ARG UBUNTU_DISTRO

ARG UID="1000"
ARG GID="1000"
ARG DOCKER_GID="999"

ARG USERNAME="carla"

# Disable interactive prompts during package installation.
ENV DEBIAN_FRONTEND=noninteractive

# Starting with Ubuntu 23.04, official Docker images include a default `ubuntu` user with UID 1000.
# This can cause conflicts when remapping the container's UID/GID to match the host user, which often also uses UID 1000.
# To prevent these conflicts, we remove the `ubuntu` user from the container.
RUN id -u ${UID} &>/dev/null \
    && userdel -r $(getent passwd ${UID} | cut -d: -f1) \
    || echo ""

# Create a dedicated non-root user and group to limit root access.
RUN groupadd --gid ${GID} ${USERNAME} \
    && useradd -m --uid ${UID} -g ${USERNAME} ${USERNAME} \
    && passwd -d ${USERNAME}

# Add the carla user to the docker group to allow running Docker commands without sudo when bind-mounting the Docker socket.
# By default, the Docker group is created with GID 999, but this should be provided as a build argument to match the Docker group GID on the host system.
ARG DOCKER_GID=999
RUN groupadd -g ${DOCKER_GID} docker \
    && usermod -a -G docker ${USERNAME}

USER ${USERNAME}

ENV HOME="/home/${USERNAME}"
WORKDIR /workspaces
