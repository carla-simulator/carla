#! /usr/bin/env bash
# Start the carla-cosmos server container with sensible defaults and print the API token.
#
#   ./run_server.sh [--image IMG] [--port 8000] [--state ~/.carla-cosmos/state] [--gpus all] [--profile auto]
#                   [--token TOKEN] [--no-guardrails]
#
# Everything the service needs (weights, venvs) is inside the image; the only
# host state is the mounted /state directory (tokens, job queue, blobs, results).
set -euo pipefail

IMAGE=${COSMOS_IMAGE:-carla-cosmos:nano}
PORT=8000
STATE=${COSMOS_STATE:-$HOME/.carla-cosmos/state}
GPUS=all
PROFILE=auto
TOKEN=${COSMOS_TOKEN:-}
GUARDRAILS=1
NAME=carla-cosmos

while [ $# -gt 0 ]; do
    case "$1" in
        --image) IMAGE=$2; shift 2 ;;
        --port) PORT=$2; shift 2 ;;
        --state) STATE=$2; shift 2 ;;
        --gpus) GPUS=$2; shift 2 ;;
        --profile) PROFILE=$2; shift 2 ;;
        --token) TOKEN=$2; shift 2 ;;
        --name) NAME=$2; shift 2 ;;
        --no-guardrails) GUARDRAILS=0; shift ;;
        -h|--help) sed -n '2,10p' "$0"; exit 0 ;;
        *) echo "unknown option $1"; exit 1 ;;
    esac
done

mkdir -p "$STATE"
STATE=$(realpath "$STATE")

if docker ps -a --format '{{.Names}}' | grep -qx "$NAME"; then
    echo "container $NAME already exists; starting it (docker rm -f $NAME to recreate)"
    docker start "$NAME" >/dev/null
else
    ARGS=(-d --name "$NAME" --restart unless-stopped -p "$PORT:8000" -v "$STATE:/state" --shm-size 16g
          --label com.carla.cosmos.role=server --label "com.carla.cosmos.port=$PORT"
          --label "com.carla.cosmos.image=$IMAGE" --label "com.carla.cosmos.state=$STATE"
          -e "COSMOS_PROFILE=$PROFILE" -e "COSMOS_GUARDRAILS=$GUARDRAILS")
    [ -n "$GPUS" ] && ARGS+=(--gpus "$GPUS")
    [ -n "$TOKEN" ] && ARGS+=(-e "COSMOS_TOKEN=$TOKEN")
    docker run "${ARGS[@]}" "$IMAGE" >/dev/null
fi

echo "waiting for the API..."
for _ in $(seq 1 60); do
    if curl -fsS "http://localhost:$PORT/v1/health/live" >/dev/null 2>&1; then break; fi
    sleep 1
done
if [ -z "$TOKEN" ]; then
    for _ in $(seq 1 30); do
        [ -s "$STATE/initial_token.txt" ] && break
        sleep 1
    done
    TOKEN=$(cat "$STATE/initial_token.txt" 2>/dev/null || true)
fi

cat <<MSG

carla-cosmos is starting (workers load the models in the background).

  URL:    http://localhost:$PORT      status page: http://localhost:$PORT/ui
  token:  ${TOKEN:-<see docker logs $NAME>}

  export COSMOS_URL=http://localhost:$PORT
  export COSMOS_TOKEN=$TOKEN
  carla-cosmos health          # readiness + worker states
  docker logs -f $NAME         # model loading progress
MSG
