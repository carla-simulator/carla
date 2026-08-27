# syntax=docker/dockerfile:1.7
# carla-cosmos all-in-one image: API + Cosmos 3 (vLLM-Omni) + Transfer 2.5 (general, AV multiview, renderer)
# with every model artifact baked in.  Build with server/build_image.sh (BuildKit required).
#
#   build args: IMAGE_VARIANT=nano|full   WITH_MODELS=1|0 (0 = code-only image for CI)
#               VLLM_BASE, VLLM_OMNI_REF, TRANSFER25_REF, UV_VERSION
#   build contexts: hfcache=<local HF cache> (optional; reuses already-downloaded weights)
#   secrets:        hf_token (needed for gated repos when downloading)

ARG VLLM_BASE=vllm/vllm-openai:v0.26.0
ARG UV_VERSION=0.11.14
ARG WITH_MODELS=1

# ----------------------------------------------------------------------------- models
FROM python:3.12-slim AS models-1
ARG IMAGE_VARIANT=nano
RUN pip install --no-cache-dir "huggingface_hub>=0.30" hf_transfer
ENV HF_HUB_DISABLE_XET=1 HF_HUB_ENABLE_HF_TRANSFER=0 HF_HUB_DOWNLOAD_TIMEOUT=120
COPY server/artifacts.lock server/prefetch.py /build/
# --build-context hfcache=DIR mounts a local HF cache at /hfcache (see build_image.sh); without it,
# prefetch downloads from Hugging Face using the hf_token secret.
RUN --mount=type=secret,id=hf_token,required=false \
    --mount=type=bind,from=hfcache,target=/hfcache,rw=false \
    set -e; \
    if [ -s /run/secrets/hf_token ]; then export HF_TOKEN="$(cat /run/secrets/hf_token)"; fi; \
    FROM_CACHE=""; [ -d /hfcache/hub ] && FROM_CACHE="--from-cache /hfcache"; \
    python /build/prefetch.py --image "${IMAGE_VARIANT}" --dest /models/hf $FROM_CACHE

FROM alpine:3.20 AS models-0
RUN mkdir -p /models/hf/hub && echo none > /models/hf/ARTIFACTS_IMAGE

FROM models-${WITH_MODELS} AS models

# ----------------------------------------------------------------------------- runtime
FROM ${VLLM_BASE} AS runtime
ARG VLLM_OMNI_REF=d3c990dca8c3787ffdfe1d1424cfda9584d6bff9
ARG TRANSFER25_REF=v1.5.4
ARG UV_VERSION
ARG IMAGE_VARIANT=nano
ARG WITH_MODELS

SHELL ["/bin/bash", "-o", "pipefail", "-c"]
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
        git git-lfs jq curl ffmpeg \
        libegl1 libgles2 libgl1 libglvnd0 libglvnd-dev libx11-6 libxext6 \
    && apt-get clean && rm -rf /var/lib/apt/lists/*
COPY --from=ghcr.io/astral-sh/uv:0.11.14 /uv /usr/local/bin/uv
ENV UV_LINK_MODE=copy UV_HTTP_TIMEOUT=900 UV_PYTHON_PREFERENCE=only-managed UV_PYTHON_INSTALL_DIR=/opt/uv-python

# -- Cosmos 3 stack: vLLM-Omni into the base image's Python (vllm already there) ----------------------
RUN git clone --filter=blob:none https://github.com/vllm-project/vllm-omni.git /opt/vllm-omni \
    && git -C /opt/vllm-omni checkout "${VLLM_OMNI_REF}" \
    && uv pip install --python "$(python3 -c 'import sys; print(sys.executable)')" --no-cache-dir /opt/vllm-omni \
    && mkdir -p /opt/venvs/cosmos3/bin && ln -s "$(python3 -c 'import sys; print(sys.executable)')" /opt/venvs/cosmos3/bin/python

# -- Transfer 2.5 stack: NVIDIA's own uv project (torch cu130, moderngl/EGL renderer). Python 3.13: NVIDIA's lock
#    only has natten/flash-attn wheels for cp310 (v1.2.0 index) and cp313 (v1.5.0 index); the repo pins 3.13.
RUN git clone --depth 1 --branch "${TRANSFER25_REF}" https://github.com/nvidia-cosmos/cosmos-transfer2.5.git /opt/cosmos-transfer2.5 \
    && cd /opt/cosmos-transfer2.5 && UV_PYTHON=3.13 uv sync --extra=cu130 --no-dev \
    && ln -s /opt/cosmos-transfer2.5/.venv /opt/venvs/transfer25

# -- carla-cosmos: API venv + worker protocol into every venv ---------------------------------------------
COPY client /opt/carla-cosmos/client
COPY server /opt/carla-cosmos/server
RUN uv venv --python 3.12 /opt/venvs/api \
    && uv pip install --python /opt/venvs/api/bin/python --no-cache-dir \
         /opt/carla-cosmos/client /opt/carla-cosmos/server/workers /opt/carla-cosmos/server \
    && uv pip install --python /opt/venvs/cosmos3/bin/python --no-cache-dir /opt/carla-cosmos/server/workers \
    && uv pip install --python /opt/venvs/transfer25/bin/python --no-cache-dir /opt/carla-cosmos/server/workers \
    && sed '1s|^#!.*|#!/opt/venvs/transfer25/bin/python|' /opt/carla-cosmos/server/tools/uvx_shim.py > /usr/local/bin/uvx \
    && chmod 0755 /usr/local/bin/uvx \
    && ( /usr/local/bin/uvx 'hf>=1.3.5' download nvidia/none --revision x --quiet 2>&1 || true ) | grep -q 'cannot resolve' \
    && /opt/venvs/transfer25/bin/python -c "import huggingface_hub, moderngl, cosmos_transfer2" \
    && /opt/venvs/cosmos3/bin/python -c "import vllm_omni, cosmos_guardrail" \
    && /opt/venvs/api/bin/python -c "import carla_cosmos_server, cosmos_workers"

# -- model artifacts (one layer per repo keeps pulls resumable) ------------------------------------------------
COPY --from=models --link /models/hf/ARTIFACTS_IMAGE /models/hf/ARTIFACTS_IMAGE
COPY --from=models --link /models/hf/hub /models/hf/hub

ENV HF_HOME=/models/hf HF_HUB_OFFLINE=1 TRANSFORMERS_OFFLINE=1 UV_OFFLINE=1 \
    COSMOS_STATE=/state COSMOS_MODELS_DIR=/models COSMOS_PROFILE=auto COSMOS_PORT=8000 \
    COSMOS_PROFILES_DIR=/opt/carla-cosmos/server/profiles \
    COSMOS_VENV_API=/opt/venvs/api COSMOS_VENV_COSMOS3=/opt/venvs/cosmos3 COSMOS_VENV_TRANSFER25=/opt/venvs/transfer25 \
    PYOPENGL_PLATFORM=egl NVIDIA_DRIVER_CAPABILITIES=compute,utility,graphics \
    CARLA_COSMOS_IMAGE_VARIANT=${IMAGE_VARIANT} PATH=/opt/venvs/api/bin:${PATH}
LABEL org.opencontainers.image.title="carla-cosmos" \
      org.opencontainers.image.description="CARLA x NVIDIA Cosmos generation service (Cosmos 3, Transfer 2.5, Transfer 2.5 AV)" \
      com.carla.cosmos.variant=${IMAGE_VARIANT}
VOLUME ["/state"]
EXPOSE 8000
WORKDIR /opt/carla-cosmos
HEALTHCHECK --interval=30s --timeout=5s --start-period=30m --retries=3 \
    CMD curl -fsS http://127.0.0.1:8000/v1/health/ready || exit 1
ENTRYPOINT ["/opt/venvs/api/bin/carla-cosmos-server"]
