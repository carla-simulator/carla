FROM ubuntu:22.04

# ----------------------------
# Build arguments for user/group
# ----------------------------
ARG USERNAME=carla
ARG USER_ID=1000
ARG GROUP_ID=1000
ARG EPIC_USER
ARG EPIC_PASS

# ----------------------------
# Install CARLA dependencies
# ----------------------------
RUN apt-get update && \
    apt-get install -y wget software-properties-common && \
    add-apt-repository ppa:ubuntu-toolchain-r/test && \
    wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key| apt-key add 

RUN apt-add-repository "deb http://archive.ubuntu.com/ubuntu focal main universe" 

RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive \
    apt-get install -y \
    sudo \
    build-essential \
    clang-10 lld-10 \
    g++-7 \
    cmake \
    ninja-build \
    libvulkan1 \
    python \
    python-dev \
    python3-dev \
    python3-pip \
    libpng-dev \
    libtiff5-dev \
    libjpeg-dev \
    tzdata \
    sed \
    curl \
    unzip \
    autoconf \
    libtool \
    rsync \
    libxml2-dev \
    libsdl2-2.0-0 \
    git \
    git-lfs

RUN sudo update-alternatives --install /usr/bin/clang++ clang++ /usr/lib/llvm-10/bin/clang++ 180 && \
    sudo update-alternatives --install /usr/bin/clang clang /usr/lib/llvm-10/bin/clang 180 && \
    sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-7 180

ENV CC="/usr/bin/clang"
ENV CXX="/usr/bin/clang++"
# NOTE: Make sure Unreal Engine is mounted at this location
# ENV UE4_ROOT="/opt/UnrealEngine_4.26"

# ----------------------------
# Cleanup
# ----------------------------
RUN rm -rf /var/lib/apt/lists/*

# ----------------------------
# Create or rename user/group "$USERNAME"
# so that we end up with:
# - group "$USERNAME" at GID=$GROUP_ID
# - user "$USERNAME"  at UID=$USER_ID
# ----------------------------
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

  
# ----------------------------
# Install Unreal Engine 4.26
# ----------------------------

ENV UE4_ROOT /home/$USERNAME/UE4.26

RUN git clone --depth 1 -b carla "https://${EPIC_USER}:${EPIC_PASS}@github.com/CarlaUnreal/UnrealEngine.git" ${UE4_ROOT}
  
RUN cd $UE4_ROOT && \
  ./Setup.sh && \
  ./GenerateProjectFiles.sh && \
  make

# ----------------------------
# Optionally add passwordless sudo for $USERNAME
# ----------------------------
RUN echo "$USERNAME ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

# ----------------------------
# Use sed to uncomment the force_color_prompt line in ~/.bashrc
# ----------------------------
RUN sed -i 's/#force_color_prompt=yes/force_color_prompt=yes/g' /home/$USERNAME/.bashrc

# ----------------------------
# Switch to "$DEV_USERNAME" by default
# ----------------------------
USER "$USERNAME"
WORKDIR /home/"$USERNAME"

CMD ["/bin/bash"]
