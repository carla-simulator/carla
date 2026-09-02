// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/types/CdrSerialization.h"
#include "carla/ros2/types/CdrTopicInfo.h"

#include <fastdds/dds/topic/TopicDataType.hpp>
#include <fastrtps/rtps/common/SerializedPayload.h>
#include <fastcdr/FastBuffer.h>
#include <fastcdr/Cdr.h>

#include <cstdint>
#include <cstring>
#include <functional>

namespace carla {
namespace ros2 {

/// Generic FastDDS TopicDataType that serializes carla::ros2::msg::* POD structs
/// directly to CDR via CdrSerialization.h, without needing fastddsgen-generated
/// per-type PubSubType classes.
///
/// Replaces all 30 hand-generated *PubSubType classes.
/// Type name and max size are provided by CdrTopicInfo<MsgType>.
template<typename MsgType>
class GenericCdrPubSubType : public eprosima::fastdds::dds::TopicDataType {
 public:
  using SerializedPayload_t = eprosima::fastrtps::rtps::SerializedPayload_t;

  GenericCdrPubSubType() {
    setName(CdrTopicInfo<MsgType>::type_name());
    // m_typeSize is max CDR payload including the 4-byte DDS encapsulation header.
    // FastDDS uses this to pre-allocate payload buffers.
    const uint32_t max_payload = static_cast<uint32_t>(
        CdrTopicInfo<MsgType>::max_serialized_size());
    // Add alignment padding + 4-byte encapsulation header, matching the pattern
    // in fastddsgen-generated constructors (e.g. ClockPubSubTypes.cpp:36-37).
    m_typeSize = max_payload +
        static_cast<uint32_t>(
            eprosima::fastcdr::Cdr::alignment(max_payload, 4u)) +
        4u;
    m_isGetKeyDefined = false;
  }

  ~GenericCdrPubSubType() override = default;

  /// Serialize a MsgType instance into the FastDDS payload buffer.
  /// Called by FastDDS DataWriter::write() before sending on the wire.
  /// Serializes into an auto-growing heap buffer first so variable-length
  /// fields (e.g. Image::data, PointCloud2::data) are not bounded by the
  /// pre-allocated payload->data size. The bytes are then memcpy'd across.
  /// FastDDS resizes payload->data before this call via getSerializedSizeProvider,
  /// so the copy will always fit for correctly sized messages.
  bool serialize(
      void* data,
      SerializedPayload_t* payload) override {
    const MsgType* msg = static_cast<const MsgType*>(data);

    // Auto-growing FastBuffer: no fixed-size ceiling, handles any payload.
    eprosima::fastcdr::FastBuffer fb;
    // Force LITTLE_ENDIANNESS so the encapsulation header is CDR_LE
    // ({0x00, 0x01}) per DDSI-RTPS v2.5 Table 10.3, regardless of host
    // endianness. ROS2 ecosystems test against CDR_LE.
    eprosima::fastcdr::Cdr ser(
        fb,
        eprosima::fastcdr::Cdr::LITTLE_ENDIANNESS,
        eprosima::fastcdr::Cdr::DDS_CDR);
    payload->encapsulation = CDR_LE;

    try {
      ser.serialize_encapsulation();
      serialize_cdr(ser, *msg);
    } catch (eprosima::fastcdr::exception::Exception& /*e*/) {
      return false;
    }

    const uint32_t len = static_cast<uint32_t>(ser.getSerializedDataLength());
    if (len > payload->max_size) {
      return false;
    }
    std::memcpy(payload->data, fb.getBuffer(), len);
    payload->length = len;
    return true;
  }

  /// Deserialize a FastDDS payload buffer into a MsgType instance.
  /// Called by FastDDS DataReader after receiving data from the wire.
  bool deserialize(
      SerializedPayload_t* payload,
      void* data) override {
    MsgType* msg = static_cast<MsgType*>(data);

    eprosima::fastcdr::FastBuffer fastbuffer(
        reinterpret_cast<char*>(payload->data),
        static_cast<size_t>(payload->length));
    // The deserializer must accept either endianness on the wire, the
    // actual byte order is determined from the encapsulation header by
    // read_encapsulation(). LITTLE_ENDIANNESS here is just the initial
    // hint Fast-CDR uses before the header is parsed.
    eprosima::fastcdr::Cdr deser(
        fastbuffer,
        eprosima::fastcdr::Cdr::LITTLE_ENDIANNESS,
        eprosima::fastcdr::Cdr::DDS_CDR);

    try {
      deser.read_encapsulation();
      payload->encapsulation = (deser.endianness() ==
          eprosima::fastcdr::Cdr::BIG_ENDIANNESS) ? CDR_BE : CDR_LE;
      deserialize_cdr(deser, *msg);
    } catch (eprosima::fastcdr::exception::Exception& /*e*/) {
      return false;
    }

    return true;
  }

  /// Return a function that gives the actual CDR-serialized size for this
  /// specific message instance. FastDDS calls this before serialize() to
  /// size (or resize) the payload buffer, so the buffer is always large enough
  /// for variable-length fields like Image::data or PointCloud2::data.
  std::function<uint32_t()> getSerializedSizeProvider(void* data) override {
    const MsgType* msg = static_cast<const MsgType*>(data);
    return [msg]() -> uint32_t {
      return cdr_serialized_size(*msg);
    };
  }

  /// Allocate a new default-initialized MsgType on the heap.
  void* createData() override {
    return static_cast<void*>(new MsgType());
  }

  /// Delete a MsgType previously returned by createData().
  void deleteData(void* data) override {
    delete static_cast<MsgType*>(data);
  }

  /// CARLA topics are not keyed — always return false.
  bool getKey(
      void* /*data*/,
      eprosima::fastrtps::rtps::InstanceHandle_t* /*ihandle*/,
      bool /*force_md5*/) override {
    return false;
  }
};

} // namespace ros2
} // namespace carla
