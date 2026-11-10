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

# --- portability helpers ------------------------------------------------------
# Linux is the primary platform and every helper below behaves there as the
# inline code it replaced. The other target is Git Bash (MSYS2) on Windows
# driving a NATIVE Windows Python: that interpreter cannot read `/d/x` paths or
# a ':'-separated PYTHONPATH, and the shell has no ss, lsof, pgrep, pkill or
# /proc, so each of those needs a fallback rather than a silently wrong answer.

# True under Git Bash, MSYS2 or Cygwin.
carla_is_windows() {
  case "$(uname -s 2>/dev/null)" in
    MINGW*|MSYS*|CYGWIN*) return 0 ;;
  esac
  return 1
}

# One path in the form a native Windows program expects (D:\x\y); unchanged
# everywhere else. Takes ONE path: convert a list element by element, because
# `cygpath -wp` on a joined list misreads any element that is already native.
carla_native_path() {
  if carla_is_windows && command -v cygpath >/dev/null 2>&1; then
    cygpath -w "$1"
  else
    printf '%s\n' "$1"
  fi
}

# Prepends the given directories, in order, to PYTHONPATH. Empty arguments are
# skipped and no separator is left dangling. The separator is the one the
# interpreter in $PYTHON splits on, so a native Windows Python gets ';' and
# D:\ paths while an MSYS/Cygwin Python keeps ':' and POSIX paths.
carla_pythonpath_prepend() {
  local sep=":" native=0 new="" d
  if carla_is_windows; then
    sep="$("${PYTHON:-python3}" -c 'import os;print(os.pathsep)' 2>/dev/null | tr -d '\r' || true)"
    [ -n "${sep}" ] || sep=";"
    [ "${sep}" = ";" ] && native=1
  fi
  for d in "$@"; do
    [ -n "${d}" ] || continue
    [ "${native}" -eq 1 ] && d="$(carla_native_path "${d}")"
    new="${new:+${new}${sep}}${d}"
  done
  export PYTHONPATH="${new}${PYTHONPATH:+${new:+${sep}}${PYTHONPATH}}"
}

# Exit 0 when something accepts TCP connections on port $1 at host $2
# (default 127.0.0.1). Uses bash's /dev/tcp, so it needs no ss or nc and works
# in Git Bash; bounded by `timeout` when present so a filtered remote port
# cannot hang a preflight.
carla_port_busy() {
  local host="${2:-127.0.0.1}"
  if command -v timeout >/dev/null 2>&1; then
    # shellcheck disable=SC2016  # expanded by the inner bash, from $0/$1
    timeout 3 bash -c 'exec 3<>"/dev/tcp/$0/$1"' "${host}" "$1" >/dev/null 2>&1
  else
    (exec 3<>"/dev/tcp/${host}/$1") >/dev/null 2>&1
  fi
}

# Who listens on local TCP port $1, as "name pid"; empty when nothing does.
# Tries ss, then lsof, then (Windows) netstat -ano + tasklist, then a plain
# connect. With none of the tools a busy port reads "in use (owner unknown)":
# a port must never be reported free merely because nothing could look.
carla_port_listener() {
  local p="$1" line pid name
  if command -v ss >/dev/null 2>&1; then
    line="$(ss -ltnpH "sport = :$p" 2>/dev/null | head -1)"
    [ -z "$line" ] && return 0
    echo "$line" | grep -oP 'users:\(\("\K[^"]+' | head -1 | tr -d '\n'
    echo -n " "
    echo "$line" | grep -oP 'pid=\K[0-9]+' | head -1
    return 0
  fi
  if command -v lsof >/dev/null 2>&1; then
    lsof -nP -iTCP:"$p" -sTCP:LISTEN -Fcp 2>/dev/null \
      | awk '/^c/{c=substr($0,2)} /^p/{p=substr($0,2)} END{if (c) print c, p}'
    return 0
  fi
  if carla_is_windows && command -v netstat >/dev/null 2>&1; then
    # A listener is the row whose foreign address is the wildcard. The state
    # column is localized (LISTENING, ESCUCHANDO, ABHÖREN...), so never match
    # on it; the PID is the last column either way.
    pid="$({ netstat -ano -p TCP; netstat -ano -p TCPv6; } 2>/dev/null | tr -d '\r' \
           | awk -v port="$p" '($3 == "0.0.0.0:0" || $3 == "[::]:0") {
               n = split($2, a, ":"); if (a[n] == port) { print $NF; exit } }')"
    if [ -n "${pid}" ]; then
      # Without MSYS_NO_PATHCONV, Git Bash rewrites /FI into a path.
      name="$(MSYS_NO_PATHCONV=1 tasklist /FI "PID eq ${pid}" /FO CSV /NH 2>/dev/null \
              | tr -d '\r' | head -1 | cut -s -d'"' -f2)"
      echo "${name:-?} ${pid}"
    fi
    return 0
  fi
  if carla_port_busy "$p"; then
    echo "in use (owner unknown)"
  fi
  return 0
}

# PIDs (space-separated) of processes named exactly like one of the arguments;
# exit 1 when there are none. pgrep -x on Linux; tasklist on Windows, where the
# names carry their .exe.
carla_proc_find() {
  local n out="" pids
  for n in "$@"; do
    if carla_is_windows; then
      pids="$(MSYS_NO_PATHCONV=1 tasklist /FI "IMAGENAME eq ${n}" /FO CSV /NH 2>/dev/null \
              | tr -d '\r' | cut -s -d'"' -f4 | tr '\n' ' ')"
    else
      pids="$(pgrep -x "${n}" 2>/dev/null | tr '\n' ' ')"
    fi
    out="${out}${pids}"
  done
  out="${out% }"
  [ -n "${out}" ] || return 1
  printf '%s\n' "${out}"
}

# Stops every process named exactly like an argument: pkill -x (SIGTERM) on
# Linux; taskkill /F /T on Windows, where a console-less server has no SIGTERM
# to receive and the launcher's child has to go with it.
carla_proc_kill() {
  local n
  for n in "$@"; do
    if carla_is_windows; then
      MSYS_NO_PATHCONV=1 taskkill /F /T /IM "${n}" >/dev/null 2>&1 || true
    else
      pkill -x "${n}" 2>/dev/null || true
    fi
  done
}
