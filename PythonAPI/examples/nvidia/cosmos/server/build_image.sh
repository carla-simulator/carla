#! /usr/bin/env bash
# Build the carla-cosmos image.
#
#   ./build_image.sh --nano|--full [--tag carla-cosmos:nano] [--hf-cache ~/.cache/huggingface]
#                    [--hf-token-file ~/.hf_token] [--no-models] [--push] [--platform linux/amd64]
#
#   --hf-cache      reuse weights from a local Hugging Face cache (HF_HOME) instead of downloading
#   --hf-token-file token for gated NVIDIA repos when downloading (or set HF_TOKEN)
#   --no-models     code-only image (~35 GB, for CI / API development); workers will fail to load models
#
# Needs Docker BuildKit (docker buildx). Final size: :nano ≈ 170 GB, :full ≈ 303 GB, -nomodels 66 GB; while
# building, BuildKit holds the weights up to three times (context, models stage, runtime layer), so plan for
# ≈ 360 GB (:nano) / ≈ 770 GB (:full) free in the Docker root — or build --no-models here and add the weights
# with tools/bake_weights.sh (≈ 2× one 40 GB layer of headroom).
set -euo pipefail

HERE=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
CONTEXT=$(realpath "$HERE/..")          # PythonAPI/examples/nvidia/cosmos (client/ + server/)
VARIANT=""
TAG=""
HF_CACHE=""
TOKEN_FILE=""
WITH_MODELS=1
PUSH=0
PLATFORM=linux/amd64
EXTRA=()

while [ $# -gt 0 ]; do
    case "$1" in
        --nano) VARIANT=nano; shift ;;
        --full) VARIANT=full; shift ;;
        --tag) TAG=$2; shift 2 ;;
        --hf-cache) HF_CACHE=$(realpath "$2"); shift 2 ;;
        --hf-token-file) TOKEN_FILE=$2; shift 2 ;;
        --no-models) WITH_MODELS=0; shift ;;
        --push) PUSH=1; shift ;;
        --platform) PLATFORM=$2; shift 2 ;;
        --build-arg) EXTRA+=(--build-arg "$2"); shift 2 ;;
        -h|--help) sed -n '2,12p' "$0"; exit 0 ;;
        *) echo "unknown option $1" >&2; exit 1 ;;
    esac
done
[ -n "$VARIANT" ] || { echo "pass --nano or --full" >&2; exit 1; }
TAG=${TAG:-carla-cosmos:$VARIANT}
[ "$WITH_MODELS" = 1 ] || TAG=${TAG}-nomodels

ARGS=(buildx build --file "$HERE/Dockerfile" --platform "$PLATFORM" --tag "$TAG" --progress=plain
      --build-arg "IMAGE_VARIANT=$VARIANT" --build-arg "WITH_MODELS=$WITH_MODELS" "${EXTRA[@]}")
# The Dockerfile always mounts the hfcache build context; provide an empty one when not given.
if [ -n "$HF_CACHE" ]; then
    [ -d "$HF_CACHE/hub" ] || { echo "$HF_CACHE has no hub/ directory (expected an HF_HOME)" >&2; exit 1; }
    ARGS+=(--build-context "hfcache=$HF_CACHE")
else
    EMPTY=$(mktemp -d); trap 'rm -rf "$EMPTY"' EXIT
    ARGS+=(--build-context "hfcache=$EMPTY")
fi
if [ -n "$TOKEN_FILE" ]; then
    ARGS+=(--secret "id=hf_token,src=$TOKEN_FILE")
elif [ -n "${HF_TOKEN:-}" ]; then
    ARGS+=(--secret "id=hf_token,env=HF_TOKEN")
fi
[ "$PUSH" = 1 ] && ARGS+=(--push) || ARGS+=(--load)

echo "building $TAG (variant=$VARIANT, models=$WITH_MODELS) from $CONTEXT"
exec docker "${ARGS[@]}" "$CONTEXT"
