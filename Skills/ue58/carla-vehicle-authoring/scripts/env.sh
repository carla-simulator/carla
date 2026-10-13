#!/usr/bin/env bash
# Load recorded CARLA paths while preserving explicit environment overrides.
# Source this before resolving authoring tools; it does not launch a server.
source "$(dirname "${BASH_SOURCE[0]}")/../../../_common/env_common.sh"
