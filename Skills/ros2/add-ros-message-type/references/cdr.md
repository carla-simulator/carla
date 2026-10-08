# CDR, type hashes and sizing

Detail layer for `add-ros-message-type`. Distilled from
`Docs/ros2/adding_message_types.md`, `types/CdrSerialization.h`,
`types/CdrTopicInfo.h` and `test/server/test_type_hash.cpp` in the checkout.
Read the upstream doc for the full step-by-step; this is the part that is easy to
get wrong.

## Wire format

Classic CDR, encoding version 1, little-endian (CDR_LE), 4-byte encapsulation
header handled by the middleware layer. Specs: OMG DDSI-RTPS v2.5 §10
(encapsulation), DDS-XTypes 1.3 §7.4.1.1 (sequence/string encoding),
REP-2011 (RIHS01 hash), REP-2016 (`USER_DATA` KV payload).

CDR is **positional**: there are no field names on the wire. A struct whose
fields are reordered, retyped or miscounted relative to the `.msg` serialises
without any error and deserialises to nonsense on the peer. This is the single
most likely bug and no test in the tree catches it — only a real subscriber does.

## POD struct rules

- `namespace carla::ros2::msg`, one header per type in `types/msg/`.
- Include only `<array>`, `<vector>`, `<string>`, `<cstdint>` and sibling
  `msg/*.h`. No DDS, no Fast-CDR — those live in `CdrSerialization.h`.
- Primitives initialised (`= 0`, `= 0.0f`); `type[N]` → `std::array<T,N>`;
  `type[]` → `std::vector<T>`; nested messages by value.
- `.msg` field types map as: `bool`→`bool`, `int8/uint8`→`int8_t/uint8_t`,
  `…16/32/64`→ the matching `int*_t`, `float32`→`float`, `float64`→`double`,
  `string`→`std::string`, `pkg/Type`→`msg::Type`.

## Serialization patterns

| Field kind | serialize | deserialize |
|---|---|---|
| primitive | `cdr << m.x;` | `cdr >> m.x;` |
| `std::string` | `cdr << m.s;` | `cdr >> m.s;` |
| `std::array<T,N>` | `cdr << m.a;` | `cdr >> m.a;` |
| nested `msg::T` | `serialize_cdr(cdr, m.t);` | `deserialize_cdr(cdr, m.t);` |
| `std::vector<uint8_t>` | `cdr << m.data;` (length automatic) | `cdr >> m.data;` |
| `std::vector<struct>` | write `static_cast<uint32_t>(size())`, then loop | read `uint32_t`, **check `kMaxCdrSequenceElements`**, `resize`, loop |

The sequence length is `uint32_t` per DDS-XTypes 1.3 §7.4.1.1. The cap check on
read is not optional: without it a corrupt length makes `resize` allocate
attacker-controlled memory. Throw
`eprosima::fastcdr::exception::BadParamException` as the existing overloads do.

**Ordering inside the file:** overloads are declared least-to-most dependent so
no forward declarations are needed. Put a new pair below every type it uses.

## `CdrTopicInfo` specialization

```cpp
template<> struct CdrTopicInfo<msg::MyComposite> {
  static const char* type_name() { return "my_pkg::msg::dds_::MyComposite_"; }
  static const char* type_hash() { return "RIHS01_<64 hex>"; }
  static size_t max_serialized_size() { return <bytes>u; }
};
```

- **`type_name()`** — the DDS mangled name `<package>::msg::dds_::<TypeName>_`.
  The trailing underscore is part of it. A mismatch means peer endpoints never
  match, with no error on CARLA's side.
- **`type_hash()`** — REP-2011 RIHS01, published in the `USER_DATA` QoS as
  `typehash=RIHS01_<hex>;` (REP-2016) and compared by peer RMWs on Iron+.
  `nullptr` is legal only for types with no canonical `.msg`: peers then log the
  missing-hash warning but still connect. Hashes are pinned per definition —
  recompute if the `.msg` ever changes.
- **`max_serialized_size()`** — preallocation hint in bytes, excluding the 4-byte
  encapsulation header. Only a hint; payloads are sized dynamically.

Sizing: `bool`/`int8`/`uint8` 1 · `int16`/`uint16` 2 · `int32`/`uint32`/`float32`
4 · `int64`/`uint64`/`float64` 8 · `std::string` 4 + capacity estimate ·
`std::array<double,9>` 72 · nested struct = recursive sum. For variable-length
types pick a sane upper bound.

## Why hashes are hardcoded

Computing RIHS01 needs `rosidl_generator_type_description`, i.e. a full ROS 2
build environment. CARLA has none at build time, so the values are computed once
in Docker and pinned. That is safe because standard message definitions are
stable per distro — and identical between Humble and Jazzy for every package
CARLA uses (`std_msgs`, `geometry_msgs`, `sensor_msgs`, `nav_msgs`,
`builtin_interfaces`, `tf2_msgs`, `rosgraph_msgs`, `ackermann_msgs`). Compute
against Jazzy; it matches Humble.

## What the tests do and do not prove

`test/server/test_type_hash.cpp`:

- `TEST(TypeHash, FormatAllTypes)` — every `CHECK_HASH(T)` entry is
  `RIHS01_` + 64 lowercase hex.
- `TEST(TypeHash, UniqueAcrossAllTypes)` — no two registered types share a hash.

Neither reads the `.msg`, so a **correctly formatted but wrong** hash passes, as
does a struct whose fields disagree with the definition. End-to-end proof is a
subscriber: publish, then `ros2 topic echo` ([[visualize-ros-rviz]]).

## Types already present

`types/msg/` covers the standard set CARLA publishes: `Header`, `Time`, `Clock`,
`String`, `Float32`, `Image`, `CameraInfo`, `RegionOfInterest`, `Imu`,
`NavSatFix`, `NavSatStatus`, `PointCloud2`, `PointField`, `Point`, `Point32`,
`Pose`, `PoseWithCovariance`, `Quaternion`, `Transform`, `TransformStamped`,
`TFMessage`, `TF2Error`, `Twist`, `TwistWithCovariance`, `Vector3`, `Odometry`,
`AckermannDrive`, `AckermannDriveStamped`, `CarlaEgoVehicleControl`,
`CarlaCollisionEvent`, `CarlaLineInvasion`. Check for the header before adding
one — several exist without a publisher using them yet, in which case the work
is entirely in [[add-ros-publisher]].
