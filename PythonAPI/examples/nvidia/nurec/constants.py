# SPDX-FileCopyrightText: © 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
#
# SPDX-License-Identifier: MIT

# Track identifier for the ego (main) vehicle
EGO_TRACK_ID = "ego"
# Default vehicle class for the ego vehicle in CARLA
EGO_LABEL = "vehicle.mercedes.coupe_2020"
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

# Maximum gRPC message length in bytes (10MB)
MAX_MESSAGE_LENGTH = 10_000_000

# Conversion factor from m/s to km/h
KPH_PER_MPS = 3.6