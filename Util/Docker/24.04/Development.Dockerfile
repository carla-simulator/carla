FROM carla-base:ue5-24.04

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
COPY --from=carla-root PythonAPI/examples/requirements.txt /tmp/examples_requirements.txt
COPY --from=carla-root PythonAPI/util/requirements.txt /tmp/util_requirements.txt

RUN python3 -m pip install -r /tmp/examples_requirements.txt \
    && python3 -m pip install -r /tmp/util_requirements.txt \
    && rm /tmp/examples_requirements.txt /tmp/util_requirements.txt

# Ubuntu 24.04 official Docker images ship a default `ubuntu` user with UID 1000;
# remove it so the host user remap below can claim that UID.
RUN id -u ${UID} &>/dev/null \
    && userdel -r $(getent passwd ${UID} | cut -d: -f1) \
    || echo ""

# Create a dedicated non-root user and group to limit root access.
# Add the user to the sudoers group and configure it password-less.
RUN groupadd --gid ${GID} ${USERNAME} \
    && useradd -m --uid ${UID} -g ${USERNAME} ${USERNAME} \
    && passwd -d ${USERNAME} \
    && usermod -a -G sudo ${USERNAME}

# Add the carla user to a group with the host's docker GID so that bind-mounting
# the Docker socket lets the user run docker commands without sudo. On Ubuntu 24.04
# the requested GID may already be claimed by a system group (systemd-journal at
# GID 999 collides with the typical host docker GID); reuse the existing group
# in that case instead of failing to create a second one.
RUN if getent group ${DOCKER_GID} >/dev/null; then \
        existing_group=$(getent group ${DOCKER_GID} | cut -d: -f1); \
        usermod -a -G ${existing_group} ${USERNAME}; \
    else \
        groupadd -g ${DOCKER_GID} docker && usermod -a -G docker ${USERNAME}; \
    fi

USER ${USERNAME}

ENV HOME="/home/${USERNAME}"
WORKDIR /workspaces
