ARG UBUNTU_DISTRO="20.04"

FROM carla-base:ue4-${UBUNTU_DISTRO} AS development

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
# - libxml2-dev: CARLA build packaging depenencies(make build.utils)
# - xdg-user-dirs: so the Unreal Engine can use it to locate the user's Documents directory
RUN packages="vulkan-tools fontconfig libxml2-dev xdg-user-dirs" && \
    apt-get update && \
    apt-get install -y $packages && \
    rm -rf /var/lib/apt/lists/*

ENV XDG_RUNTIME_DIR=/run/user/${UID}

# Install runtime python libraries (to run examples and utils)
USER root

COPY .tmp/examples_requirements.txt /requirements/examples_requirements.txt
COPY .tmp/util_requirements.txt /requirements/util_requirements.txt

RUN python3.8 -m pip install -r requirements/examples_requirements.txt
RUN python3.8 -m pip install -r requirements/util_requirements.txt

RUN python3.9 -m pip install -r requirements/examples_requirements.txt
RUN python3.9 -m pip install -r requirements/util_requirements.txt

RUN python3.10 -m pip install -r requirements/examples_requirements.txt
RUN python3.10 -m pip install -r requirements/util_requirements.txt

RUN python3.11 -m pip install -r requirements/examples_requirements.txt
RUN python3.11 -m pip install -r requirements/util_requirements.txt

RUN python3.12 -m pip install -r requirements/examples_requirements.txt
RUN python3.12 -m pip install -r requirements/util_requirements.txt

# Starting with Ubuntu 23.04, official Docker images include a default `ubuntu` user with UID 1000.
# This can cause conflicts when remapping the container's UID/GID to match the host user, which often also uses UID 1000.
# To prevent these conflicts, we remove the `ubuntu` user from the container.
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
# By default, the Docker group is created with GID 999, but this should be provided as a build argument to match the Docker group GID on the host system.
RUN echo ${DOCKER_GID}
RUN groupadd -g ${DOCKER_GID} docker \
    && usermod -a -G docker ${USERNAME}

USER ${USERNAME}

ENV HOME="/home/${USERNAME}"
WORKDIR /workspaces

FROM development AS monolith

USER ${USERNAME}

# Built engine
ENV UE4_ROOT="/workspaces/unreal-egine"
RUN --mount=type=secret,id=epic_user,uid=${UID} \
    --mount=type=secret,id=epic_token,uid=${UID} \
    bash /build_scripts/build_ue4.sh \
      --ue4-root ${UE4_ROOT} \
      --epic-user $(cat /run/secrets/epic_user) \
      --epic-token $(cat /run/secrets/epic_token)

# Built carla
ENV CARLA_UE4_ROOT="/workspaces/carla"
ARG BRANCH=ue4-dev
RUN git clone --depth 1 --branch ${BRANCH} https://github.com/carla-simulator/carla.git ${CARLA_UE4_ROOT}

WORKDIR ${CARLA_UE4_ROOT}

# NOTE: Don't run these commands together as Update.sh truncates the output
RUN ./Update.sh
RUN make PythonAPI
RUN make CarlaUE4Editor
RUN make package && rm -rf ${CARLA_UE4_ROOT}/Dist
