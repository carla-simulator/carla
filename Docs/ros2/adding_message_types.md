# Adding a new ROS 2 message type to CARLA

CARLA publishes ROS 2 data by serializing hand-written C++ POD structs directly
into CDR byte buffers using Fast-CDR. There is no IDL compiler or code-generation
step at build time. As a result, supporting a new message type is a manual
7-step process. This guide walks through each step.

## Prerequisites

- Docker installed and running (needed only for Step 4).
- The `.msg` file for the type you want to add. Standard ROS 2 types (e.g.
  `sensor_msgs/msg/NavSatFix`) already have one in any ROS 2 installation.
  If you are defining a new `carla_msgs` type, write it first (see Step 1).

---

## Step 1 - Write the `.msg` file (skip for standard ROS 2 types)

If the type already exists in a standard ROS 2 package, skip this step and
use the existing `.msg` file in Step 4.

For a new `carla_msgs` type, create a `.msg` file following the
[ROS 2 interface definition format](https://design.ros2.org/articles/legacy_interface_definition.html).
Each line is either a field declaration or a comment (`# ...`):

```
# carla_msgs/msg/CarlaSpeedometer.msg
std_msgs/Header header
float32 speed   # m/s
```

Field type names for nested types use `pkg_name/TypeName` (no `msg/` in the
field line itself). Supported primitives: `bool`, `int8`, `uint8`, `int16`,
`uint16`, `int32`, `uint32`, `int64`, `uint64`, `float32`, `float64`, `string`.
Fixed arrays use `type[N]`; unbounded sequences use `type[]`.

---

## Step 2 - Create the C++ POD struct

Create `LibCarla/source/carla/ros2/types/msg/MyType.h`. Rules:

- The struct lives in `namespace carla::ros2::msg`.
- Include only `<array>`, `<vector>`, `<string>`, `<cstdint>`, and sibling
  `msg/*.h` headers - no DDS or Fast-CDR includes.
- Initialize primitive fields to zero with `= 0` or `= 0.0`.
- Use `std::array<T, N>` for fixed-length arrays, `std::vector<T>` for
  unbounded sequences.
- Nest other message types by value.

**Leaf type** (all fields are primitives):

```cpp
// LibCarla/source/carla/ros2/types/msg/MyLeaf.h
#pragma once
#include <cstdint>

namespace carla { namespace ros2 { namespace msg {

struct MyLeaf {
  float speed = 0.0f;
  uint32_t count = 0u;
};

}}} // namespace carla::ros2::msg
```

**Composite type** (nested message, fixed array, string):

```cpp
// LibCarla/source/carla/ros2/types/msg/MyComposite.h
#pragma once
#include <array>
#include <string>
#include "carla/ros2/types/msg/Header.h"
#include "carla/ros2/types/msg/Vector3.h"

namespace carla { namespace ros2 { namespace msg {

struct MyComposite {
  Header header;
  std::string frame_id;
  Vector3 velocity;
  std::array<double, 9> covariance = {};   // fixed 3x3 matrix
};

}}} // namespace carla::ros2::msg
```

**Type with a sequence** (variable-length vector):

```cpp
// LibCarla/source/carla/ros2/types/msg/MyList.h
#pragma once
#include <vector>
#include "carla/ros2/types/msg/SomeElement.h"

namespace carla { namespace ros2 { namespace msg {

struct MyList {
  std::vector<SomeElement> items;
};

}}} // namespace carla::ros2::msg
```

### Side-by-side example: `.msg` to POD

Full translation of `sensor_msgs/msg/Imu.msg` into its equivalent POD struct.
Every `.msg` line maps to exactly one C++ field in the same order; nested types
are resolved to the corresponding `msg::*` struct and included by header.

`sensor_msgs/msg/Imu.msg`:

```
# sensor_msgs/msg/Imu.msg
std_msgs/Header header
geometry_msgs/Quaternion orientation
float64[9] orientation_covariance
geometry_msgs/Vector3 angular_velocity
float64[9] angular_velocity_covariance
geometry_msgs/Vector3 linear_acceleration
float64[9] linear_acceleration_covariance
```

`LibCarla/source/carla/ros2/types/msg/Imu.h`:

```cpp
// LibCarla/source/carla/ros2/types/msg/Imu.h
#pragma once
#include <array>
#include "carla/ros2/types/msg/Header.h"
#include "carla/ros2/types/msg/Quaternion.h"
#include "carla/ros2/types/msg/Vector3.h"

namespace carla { namespace ros2 { namespace msg {

struct Imu {
  Header header;
  Quaternion orientation;
  std::array<double, 9> orientation_covariance = {};
  Vector3 angular_velocity;
  std::array<double, 9> angular_velocity_covariance = {};
  Vector3 linear_acceleration;
  std::array<double, 9> linear_acceleration_covariance = {};
};

}}} // namespace carla::ros2::msg
```

Mapping rules used above:

| `.msg` token                   | C++ field                                |
| ------------------------------ | ---------------------------------------- |
| `std_msgs/Header header`       | `Header header;` (nested by value)       |
| `geometry_msgs/Quaternion q`   | `Quaternion q;` (nested by value)        |
| `float64[9] covariance`        | `std::array<double, 9> covariance = {};` |
| `float64 x`                    | `double x = 0.0;`                        |
| `string frame_id`              | `std::string frame_id;`                  |
| `type[] items`                 | `std::vector<type> items;`               |

Note: the `dds_::Imu_` mangling in `type_name()` (Step 5) and the DDS wire
format are derived automatically from this POD layout via Fast-CDR; there is
no separate IDL definition to maintain.

---

## Step 3 - Register CDR serialization

Open `LibCarla/source/carla/ros2/types/CdrSerialization.h` and add:

1. An `#include` for your new header alongside the others at the top.
2. A `serialize_cdr` / `deserialize_cdr` overload pair.

**Placement:** add the overloads after the overloads of any types your struct
depends on (the file is ordered least-to-most dependent to avoid forward
declarations).

**Leaf type example** (primitives only - use `cdr <<` / `cdr >>`):

```cpp
inline void serialize_cdr(
    eprosima::fastcdr::Cdr& cdr, const msg::MyLeaf& m) {
  cdr << m.speed;
  cdr << m.count;
}

inline void deserialize_cdr(
    eprosima::fastcdr::Cdr& cdr, msg::MyLeaf& m) {
  cdr >> m.speed;
  cdr >> m.count;
}
```

**Composite type example** (nested types call their own overload; strings and
`std::array` serialize directly via `cdr <<`):

```cpp
inline void serialize_cdr(
    eprosima::fastcdr::Cdr& cdr, const msg::MyComposite& m) {
  serialize_cdr(cdr, m.header);   // nested msg:: type
  cdr << m.frame_id;              // std::string
  serialize_cdr(cdr, m.velocity); // nested msg:: type
  cdr << m.covariance;            // std::array<double, 9>
}

inline void deserialize_cdr(
    eprosima::fastcdr::Cdr& cdr, msg::MyComposite& m) {
  deserialize_cdr(cdr, m.header);
  cdr >> m.frame_id;
  deserialize_cdr(cdr, m.velocity);
  cdr >> m.covariance;
}
```

**Sequence example** (`std::vector` of structs - write length manually):

```cpp
inline void serialize_cdr(
    eprosima::fastcdr::Cdr& cdr, const msg::MyList& m) {
  // CDR sequence length is uint32_t per DDS-XTypes 1.3 clause 7.4.1.1.
  cdr << static_cast<uint32_t>(m.items.size());
  for (const auto& item : m.items) {
    serialize_cdr(cdr, item);
  }
}

inline void deserialize_cdr(
    eprosima::fastcdr::Cdr& cdr, msg::MyList& m) {
  uint32_t n{0u};
  cdr >> n;
  if (n > kMaxCdrSequenceElements) {
    throw eprosima::fastcdr::exception::BadParamException(
        "MyList::items length exceeds sane CDR sequence cap");
  }
  m.items.resize(static_cast<size_t>(n));
  for (auto& item : m.items) {
    deserialize_cdr(cdr, item);
  }
}
```

For `std::vector<uint8_t>` (byte arrays, e.g. `Image::data`) FastCDR handles
the length prefix automatically via `cdr << m.data` - no manual loop needed.

---

## Step 4 - Compute the RIHS01 type hash

ROS 2 Iron and later check a type hash in the DDS `USER_DATA` QoS field when
two nodes discover each other. If the hash is absent, each endpoint logs:

```
[WARN] [rmw_cyclonedds_cpp]: Failed to parse type hash for topic 'rt/...'
```

CARLA embeds a hardcoded RIHS01 hash in `CdrTopicInfo<T>::type_hash()` for
every message type. Use `Util/ros2/compute_type_hash.sh` to get the correct
hash for a type. Docker is the only requirement - no local ROS 2 installation.

**Standard ROS 2 type** (the `.msg` is already installed inside the container):

```sh
# Extract Imu.msg from the container first, then compute:
docker run --rm osrf/ros:jazzy-desktop \
    cat /opt/ros/jazzy/share/sensor_msgs/msg/Imu.msg > /tmp/Imu.msg

Util/ros2/compute_type_hash.sh sensor_msgs/msg/Imu /tmp/Imu.msg
# Output: RIHS01_7d9a00ff...
```

**New `carla_msgs` type** (provide your own `.msg` file):

```sh
Util/ros2/compute_type_hash.sh \
    carla_msgs/msg/CarlaSpeedometer \
    /path/to/carla_msgs/msg/CarlaSpeedometer.msg
# Output: RIHS01_<64 hex chars>
```

The script prints a single line to stdout:

```
RIHS01_<64 lowercase hex digits>
```

Paste this value into `CdrTopicInfo.h` in Step 5.

**Troubleshooting:** if `colcon build` fails inside Docker, the most common
cause is a `.msg` dependency that is not in the standard `jazzy` install. Add
the missing package to the `rosidl_generate_interfaces` workspace or, for
bleeding-edge packages, switch to a newer `osrf/ros` tag.

---

## Step 5 - Register in `CdrTopicInfo.h`

Add a specialization to
`LibCarla/source/carla/ros2/types/CdrTopicInfo.h`:

```cpp
template<> struct CdrTopicInfo<msg::MyComposite> {
  static const char* type_name() {
    // DDS mangled name: <package>::msg::dds_::<TypeName>_
    return "my_pkg::msg::dds_::MyComposite_";
  }
  static const char* type_hash() {
    return "RIHS01_<hash from Step 4>";
  }
  static size_t max_serialized_size() { return <byte count>u; }
};
```

Also add an `#include` for the new header at the top of `CdrTopicInfo.h`.

**`type_name()`:** follows the DDS mangled name convention used by ROS 2 RMWs:
`<package>::msg::dds_::<TypeName>_` (note the trailing underscore).

**`type_hash()`:** paste the `RIHS01_...` string from Step 4. For
CARLA-specific types whose hash depends on a `carla_msgs` version that may
vary across installations, return `nullptr` to skip setting `USER_DATA`.

**`max_serialized_size()`:** a preallocation hint in bytes, excluding the
4-byte DDS encapsulation header. For fixed types, sum the byte sizes of all
fields. For variable-length types, choose a reasonable upper bound. This is
only a hint; the actual payload is sized dynamically.

Quick sizing guide:
- `bool`, `uint8`, `int8`: 1 byte
- `uint16`, `int16`: 2 bytes
- `uint32`, `int32`, `float32`: 4 bytes
- `uint64`, `int64`, `float64`, `double`: 8 bytes
- `std::string`: 4 (length) + capacity estimate in bytes
- `std::array<double, 9>`: 72 bytes
- Nested struct: sum its fields recursively

---

## Step 6 - Add a test

Open `LibCarla/source/test/server/test_type_hash.cpp` and add:

1. One `CHECK_HASH(MyComposite)` call inside `TEST(TypeHash, FormatAllTypes)`.
2. One `CdrTopicInfo<msg::MyComposite>::type_hash()` entry in the `hashes`
   vector inside `TEST(TypeHash, UniqueAcrossAllTypes)`.

These two tests verify that the hash string matches the `RIHS01_<64 hex>`
format and is unique across all registered types.

---

## Step 7 - Build and verify

```sh
docker exec carla-development-ue4-20.04 make LibCarla ARGS="--ros2"
docker exec carla-development-ue4-20.04 make check.LibCarla
```

All tests must pass. The new `CHECK_HASH` entry will fail if the hash is
malformed or duplicated.

---

## FAQ

**Why are hashes hardcoded?**
CARLA has no IDL compiler (`rosidl`, `idlc`) running at build time. Computing
RIHS01 hashes correctly requires `rosidl_generator_type_description`, which
only runs inside a ROS 2 build environment. Hardcoding the values from a
one-time Docker build is zero-dependency and correct because standard message
definitions are stable per distro.

**Are hashes the same on Humble and Jazzy?**
Yes, for any message whose definition has not changed between the two distros
(all messages in `std_msgs`, `geometry_msgs`, `sensor_msgs`, `nav_msgs`,
`builtin_interfaces`, `tf2_msgs`, `rosgraph_msgs`, `ackermann_msgs`). Compute
the hash against Jazzy - it will match on Humble too.

**What is the wire format?**
Classic CDR, encoding version 1, little-endian (CDR_LE). Defined in:
- OMG DDSI-RTPS v2.5 Section 10 (encapsulation header)
- DDS-XTypes 1.3 clause 7.4.1.1 (sequence/string encoding)
- [REP-2011](https://ros.org/reps/rep-2011.html) (RIHS01 hash algorithm)
- [REP-2016](https://ros.org/reps/rep-2016.html) (USER_DATA KV format)

---

## References

- [OMG DDSI-RTPS v2.5 specification (PDF)](https://www.omg.org/spec/DDSI-RTPS/2.5/PDF)
