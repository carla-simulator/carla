// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

// Only meaningful when building the CycloneDDS middleware target.
// CARLA_ROS2_DDS_TESTING suppresses DDS includes for unit tests.
#ifndef CARLA_ROS2_DDS_TESTING
#ifdef CARLA_ROS2_DDS_CYCLONEDDS

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cstdio>

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

// ============================================================
// Public accessor helpers
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
// Internal: serdata allocation
// ============================================================

static struct ddsi_serdata* carla_cdr_alloc_serdata(
    const struct ddsi_sertype* type,
    enum ddsi_serdata_kind kind,
    uint32_t cdr_size)
{
  void* mem = malloc(sizeof(struct carla_cdr_serdata) + static_cast<size_t>(cdr_size));
  if (!mem) { return nullptr; }
  struct carla_cdr_serdata* csd = reinterpret_cast<struct carla_cdr_serdata*>(mem);
  ddsi_serdata_init(&csd->sd, type, kind);
  csd->cdr_size = cdr_size;
  return &csd->sd;
}

// ============================================================
// serdata ops
// ============================================================

static bool carla_cdr_serdata_eqkey(
    const struct ddsi_serdata* /*a*/,
    const struct ddsi_serdata* /*b*/)
{
  return true;  // no keyed topics — all instances are equivalent
}

static uint32_t carla_cdr_serdata_get_size(const struct ddsi_serdata* d) {
  return reinterpret_cast<const struct carla_cdr_serdata*>(d)->cdr_size;
}

static struct ddsi_serdata* carla_cdr_from_ser(
    const struct ddsi_sertype* type,
    enum ddsi_serdata_kind kind,
    const struct nn_rdata* /*fragchain*/,
    size_t size)
{
  // Fragment-based receive: allocate storage without copying.
  // CARLA uses local IPC so fragmented delivery does not occur in practice.
  return carla_cdr_alloc_serdata(type, kind, static_cast<uint32_t>(size));
}

static struct ddsi_serdata* carla_cdr_from_ser_iov(
    const struct ddsi_sertype* type,
    enum ddsi_serdata_kind kind,
    ddsrt_msg_iovlen_t niov,
    const ddsrt_iovec_t* iov,
    size_t size)
{
  struct ddsi_serdata* sd =
      carla_cdr_alloc_serdata(type, kind, static_cast<uint32_t>(size));
  if (!sd) { return nullptr; }
  uint8_t* dest = reinterpret_cast<uint8_t*>(
      reinterpret_cast<struct carla_cdr_serdata*>(sd) + 1);
  size_t off = 0u;
  for (ddsrt_msg_iovlen_t i = 0; i < niov && off < size; ++i) {
    const size_t avail = static_cast<size_t>(iov[i].iov_len);
    const size_t copy  = (off + avail > size) ? (size - off) : avail;
    memcpy(dest + off, iov[i].iov_base, copy);
    off += copy;
  }
  return sd;
}

static struct ddsi_serdata* carla_cdr_from_keyhash(
    const struct ddsi_sertype* type,
    const struct ddsi_keyhash* /*keyhash*/)
{
  return carla_cdr_alloc_serdata(type, SDK_KEY, 0u);
}

static struct ddsi_serdata* carla_cdr_from_sample(
    const struct ddsi_sertype* type,
    enum ddsi_serdata_kind kind,
    const void* /*sample*/)
{
  // Publishing via dds_write() is not supported — use dds_writecdr() instead.
  // Provide a stub to satisfy the ops table contract.
  return carla_cdr_alloc_serdata(type, kind, 0u);
}

static void carla_cdr_to_ser(
    const struct ddsi_serdata* d,
    size_t off,
    size_t sz,
    void* buf)
{
  memcpy(buf, carla_cdr_data(d) + off, sz);
}

static struct ddsi_serdata* carla_cdr_to_ser_ref(
    const struct ddsi_serdata* d,
    size_t off,
    size_t sz,
    ddsrt_iovec_t* ref)
{
  // NOLINTNEXTLINE — CycloneDDS iov_base is void*; we provide a non-const pointer
  ref->iov_base = const_cast<void*>(
      static_cast<const void*>(carla_cdr_data(d) + off));
  ref->iov_len = static_cast<ddsrt_iov_len_t>(sz);
  return ddsi_serdata_ref(d);
}

static void carla_cdr_to_ser_unref(
    struct ddsi_serdata* d,
    const ddsrt_iovec_t* /*ref*/)
{
  ddsi_serdata_unref(d);
}

static bool carla_cdr_to_sample(
    const struct ddsi_serdata* /*d*/,
    void* /*sample*/,
    void** /*bufptr*/,
    void* /*buflim*/)
{
  // Receiving via dds_take() is not supported — use dds_takecdr() instead.
  return false;
}

static struct ddsi_serdata* carla_cdr_to_untyped(const struct ddsi_serdata* d) {
  return ddsi_serdata_ref(d);
}

static bool carla_cdr_untyped_to_sample(
    const struct ddsi_sertype* /*type*/,
    const struct ddsi_serdata* /*d*/,
    void* /*sample*/,
    void** /*bufptr*/,
    void* /*buflim*/)
{
  return false;
}

static void carla_cdr_serdata_free(struct ddsi_serdata* d) {
  free(d);
}

static size_t carla_cdr_serdata_print(
    const struct ddsi_sertype* /*type*/,
    const struct ddsi_serdata* d,
    char* buf,
    size_t size)
{
  if (size == 0u) { return 0u; }
  int n = snprintf(buf, size, "<carla_cdr %u bytes>", carla_cdr_size(d));
  return static_cast<size_t>(n < 0 ? 0 : n);
}

static void carla_cdr_serdata_get_keyhash(
    const struct ddsi_serdata* /*d*/,
    struct ddsi_keyhash* buf,
    bool /*force_md5*/)
{
  memset(buf, 0, sizeof(*buf));
}

static const struct ddsi_serdata_ops carla_cdr_serdata_ops = {
  carla_cdr_serdata_eqkey,      // eqkey
  carla_cdr_serdata_get_size,   // get_size
  carla_cdr_from_ser,           // from_ser
  carla_cdr_from_ser_iov,       // from_ser_iov
  carla_cdr_from_keyhash,       // from_keyhash
  carla_cdr_from_sample,        // from_sample
  carla_cdr_to_ser,             // to_ser
  carla_cdr_to_ser_ref,         // to_ser_ref
  carla_cdr_to_ser_unref,       // to_ser_unref
  carla_cdr_to_sample,          // to_sample
  carla_cdr_to_untyped,         // to_untyped
  carla_cdr_untyped_to_sample,  // untyped_to_sample
  carla_cdr_serdata_free,       // free
  carla_cdr_serdata_print,      // print
  carla_cdr_serdata_get_keyhash // get_keyhash
};

// ============================================================
// sertype ops
// ============================================================

static void carla_cdr_sertype_free(struct ddsi_sertype* tp) {
  ddsi_sertype_fini(tp);
  free(tp);
}

static void carla_cdr_sertype_zero_samples(
    const struct ddsi_sertype* /*d*/,
    void* /*samples*/,
    size_t /*count*/)
{
  // CDR passthrough — no typed samples to zero
}

static void carla_cdr_sertype_realloc_samples(
    void** ptrs,
    const struct ddsi_sertype* /*d*/,
    void* /*old*/,
    size_t /*oldcount*/,
    size_t count)
{
  for (size_t i = 0u; i < count; ++i) {
    ptrs[i] = nullptr;
  }
}

static void carla_cdr_sertype_free_samples(
    const struct ddsi_sertype* /*d*/,
    void** ptrs,
    size_t count,
    dds_free_op_t /*op*/)
{
  for (size_t i = 0u; i < count; ++i) {
    free(ptrs[i]);
  }
}

static bool carla_cdr_sertype_equal(
    const struct ddsi_sertype* /*a*/,
    const struct ddsi_sertype* /*b*/)
{
  // Type names are already matched by the caller before invoking this.
  // Two carla_cdr_sertypes with the same name are considered equal.
  return true;
}

static uint32_t carla_cdr_sertype_hash(const struct ddsi_sertype* /*tp*/) {
  return 0u;
}

static const struct ddsi_sertype_ops carla_cdr_sertype_ops = {
  ddsi_sertype_v0,                    // version
  nullptr,                            // arg
  carla_cdr_sertype_free,             // free
  carla_cdr_sertype_zero_samples,     // zero_samples
  carla_cdr_sertype_realloc_samples,  // realloc_samples
  carla_cdr_sertype_free_samples,     // free_samples
  carla_cdr_sertype_equal,            // equal
  carla_cdr_sertype_hash,             // hash
  nullptr,                            // type_id
  nullptr,                            // type_map
  nullptr,                            // type_info
  nullptr,                            // derive_sertype
  nullptr,                            // get_serialized_size
  nullptr                             // serialize_into
};

// ============================================================
// Public API
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
  return dds_create_topic_sertype(
      participant, topic_name, registered_sertype,
      nullptr, nullptr, nullptr);
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

#endif // CARLA_ROS2_DDS_CYCLONEDDS
#endif // !CARLA_ROS2_DDS_TESTING
