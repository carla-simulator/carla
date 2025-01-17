#!/usr/bin/env bash
set -e

# 1) Figure out script & project directories
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="${SCRIPT_DIR}/.."

# 2) Define image name (customize as desired)
IMAGE_NAME="carla-0.9.15.2-jammy-dev"

echo "Building Docker image '${IMAGE_NAME}' from Dockerfile in '${PROJECT_ROOT}'"

# 3) Build the Docker image, passing the user info
docker build \
  -f "${PROJECT_ROOT}/Dockerfile" \
  -t "${IMAGE_NAME}" \
  --build-arg USERNAME="$(whoami)" \
  --build-arg USER_UID="$(id -u)" \
  --build-arg USER_GID="$(id -g)" \
  "${PROJECT_ROOT}" 