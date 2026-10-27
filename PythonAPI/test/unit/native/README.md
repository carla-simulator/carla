These host tests require CPython development headers and GTest; no simulator is needed.

```sh
cmake -S PythonAPI/test/unit/native -B Build/RawDataBufferTests -DCMAKE_BUILD_TYPE=Release
cmake --build Build/RawDataBufferTests --config Release
ctest --test-dir Build/RawDataBufferTests -C Release --output-on-failure
```

The eight C API cases check native ownership, independent and derived views, read-only byte layout, zero-copy updates, empty buffers, private-type lifetime and constructor rejection.

To exercise the actual sensor-binding getters with real serialized radar/image/optical-flow packets, use a persistent Linux Python API backend build containing its compiled binding objects, `compile_commands.json`, and Ninja target `carla`:

```sh
python3 PythonAPI/test/unit/native/build_sensor_data_fixture.py --binding-build /path/to/backend/build --output Build/SensorDataFixture
PYTHONPATH=Build/SensorDataFixture python3 PythonAPI/test/unit/native/sensor_data_binding_regressions.py -v
```

The builder includes the production sensor translation unit unchanged and adds private test-only producer/weak-owner functions. It compiles one replacement object and links a separate module using the other configured objects and libraries. Production source, object and module hashes are checked before and after. Private factory exports never enter the production target.

The eleven binding cases check actual packet storage lifetime before any post-deletion read, including slices, casts, NumPy consumers, multiple exports, empty radar data and color-coded optical-flow images. Native weak ownership observes the allocation directly; the tests do not depend on keeping its Python wrapper alive. The existing unrelated FakeImage sequence-binding error is preserved.

Use the matching production source/header/library build. An explicit `--source` overlay is available for scoped component experiments; its recorded results do not establish a full exact-head production build. The configured-command fixture builder currently targets Linux; the core C API tests are portable.
