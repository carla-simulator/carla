#!/usr/bin/env bash
set -e

# 1) Figure out script & project directories
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="${SCRIPT_DIR}/.."
DOCKERFILE="carla-ue4.dockerfile"

# 2) Load .env file one level up
if [ -f "${PROJECT_ROOT}/.env" ]; then
    export $(grep -v '^#' "${PROJECT_ROOT}/.env" | xargs)
else
    echo "Error: .env file not found in ${PROJECT_ROOT}. Please create one with EPIC_USER and EPIC_PASS."
    exit 1
fi

# Ensure credentials are loaded
if [ -z "${EPIC_USER}" ] || [ -z "${EPIC_PASS}" ]; then
    echo "Error: EPIC_USER or EPIC_PASS not set in the .env file."
    exit 1
fi

# 3) Define image name (customize as desired)
IMAGE_NAME="carla-0.9.15.2-ue4-jammy-dev"

echo "Building Docker image '${IMAGE_NAME}' from Dockerfile in '${PROJECT_ROOT}'"

# 4) Build the Docker image, passing the user info
docker build \
  -f "${PROJECT_ROOT}/${DOCKERFILE}" \
  -t "${IMAGE_NAME}" \
  --build-arg EPIC_USER="${EPIC_USER}" \
  --build-arg EPIC_PASS="${EPIC_PASS}" \
  --build-arg USERNAME="$(whoami)" \
  --build-arg USER_UID="$(id -u)" \
  --build-arg USER_GID="$(id -g)" \
  --progress=plain \
  "${PROJECT_ROOT}" 