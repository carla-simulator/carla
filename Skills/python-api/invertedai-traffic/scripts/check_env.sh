#!/usr/bin/env bash
# Prerequisite checks for invertedai-traffic. Read-only, no sudo.
#   check_env.sh            offline: venv, imports, key present, key file safe, server
#   check_env.sh --online   also makes one authenticated location_info call to
#                           prove the key works (costs one API request)
# Exits non-zero ONLY on hard blockers.
printf '\n\033[1;36m>> using skill: %s\033[0m\n' \
  "$(basename "$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)")"

set -uo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck disable=SC1091
source "${HERE}/env.sh" >/dev/null
ONLINE=0; [ "${1:-}" = "--online" ] && ONLINE=1

BLOCKED=0
pass() { echo "PASS  $*"; }
warn() { echo "WARN  $*"; }
fail() { echo "FAIL  $*"; BLOCKED=1; }

# 1. venv
if [ ! -x "${IAI_PY}" ]; then
  fail "no interpreter at ${IAI_PY} — run scripts/install_iai.sh (or set IAI_VENV in ${IAI_ENV_FILE})"
else
  pass "IAI interpreter ${IAI_PY} ($("${IAI_PY}" -V 2>&1))"
  if ! "${IAI_PY}" -c 'import sys; sys.exit(sys.version_info >= (3, 13))'; then
    fail "invertedai is pinned for Python < 3.13; rebuild the venv with python3.10-3.12"
  fi
  if v=$("${IAI_PY}" -c 'import invertedai, importlib.metadata as m; print(m.version("invertedai"))' 2>/dev/null); then
    pass "invertedai ${v} importable"
  else
    fail "invertedai not importable in ${IAI_PY} — run scripts/install_iai.sh"
  fi
  if ! "${IAI_PY}" -c 'import carla' 2>/dev/null; then
    fail "carla not importable in ${IAI_PY} — the venv needs --system-site-packages, or install the wheel into it (install-python-api)"
  fi
fi

# 2. key
if [ -z "${IAI_API_KEY:-}" ]; then
  fail "IAI_API_KEY not set. Put the line IAI_API_KEY=<key> in ${IAI_ENV_FILE} and chmod 600 it (key from https://www.inverted.ai)"
else
  pass "IAI_API_KEY present (…${IAI_API_KEY: -4}, from ${IAI_KEY_SOURCE})"
fi
if [ -f "${IAI_ENV_FILE}" ]; then
  mode=$(stat -c '%a' "${IAI_ENV_FILE}" 2>/dev/null || echo "?")
  case "${mode}" in 600|400) pass "${IAI_ENV_FILE} mode ${mode}" ;;
    *) warn "${IAI_ENV_FILE} mode ${mode}: other users can read the key — chmod 600 it" ;; esac
  d=$(dirname "${IAI_ENV_FILE}")
  if git -C "${d}" rev-parse --is-inside-work-tree >/dev/null 2>&1; then
    if git -C "${d}" check-ignore -q "${IAI_ENV_FILE}"; then
      pass "${IAI_ENV_FILE} is git-ignored"
    else
      fail "${IAI_ENV_FILE} is NOT git-ignored — add it to .gitignore before anything else"
    fi
    if git -C "${d}" ls-files --error-unmatch "${IAI_ENV_FILE}" >/dev/null 2>&1; then
      fail "${IAI_ENV_FILE} is tracked by git — git rm --cached it and rotate the key"
    fi
  fi
fi

# 3. server (and the online key check), only if the interpreter works
if [ -x "${IAI_PY}" ]; then
  "${IAI_PY}" - "${CARLA_HOST}" "${CARLA_PORT}" "${ONLINE}" <<'PY'
import os, sys
host, port, online = sys.argv[1], int(sys.argv[2]), sys.argv[3] == "1"
rc = 0
try:
    import carla
    c = carla.Client(host, port); c.set_timeout(4.0)
    sv, cv = c.get_server_version(), c.get_client_version()
    m = c.get_world().get_map().name.split("/")[-1]
    print(f"PASS  server reachable at {host}:{port} (server {sv}, client {cv}, map {m})")
    if sv != cv:
        print(f"WARN  client/server version MISMATCH: client {cv} vs server {sv}")
except Exception as e:
    print(f"FAIL  no CARLA server at {host}:{port} — start one first, or set CARLA_HOST/CARLA_PORT ({e})")
    rc = 1
if online and os.environ.get("IAI_API_KEY"):
    try:
        import logging
        import invertedai as iai
        logging.getLogger("invertedai.utils").addFilter(
            lambda r: "Commercial access denied" not in r.getMessage())
        iai.add_apikey(os.environ["IAI_API_KEY"])
        r = iai.location_info(location="carla:Town10HD")
        print(f"PASS  key accepted by the Inverted AI API (carla:Town10HD, max agents {r.max_agent_number})")
    except Exception as e:
        print(f"FAIL  Inverted AI API rejected the call: {type(e).__name__}: {e}")
        rc = 1
sys.exit(rc)
PY
  [ $? -eq 0 ] || BLOCKED=1
fi

if [ "${BLOCKED}" -eq 0 ]; then
  echo; echo "prerequisites OK — hard blockers clear"
else
  echo; echo "prerequisites BLOCKED — resolve FAIL lines above"
fi
exit "${BLOCKED}"
