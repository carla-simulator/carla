#!/usr/bin/env bash
# Prerequisite checks for restyle-carla-ue58-signs. Read-only, fast, never hangs:
# every network call has a short timeout.
# Hard blockers: no curl/python3, unreal-mcp not answering, sign catalog not loaded.
printf '\n\033[1;36m>> using skill: %s\033[0m\n' \
  "$(basename "$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)")"

set -uo pipefail
# Recorded paths (UNREAL_MCP_URL among them, if the user set one); an export still wins.
. "$(dirname "${BASH_SOURCE[0]}")/../../../_common/env_common.sh"
set +e
URL="${UNREAL_MCP_URL:-http://localhost:6256/mcp}"
rc=0
ok(){   echo "  PASS $*"; }
warn(){ echo "  WARN $*"; }
bad(){  echo "  FAIL $*"; rc=1; }
H=(-H 'Content-Type: application/json' -H 'Accept: application/json, text/event-stream')

echo "== tools =="
command -v curl    >/dev/null && ok "curl"    || bad "curl missing (the MCP client uses it)"
command -v python3 >/dev/null && ok "python3" || bad "python3 missing"

echo "== Unreal Editor MCP server (${URL}) =="
init='{"jsonrpc":"2.0","id":1,"method":"initialize","params":{"protocolVersion":"2025-06-18","capabilities":{},"clientInfo":{"name":"check","version":"0"}}}'
resp="$(curl -s -i -m 5 -X POST "${URL}" "${H[@]}" -d "${init}" 2>/dev/null)"
if ! grep -q '"result"' <<<"${resp}"; then
  if curl -s -m 2 -o /dev/null "${URL}" 2>/dev/null; then
    bad "the port is open but the server does not answer: the editor is still loading the map (wait and retry)"
  else
    bad "nothing listening: open the CarlaUnreal editor (not -game, not a package) and wait for the map to load"
    echo "       its log says 'LogModelContextProtocol: Starting MCP server on port 6256'"
  fi
  exit 1
fi
ok "unreal-mcp answers"
sid="$(grep -i '^mcp-session-id:' <<<"${resp}" | awk '{print $2}' | tr -d '\r')"
curl -s -m 5 -X POST "${URL}" "${H[@]}" -H "Mcp-Session-Id: ${sid}" \
  -d '{"jsonrpc":"2.0","method":"notifications/initialized"}' >/dev/null 2>&1

call(){ # toolset tool json-args
  curl -s -m 60 -X POST "${URL}" "${H[@]}" -H "Mcp-Session-Id: ${sid}" \
    -d "{\"jsonrpc\":\"2.0\",\"id\":2,\"method\":\"tools/call\",\"params\":{\"name\":\"call_tool\",\"arguments\":{\"toolset_name\":\"$1\",\"tool_name\":\"$2\",\"arguments\":$3}}}" 2>/dev/null
}

echo "== content =="
cat_resp="$(call editor_toolset.toolsets.asset.AssetTools exists '{"path":"/CarlaDigitalTwinsTool/Carla/Blueprints/LevelDesign/Signs/Catalog/MUTCD/Priority/DA_stop_R1_1"}')"
if grep -q 'true' <<<"${cat_resp}"; then
  ok "regional sign catalog (CarlaDigitalTwinsTool content) is loaded"
else
  bad "no regional sign catalog: install the CarlaDigitalTwinsTool content (Docs/content_authoring_maps_digital_twin.md, step 4)"
fi
lvl="$(call editor_toolset.toolsets.scene.SceneTools get_current_level '{}' | grep -o '/Game/[^"\\]*' | head -1)"
[ -n "${lvl}" ] && ok "level open: ${lvl}" || warn "could not read the open level"

echo
[ ${rc} -eq 0 ] && echo "READY" || echo "NOT READY"
exit ${rc}
