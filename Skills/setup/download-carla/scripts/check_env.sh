#!/usr/bin/env bash
# Prerequisite checks for download-carla. Read-only, no sudo, downloads nothing.
# Exits non-zero only on hard blockers (no way to fetch, or no space to fetch into).
# On-camera banner: every skill run announces itself, so a terminal recording
# shows which skill is doing the work rather than just its output. The name is
# taken from the directory so it cannot drift from the skill it belongs to.
printf '\n\033[1;36m>> using skill: %s\033[0m\n' \
  "$(basename "$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)")"

set -uo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck disable=SC1091
source "${HERE}/env.sh" >/dev/null

rc=0
ok(){   echo "PASS  $*"; }
warn(){ echo "WARN  $*"; }
bad(){  echo "FAIL  $*"; rc=1; }

echo "== Tools =="
command -v curl >/dev/null && ok "curl (resumable downloads, redirect resolution)" \
  || warn "curl missing — falls back to urllib: no resume, slower"
command -v tar  >/dev/null && ok "tar" || bad "tar missing — cannot extract a Linux release"
command -v git  >/dev/null && ok "git (needed only for the 'git' mode)" || warn "git missing — 'git' mode unavailable"
command -v docker >/dev/null && ok "docker (needed only for the 'docker' mode)" || warn "docker missing — 'docker' mode unavailable"
"${PYTHON}" -c 'import urllib.request, tarfile, zipfile' 2>/dev/null \
  && ok "${PYTHON} has urllib/tarfile/zipfile" || bad "${PYTHON} cannot import urllib/tarfile/zipfile"

echo "== Network =="
if command -v curl >/dev/null; then
  curl -fsS --max-time 15 -o /dev/null "https://api.github.com/repos/carla-simulator/carla/releases?per_page=1" \
    && ok "GitHub API reachable (release list + download URLs)" \
    || bad "cannot reach api.github.com — no way to resolve downloads"
  # The mirrors are different hosts from the API; reaching one says nothing about
  # the others, and this is where the multi-GB bytes come from. Two are probed
  # because they hold different sets: downloads.carlasim.com serves 0.9.16 and the
  # nightly, the Backblaze bucket is the only host still serving the older
  # releases and the 0.10 (UE5) line. The retired carla-releases.b-cdn.net zone,
  # which the older release bodies still link to, 403s everything — the script
  # remaps around it, so it is deliberately not probed.
  mirrors_up=0
  curl -fsSI --max-time 15 -o /dev/null "https://downloads.carlasim.com/Linux/Dev/CARLA_Latest.tar.gz" \
    && { ok "downloads.carlasim.com reachable (0.9.16 + nightly)"; mirrors_up=1; } \
    || warn "downloads.carlasim.com unreachable"
  curl -fsSI --max-time 15 -o /dev/null "https://carla-releases.s3.us-east-005.backblazeb2.com/Linux/Dev/CARLA_Latest.tar.gz" \
    && { ok "carla-releases Backblaze mirror reachable (all versions)"; mirrors_up=1; } \
    || warn "Backblaze mirror unreachable"
  [ "${mirrors_up}" -eq 1 ] || bad "no download mirror reachable — downloads will fail"
else
  warn "curl missing — cannot probe connectivity here"
fi

echo "== Destination and disk =="
DEST="${CARLA_DOWNLOAD_DIR}"
PROBE="${DEST}"; while [ ! -d "${PROBE}" ] && [ "${PROBE}" != "/" ]; do PROBE="$(dirname "${PROBE}")"; done
if [ -w "${PROBE}" ]; then ok "writable: ${PROBE}"; else bad "not writable: ${PROBE} — set CARLA_DOWNLOAD_DIR"; fi
FREE_G=$(df -BG --output=avail "${PROBE}" 2>/dev/null | tail -1 | tr -dc '0-9')
# A 0.9.x release is ~8.3 GB compressed and needs roughly as much again to
# extract; AdditionalMaps adds ~14.8 GB on top. Hence the two thresholds.
if [ "${FREE_G:-0}" -ge 20 ]; then
  ok "${FREE_G}G free (enough for a release + extraction)"
elif [ "${FREE_G:-0}" -ge 10 ]; then
  warn "${FREE_G}G free — tight: a release needs ~19G (8.3G archive + extraction)"
else
  bad "${FREE_G:-?}G free — a CARLA release needs ~19G; free space or set CARLA_DOWNLOAD_DIR elsewhere"
fi
[ "${FREE_G:-0}" -ge 50 ] || warn "AdditionalMaps (--with-maps) needs ~15G more on top"

echo "== Result =="
[ "$rc" -eq 0 ] && echo "prerequisites OK (warnings are non-blocking)" \
               || echo "prerequisites BLOCKED — resolve FAIL lines above"
exit $rc
