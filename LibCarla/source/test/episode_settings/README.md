# Episode settings conversion component checks

This standalone target compiles the actual RPC `EpisodeSettings.h` conversion
bodies and the actual Carla plugin `FEpisodeSettings` declaration. Reflection
macros, UHT's unused generated include and the `TOptional` boundary are inert.
The fixture does not build Unreal, execute rendering or replace conversion code.
Keep it separate from the normal host test binaries so their RPC class
definitions are never mixed with definitions enabling Unreal conversions.

After configuring CARLA dependencies, pass their source directories:

```sh
cmake -S LibCarla/source/test/episode_settings -B Build/EpisodeSettingsUnits \
  -DCARLA_BOOST_SOURCE_DIR=/path/to/boost-src \
  -DCARLA_RPCLIB_SOURCE_DIR=/path/to/rpclib-src
cmake --build Build/EpisodeSettingsUnits
ctest --test-dir Build/EpisodeSettingsUnits --output-on-failure
```

Use a Visual Studio developer environment on Windows. `CARLA_SOURCE_ROOT` can
point at another checkout for an unchanged-upstream comparison using the same
test fixture. The one CTest executes eleven checks: both conversion directions
at four positive distances, the zero/no-limit setting, the existing tile/actor
conversions and a full settings round trip.

The Python API documents `max_culling_distance` in metres. The engine consumes
centimetres, as described in [Epic's units reference](https://dev.epicgames.com/documentation/unreal-engine/units-of-measurement-in-unreal-engine).
Actual mesh culling and an Unreal build remain separate runtime checks.
