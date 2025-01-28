#!/usr/bin/env bash
set -e

###############################################################################
# Usage:
#   ./build_image.sh [--monolith]
#
# If --monolith is supplied, it will:
#   - Use:       carla-ue4.dockerfile
#   - Tag:       carla-0.9.15.2-ue4-jammy-dev
#   - Pass EPIC creds as needed
#
# Otherwise (default/lightweight):
#   - Use:       carla.dockerfile
#   - Tag:       carla-0.9.15.2-jammy-dev
###############################################################################

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

MONOLITH=0
if [[ "$1" == "--monolith" ]]; then
    MONOLITH=1
    echo "[INFO] Monolithic build enabled."
fi

if [ $MONOLITH -eq 1 ]; then
    DOCKERFILE="carla-ue4.dockerfile"
    IMAGE_NAME="carla-0.9.15.2-ue4-jammy-dev"
else
    DOCKERFILE="carla.dockerfile"
    IMAGE_NAME="carla-0.9.15.2-jammy-dev"
fi

# ------------------------------------------------------------------------------
# Load .env file for EPIC_USER, EPIC_PASS, etc.
# (Optional for the lightweight version, but mandatory for monolith.)
# ------------------------------------------------------------------------------
if [ -f "${PROJECT_ROOT}/.env" ]; then
    # shellcheck disable=SC2046
    export $(grep -v '^#' "${PROJECT_ROOT}/.env" | xargs)
else
    echo "[WARN]: .env file not found in ${PROJECT_ROOT}."
    echo "If you are building the monolithic version, you need EPIC credentials!"
fi

if [ $MONOLITH -eq 1 ]; then
    # Ensure credentials are loaded for UE4
    if [ -z "${EPIC_USER}" ] || [ -z "${EPIC_PASS}" ]; then
        echo "[ERROR]: EPIC_USER or EPIC_PASS not set in the .env file. Aborting monolithic build."
        exit 1
    fi
fi

# ------------------------------------------------------------------------------
# Build the Docker image
# ------------------------------------------------------------------------------
echo "[INFO] Building Docker image '${IMAGE_NAME}' using Dockerfile '${DOCKERFILE}'"

# We always build in the project root so Docker sees the Dockerfile paths
docker build \
  -f "${PROJECT_ROOT}/${DOCKERFILE}" \
  -t "${IMAGE_NAME}" \
  --build-arg EPIC_USER="${EPIC_USER:-}" \
  --build-arg EPIC_PASS="${EPIC_PASS:-}" \
  --build-arg USERNAME="$(whoami)" \
  --build-arg USER_UID="$(id -u)" \
  --build-arg USER_GID="$(id -g)" \
  $([ $MONOLITH -eq 1 ] && echo "--progress=plain") \
  "${PROJECT_ROOT}"
