#!/usr/bin/env bash
# Bake the model weights into a "-nomodels" carla-cosmos image one layer at a time (docker commit).
#
# The Dockerfile path (build_image.sh) holds the weights ~3x on the build host while BuildKit copies the
# build context, the models stage and the runtime layer.  This script needs ~2x the *shard* size instead
# (default 40 GB shards) and works on hosts with a few hundred GB free.  Layers stay small enough to be
# pushed/pulled resumably.  Each shard is placed by server/prefetch.py --select (sizes and sha256 checked
# against artifacts.lock) from one or more local Hugging Face caches; nothing is downloaded.
#
#   tools/bake_weights.sh --variant nano --base carla-cosmos:nano-nomodels --tag carla-cosmos:nano \
#                         --hf-cache ~/.cache/huggingface
#   tools/bake_weights.sh --variant full --base carla-cosmos:nano --tag carla-cosmos:full \
#                         --hf-cache ~/.cache/huggingface --hf-cache /var/tmp/cosmos-hf-cache
#
# --base may already carry weights (e.g. :nano when baking :full): artifacts recorded in its
# /models/hf/ARTIFACTS_IMAGE are skipped, so :full = :nano + Cosmos3-Super layers.
set -euo pipefail

VARIANT= BASE= TAG= MAX_GB=40 NO_HASH= ONLY= CACHES=()
while [ $# -gt 0 ]; do
    case "$1" in
        --variant) VARIANT=$2; shift 2 ;;
        --base) BASE=$2; shift 2 ;;
        --tag) TAG=$2; shift 2 ;;
        --hf-cache) CACHES+=("$(realpath "$2")"); shift 2 ;;
        --max-layer-gb) MAX_GB=$2; shift 2 ;;
        --no-hash) NO_HASH=--no-hash; shift ;;
        --only) ONLY=$2; shift 2 ;;
        -h|--help) sed -n '2,20p' "$0"; exit 0 ;;
        *) echo "unknown argument: $1" >&2; exit 2 ;;
    esac
done
USER_ONLY=$ONLY
[ -n "$VARIANT" ] && [ -n "$BASE" ] && [ ${#CACHES[@]} -gt 0 ] || { echo "need --variant, --base and at least one --hf-cache" >&2; exit 2; }
TAG=${TAG:-carla-cosmos:$VARIANT}
for c in "${CACHES[@]}"; do [ -d "$c/hub" ] || { echo "$c has no hub/ directory (expected an HF_HOME)" >&2; exit 1; }; done

SERVER_DIR=$(cd "$(dirname "$0")/.." && pwd)
PY=/opt/venvs/transfer25/bin/python
WORK=$(mktemp -d "${TMPDIR:-/tmp}/cosmos-bake.XXXXXX")
trap 'rm -rf "$WORK"' EXIT
cp "$SERVER_DIR/prefetch.py" "$SERVER_DIR/artifacts.lock" "$WORK/"

# the image's own lock must match ours, otherwise the runtime would look for other revisions
if ! docker run --rm --entrypoint cat "$BASE" /opt/carla-cosmos/server/artifacts.lock | cmp -s - "$WORK/artifacts.lock"; then
    echo "artifacts.lock in $BASE differs from $SERVER_DIR/artifacts.lock; rebuild the -nomodels image first" >&2; exit 1
fi
BASE_IMAGE_VARIANT=$(docker run --rm --entrypoint cat "$BASE" /models/hf/ARTIFACTS_IMAGE 2>/dev/null | tr -d '[:space:]' || echo none)

# artifacts to add = image's artifacts minus those already in the base
ONLY=$(python3 - "$WORK/artifacts.lock" "$VARIANT" "$BASE_IMAGE_VARIANT" "$ONLY" <<'PY'
import json, sys
lock = json.load(open(sys.argv[1])); variant, base, only = sys.argv[2:5]
want = list(lock["images"][variant]["artifacts"])
have = set(lock["images"].get(base, {}).get("artifacts", [])) if base != "none" else set()
if only: want = [a for a in want if a in only.split(",")]
print(",".join(a for a in want if a not in have))
PY
)
[ -n "$ONLY" ] || { echo "$BASE already contains every artifact of $VARIANT; nothing to bake"; exit 0; }

MOUNTS=(-v "$WORK:/mnt/bake:ro"); CACHE_ARGS=(); i=0
for c in "${CACHES[@]}"; do MOUNTS+=(-v "$c:/mnt/hfcache$i:ro"); CACHE_ARGS+=(--from-cache "/mnt/hfcache$i"); i=$((i + 1)); done

python3 "$WORK/prefetch.py" --image "$VARIANT" --lock "$WORK/artifacts.lock" --only "$ONLY" --plan --max-layer-gb "$MAX_GB" > "$WORK/plan.json"
N=$(python3 -c "import json,sys; print(len(json.load(open(sys.argv[1]))))" "$WORK/plan.json")
echo "baking $VARIANT onto $BASE -> $TAG: artifacts [$ONLY] in $N layer(s) of <= $MAX_GB GB from ${CACHES[*]}"

CUR=$BASE; CNAME=cosmos-bake-$$
for ((s = 0; s < N; s++)); do
    python3 -c "import json,sys; json.dump(json.load(open(sys.argv[1]))[int(sys.argv[2])], open(sys.argv[3], 'w'))" "$WORK/plan.json" "$s" "$WORK/shard.json"
    FINAL=(); [ $((s + 1)) -eq "$N" ] && FINAL=(--finalize)
    echo "== layer $((s + 1))/$N"
    docker rm -f "$CNAME" >/dev/null 2>&1 || true
    docker run --name "$CNAME" "${MOUNTS[@]}" --entrypoint "$PY" "$CUR" \
        /mnt/bake/prefetch.py --image "$VARIANT" --lock /mnt/bake/artifacts.lock --dest /models/hf \
        "${CACHE_ARGS[@]}" --select /mnt/bake/shard.json "${FINAL[@]}" $NO_HASH | grep -vE '^   reused'
    NEXT="${TAG}-bake$((s + 1))"
    CHANGES=(-c "ENV CARLA_COSMOS_IMAGE_VARIANT=$VARIANT" -c "LABEL com.carla.cosmos.variant=$VARIANT" -c "LABEL com.carla.cosmos.weights=$VARIANT")
    docker commit "${CHANGES[@]}" "$CNAME" "$NEXT" >/dev/null
    docker rm "$CNAME" >/dev/null
    [ "$CUR" != "$BASE" ] && docker rmi "$CUR" >/dev/null
    CUR=$NEXT
    df -h "$(docker info -f '{{.DockerRootDir}}' 2>/dev/null || echo /var/lib/docker)" | awk 'NR==2 {print "   docker root free: " $4}'
done
docker tag "$CUR" "$TAG" && docker rmi "$CUR" >/dev/null

echo "== verifying $TAG against artifacts.lock (sizes)"
VERIFY_ONLY=(); [ -n "$USER_ONLY" ] && VERIFY_ONLY=(--only "$USER_ONLY")
docker run --rm "${MOUNTS[@]}" --entrypoint "$PY" "$TAG" \
    /mnt/bake/prefetch.py --image "$VARIANT" --lock /mnt/bake/artifacts.lock --dest /models/hf --no-hash --no-download "${VERIFY_ONLY[@]}" | tail -1
docker run --rm --entrypoint cat "$TAG" /models/hf/ARTIFACTS_IMAGE
docker images "$TAG" --format 'built {{.Repository}}:{{.Tag}} ({{.Size}})'
