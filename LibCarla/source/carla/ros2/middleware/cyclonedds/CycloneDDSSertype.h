// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

// Only meaningful when building the CycloneDDS middleware target.
// CARLA_ROS2_MIDDLEWARE_TESTING suppresses DDS includes for unit tests.
#ifndef CARLA_ROS2_MIDDLEWARE_TESTING
#ifdef CARLA_ROS2_MIDDLEWARE_CYCLONEDDS

#include <cstdint>
#include <cstdlib>
#include <cstring>

#include <dds/dds.h>
#include <dds/ddsi/ddsi_serdata.h>
#include <dds/ddsi/ddsi_sertype.h>

namespace carla {
namespace ros2 {

// ============================================================
// Data structures
// ============================================================

/// Custom serdata for raw CDR passthrough.
///
/// CDR bytes (including the 4-byte DDS encapsulation header) are stored
/// inline immediately after this struct in the same allocation. Access them
/// via carla_cdr_data() / carla_cdr_size().
struct carla_cdr_serdata {
  struct ddsi_serdata sd;    ///< Base — must be first member (C struct embedding)
  uint32_t           cdr_size;
  // CDR bytes follow: reinterpret_cast<uint8_t*>(this + 1)
};

/// Custom sertype for raw CDR passthrough.
///
/// Stateless — all per-topic identity comes from the type_name stored in
/// the base ddsi_sertype.
struct carla_cdr_sertype {
  struct ddsi_sertype c;     ///< Base — must be first member (C struct embedding)
};

// CDR encoding requires 4-byte alignment for multi-byte primitives.
// The CDR bytes follow immediately after carla_cdr_serdata in the allocation,
// so the struct size must be a multiple of 4 to preserve alignment.
static_assert(sizeof(struct carla_cdr_serdata) % 4 == 0,
    "carla_cdr_serdata size must be 4-byte aligned for CDR encoding");

// ============================================================
// Public accessor helpers (inline — trivial, used by pub/sub)
// ============================================================

/// Return a pointer to the inline CDR bytes (including the 4-byte header).
inline const uint8_t* carla_cdr_data(const struct ddsi_serdata* sd) {
  const struct carla_cdr_serdata* csd =
      reinterpret_cast<const struct carla_cdr_serdata*>(sd);
  return reinterpret_cast<const uint8_t*>(csd + 1);
}

/// Return the total CDR byte count (including the 4-byte encapsulation header).
inline uint32_t carla_cdr_size(const struct ddsi_serdata* sd) {
  return reinterpret_cast<const struct carla_cdr_serdata*>(sd)->cdr_size;
}

// ============================================================
// Declarations — defined once in CycloneDDSSertype.cpp
// ============================================================

/// Ops tables — defined in CycloneDDSSertype.cpp so that every translation
/// unit sees the same object address. CycloneDDS compares ops table pointers
/// (not just the equal() callback) when checking sertype compatibility.
extern const struct ddsi_serdata_ops carla_cdr_serdata_ops;
extern const struct ddsi_sertype_ops carla_cdr_sertype_ops;

/// Allocate a carla_cdr_serdata with cdr_size bytes of inline storage.
/// Defined in CycloneDDSSertype.cpp.
extern struct ddsi_serdata* carla_cdr_alloc_serdata(
    const struct ddsi_sertype* type,
    enum ddsi_serdata_kind kind,
    uint32_t cdr_size);

/// Return the single shared CycloneDDS participant for this process.
///
/// DDS best practice is one participant per application. All CARLA publishers
/// and subscribers share this participant so that:
///  - Sertype interning across topics with the same type name is correct.
///  - No cascading dds_delete() from individual pub/sub destructors can
///    invalidate sertypes that other endpoints are still using.
///
/// The participant is created lazily on the first call (C++11 function-local
/// static, thread-safe) and lives until process exit.
extern dds_entity_t carla_cdr_get_participant();

// ============================================================
// Public API (inline — reference extern declarations above)
// ============================================================

/// Create a CycloneDDS topic that transports raw CDR bytes.
///
/// Allocates and registers a carla_cdr_sertype for the given type_name, then
/// creates the topic.  On return, @p registered_sertype holds the actual
/// registered sertype pointer (which may differ from the newly allocated one
/// if CycloneDDS interned an identical pre-existing sertype).  The caller must
/// NOT free this pointer — its lifetime is managed by the DDS participant.
///
/// @param participant           CycloneDDS participant entity
/// @param topic_name            DDS topic name
/// @param type_name             DDS type name (used for endpoint matching)
/// @param registered_sertype    Out-parameter: receives the live sertype pointer
/// @return topic entity on success, or a negative dds_return_t on failure
inline dds_entity_t carla_cdr_create_topic(
    dds_entity_t participant,
    const char* topic_name,
    const char* type_name,
    struct ddsi_sertype** registered_sertype)
{
  struct carla_cdr_sertype* st =
      reinterpret_cast<struct carla_cdr_sertype*>(
          malloc(sizeof(struct carla_cdr_sertype)));
  if (!st) { return static_cast<dds_entity_t>(DDS_RETCODE_OUT_OF_RESOURCES); }
  ddsi_sertype_init_flags(
      &st->c,
      type_name,
      &carla_cdr_sertype_ops,
      &carla_cdr_serdata_ops,
      DDSI_SERTYPE_FLAG_TOPICKIND_NO_KEY);
  *registered_sertype = &st->c;
  dds_entity_t topic = dds_create_topic_sertype(
      participant, topic_name, registered_sertype,
      nullptr, nullptr, nullptr);
  if (topic < 0) {
    // dds_create_topic_sertype takes ownership only on success.
    // On failure, release the sertype we allocated to avoid a leak.
    ddsi_sertype_fini(&st->c);
    free(st);
    *registered_sertype = nullptr;
  }
  return topic;
}

/// Wrap pre-serialized CDR bytes in a ddsi_serdata for use with dds_writecdr().
///
/// The returned serdata has refcount 1.  Pass it to dds_writecdr(); CycloneDDS
/// will call ddsi_serdata_unref() after the write completes.
///
/// @param type      The registered sertype from carla_cdr_create_topic()
/// @param cdr_data  CDR bytes including the 4-byte DDS encapsulation header
/// @param cdr_size  Total byte count
/// @return new ddsi_serdata*, or nullptr on allocation failure
inline struct ddsi_serdata* carla_cdr_wrap(
    const struct ddsi_sertype* type,
    const uint8_t* cdr_data,
    uint32_t cdr_size)
{
  struct ddsi_serdata* sd = carla_cdr_alloc_serdata(type, SDK_DATA, cdr_size);
  if (!sd) { return nullptr; }
  uint8_t* dest = reinterpret_cast<uint8_t*>(
      reinterpret_cast<struct carla_cdr_serdata*>(sd) + 1);
  memcpy(dest, cdr_data, static_cast<size_t>(cdr_size));
  return sd;
}

} // namespace ros2
} // namespace carla

#endif // CARLA_ROS2_MIDDLEWARE_CYCLONEDDS
#endif // !CARLA_ROS2_MIDDLEWARE_TESTING
