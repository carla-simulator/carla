#!/bin/bash
# Regenerate FastDDS C++ types from IDL files.
# Requires: fastddsgen (from Fast-DDS-Gen v2.5.x) + Java JDK
# JDK is pre-installed in the CARLA Docker dev container.

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
CARLA_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"

FASTDDSGEN="${1:-$(find "${CARLA_ROOT}/Build" -name fastddsgen -path '*/bin/*' 2>/dev/null | head -1)}"
IDL_DIR="${CARLA_ROOT}/LibCarla/source/carla/ros2/types/idl"
OUT_DIR="${CARLA_ROOT}/LibCarla/source/carla/ros2/types/fastdds"

if [[ -z "$FASTDDSGEN" || ! -x "$FASTDDSGEN" ]]; then
  echo "ERROR: fastddsgen not found. Run 'make setup' first or pass path as argument."
  exit 1
fi

echo "Using fastddsgen: $FASTDDSGEN"
echo "IDL source: $IDL_DIR"
echo "Output dir: $OUT_DIR"

for pkg_dir in "$IDL_DIR"/*/; do
  pkg_name=$(basename "$pkg_dir")
  pkg_out="$OUT_DIR/$pkg_name"
  mkdir -p "$pkg_out"
  for idl in "$pkg_dir"*.idl; do
    [ -f "$idl" ] || continue
    echo "Generating: $pkg_name/$(basename "$idl")"
    "$FASTDDSGEN" -replace -d "$pkg_out" -I "$IDL_DIR" "$idl"
  done
done

echo "Done. Review changes with: git diff LibCarla/source/carla/ros2/types/fastdds/"
