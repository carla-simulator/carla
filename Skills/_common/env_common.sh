#!/usr/bin/env bash
# Shared configuration load for every skill's env.sh. Sourced right after
# `set -euo pipefail`, before the skill resolves anything itself.
#
# A newcomer cannot supply paths when registering the MCP server: the paths do
# not exist yet, and the skill that creates them runs later. So the install
# skills record what they made in a config file, and this reads it back.
#
# Precedence, highest first:
#   1. an exported environment variable   one-off override, CI
#   2. ./.carla-tools.env                 a repo carrying its own CARLA
#   3. the user config                    the normal case
#   4. the sourcing env.sh's own search    last resort
#
# Only keys with no value yet are filled in, which is what keeps an explicit
# export winning. The config outranks each env.sh's search list because once the
# user has confirmed which of several checkouts to use, detection must not
# silently pick the other one.

carla_config_file() {
  if [ -n "${CARLA_TOOLS_CONFIG:-}" ]; then printf '%s' "${CARLA_TOOLS_CONFIG}"; return; fi
  printf '%s/carla-agentic-tools/config.env' "${XDG_CONFIG_HOME:-${HOME}/.config}"
}

# Reads KEY=value lines. Deliberately not `source`: a config file must not be
# able to run commands, and values are taken literally with no expansion.
carla_config_load() {
  local file key value
  for file in "$(carla_config_file)" "${PWD}/.carla-tools.env"; do
    [ -f "${file}" ] || continue
    while IFS='=' read -r key value; do
      case "${key}" in ''|'#'*) continue ;; esac
      key="${key%%[[:space:]]*}"
      case "${key}" in
        [A-Z]*) ;;
        *) continue ;;
      esac
      # An existing value — exported, or from the higher-precedence file — stays.
      if [ -z "$(eval "printf '%s' \"\${${key}:-}\"")" ]; then
        export "${key}=${value}"
      fi
    done < "${file}"
  done
}

carla_config_load
