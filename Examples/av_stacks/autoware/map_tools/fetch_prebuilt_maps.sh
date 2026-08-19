#!/usr/bin/env bash
# Fetch the prebuilt CARLA Autoware map pair (pointcloud + lanelet2) from
# carla-simulator/autoware-contents and arrange it in the Autoware layout:
#   <out>/pointcloud_map.pcd
#   <out>/lanelet2_map.osm
#   <out>/map_projector_info.yaml   (projector_type: Local)
#
# Prebuilt maps exist for Town01..Town07 and Town10HD (UE4-era content;
# TIER IV verified the Town10 pair works against UE5 Town10HD_Opt).
# Note: the lanelet2 files are y-inverted relative to CARLA coordinates
# (that is the convention Autoware expects) and lack traffic-light
# regulatory elements; use generate_lanelet2_map.py against a live server
# if you need those injected.
#
# Usage: fetch_prebuilt_maps.sh <Town> [out_dir]
#   <Town>   Town01..Town07 | Town10HD (aliases: Town10, Town10HD_Opt)
#   out_dir  default: <this dir>/maps/<Town>  (where run_carla_autoware.sh looks)

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BASE_URL="https://bitbucket.org/carla-simulator/autoware-contents/raw/master/maps"

usage() {
  sed -n '2,17p' "$0" | sed 's/^# \{0,1\}//'
  exit 1
}

TOWN="${1:-}"
[[ -n "$TOWN" ]] || usage

# Normalize aliases (packaged UE5 name Town10HD_Opt -> prebuilt Town10HD).
case "$TOWN" in
  Town10 | Town10HD_Opt | Town10HD) TOWN="Town10HD" ;;
esac

case "$TOWN" in
  Town0[1-7] | Town10HD) ;;
  *)
    echo "ERROR: no prebuilt Autoware maps for '$TOWN'." >&2
    echo "Available: Town01 Town02 Town03 Town04 Town05 Town06 Town07 Town10HD" >&2
    echo "For other towns, generate artifacts with generate_map_artifacts.py instead." >&2
    exit 1
    ;;
esac

OUT_DIR="${2:-$SCRIPT_DIR/maps/$TOWN}"
mkdir -p "$OUT_DIR"

PCD_URL="$BASE_URL/point_cloud_maps/$TOWN.pcd"
OSM_URL="$BASE_URL/vector_maps/lanelet2/$TOWN.osm"

# Bitbucket serves these via a 302 to S3 (git-lfs), hence -L.
echo "Downloading $PCD_URL"
curl -fSL --retry 3 --connect-timeout 30 -o "$OUT_DIR/pointcloud_map.pcd" "$PCD_URL"
echo "Downloading $OSM_URL"
curl -fSL --retry 3 --connect-timeout 30 -o "$OUT_DIR/lanelet2_map.osm" "$OSM_URL"

# Sanity checks: PCD header magic and non-trivial osm.
head -c 200 "$OUT_DIR/pointcloud_map.pcd" | grep -q "PCD" \
  || { echo "ERROR: downloaded pointcloud_map.pcd does not look like a PCD file" >&2; exit 1; }
grep -q "<osm" "$OUT_DIR/lanelet2_map.osm" \
  || { echo "ERROR: downloaded lanelet2_map.osm does not look like an OSM file" >&2; exit 1; }

cat > "$OUT_DIR/map_projector_info.yaml" <<'EOF'
# Autoware map projector definition (consumed by map_projection_loader).
projector_type: Local
EOF

echo
echo "Autoware map directory ready: $OUT_DIR"
ls -l "$OUT_DIR"
echo
echo "Use with: ros2 launch autoware_launch e2e_simulator.launch.xml map_path:=$(cd "$OUT_DIR" && pwd) vehicle_model:=sample_vehicle sensor_model:=awsim_sensor_kit"
