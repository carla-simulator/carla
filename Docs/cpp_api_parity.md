# C++ API parity roadmap

This document summarizes the first track of the C++ API effort.

## Why this track exists

CARLA already exposes a substantial C++ client API through `LibCarla`, but it is much less discoverable than the Python API. Community demand repeatedly points to that gap:

- #9554 — C++ equivalent of Python `BasicAgent` / `BehaviorAgent`
- #6091 — request to include the CARLA C++ client API in another project
- #3731 — long-running discussion about the future of the C++ API

The goal of this track is not to rewrite the core API. It is to make the existing C++ surface easier to find, easier to understand, and easier to use.

## Scope for the first PR

- Add a practical C++ quickstart
- Add a small set of common usage examples
- Explain how the C++ client maps to the Python workflow
- Call out where the C++ API is intentionally lower-level
- Link back to the community requests above so the motivation is visible

## Suggested contents

- Connecting a `carla::client::Client`
- Querying the world and map
- Spawning and destroying actors
- Applying batch commands
- Working with sensors and snapshots
- Using recorder and replay helpers

## Current implementation status

Implemented, in the [C++ tutorial](tuto_cpp.md) and `Examples/CppClient`:

- C++ quickstart and run instructions
- Python → C++ API mapping (core client/world/spawn/destroy)
- Basic client workflow example (connect/load world/spawn/control/destroy)
- Batch commands, world snapshots, and recorder/replay helpers
- Navigation agents in C++: `BasicAgent`, `BehaviorAgent`, `ConstantVelocityAgent`, `LocalPlanner` and `GlobalRoutePlanner` in `LibCarla/source/carla/agents/navigation`, the C++ equivalent requested in #9554
- `carla-client` decoupled from Python: `Boost::python` and the `LIBCARLA_WITH_PYTHON_SUPPORT` macro removed, `PythonUtil.h` moved to `PythonAPI/carla/include/carla`. Downstream C++ projects can link `carla-client` without pulling in any Python dependency, a partial step toward the Boost.Python concern in #3731.

Still pending:

- An explicit section documenting why and where C++ is intentionally lower-level than the Python wrappers

## Non-goals

- No breaking API changes
- No large feature redesign
- No UI toolkit dependency

## Success criteria

- A new C++ user can find a clear entry point quickly
- Common tasks are documented in C++ rather than only in Python
- The public C++ API feels like a first-class path, not an internal-only layer

## Engine compatibility

The C++ client lives in LibCarla and talks to the server over RPC, so it does not depend on the engine the server was built with, and the examples avoid server-side, engine-specific APIs. As with the Python client, build it from the same CARLA version as the server it connects to: the RPC protocol is not compatible across versions.
