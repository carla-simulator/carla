# Episode concurrency regressions

These tests construct the production `Client` and `Episode`, exchange real RPC
messages over loopback, and deserialize world-state packets delivered through
CARLA's TCP streaming transport. They cover map invalidation, concurrent error
publication, empty error messages, and latest-pending-message delivery. They do
not start Unreal Engine or validate vehicle physics.

The source is also included automatically in `libcarla_test_client`. This CMake
entry point builds only the seven Episode cases and gives CTest a 60-second
process timeout. Fixture teardown fails fast if an in-flight callback retains
the Episode for more than five seconds.

## Build and run

Use a C++20 compiler, CMake 3.27.2 or later, and the ordinary CARLA client build
prerequisites. CARLA's CMake configuration downloads its client dependencies;
Unreal Engine and simulator content are not needed. On the original `ue58-dev`
base, also apply the conditional Boost.Python dependency selection from
[PR #9994](https://github.com/carla-simulator/carla/pull/9994), which permits
configuration with `BUILD_PYTHON_API=OFF`.

From the repository root, on Linux:

```sh
export LC_ALL=C.UTF-8 LANG=C.UTF-8
cmake -S . -B Build/EpisodeTests \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES="$PWD/Util/Tests/EpisodeConcurrency/CMakeLists.txt" \
  -DBUILD_CARLA_UNREAL=OFF -DBUILD_CARLA_SERVER=OFF \
  -DBUILD_CARLA_CLIENT=ON -DBUILD_LIBCARLA_TESTS=ON \
  -DBUILD_PYTHON_API=OFF -DBUILD_EXAMPLES=OFF \
  -DBUILD_OSM_WORLD_RENDERER=OFF -DENABLE_ROS2=OFF -DENABLE_ROS2_DEMO=OFF \
  -DENABLE_RSS=OFF -DENABLE_OSM2ODR=OFF -DENABLE_PYTORCH=OFF \
  -DINSTALL_LIBCARLA=OFF
cmake --build Build/EpisodeTests --target carla-episode-tests --parallel 8
ctest --test-dir Build/EpisodeTests --output-on-failure --repeat until-fail:20
```

For ThreadSanitizer, use a separate build directory and add these options to the
configure command, so the actual client and dependencies are instrumented too:

```sh
-DCMAKE_CXX_FLAGS="-fsanitize=thread -fno-omit-frame-pointer" \
-DCMAKE_C_FLAGS="-fsanitize=thread -fno-omit-frame-pointer" \
-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=thread
```

Run with `TSAN_OPTIONS=halt_on_error=1`. If GCC's ThreadSanitizer exits at startup
with an unexpected memory mapping on Linux AArch64, run CTest with
`setarch aarch64 -R ctest ...` to disable ASLR for that process tree.

For AddressSanitizer and UndefinedBehaviorSanitizer, configure another build with
`-fsanitize=address,undefined` in place of `-fsanitize=thread` for all three
options, then run the same CTest command. Do not combine ThreadSanitizer with
AddressSanitizer.

To reproduce the original races, retain the tests and build against the original
`Episode.h` and `Episode.cpp`. Under ThreadSanitizer, these individual filters
report concurrent accesses to the original map-change flag or pending-message
state:

```sh
Build/EpisodeTests/carla-episode-tests --gtest_filter=episode_concurrency.map_change_has_one_consumer
Build/EpisodeTests/carla-episode-tests --gtest_filter=episode_concurrency.map_change_publication_races_consumers
Build/EpisodeTests/carla-episode-tests --gtest_filter=episode_concurrency.pending_exception_publication_races_streaming
```
