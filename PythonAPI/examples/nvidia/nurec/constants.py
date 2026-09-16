# SPDX-FileCopyrightText: © 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
#
# SPDX-License-Identifier: MIT

# Track identifier for the ego (main) vehicle
EGO_TRACK_ID = "ego"
# Default vehicle class for the ego vehicle in CARLA.
# CARLA UE5 renamed the vehicle catalog; the UE4-era Mercedes coupe is gone.
# vehicle.ue4.mercedes.ccc is its closest surviving relative.
EGO_LABEL = "vehicle.ue4.mercedes.ccc"
# Dimensions for the ego vehicle (Unused)
EGO_DIMS = None

# Track flags - used to mark special properties of tracks
# Flag identifying the ego (main) vehicle track
EGO_FLAG = "EGO"
# Flag identifying dynamic (moving) objects
DYNAMIC_FLAG = "DYNAMIC"
# Flag identifying objects that can be controlled by the system
CONTROLLABLE_FLAG = "CONTROLLABLE"

# List of labels that are considered vehicles for classification purposes
VEHICLE_LABELS = ["automobile", "heavy_truck", "bus", "trailer", "rider"]

# Track identifier for the spectator (camera) viewpoint
SPECTATOR_TRACK_ID = "spectator"
# Label for the spectator track
SPECTATOR_LABEL = "spectator"
# Flag identifying the spectator (camera) track
SPECTATOR_FLAG = "SPECTATOR"

# Maximum gRPC message length in bytes (64MB).
# Sized for uncompressed RGB_UINT8_PLANAR frames: 4K (3840x2160x3) is ~25MB.
MAX_MESSAGE_LENGTH = 64_000_000

# Default NuRec render container. Overridable via the NUREC_IMAGE env var or
# the `image` argument of NuRecRenderService / NurecScenario.
DEFAULT_NUREC_IMAGE = "nvcr.io/nvidia/nre/nre-ga:26.04.01"

# Default port for the NuRec gRPC server. Deliberately NOT 2000: that is
# CARLA's own RPC port and the container runs with --net=host.
DEFAULT_NUREC_PORT = 46435

# Conversion factor from m/s to km/h
KPH_PER_MPS = 3.6