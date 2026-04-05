// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
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
#include <functional>

namespace carla {
namespace ros2 {

/// Generic FastDDS TopicDataType that serializes carla::ros2::msg::* POD structs
/// directly to CDR via CdrSerialization.h, without needing fastddsgen-generated
/// per-type PubSubType classes.
///
/// Replaces all 30 hand-generated *PubSubType classes and FastDDSTypeMap<>.
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

  /// Serialize a MsgType instance into the pre-allocated FastDDS payload buffer.
  /// Called by FastDDS DataWriter::write() before sending on the wire.
  bool serialize(
      void* data,
      SerializedPayload_t* payload) override {
    const MsgType* msg = static_cast<const MsgType*>(data);

    eprosima::fastcdr::FastBuffer fastbuffer(
        reinterpret_cast<char*>(payload->data),
        static_cast<size_t>(payload->max_size));
    eprosima::fastcdr::Cdr ser(
        fastbuffer,
        eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
        eprosima::fastcdr::Cdr::DDS_CDR);
    payload->encapsulation = (ser.endianness() ==
        eprosima::fastcdr::Cdr::BIG_ENDIANNESS) ? CDR_BE : CDR_LE;

    try {
      ser.serialize_encapsulation();
      serialize_cdr(ser, *msg);
    } catch (eprosima::fastcdr::exception::Exception& /*e*/) {
      return false;
    }

    payload->length = static_cast<uint32_t>(ser.getSerializedDataLength());
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
    eprosima::fastcdr::Cdr deser(
        fastbuffer,
        eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
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

  /// Return a function that gives the CDR-serialized size for the given instance.
  /// FastDDS uses this to size the payload buffer before calling serialize().
  std::function<uint32_t()> getSerializedSizeProvider(void* /*data*/) override {
    return []() -> uint32_t {
      return static_cast<uint32_t>(
          CdrTopicInfo<MsgType>::max_serialized_size()) + 4u;
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
