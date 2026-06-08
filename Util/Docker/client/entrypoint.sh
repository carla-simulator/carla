#!/bin/bash
set -e

# source CARLA environment setup script
source /setup_carla_env.sh

# run the actual command
exec "$@"