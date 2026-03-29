ARG UBUNTU_DISTRO="22.04"

FROM carla-base:ue5-${UBUNTU_DISTRO}

ARG UBUNTU_DISTRO

ARG UID="1000"
ARG GID="1000"
ARG DOCKER_GID="999"

ARG USERNAME="carla"

# Disable interactive prompts during package installation.
ENV DEBIAN_FRONTEND=noninteractive

# Install sudo if needed for privileged commands.
RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        sudo \
    && rm -rf /var/lib/apt/lists/*

# Install development utility tools
# - vulkan-tools: for testing Vulkan rendering
# - fontconfig: required for loading system fonts (e.g., in manual_control.py)
# - xdg-user-dirs: so the Unreal Engine can use it to locate the user's Documents directory
RUN packages="vulkan-tools fontconfig xdg-user-dirs" && \
    apt-get update && \
    apt-get install -y $packages && \
    rm -rf /var/lib/apt/lists/*

ENV XDG_RUNTIME_DIR=/run/user/${UID}

# Install runtime python libraries (to run examples and utils)
COPY .tmp/examples_requirements.txt /tmp/examples_requirements.txt
COPY .tmp/util_requirements.txt /tmp/util_requirements.txt

RUN python3 -m pip install -r /tmp/examples_requirements.txt \
    && python3 -m pip install -r /tmp/util_requirements.txt \
    && rm /tmp/examples_requirements.txt /tmp/util_requirements.txt

# Starting with Ubuntu 23.04, official Docker images include a default `ubuntu` user with UID 1000.
# This can cause conflicts when remapping the container's UID/GID to match the host user.
RUN id -u ${UID} &>/dev/null \
    && userdel -r $(getent passwd ${UID} | cut -d: -f1) \
    || echo ""

# Create a dedicated non-root user and group to limit root access.
# Add the user to the sudoers group and configure it password-less.
RUN groupadd --gid ${GID} ${USERNAME} \
    && useradd -m --uid ${UID} -g ${USERNAME} ${USERNAME} \
    && passwd -d ${USERNAME} \
    && usermod -a -G sudo ${USERNAME}

# Add the carla user to the docker group to allow running Docker commands without sudo when bind-mounting the Docker socket.
RUN groupadd -g ${DOCKER_GID} docker \
    && usermod -a -G docker ${USERNAME}

USER ${USERNAME}

ENV HOME="/home/${USERNAME}"
WORKDIR /workspaces
