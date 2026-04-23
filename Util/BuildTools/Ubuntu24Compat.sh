#!/bin/bash
# Compatibility shim for building CARLA with the UE4 bundled clang on modern
# Linux distributions.  Source this file AFTER exporting CC and CXX.
#
# Two independent issues are detected and patched at runtime so older distros
# that don't have these problems are completely unaffected:
#
#   1. Bundled ld (2019) cannot read .relr.dyn sections present in glibc ≥ 2.36
#      (Ubuntu 22.04+, Fedora 36+, Arch …).  Fix: wrap CC/CXX to use lld.
#
#   2. CMake ≥ 4.0 errors on cmake_minimum_required < 3.5 in third-party sources.
#      Fix: wrap cmake to inject -DCMAKE_POLICY_VERSION_MINIMUM=3.5.
#
# Usage (after CC/CXX are exported):
#   source "$(dirname "$0")/Ubuntu24Compat.sh"

_WRAP_DIR="${CARLA_BUILD_FOLDER}"
mkdir -p "${_WRAP_DIR}"

# ---------------------------------------------------------------------------
# 1. Bundled ld compatibility check
# ---------------------------------------------------------------------------
_NEED_LLD=false
_LD_TEST_SRC="$(mktemp /tmp/ue4_ld_test_XXXXXX.c)"
_LD_TEST_BIN="$(mktemp /tmp/ue4_ld_test_XXXXXX)"
echo 'int main(void){return 0;}' > "${_LD_TEST_SRC}"
if ! "${CC}" "${_LD_TEST_SRC}" -o "${_LD_TEST_BIN}" 2>/dev/null; then
    _NEED_LLD=true
fi
rm -f "${_LD_TEST_SRC}" "${_LD_TEST_BIN}"

if ${_NEED_LLD}; then
    if ! which ld.lld >/dev/null 2>&1; then
        echo "ERROR: The UE4 bundled linker cannot link on this system."
        echo "       Install lld and try again:  sudo apt install lld"
        exit 1
    fi
    _CC_REAL="${CC}"
    _CXX_REAL="${CXX}"
    CC="${_WRAP_DIR}/clang.sh"
    CXX="${_WRAP_DIR}/clang++.sh"
    # -Wno-unused-command-line-argument silences the warning that -fuse-ld=lld
    # produces when clang is invoked for compilation only (not linking).
    printf '#!/bin/bash\nexec "%s" -fuse-ld=lld -Wno-unused-command-line-argument "$@"\n' \
        "${_CC_REAL}"  > "${CC}"
    printf '#!/bin/bash\nexec "%s" -fuse-ld=lld -Wno-unused-command-line-argument "$@"\n' \
        "${_CXX_REAL}" > "${CXX}"
    chmod +x "${CC}" "${CXX}"
    export CC CXX
    export PATH="${_WRAP_DIR}:${PATH}"
fi

# ---------------------------------------------------------------------------
# 2. System Python PEP 668 check (Ubuntu ≥ 24)
# ---------------------------------------------------------------------------
# Ubuntu 24+ ships Python as "externally managed" (PEP 668), so a bare
# `pip install` into the system environment is blocked.  The wheel is still
# built and placed in PythonAPI/carla/dist/ for manual installation.
_OS_MAJOR=$(. /etc/os-release 2>/dev/null && printf '%s' "${VERSION_ID}" | cut -d. -f1)
if [ "${_OS_MAJOR:-0}" -ge 24 ] 2>/dev/null; then
    export _SKIP_PIP_INSTALL=true
fi

# ---------------------------------------------------------------------------
# 3. CMake 4.x compatibility check
# ---------------------------------------------------------------------------
_CMAKE_MAJOR=$(cmake --version 2>/dev/null | grep -oP '(?<=cmake version )\d+' | head -1)
if [ "${_CMAKE_MAJOR:-0}" -ge 4 ]; then
    printf '#!/bin/bash\nexec /usr/bin/cmake -DCMAKE_POLICY_VERSION_MINIMUM=3.5 "$@"\n' \
        > "${_WRAP_DIR}/cmake"
    chmod +x "${_WRAP_DIR}/cmake"
    # Only prepend to PATH if not already there (avoid duplicates on re-source)
    case ":${PATH}:" in
        *":${_WRAP_DIR}:"*) ;;
        *) export PATH="${_WRAP_DIR}:${PATH}" ;;
    esac
fi
