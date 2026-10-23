# Client primitive benchmarks

`recurrent_shared_future.cpp` measures the production `RecurrentSharedFuture`
with actual `WorldSnapshot` values. It does not require Unreal or a CARLA server.

## Build and run

Use a C++20 compiler, CMake 3.20+, Boost headers, and CARLA's rpclib headers.
For example, on Ubuntu, from the CARLA repository root:

```bash
sudo apt-get install g++ cmake ninja-build libboost-dev git
git clone https://github.com/carla-simulator/rpclib.git /tmp/carla-benchmark-rpclib
git -C /tmp/carla-benchmark-rpclib checkout bbfe141f4b5c1396cedb91071f8a839a6fa9de35
cmake -S Util/Benchmarks -B Build/ClientBenchmarks -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCARLA_RPCLIB_INCLUDE_DIR=/tmp/carla-benchmark-rpclib/include
cmake --build Build/ClientBenchmarks
Build/ClientBenchmarks/carla-recurrent-future-benchmark
```

Windows also supports this CMake project with a Visual Studio generator. Set
`Boost_ROOT` to the extracted Boost directory if Boost is not installed, set
`CARLA_RPCLIB_INCLUDE_DIR` as above, build `--config Release`, and run the executable
from the build directory's `Release` subdirectory.

## Method and interpretation

Each historical waiter calls `WaitFor(0ms)` and then parks outside the future.
All waiters have timed out before publication starts; keeping the threads alive
ensures distinct thread identities. The benchmark warms up with 1,000 updates,
then reports seven samples of 100,000 `SetValue` calls. It also reports the
snapshot state's reference count and repeated same-thread zero-timeout cost.

Compare the same source and compiler settings against the original and changed
headers. Repeat both binaries in alternating order to reduce thermal and load
bias. The two state references held by the benchmark itself are expected;
additional references indicate retained inactive waiters.

On Windows 11 x64, Intel Core Ultra 7 155H, MSVC 19.51 Release, comparing
`ue58-dev` commit `defabe35` with the waiter-lifetime fix, two alternating runs
produced these ranges of seven-sample medians:

| Inactive waiters | Original publish time | Fixed publish time | Original/fixed state references |
| ---: | ---: | ---: | ---: |
| 0 | 22–23 ns | 26–28 ns | 2 / 2 |
| 1 | 49–54 ns | 24–26 ns | 3 / 2 |
| 32 | 782–827 ns | 24 ns | 34 / 2 |
| 128 | 3.23–3.25 µs | 23–24 ns | 130 / 2 |
| 256 | 8.50–9.10 µs | 23–30 ns | 258 / 2 |

Repeated `WaitFor(0ms)` improved from 187–201 ns to 86–88 ns in those runs.
Absolute timings vary with the host and workload. These results measure the
future primitive; they do not establish an improvement in simulator FPS.

The waiter-lifetime fix changes the private future/episode object layout.
Rebuild LibCarla and its consumers together when evaluating the change.
