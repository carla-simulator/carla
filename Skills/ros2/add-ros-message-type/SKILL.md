---
name: add-ros-message-type
description: Adds a new ROS 2 message type to CARLA's native ROS 2 interface — the hand-written POD struct under LibCarla/source/carla/ros2/types/msg, its Fast-CDR serialize/deserialize pair, the RIHS01 type hash computed in Docker, the CdrTopicInfo registration and the type-hash test. Use when the user asks to "add a ROS message type", "support sensor_msgs/X in CARLA", "add a carla_msgs type", "why is my type hash warning appearing", or needs a message CARLA does not publish yet. There is no IDL codegen — every step is manual.
license: MIT
compatibility: A CARLA checkout whose LibCarla/source/carla/ros2 exists (native ROS 2 branch), Docker running (only for the RIHS01 hash — no local ROS 2 needed), and the toolchain to build LibCarla with --ros2. Editing only; nothing is published or run.
metadata:
  group: ros2
  prerequisites: scripts/check_env.sh
  reference: references/cdr.md
---

# Add a ROS 2 message type

> **Paths.** `scripts/…` and `references/…` below are relative to the
> directory holding this SKILL.md. Your working directory is the user's
> project, not that directory, so prefix them with its absolute path or the
> command is not found.

CARLA serialises hand-written C++ PODs straight into CDR buffers with Fast-CDR.
**There is no IDL compiler in the build**, so a new type is a five-file manual
change. Skipping any one of them fails in a different way — silently on the wire,
loudly at build, or only on a peer's Jazzy console.

Upstream reference: `Docs/ros2/adding_message_types.md` in the checkout. This
skill adds the preflight, the wrapped hash computation, and the verify loop.

## On CARLA 0.10.0 (the UE5 line: 5.5 and 5.8)

The hand-written-POD approach, the Fast-CDR serialize/deserialize pair, the
RIHS01 type hash and the registration step are the same on 0.10.0 — there is
still no IDL codegen.

What differs is the neighbourhood: types live beside a larger set on 0.10.0
(Autoware messages among them), and QoS moved from `PublisherQos.h` to
`QosProfile.h`. If your new type is for a publisher, read [[add-ros-publisher]]
first for the `BasicPublisher` refactor — a type added for a
`CarlaPublisher`-shaped class will not compile there.

## Instructions

```
Progress:
- [ ] Step 1: Check prerequisites (bash scripts/check_env.sh), clear FAILs
- [ ] Step 2: Get the .msg definition (standard type, or write a carla_msgs one)
- [ ] Step 3: POD struct in types/msg/<Type>.h
- [ ] Step 4: serialize_cdr/deserialize_cdr pair in types/CdrSerialization.h
- [ ] Step 5: RIHS01 hash (bash scripts/type_hash.sh <pkg>/msg/<Type>)
- [ ] Step 6: CdrTopicInfo<> specialization (type_name, type_hash, max_serialized_size)
- [ ] Step 7: CHECK_HASH + hashes entry in test/server/test_type_hash.cpp
- [ ] Step 8: bash scripts/verify.sh   (build + type-hash tests)
```

The five files, in dependency order:

| File | What goes in |
|---|---|
| `types/msg/<Type>.h` | the POD struct, `namespace carla::ros2::msg` |
| `types/CdrSerialization.h` | include + `serialize_cdr`/`deserialize_cdr` overloads |
| `types/CdrTopicInfo.h` | include + `CdrTopicInfo<msg::Type>` specialization |
| `test/server/test_type_hash.cpp` | `CHECK_HASH(Type)` + an entry in `hashes` |
| *(new carla_msgs only)* your `.msg` | the canonical definition the hash is computed from |

### Step 1: Check prerequisites

```bash
bash scripts/check_env.sh
```

### Steps 3-4: struct and serialization

Rules that matter (full patterns and the sequence/string cases in
[`references/cdr.md`](references/cdr.md)):

- Struct lives in `namespace carla::ros2::msg`; includes only `<array>`,
  `<vector>`, `<string>`, `<cstdint>` and sibling `msg/*.h` — **no DDS or
  Fast-CDR headers**.
- Every primitive gets `= 0` / `= 0.0`; fixed arrays are `std::array<T,N>`,
  sequences `std::vector<T>`, nested messages by value.
- Field **order and count must mirror the `.msg` exactly** — CDR is positional,
  so a wrong order produces garbage on the wire with no error anywhere.
- Add the overload pair **after** the overloads of the types it depends on
  (`CdrSerialization.h` is ordered least-to-most dependent, no forward decls).
- Nested `msg::` types call their own overload; `std::string` and `std::array`
  go through `cdr <<`; `std::vector<uint8_t>` is length-prefixed automatically;
  a `std::vector` of structs needs the `uint32_t` length written by hand and
  checked against `kMaxCdrSequenceElements` on read.

### Step 5: the RIHS01 hash

```bash
# standard type — the .msg is pulled out of osrf/ros:jazzy-desktop for you
bash scripts/type_hash.sh sensor_msgs/msg/Imu

# new carla_msgs type — pass your own definition
bash scripts/type_hash.sh carla_msgs/msg/CarlaSpeedometer ./CarlaSpeedometer.msg
```

Prints one `RIHS01_<64 hex>` line (and the definition it hashed, to stderr, so you
can eyeball it against your struct). It refuses to print anything that is not in
that exact format, because a malformed hash pasted into the source only shows up
as a peer-side warning.

> **`Util/ros2/compute_type_hash.sh` is broken as shipped** (verified 2026-08 on
> `ue4-dev` with `osrf/ros:jazzy-desktop`) — four independent faults, so Step 4 of
> the upstream doc cannot work:
>
> 1. the in-container script runs `set -u` then sources `setup.bash`, which
>    dereferences `AMENT_TRACE_SETUP_FILES` → `unbound variable`;
> 2. `--log-base` is passed **after** `--cmake-args`, which swallows it, so CMake
>    gets it → `Unknown argument --log-base`, 0 packages built;
> 3. `docker run` has no `--user`, so its temp workspace becomes root-owned and its
>    own cleanup `rm` fails, making the exit code meaningless;
> 4. it extracts the hash with `jq`, which that image does not have → the
>    misleading `'<type>' not found in <json>`.
>
> `type_hash.sh` runs upstream first and, on failure, retries with a patched
> **temp copy** (plus a mounted python `jq` shim). Your checkout is never modified,
> and the hash still comes from `rosidl`'s own type-description output. With that
> in place `sensor_msgs/msg/Imu` reproduces the value pinned in `CdrTopicInfo.h`
> exactly — which is also the best available self-test of this whole procedure.
> The proper fix is four one-liners upstream.

The hash is what ROS 2 Iron+ compares in the `USER_DATA` QoS during discovery.
Get it wrong or omit it and peers log
`Failed to parse type hash for topic 'rt/...'` — the connection still forms, so
this is easy to miss. Return `nullptr` **only** for a type with no canonical
`.msg`.

### Steps 6-7: registration and test

`type_name()` is the DDS mangled name `<package>::msg::dds_::<TypeName>_` —
trailing underscore included; a mismatch means peers never match the endpoint.
`max_serialized_size()` is a preallocation hint in bytes excluding the 4-byte
encapsulation header (sizing table in the reference). Then add both test entries
— they are what catch a malformed or duplicated hash.

### Step 8: verify

```bash
bash scripts/verify.sh              # make LibCarla ARGS="--ros2" + make check.LibCarla
BUILD_ONLY=1 bash scripts/verify.sh # compile-only fast loop
```

Green tests prove the hash is **well-formed and unique** — not that the wire
format is right. For that, publish the type and read it from a real ROS 2 node
([[visualize-ros-rviz]] `echo`): a field-order bug shows up there and nowhere
else.

## Examples

**Example 1: support an existing ROS type CARLA lacks**

User says: "make CARLA publish `nav_msgs/Odometry` for the ego"

`Odometry.h` already exists in `types/msg/` — check first. If a type is missing,
this skill adds it; then [[add-ros-publisher]] does the publishing side. Adding a
type alone changes no topic.

**Example 2: a new carla_msgs type**

User says: "add a CarlaSpeedometer message with the ego speed"

Write `CarlaSpeedometer.msg` (`std_msgs/Header header` + `float32 speed`) →
POD → overloads → `type_hash.sh carla_msgs/msg/CarlaSpeedometer ./…msg` →
`CdrTopicInfo` (`carla_msgs::msg::dds_::CarlaSpeedometer_`) → tests → verify.

**Example 3: the type-hash warning**

User says: "peers log `Failed to parse type hash` for one of my topics"

That type's `type_hash()` returns `nullptr` or a malformed string. Recompute with
`type_hash.sh` and paste it in; re-run `verify.sh`.

## Troubleshooting

**Problem: `compute_type_hash.sh` fails inside Docker on `colcon build`**
Cause: the `.msg` depends on a package not present in the `jazzy` image.
Solution: add the dependency to the workspace, or use a newer image
(`ROS_IMAGE=osrf/ros:<tag>-desktop bash scripts/type_hash.sh …`).

**Problem: the type builds and publishes, but subscribers get garbage**
Cause: field order/type mismatch between the POD and the `.msg` — CDR is
positional and carries no field names.
Solution: diff the struct against the `.msg` line by line; the reference has a
side-by-side example.

**Problem: `make check.LibCarla` fails on a duplicate hash**
Cause: the same `RIHS01_…` string registered for two types (copy-paste).
Solution: recompute per type; the uniqueness test exists for exactly this.

**Problem: build fails with a Fast-CDR error in the new overload**
Cause: an overload placed before the types it depends on, or a nested type
serialised with `cdr <<` instead of `serialize_cdr`.
Solution: move it below its dependencies; nested `msg::` types always recurse.

## Outputs

Edited sources in the checkout (five files) and a green `check.LibCarla`. No
topic changes on their own — publishing the new type is [[add-ros-publisher]].

Wire-format detail (CDR encoding, the sizing table, sequence rules, hash
semantics) in [`references/cdr.md`](references/cdr.md).
