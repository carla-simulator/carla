# Update: CMake minimum versions and default CMAKE_BUILD_TYPE

Summary

This document summarizes the recent changes made to the CARLA repository to address a CMake compatibility error and to make CMake configuration more user-friendly.

Problem

- CMake failed with: "Compatibility with CMake < 3.5 has been removed from CMake." Some CMakeLists used very old minimum versions (e.g., 2.8.9, 3.1) and newer CMake versions treat that as incompatible.
- The project also fatal-error'd during configure when `CMAKE_BUILD_TYPE` was empty because `LibCarla` expects specific build types (Client/Server/Pytorch/ros2).

What I changed

1. Bumped required CMake versions
   - Updated `cmake_minimum_required` in multiple CMake scripts from 3.5.1 (and older) to 3.10 to silence deprecation warnings and require modern CMake (>= 3.10).
   - Files updated include (non-exhaustive list):
     - `CMakeLists.txt` (root)
     - `LibCarla/cmake/CMakeLists.txt`
     - `LibCarla/cmake/client/CMakeLists.txt`
     - `LibCarla/cmake/server/CMakeLists.txt`
     - `LibCarla/cmake/fast_dds/CMakeLists.txt`
     - `LibCarla/cmake/pytorch/CMakeLists.txt`
     - `LibCarla/cmake/test/CMakeLists.txt`
     - `osm-world-renderer/CMakeLists.txt`
     - `Util/DockerUtils/fbx/CMakeLists.txt` (was 2.8.9)
     - `Build/libosm2dr-source/CMakeLists.txt` (was 3.1)

2. Defaulted CMAKE_BUILD_TYPE when missing
   - Added logic to the top-level `CMakeLists.txt` to set a default `CMAKE_BUILD_TYPE` = `Client` (cache) when none is specified and the generator is single-configuration.
   - This prevents the fatal error triggered in `LibCarla/cmake/CMakeLists.txt` when `CMAKE_BUILD_TYPE` is empty.

Commands run (for validation)

- Search for CMake minimums and references (used programmatically during changes):

  - grep for cmake_minimum_required across repo

- Validate CMake configure (explicit build type):

```bash
cmake -S . -B Build/test-config -DCMAKE_BUILD_TYPE=Client
```

- Validate CMake configure after bumping to 3.10 (explicit build type):

```bash
cmake -S . -B Build/test-config2 -DCMAKE_BUILD_TYPE=Client
```

- Validate CMake configure without specifying build type (now defaults to Client):

```bash
cmake -S . -B Build/test-config-default
```

Expected output includes an informational line:

-- No CMAKE_BUILD_TYPE specified. Defaulting to 'Client'.

and then "Configuring done" and "Build files have been written to: ..."

Notes & rationale

- I chose 3.10 as the minimum because CMake explicitly warned that compatibility with CMake < 3.10 will be removed in a future version; 3.10 is a reasonable baseline for modern features and widely available on CI and modern distributions.
- Defaulting `CMAKE_BUILD_TYPE` to `Client` keeps the behavior consistent with how the CARLA project expects to be configured and avoids fatal errors for users who forget to pass a build type.
- The default is only applied for single-configuration generators (i.e., when `CMAKE_CONFIGURATION_TYPES` is not set). Multi-configuration generators (Visual Studio, Xcode) are unaffected.

Files changed (concrete)

- `CMakeLists.txt` (root): set `cmake_minimum_required(VERSION 3.10)` and add default `CMAKE_BUILD_TYPE` handling.
- `LibCarla/cmake/CMakeLists.txt`: bumped to 3.10 (and other LibCarla sub CMake files listed above).
- `Build/libosm2dr-source/CMakeLists.txt` and `Util/DockerUtils/fbx/CMakeLists.txt` were updated from very old versions to 3.10.

Repro steps for maintainers

1. Ensure you have CMake >= 3.10 installed on your machine or CI.
2. From the repo root, run:

```bash
cmake -S . -B Build/test-config-default
```

3. If you want a specific build type use:

```bash
cmake -S . -B Build/test-config -DCMAKE_BUILD_TYPE=Server
```

Follow-ups (recommended)

- Update documentation (README.md and relevant Docs pages) to state the new minimum CMake version (>= 3.10).
- Update CI job definitions to ensure they use CMake >= 3.10.
- Optionally: convert the `cmake_minimum_required` lines to the range syntax (e.g. `3.5...3.10`) if the project needs to advertise backward-compatibility to a specific older policy while marking newer policy compatibility.
- Consider adding a quick pre-check script in `Util/BuildTools/` that asserts CMake minimum version and prints actionable instructions.

Contact

If you want, I can open a PR with these changes (branch already exists: `test-changes-build-karthik`) and add a brief PR description. I can also:

- Update docs to require CMake >= 3.10,
- Add the pre-check script,
- Or revert any change you prefer not to accept.


---
Document generated automatically by build modifications session.
