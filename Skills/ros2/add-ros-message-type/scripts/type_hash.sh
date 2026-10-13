#!/usr/bin/env bash
# Compute the RIHS01 type hash for a ROS 2 message type (Step 4), pulling the
# .msg out of a ROS 2 container first when the type is a standard one.
#
# Usage:
#   bash type_hash.sh <pkg/msg/TypeName> [path/to/TypeName.msg]
#
#   With a .msg path      -> hashes that file (the case for a new carla_msgs type).
#   Without               -> extracts <pkg>/msg/<TypeName>.msg from
#                            osrf/ros:jazzy-desktop, then hashes it. Works for any
#                            type installed in that image (std_msgs, sensor_msgs,
#                            geometry_msgs, nav_msgs, tf2_msgs, rosgraph_msgs,
#                            ackermann_msgs, ...).
#
# Prints the RIHS01_<64 hex> line to stdout, ready to paste into CdrTopicInfo.h.
# Delegates the actual hashing to the checkout's Util/ros2/compute_type_hash.sh
# so the algorithm is never forked here.
#
# Hashes are stable across Humble and Jazzy for any message whose definition did
# not change between them, which is true of every standard package CARLA uses —
# so computing against Jazzy is correct for both.
set -euo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck disable=SC1091
source "${HERE}/env.sh" >/dev/null

TYPE="${1:-}"
MSG="${2:-}"
IMAGE="${ROS_IMAGE:-osrf/ros:jazzy-desktop}"

[ -n "${TYPE}" ] || { echo "usage: bash type_hash.sh <pkg/msg/TypeName> [file.msg]" >&2; exit 2; }
[ -f "${CARLA_HASH_TOOL}" ] \
  || { echo "ERROR: ${CARLA_HASH_TOOL} not found — wrong CARLA_UE4_ROOT, or a checkout without the tool." >&2; exit 1; }

# pkg/msg/TypeName -> pkg, TypeName. The middle segment is always "msg".
PKG="${TYPE%%/*}"
NAME="${TYPE##*/}"
case "${TYPE}" in
  */msg/*) ;;
  *) echo "ERROR: type must be <pkg>/msg/<TypeName> (got '${TYPE}')." >&2; exit 2 ;;
esac

CLEANUP=""
if [ -z "${MSG}" ]; then
  command -v docker >/dev/null 2>&1 \
    || { echo "ERROR: docker needed to extract ${TYPE} from ${IMAGE}, or pass the .msg path." >&2; exit 1; }
  MSG="$(mktemp "/tmp/${NAME}.XXXXXX.msg")"
  CLEANUP="${MSG}"
  echo "[hash] extracting ${PKG}/msg/${NAME}.msg from ${IMAGE}..." >&2
  # The distro inside the image decides the share path; derive it from the tag.
  DISTRO="${IMAGE##*:}"; DISTRO="${DISTRO%%-*}"
  if ! docker run --rm "${IMAGE}" \
        cat "/opt/ros/${DISTRO}/share/${PKG}/msg/${NAME}.msg" > "${MSG}" 2>/dev/null; then
    rm -f "${CLEANUP}"
    echo "ERROR: ${PKG}/msg/${NAME}.msg is not installed in ${IMAGE}." >&2
    echo "       Pass the .msg file explicitly, or use a newer image via ROS_IMAGE=." >&2
    exit 1
  fi
  [ -s "${MSG}" ] || { rm -f "${CLEANUP}"; echo "ERROR: extracted ${NAME}.msg is empty." >&2; exit 1; }
fi

[ -f "${MSG}" ] || { echo "ERROR: no such .msg file: ${MSG}" >&2; exit 1; }
echo "[hash] definition:" >&2
sed 's/^/  /' "${MSG}" >&2

# The tool builds the type inside a ROS 2 container and prints one line.
#
# Upstream Util/ros2/compute_type_hash.sh is BROKEN in three ways on this HEAD
# (all verified here, 2026-08, osrf/ros:jazzy-desktop):
#
#   1. the in-container script runs under `set -u` and then sources
#      /opt/ros/<distro>/setup.bash, which dereferences AMENT_TRACE_SETUP_FILES
#      unguarded -> "unbound variable" before colcon starts;
#   2. `--log-base` is passed AFTER `--cmake-args`, which swallows every
#      following argument, so CMake gets it and errors with
#      "Unknown argument --log-base" -> 0 packages built;
#   3. `docker run` has no --user, so the workspace it creates in the host's
#      mktemp dir ends up root-owned and upstream's own cleanup `rm` fails,
#      making its exit code useless;
#   4. the hash is extracted from the generated JSON with `jq`, which is NOT
#      installed in osrf/ros:jazzy-desktop -> "jq: command not found", reported
#      as the misleading "'<type>' not found in <json>".
#
# So: try upstream as-is first (in case it has been fixed), and on ANY failure
# retry with a TEMP COPY carrying those three fixes. The hashing algorithm always
# comes from upstream's file — nothing in the checkout is modified, and the real
# fix belongs there (three one-liners).
run_tool() { bash "$1" "${TYPE}" "${MSG}"; }

ERRLOG="$(mktemp /tmp/carla_type_hash_err.XXXXXX)"
OUT=""
if ! OUT="$(run_tool "${CARLA_HASH_TOOL}" 2>"${ERRLOG}")"; then
  echo "[hash] upstream compute_type_hash.sh failed; retrying with a patched temp copy" >&2
  echo "[hash] (set -u vs setup.bash, --log-base placement, docker --user). Your" >&2
  echo "[hash] checkout is NOT modified — see this script's header for the details." >&2
  PATCHED="$(mktemp /tmp/compute_type_hash.XXXXXX.sh)"
  # (2) is fixed by DROPPING the misplaced --log-base line rather than moving it:
  # colcon then uses its default log directory, which this flow never reads.
  # (4) is fixed by mounting a tiny python `jq` shim over the missing binary,
  # which leaves upstream's jq expression untouched — far less brittle than
  # rewriting that quoted-inside-docker-inside-bash command with sed.
  JQ_SHIM="$(mktemp /tmp/jqshim.XXXXXX)"
  cat > "${JQ_SHIM}" <<'SHIM'
#!/usr/bin/env python3
"""Minimal `jq` stand-in for exactly one call shape, used only inside the
container that lacks jq:

    jq -re --arg t <TYPE> '.type_hashes[] | select(.type_name == $t) | .hash_string' FILE

Anything else exits 2 rather than pretending to be jq.
"""
import json
import sys

a = sys.argv[1:]
if "--arg" not in a or len(a) < 2:
    sys.exit(2)
i = a.index("--arg")
wanted = a[i + 2]                      # value of --arg t
path = a[-1]                           # the JSON file is the last argument
if ".type_hashes[]" not in " ".join(a) or "hash_string" not in " ".join(a):
    sys.exit(2)
with open(path) as fh:
    doc = json.load(fh)
for entry in doc.get("type_hashes", []):
    if entry.get("type_name") == wanted:
        print(entry["hash_string"])
        sys.exit(0)
sys.exit(1)                            # -re semantics: not found -> non-zero
SHIM
  chmod +x "${JQ_SHIM}"
  sed -E \
    -e 's|^([[:space:]]*)source (/opt/ros/[^[:space:]]+/setup\.bash)$|\1set +u; source \2; set -u|' \
    -e '/^[[:space:]]*--log-base \/tmp\/colcon-log \\$/d' \
    -e "s|^(docker run --rm) \\\\\$|\1 --user \"\$(id -u):\$(id -g)\" --volume=\"${JQ_SHIM}:/usr/local/bin/jq:ro\" \\\\|" \
    "${CARLA_HASH_TOOL}" > "${PATCHED}"
  # Refuse to run a copy the patches did not actually land in — a silent no-op
  # here would look like "upstream is still broken" for the wrong reason.
  grep -q 'set +u; source /opt/ros' "${PATCHED}" \
    || { rm -f "${PATCHED}" "${ERRLOG}"; echo "ERROR: setup.bash patch did not apply — upstream script changed shape." >&2; exit 1; }
  # Exit status is not the gate: upstream can print a valid hash and still fail
  # in its own cleanup. The RIHS01 format check below is the gate.
  OUT="$(run_tool "${PATCHED}" 2>>"${ERRLOG}" || true)"
  rm -f "${PATCHED}" "${JQ_SHIM}"
fi
if [ -z "${OUT}" ]; then
  echo "[hash] no hash produced. Upstream stderr:" >&2
  tail -30 "${ERRLOG}" >&2
  rm -f "${ERRLOG}"; [ -n "${CLEANUP}" ] && rm -f "${CLEANUP}"
  exit 1
fi
rm -f "${ERRLOG}"
[ -n "${CLEANUP}" ] && rm -f "${CLEANUP}"

# Guard against a partial/garbled result being pasted into the source.
if ! printf '%s' "${OUT}" | grep -Eq '^RIHS01_[0-9a-f]{64}$'; then
  echo "ERROR: unexpected hash output (want RIHS01_<64 hex>):" >&2
  printf '%s\n' "${OUT}" >&2
  exit 1
fi
echo "[hash] ${TYPE}" >&2
printf '%s\n' "${OUT}"
