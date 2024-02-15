#!/bin/bash
UE4_TRUE_SCRIPT_NAME=$(echo \"$0\" | xargs readlink -f)
UE4_PROJECT_ROOT=$(dirname "$UE4_TRUE_SCRIPT_NAME")
EGG_FILE=(${UE4_PROJECT_ROOT}/PythonAPI/carla/dist/*.egg)
eval PYTHONPATH=${EGG_FILE}:${UE4_PROJECT_ROOT}/PythonAPI/carla/:${PYTHONPATH} $@
