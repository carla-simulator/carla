FROM ubuntu:22.04

# ------------------------------------------------------------------------------
# Build arguments for user/group
# ------------------------------------------------------------------------------
ARG USERNAME=carla
ARG USER_ID=1000
ARG GROUP_ID=1000

# Place the appropiate credentials in .env file at the root of this repo
ARG EPIC_USER="username"
ARG EPIC_PASS="github_token"

# ------------------------------------------------------------------------------
# Set environment variables for NVIDIA support
# ------------------------------------------------------------------------------
ENV NVIDIA_DRIVER_CAPABILITIES=all
ENV NVIDIA_VISIBLE_DEVICES=all
    
# ------------------------------------------------------------------------------
# Set the XDG_RUNTIME_DIR environment variable for ./Setup.sh
# ------------------------------------------------------------------------------
ENV XDG_RUNTIME_DIR=/run/user/1000

# ------------------------------------------------------------------------------
# Explicitly tell Vulkan to use NVIDIA's ICD
# ------------------------------------------------------------------------------
ENV VK_ICD_FILENAMES=/usr/share/vulkan/icd.d/nvidia_icd.json

# ------------------------------------------------------------------------------
# Create or rename user/group "$USERNAME"
# so that we end up with:
# - group "$USERNAME" at GID=$GROUP_ID
# - user "$USERNAME"  at UID=$USER_ID
# ------------------------------------------------------------------------------
RUN if [ -z "$(getent group $GROUP_ID)" ]; then \
      groupadd -g $GROUP_ID "$USERNAME"; \
    else \
      existing_group="$(getent group $GROUP_ID | cut -d: -f1)"; \
      if [ "$existing_group" != "$USERNAME" ]; then \
        groupmod -n "$USERNAME" "$existing_group"; \
      fi; \
    fi && \
    if [ -z "$(getent passwd $USER_ID)" ]; then \
      useradd -m -u $USER_ID -g $GROUP_ID "$USERNAME"; \
    else \
      existing_user="$(getent passwd $USER_ID | cut -d: -f1)"; \
      if [ "$existing_user" != "$USERNAME" ]; then \
        usermod -l "$USERNAME" -d /home/"$USERNAME" -m "$existing_user"; \
      fi; \
    fi

# ------------------------------------------------------------------------------
# Install Unreal Engine build dependencies
# ------------------------------------------------------------------------------
RUN apt-get update && \
    apt-get install -y \
    git \
    ca-certificates \
    curl \
    wget \
    build-essential \
    xdg-user-dirs \
    xdg-utils \
    sudo \
    libvulkan1 \
    mesa-vulkan-drivers \
    libsdl2-dev \
    vulkan-tools
    # (optional) vulkan-tools -> useful for debugging

# ------------------------------------------------------------------------------
# Install CARLA build dependencies (make PythonAPI, make CarlaUE4Editor)
# ------------------------------------------------------------------------------
RUN apt-get install -y \
    cmake \
    ninja-build \
    python3 \
    python3-dev \
    python3-setuptools \
    python3-distro \
    python3-wheel \
    libtiff5-dev \
    libjpeg-dev \
    autoconf \
    rsync \
    unzip

# ------------------------------------------------------------------------------
# (Optional) Install CARLA build packaging depenencies(make build.utils)
# ------------------------------------------------------------------------------
RUN apt-get install -y \
    libxml2-dev 

# ------------------------------------------------------------------------------
# Cleanup
# ------------------------------------------------------------------------------
RUN rm -rf /var/lib/apt/lists/*

# ------------------------------------------------------------------------------
# Optionally add passwordless sudo for $USERNAME
# ------------------------------------------------------------------------------
RUN echo "$USERNAME ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

# ------------------------------------------------------------------------------
# Use sed to uncomment the force_color_prompt line in ~/.bashrc
# ------------------------------------------------------------------------------
RUN sed -i 's/#force_color_prompt=yes/force_color_prompt=yes/g' /home/$USERNAME/.bashrc

# ------------------------------------------------------------------------------
# Create the UE4 directory with the right ownership
# ------------------------------------------------------------------------------
RUN mkdir -p /opt/UE4.26 && \
    chown -R $USERNAME:$USERNAME /opt/UE4.26

ENV UE4_ROOT="/opt/UE4.26"

# ------------------------------------------------------------------------------
# Create the repo mount directory with the right ownership
# ------------------------------------------------------------------------------
RUN mkdir -p /workspaces && \
    chown -R $USERNAME:$USERNAME /workspaces

# ------------------------------------------------------------------------------
# Switch to "$USERNAME" by default
# ------------------------------------------------------------------------------
USER $USERNAME
    
# ------------------------------------------------------------------------------
# Install Unreal Engine 4.26
# ------------------------------------------------------------------------------
RUN git clone --depth 1 -b carla "https://${EPIC_USER}:${EPIC_PASS}@github.com/CarlaUnreal/UnrealEngine.git" ${UE4_ROOT}
      
RUN cd $UE4_ROOT && \
    ./Setup.sh && \
    ./GenerateProjectFiles.sh && \
    make

ENV CC=/opt/UE4.26/Engine/Extras/ThirdPartyNotUE/SDKs/HostLinux/Linux_x64/v17_clang-10.0.1-centos7/x86_64-unknown-linux-gnu/bin/clang
ENV CXX=/opt/UE4.26/Engine/Extras/ThirdPartyNotUE/SDKs/HostLinux/Linux_x64/v17_clang-10.0.1-centos7/x86_64-unknown-linux-gnu/bin/clang++

# ------------------------------------------------------------------------------
# Set repo working directory
# ------------------------------------------------------------------------------
WORKDIR /workspaces

# ------------------------------------------------------------------------------
# Install CARLA 0.9.15.2
# ------------------------------------------------------------------------------
ARG CARLA_GIT_TAG="0.9.15.2"
ARG BRANCH="feature/carla-${CARLA_GIT_TAG}-jammy-devcontainer"
ARG CLONE_DIR="carla-${CARLA_GIT_TAG}"

RUN git clone --depth 1 --branch ${BRANCH} https://github.com/wambitz/carla.git ${CLONE_DIR}

# Change working directory
WORKDIR /workspaces/${CLONE_DIR}

# NOTE: Don't run these commands together as Update.sh truncates the output
RUN ./Update.sh
RUN make PythonAPI 
RUN make CarlaUE4Editor 
RUN make build.utils 
RUN make package 

# ------------------------------------------------------------------------------
# Entry point
# ------------------------------------------------------------------------------
CMD ["/bin/bash"]
