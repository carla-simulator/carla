// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

/// @file MsgPack.h
/// @brief MessagePack serialization utilities for CARLA
///
/// Provides convenient serialization and deserialization using MessagePack
/// format for efficient binary data exchange.

#ifndef CARLA_MSGPACK_H
#define CARLA_MSGPACK_H

#include "carla/Buffer.h"

#include <rpc/msgpack.hpp>
#include <cstddef>

namespace carla {

  /// @brief MessagePack serialization helper class.
  ///
  /// MsgPack provides static methods for serializing (packing) and
  /// deserializing (unpacking) objects using the MessagePack format.
  /// This is used for efficient binary data exchange in CARLA's RPC system.
  ///
  /// @example
  /// ```cpp
  /// // Pack an object into a buffer
  /// MyData data = {42, "hello"};
  /// Buffer buffer = MsgPack::Pack(data);
  ///
  /// // Unpack from buffer
  /// MyData unpacked = MsgPack::UnPack<MyData>(buffer);
  ///
  /// // Unpack from raw data
  /// MyData unpacked = MsgPack::UnPack<MyData>(raw_data, size);
  /// ```
  class MsgPack {
  public:
    /// @brief Serialize an object to MessagePack format.
    ///
    /// @tparam T Type of object to serialize (must be MessagePack-serializable).
    /// @param obj Object to serialize.
    /// @return Buffer containing serialized data.
    ///
    /// @example
    /// ```cpp
    /// std::vector<int> numbers = {1, 2, 3};
    /// Buffer buffer = MsgPack::Pack(numbers);
    /// ```
    template <typename T>
    static Buffer Pack(const T &obj) {
      namespace mp = ::clmdep_msgpack;
      mp::sbuffer sbuf;
      mp::pack(sbuf, obj);
      return Buffer(reinterpret_cast<const unsigned char *>(sbuf.data()), sbuf.size());
    }

    /// @brief Deserialize an object from a MessagePack buffer.
    ///
    /// @tparam T Type of object to deserialize.
    /// @param buffer Buffer containing serialized data.
    /// @return Deserialized object of type T.
    ///
    /// @throws msgpack::unpack_error If deserialization fails.
    ///
    /// @example
    /// ```cpp
    /// std::vector<int> numbers = MsgPack::UnPack<std::vector<int>>(buffer);
    /// ```
    template <typename T>
    static T UnPack(const Buffer &buffer) {
      namespace mp = ::clmdep_msgpack;
      return mp::unpack(reinterpret_cast<const char *>(buffer.data()), buffer.size()).template as<T>();
    }

    /// @brief Deserialize an object from raw MessagePack data.
    ///
    /// @tparam T Type of object to deserialize.
    /// @param data Pointer to serialized data.
    /// @param size Size of data in bytes.
    /// @return Deserialized object of type T.
    ///
    /// @throws msgpack::unpack_error If deserialization fails.
    ///
    /// @example
    /// ```cpp
    /// std::vector<int> numbers = MsgPack::UnPack<std::vector<int>>(raw_data, size);
    /// ```
    template <typename T>
    static T UnPack(const unsigned char *data, size_t size) {
      namespace mp = ::clmdep_msgpack;
      return mp::unpack(reinterpret_cast<const char *>(data), size).template as<T>();
    }
  };

} // namespace carla

#endif // CARLA_MSGPACK_H
