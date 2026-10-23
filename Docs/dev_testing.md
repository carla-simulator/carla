# Running the tests

CARLA has C++ unit tests for LibCarla, Python unit tests for the client module, and Python smoke and integration tests that drive a running server. Run the unit tests after any change to LibCarla or the Python API, and the smoke tests after any change to the server.

- [__Test suites__](#test-suites)
- [__LibCarla unit tests__](#libcarla-unit-tests)
- [__Python API unit tests__](#python-api-unit-tests)
- [__Smoke tests__](#smoke-tests)
- [__Integration tests__](#integration-tests)
- [__Other test suites__](#other-test-suites)
- [__What CI runs__](#what-ci-runs)

## Test suites

| Suite | Location | Needs a server | Runner |
|---|---|---|---|
| LibCarla unit tests | `LibCarla/source/test/` | No | GoogleTest binaries `libcarla_test_server` and `libcarla_test_client` |
| Python API unit tests | `PythonAPI/test/unit/` | No | `nose2` |
| Smoke tests | `PythonAPI/test/smoke/`, listed in `PythonAPI/test/smoke_test_list.txt` | Yes, on port 3654 | `nose2` |
| Integration tests | `PythonAPI/test/API/`, listed in `PythonAPI/test/integration_tests_list.txt` | Yes, on port 2000 | `nose2` |
| Lens validation | `PythonAPI/test/lens_validation/` | Only for the capture tests | `pytest` |
| Content pack tool tests | `Util/ContentPacks/test_carla_pack.py` | No | CMake target `content-packs-test` |
| Windows setup tests | `Util/SetupUtils/tests/` | No | Python `unittest` on Windows |

All commands on this page run from the CARLA root directory unless they start with a `cd`, and assume the `Build` directory from [Building CARLA in Linux](build_linux_ue5.md). If you configured with a preset, replace `Build` with `Build/<preset>`.

## LibCarla unit tests

The tests are built by default (`BUILD_LIBCARLA_TESTS` is `ON`), so `cmake --build Build` already produces them. To build only the test binaries:

```sh
cmake --build Build --target libcarla_test_server
cmake --build Build --target libcarla_test_client
```

`libcarla_test_server` holds the tests in `test/common/` and `test/server/`, `libcarla_test_client` the tests in `test/common/` and `test/client/`. The ROS2 middleware tests are compiled into `libcarla_test_server` only when the build is configured with `-DENABLE_ROS2=ON`.

The OpenDRIVE tests read their maps from `Build/test-content`. Fetch the same version CI uses:

```sh
rm -rf Build/test-content
git clone --depth 1 -b 0.1.4 https://github.com/carla-simulator/opendrive-test-files.git Build/test-content
```

!!! Note
    `Build/test-content` is compiled into the binaries as an absolute path under the source tree. Clone it there even when the binaries live in a preset directory such as `Build/Release`.

Run the binaries:

```sh
./Build/LibCarla/libcarla_test_server
./Build/LibCarla/libcarla_test_client
```

Both link `gtest_main`, so the standard GoogleTest flags work. To run a subset:

```sh
./Build/LibCarla/libcarla_test_client --gtest_filter='road.*'
```

## Python API unit tests

These tests import the `carla` module but do not connect to a server. Install the Python API and the test requirements, then run `nose2` from the `unit` folder:

```sh
cmake --build Build --target carla-python-api-install
pip3 install -r PythonAPI/test/requirements.txt
cd PythonAPI/test/unit
python3 -m nose2 -v
```

## Smoke tests

The smoke tests connect to `localhost:3654`, not the default port 2000, so start a dedicated server on that port. From a package:

```sh
./Linux/CarlaUnreal.sh -carla-rpc-port=3654
```

With the Python API and the test requirements installed (see [Python API unit tests](#python-api-unit-tests)), check that the server answers, then run the list from the `test` folder:

```sh
cd PythonAPI/test
python3 test_connection.py -p 3654 --timeout=60.0
python3 -m nose2 -v $(cat smoke_test_list.txt)
```

To run one module or one test, pass its dotted name instead of the list:

```sh
python3 -m nose2 -v smoke.test_sync
python3 -m nose2 -v smoke.test_map.TestMap
```

Add `-c smoke/unittest.cfg -X` to write the results as JUnit XML to `test-results.xml`.

The tests load maps and change world settings, and restore the starting map after each test, so do not run them against a server you are using for something else.

!!! Note
    `smoke.test_ros2` needs a server launched with `-ros2` (`./Linux/CarlaUnreal.sh -carla-rpc-port=3654 -ros2`). Without it, those tests fail. The tests check whichever middleware the server runs, so run them once per middleware, launching with `-rmw=fastdds`, `-rmw=cyclonedds` or `-rmw=zenoh`.

## Integration tests

The integration tests in `PythonAPI/test/API/` use the default client address, `127.0.0.1:2000`. With a server on the default port:

```sh
cd PythonAPI/test
python3 -m nose2 -v $(cat integration_tests_list.txt)
```

`PythonAPI/test/API/Tests.md` and the `spawn_actor.py` and `prop_control.py` scripts in the same folder are manual checks, not part of the list.

## Other test suites

__Windows setup__ tests run the batch scripts with recording executables instead of downloading dependencies or building Unreal. They require Windows, Python and the .NET Framework C# compiler included with Windows. No CARLA build is needed:

```sh
python -m unittest discover -s Util/SetupUtils/tests -v
```

__Lens validation__ checks the `sensor.camera.rt_lens` lens models. The math tests run offline; the capture tests skip unless you point them at a server. The full procedure is in `PythonAPI/test/lens_validation/README.md`.

```sh
cd PythonAPI/test/lens_validation
pip3 install numpy opencv-python scipy pytest
pytest . -q
```

__Content pack tool__ tests, covered in [Content packs](content_authoring_packs.md):

```sh
cmake --build Build --target content-packs-test
```

__Standalone scripts__ in `PythonAPI/test/` connect to a running server and print or save a report. They are not part of any list; run them with `--help` for their options.

| Script | What it checks |
|---|---|
| `tm_benchmark.py` | Traffic Manager lane keeping, speed adherence and collisions. `run --label <name> --out <file>.json` scores a build, `compare <baseline>.json <candidate>.json` diffs two runs |
| `test_vehicle_physics.py` | Acceleration, braking and turning for one vehicle blueprint |
| `test_two_wheeler_lean.py` | Inward lean of the two-wheelers. Replaces the current map, so give it its own server; `--port` is required |
| `test_raycast_sensor.py` | LiDAR and radar output and throughput |
| `test_ground_truth.py` | A sensor's output on a seeded, synchronous drive against a saved capture. `--generate` writes the capture to `_ground_truth/` first |

## What CI runs

Pull requests to `ue58-dev` run `.github/workflows/ue5_pr.yml` on Ubuntu. It configures with the `Release` preset, `-DENABLE_ROS2=ON` and `-DBUILD_LIBCARLA_TESTS=ON`, builds the client, the Python API and the server, then builds and runs the LibCarla unit tests:

```sh
cmake --preset Release -DENABLE_ROS2=ON -DBUILD_LIBCARLA_TESTS=ON
cmake --build Build/Release --target libcarla_test_server
cmake --build Build/Release --target libcarla_test_client
./Build/Release/LibCarla/libcarla_test_server
./Build/Release/LibCarla/libcarla_test_client
```

It then builds the package. CI does not run the Python unit, smoke or integration tests, so run them locally before opening a pull request that touches the Python API or the server.

Changes to the Windows setup scripts or their tests also run `.github/workflows/windows_setup.yml` on Windows. This runs the setup regression tests above, independently of the Ubuntu build.
