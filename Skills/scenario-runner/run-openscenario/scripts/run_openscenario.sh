#!/usr/bin/env bash
# List, validate and run OpenSCENARIO files through ScenarioRunner.
#
#   bash run_openscenario.sh list
#   bash run_openscenario.sh validate /abs/path/file.xosc
#   bash run_openscenario.sh run      /abs/path/file.xosc      # OpenSCENARIO 1.x
#   bash run_openscenario.sh run2     /abs/path/file.osc       # OpenSCENARIO 2.0
#
# Knobs (env): PARAMS='k: v, k2: v2'   global ParameterDeclarations override (1.x only)
#              SYNC=1 FRAME_RATE=20    synchronous mode
#              OUTPUT/FILE/JSON/JUNIT=1, OUTPUT_DIR, RECORD, DEBUG=1, TIMEOUT
set -uo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck disable=SC1091
source "${HERE}/env.sh"
# env.sh runs `set -euo pipefail`, and sourcing it applies -e to THIS shell. This
# wrapper must survive a non-zero exit from the tool it runs, or its verification
# and cleanup are skipped precisely when a run failed.
set +e


CMD="${1:-}"
FILE="${2:-}"
[ -n "${SCENARIO_RUNNER_ROOT}" ] || { echo "SCENARIO_RUNNER_ROOT is not set — run check_env.sh" >&2; exit 2; }

case "${CMD}" in
list)
  # Examples moved between directories on ue5-master, so look in both.
  for d in examples osc_examples; do
    [ -d "${SCENARIO_RUNNER_ROOT}/srunner/${d}" ] || continue
    n1=$(find "${SCENARIO_RUNNER_ROOT}/srunner/${d}" -maxdepth 1 -name '*.xosc' | wc -l)
    n2=$(find "${SCENARIO_RUNNER_ROOT}/srunner/${d}" -maxdepth 1 -name '*.osc'  | wc -l)
    [ "${n1}" -eq 0 ] && [ "${n2}" -eq 0 ] && continue
    echo "== srunner/${d}/  (${n1} .xosc, ${n2} .osc) =="
    find "${SCENARIO_RUNNER_ROOT}/srunner/${d}" -maxdepth 1 \( -name '*.xosc' -o -name '*.osc' \) \
      | sort | sed "s|${SCENARIO_RUNNER_ROOT}/|  |"
  done
  [ -d "${SCENARIO_RUNNER_ROOT}/srunner/examples/catalogs" ] && \
    echo "== catalogs (referenced relative to the .xosc) ==" && \
    ls "${SCENARIO_RUNNER_ROOT}/srunner/examples/catalogs" | sed 's/^/  /'
  exit 0
  ;;
validate)
  [ -n "${FILE}" ] || { echo "usage: bash run_openscenario.sh validate <file.xosc>" >&2; exit 2; }
  "${PYTHON}" - "${FILE}" "${SCENARIO_RUNNER_ROOT}" "${CARLA_HOST}" "${CARLA_PORT}" <<'PY'
import os, sys, xml.etree.ElementTree as ET
path, root, host, port = sys.argv[1], sys.argv[2], sys.argv[3], int(sys.argv[4])
rc = 0
if not os.path.isfile(path):
    sys.exit(f"FAIL  {path} does not exist (ScenarioRunner checks this first, and "
             f"relative paths resolve against your CWD)")
try:
    tree = ET.parse(path)
except ET.ParseError as e:
    sys.exit(f"FAIL  not well-formed XML: {e}")
r = tree.getroot()
print(f"PASS  well-formed XML, root <{r.tag}>")
if r.tag != "OpenSCENARIO":
    print(f"WARN  root is <{r.tag}>, expected <OpenSCENARIO> — is this an OSC 2.0 .osc file? use run2")

hdr = r.find("FileHeader")
if hdr is not None:
    print(f"PASS  FileHeader rev {hdr.get('revMajor')}.{hdr.get('revMinor')} "
          f"author={hdr.get('author')!r}")
    if (hdr.get("revMajor"), hdr.get("revMinor")) not in (("1", "0"), ("0", "9"), ("1", None)):
        print("WARN  ScenarioRunner ships the 0.9.1/1.0 schema only; 1.1+ elements will fail validation")

# Global parameters are the only ones --openscenarioparams can override.
params = [(p.get("name"), p.get("value")) for p in r.findall("ParameterDeclarations/ParameterDeclaration")]
if params:
    print(f"PASS  {len(params)} global parameter(s), overridable with PARAMS=")
    for n, v in params:
        print(f"        {n} = {v}")
else:
    print("INFO  no global ParameterDeclarations — PARAMS= has nothing to override")

# Catalog paths are relative to the .xosc, which is why the bundled examples must
# stay next to their catalogs/ directory.
base = os.path.dirname(os.path.abspath(path))
for cl in r.findall("CatalogLocations/*"):
    d = cl.find("Directory")
    p = d.get("path") if d is not None else None
    if p:
        full = p if os.path.isabs(p) else os.path.join(base, p)
        ok = os.path.isdir(full)
        print(f"{'PASS' if ok else 'FAIL'}  catalog {cl.tag}: {p} -> {full}")
        rc |= 0 if ok else 1

logic = r.find("RoadNetwork/LogicFile")
town = logic.get("filepath") if logic is not None else None
print(f"INFO  RoadNetwork: {town!r}")
ents = [e.get("name") for e in r.findall("Entities/ScenarioObject")]
print(f"INFO  {len(ents)} entit(ies): {', '.join(ents) or '<none>'}")
ext = [e.get("name") for e in r.findall("Entities/ScenarioObject")
       if e.find("ObjectController") is None]
# A controller can also be attached from the storyboard, so only call an entity
# undriven when neither route assigns one.
assigned = bool(r.findall(".//AssignControllerAction") or r.findall(".//ActivateControllerAction"))
if ext:
    how = "no ObjectController and no Assign/ActivateControllerAction" if not assigned \
          else "no ObjectController (a storyboard AssignControllerAction may cover it)"
    print(f"{'WARN' if not assigned else 'INFO'}  {how}: {', '.join(ext)}")
    if not assigned:
        print("WARN    nothing will drive them — manual_control.py -a --rolename=<name>, or an agent")

try:
    import carla
    c = carla.Client(host, port); c.set_timeout(4.0)
    maps = [m.split('/')[-1] for m in c.get_available_maps()]
    cur = c.get_world().get_map().name.split('/')[-1]
    print(f"PASS  server reachable, map {cur}")
    if town and town not in maps and not town.endswith(".xodr"):
        print(f"FAIL  map {town!r} is not available on this server ({len(maps)} maps)")
        rc |= 1
    elif town:
        print(f"PASS  map {town!r} is available")
except Exception as e:
    print(f"WARN  no server at {host}:{port} — map check skipped ({e})")

try:
    import xmlschema
    xsd = os.path.join(root, "srunner", "openscenario", "0.9.x", "OpenSCENARIO_v0.9.1.xsd")
    if os.path.isfile(xsd):
        s = xmlschema.XMLSchema(xsd)
        try:
            s.validate(path)
            print("PASS  validates against the bundled OpenSCENARIO schema")
        except Exception as e:
            print(f"FAIL  schema validation: {str(e).splitlines()[0]}")
            rc |= 1
    else:
        print(f"WARN  schema not found at {xsd}")
except ImportError:
    print("WARN  xmlschema not installed — pip install 'xmlschema==1.0.18'")
sys.exit(rc)
PY
  exit $?
  ;;
run|run2)
  [ -n "${FILE}" ] || { echo "usage: bash run_openscenario.sh ${CMD} <file>" >&2; exit 2; }
  [ -f "${FILE}" ] || { echo "ERROR: ${FILE} does not exist" >&2; exit 3; }
  FLAG=--openscenario; [ "${CMD}" = "run2" ] && FLAG=--openscenario2
  ARGS=("${FLAG}" "$(cd "$(dirname "${FILE}")" && pwd)/$(basename "${FILE}")"
        --host "${CARLA_HOST}" --port "${CARLA_PORT}" --trafficManagerPort "${CARLA_TM_PORT}"
        --timeout "${TIMEOUT:-120}")
  if [ -n "${PARAMS:-}" ]; then
    if [ "${CMD}" = "run2" ]; then
      echo "[run] NOTE PARAMS is ignored for OpenSCENARIO 2.0 (--openscenarioparams needs --openscenario)"
    else
      ARGS+=(--openscenarioparams "${PARAMS}")
    fi
  fi
  [ "${SYNC:-0}" = "1" ]      && ARGS+=(--sync --frameRate "${FRAME_RATE:-20}")
  [ "${DEBUG:-0}" = "1" ]     && ARGS+=(--debug)
  [ "${OUTPUT:-0}" = "1" ]    && ARGS+=(--output)
  [ "${FILE_OUT:-0}" = "1" ]  && ARGS+=(--file)
  [ "${JSON:-0}" = "1" ]      && ARGS+=(--json)
  [ "${JUNIT:-0}" = "1" ]     && ARGS+=(--junit)
  [ -n "${OUTPUT_DIR:-}" ]    && { mkdir -p "${OUTPUT_DIR}"; ARGS+=(--outputDir "${OUTPUT_DIR}"); }
  [ -n "${RECORD:-}" ]        && { mkdir -p "${SCENARIO_RUNNER_ROOT}/${RECORD}"; ARGS+=(--record "${RECORD}"); }

  reset_async() {
    [ "${SYNC:-0}" = "1" ] || return 0
    "${PYTHON}" - "${CARLA_HOST}" "${CARLA_PORT}" "${CARLA_TM_PORT}" <<'PY' || true
import sys
try:
    import carla
    c = carla.Client(sys.argv[1], int(sys.argv[2])); c.set_timeout(5.0)
    w = c.get_world(); s = w.get_settings()
    if s.synchronous_mode:
        s.synchronous_mode = False; s.fixed_delta_seconds = None
        w.apply_settings(s)
        c.get_trafficmanager(int(sys.argv[3])).set_synchronous_mode(False)
        print("[run] world reset to asynchronous mode")
except Exception as e:
    print(f"[run] WARNING could not reset async mode: {e}")
PY
  }
  trap reset_async EXIT INT TERM

  echo "[run] ${PYTHON} scenario_runner.py ${ARGS[*]}"
  echo "[run] NOTE the OSC timeout is hard-coded to 100000s; interrupt a stalled run yourself"
  cd "${SCENARIO_RUNNER_ROOT}"
  # -u: Python block-buffers stdout when it is not a tty, so redirecting a long
  # run to a log otherwise shows nothing until it finishes.
  "${PYTHON}" -u scenario_runner.py "${ARGS[@]}"
  RC=$?
  echo "[run] scenario_runner exited ${RC}"
  exit ${RC}
  ;;
*)
  echo "usage: bash run_openscenario.sh {list|validate|run|run2} [file]" >&2
  exit 2
  ;;
esac
