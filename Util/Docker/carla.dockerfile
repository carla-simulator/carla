FROM ubuntu:22.04

# ------------------------------------------------------------------------------
# Build arguments for user/group
# ------------------------------------------------------------------------------
ARG USERNAME=carla
ARG USER_ID=1000
ARG GROUP_ID=1000

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
      groupadd --gid "$GROUP_ID" "$USERNAME"; \
    else \
      groupmod --new-name "$USERNAME" "$(getent group $GROUP_ID | cut -d: -f1)"; \
    fi && \
    if [ -z "$(getent passwd $USER_ID)" ]; then \
      useradd --uid "$USER_ID" --gid "$GROUP_ID" -m "$USERNAME"; \
    else \
      usermod --login "$USERNAME" --gid "$GROUP_ID" "$(getent passwd $USER_ID | cut -d: -f1)"; \
      if [ ! -d "/home/$USERNAME" ]; then \
        mkdir "/home/$USERNAME"; \
        chown "$USER_ID:$GROUP_ID" "/home/$USERNAME"; \
      fi; \
    fi

# ------------------------------------------------------------------------------
# Install Unreal Engine dependencies for GUI support in containers
# ------------------------------------------------------------------------------
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    ca-certificates \
    xdg-user-dirs \
    xdg-utils \
    sudo \
    libvulkan1 \
    mesa-vulkan-drivers \
    libsdl2-dev \
    vulkan-tools
    # (optional) vulkan-tools -> useful for debugging

# ------------------------------------------------------------------------------
# Install CARLA build dependencies
# ------------------------------------------------------------------------------
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    git \
    curl \
    wget \
    build-essential \
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
    rsync

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
# Create the UE4 directory env variable (if needed later)
# ------------------------------------------------------------------------------
ENV UE4_ROOT="/opt/UE4.26"

# ------------------------------------------------------------------------------
# Create the repo mount directory with the right ownership
# ------------------------------------------------------------------------------
RUN mkdir -p /workspace && \
    chown -R $USERNAME:$USERNAME /workspace

# ------------------------------------------------------------------------------
# Switch to "$USERNAME" by default and set working directory
# ------------------------------------------------------------------------------
USER $USERNAME
WORKDIR /workspace

CMD ["/bin/bash"]
