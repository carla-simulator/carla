#! /bin/bash

# Fetch the NVIDIA DLSS SDK into a user-local path and export DLSS_SDK so the
# engine's DLSSRRDenoiser plugin (DLSS Ray Reconstruction denoiser for the
# rt_lens sensor + DLSS Super Resolution scene-capture upscaler) builds with
# real NGX support.
#
# The SDK is licensed by NVIDIA and may not be redistributed inside the CARLA
# or CarlaUnreal/UnrealEngine repositories, so it is never vendored: each user
# obtains it directly from NVIDIA's public GitHub repository under NVIDIA's
# own license terms, and the build locates it exclusively through the
# DLSS_SDK environment variable. The engine build requires the SDK: without
# it the DLSS-RR denoiser and DLSS upscaling would silently degrade to the
# NFOR denoiser / spatial upscale, so a missing SDK stops the setup here
# instead of surfacing later as an engine build error. Building without DLSS
# on purpose is an explicit opt-in via DLSS_SDK=disabled.
#
# The same DLSS_SDK variable also serves at runtime: the plugin points NGX at
# $DLSS_SDK/lib/Linux_x86_64/rel to load the model snippets
# (libnvidia-ngx-dlssd.so.* for Ray Reconstruction, libnvidia-ngx-dlss.so.*
# for Super Resolution).

set -e

if [ "$(echo "$DLSS_SDK" | tr '[:upper:]' '[:lower:]')" = "disabled" ]; then
    echo "DLSS_SDK=disabled: skipping the DLSS SDK setup. The DLSS-RR denoiser"
    echo "and DLSS upscaling will be unavailable (NFOR denoiser and spatial"
    echo "upscale remain functional)."
    exit 0
fi

DLSS_SDK_URL=${DLSS_SDK_URL:-https://github.com/NVIDIA/DLSS.git}
DLSS_SDK_DEFAULT_PATH=${DLSS_SDK_DEFAULT_PATH:-$HOME/SDKs/DLSS}

sdk_is_valid() {
    [ -d "$1/include" ] && \
    [ -f "$1/lib/Linux_x86_64/libnvsdk_ngx.a" ] && \
    ls "$1"/lib/Linux_x86_64/rel/libnvidia-ngx-dlssd.so.* > /dev/null 2>&1
}

if [ -n "$DLSS_SDK" ] && sdk_is_valid "$DLSS_SDK"; then
    echo "Found DLSS SDK at $DLSS_SDK"
    exit 0
fi

if sdk_is_valid "$DLSS_SDK_DEFAULT_PATH"; then
    echo "Found DLSS SDK at $DLSS_SDK_DEFAULT_PATH"
else
    echo "DLSS SDK not found. Cloning from $DLSS_SDK_URL into $DLSS_SDK_DEFAULT_PATH..."
    mkdir -p "$(dirname "$DLSS_SDK_DEFAULT_PATH")"
    if ! git clone --depth 1 "$DLSS_SDK_URL" "$DLSS_SDK_DEFAULT_PATH"; then
        echo "Error: could not clone the DLSS SDK from $DLSS_SDK_URL."
        echo "The engine build requires it; to deliberately build without DLSS"
        echo "support, set DLSS_SDK=disabled."
        exit 1
    fi
    if ! sdk_is_valid "$DLSS_SDK_DEFAULT_PATH"; then
        echo "Error: $DLSS_SDK_DEFAULT_PATH does not look like a Linux DLSS SDK"
        echo "(missing include/, lib/Linux_x86_64/libnvsdk_ngx.a or the dlssd snippet)."
        echo "The engine build requires it; to deliberately build without DLSS"
        echo "support, set DLSS_SDK=disabled."
        exit 1
    fi
fi

if ! grep -q "^export DLSS_SDK=" ~/.bashrc 2>/dev/null; then
    echo -e "\n#NVIDIA DLSS SDK (CARLA rt_lens denoiser / DLSS upscaling)\nexport DLSS_SDK=$DLSS_SDK_DEFAULT_PATH" >> ~/.bashrc
    echo "Exported DLSS_SDK=$DLSS_SDK_DEFAULT_PATH to ~/.bashrc"
fi
echo "DLSS SDK ready at $DLSS_SDK_DEFAULT_PATH"
