#!/usr/bin/env bash
# ScenarioRunner catches execution exceptions without setting a failing status.
# A criteria verdict describes the scenario outcome after execution completed.
carla_sr_result() {
  local status="$1" log="$2"
  if [ "${status}" -ne 0 ]; then
    echo "[run] FAIL ScenarioRunner exited ${status}" >&2
    return "${status}"
  fi
  if grep -q 'Traceback (most recent call last):' "${log}"; then
    echo "[run] FAIL ScenarioRunner reported an execution exception" >&2
    return 1
  fi
  if ! grep -q 'Criteria Information' "${log}"; then
    echo "[run] FAIL no criteria report was produced" >&2
    tail -5 "${log}" | sed 's/^/[run]        /' >&2
    return 1
  fi
  echo "[run] PASS a criteria report was produced"
  if grep -qE 'GLOBAL RESULT.*(FAILURE|TIMEOUT)' "${log}"; then
    echo "[run] NOTE the scenario completed with unsuccessful criteria"
  fi
}
