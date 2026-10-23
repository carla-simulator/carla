# Streaming listener regression tests

This standalone target compiles the production streaming classes and their TCP
session implementations. It needs CMake 3.20+, a C++20 compiler, Boost headers,
GoogleTest and the CARLA rpclib fork's headers. It needs neither Unreal Engine nor
a CARLA runtime. The fixture binds ephemeral loopback ports and constructs real
sessions; it tests membership and visibility without starting their I/O callbacks.

On Ubuntu 24.04:

```sh
sudo apt-get update
sudo apt-get install -y cmake ninja-build g++ libboost-dev libgtest-dev git
git clone --depth 1 --branch carla https://github.com/carla-simulator/rpclib.git Build/stream-rpclib
cmake -S Util/Tests/StreamListeners -B Build/StreamListeners -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug -DCARLA_RPCLIB_INCLUDE_DIR="$PWD/Build/stream-rpclib/include"
cmake --build Build/StreamListeners --parallel 2
ctest --test-dir Build/StreamListeners --output-on-failure --repeat until-fail:20
```

The same target supports MSVC: pass `-DBOOST_ROOT=<boost-root>`,
`-DGTest_DIR=<installed-gtest>/lib/cmake/GTest` and
`-DCARLA_RPCLIB_INCLUDE_DIR=<rpclib>/include`, then build and run CTest with
`--config Debug` / `-C Debug`. Build and install GoogleTest with the same compiler,
architecture and runtime library first. For example, use
`-Dgtest_force_shared_crt=ON` with the default MSVC runtime.

## Sanitizers

Use separate build directories and set `-DCARLA_STREAM_SANITIZERS=thread` or
`-DCARLA_STREAM_SANITIZERS=address,undefined`. Both the actual production subset
and the test executable are instrumented. Run the full suite twenty times:

```sh
TSAN_OPTIONS=halt_on_error=1 ./Build/StreamListenersTsan/carla-stream-listener-tests --gtest_repeat=20
ASAN_OPTIONS=detect_leaks=1 UBSAN_OPTIONS=halt_on_error=1 \
  ./Build/StreamListenersAsan/carla-stream-listener-tests --gtest_repeat=20
```

If ThreadSanitizer aborts at startup with `unexpected memory mapping` on a Linux
kernel with wider address randomization, use a process-local launcher:

```sh
TSAN_OPTIONS=halt_on_error=1 setarch "$(uname -m)" -R \
  ./Build/StreamListenersTsan/carla-stream-listener-tests --gtest_repeat=20
```

This changes only that process's address randomization. It requires permission to
use `personality(2)`; restricted containers may block it. A startup failure is
not a test pass. No suppressions or system-wide address randomization changes are
needed. The focused workflow runs this launcher directly, so CTest discovery does
not launch an unwrapped ThreadSanitizer process.

The tests cover client churn, concurrent force/clear and ROS visibility changes,
Dispatcher stream creation/removal concurrent with ROS queries, and the existing
reset rules: the last disconnect and `ClearSessions` clear forced activity while
retaining ROS visibility. The ROS bit and client bit remain independent.

## Getter benchmark

Configure a separate **Release, unsanitized** build with
`-DCARLA_STREAM_BUILD_BENCHMARK=ON`, then run
`./Build/StreamListenersRelease/carla-stream-listener-benchmark`. It prints JSON
with seven samples of ten million queries for inactive, forced, ROS-only and
one-client states, plus a checksum and `sizeof(MultiStreamState)`.

The query is in a separate translation unit with link-time optimization disabled
to prevent hoisting. All benchmark mutations happen before timing, so comparing
an earlier revision does not measure a data race. For a comparison, compile each
revision independently with identical dependencies, compiler and flags; alternate
the two executables for at least three rounds on the same idle CPU (for example,
`taskset -c 0 <executable>` on Linux), and compare pooled sample medians. Record the
CPU, compiler and build flags with the JSON. Fractions of a nanosecond are sensitive
to noise; this benchmark checks getter cost, not end-to-end sensor throughput.
