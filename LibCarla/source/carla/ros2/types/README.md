# ROS2 Type Definitions

IDL files are the canonical source of truth for all ROS2 message types used in CARLA.

## Folder Structure

```
types/
├── idl/                      Canonical IDL definitions (edit these first)
│   ├── ackermann_msgs/
│   ├── builtin_interfaces/
│   ├── carla_msgs/
│   ├── geometry_msgs/
│   ├── nav_msgs/
│   ├── rosgraph_msgs/
│   ├── sensor_msgs/
│   ├── std_msgs/
│   └── tf2_msgs/
├── fastdds/                  Generated FastDDS C++ types (kept in git)
│   ├── ackermann_msgs/
│   ├── builtin_interfaces/
│   ├── carla_msgs/
│   ├── geometry_msgs/
│   ├── nav_msgs/
│   ├── rosgraph_msgs/
│   ├── sensor_msgs/
│   ├── std_msgs/
│   └── tf2_msgs/
├── cyclonedds/               Generated CycloneDDS C types (kept in git)
│   ├── ackermann_msgs/
│   ├── builtin_interfaces/
│   ├── carla_msgs/
│   ├── geometry_msgs/
│   ├── nav_msgs/
│   ├── rosgraph_msgs/
│   ├── sensor_msgs/
│   ├── std_msgs/
│   └── tf2_msgs/
├── msg/                      Backend-neutral POD structs (hand-written)
├── FastDDSConversions.h      to_fastdds() / from_fastdds() inline conversions
└── CycloneDDSConversions.h   to_cyclonedds() / from_cyclonedds() inline conversions
```

## How to Modify a Type

1. Edit the IDL file in `idl/<package>/<Type>.idl`
2. Regenerate backend code (see below)
3. Review changes with `git diff`
4. Commit both the IDL change and regenerated files

## Code Generation

### CycloneDDS (automatic)

CycloneDDS C types are regenerated at build time via `idlc` (built during `make setup`).
No manual step needed — just edit the IDL and rebuild.

### FastDDS (automatic with JDK)

FastDDS C++ types are regenerated at build time via `fastddsgen` if it's installed.
The CARLA Docker dev container includes JDK and builds `fastddsgen` during `make setup`.

If `fastddsgen` is not available, the build uses the checked-in generated files and
emits a warning if any IDL file is newer than its corresponding output.

**Manual regeneration** (outside Docker or without JDK):
```bash
Util/BuildTools/regenerate_fastdds_types.sh
```

**Adding JDK** (if not using the Docker dev container):
```bash
apt-get install default-jdk
```

## Naming Workarounds

Two types have naming differences between IDL and the generated FastDDS code:

- **Clock**: The IDL field is named `clock_time` (not `clock`) to avoid a
  case-insensitive collision with the struct name on some platforms.
- **String**: The IDL struct is named `StringMsg` (not `String`) to avoid
  collision with the IDL built-in `string` keyword.

## Generated Files in Git

Generated files are checked into git because regeneration is infrequent and
having them in-tree simplifies builds without code generation tools installed.
