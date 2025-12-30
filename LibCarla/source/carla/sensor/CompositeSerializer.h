// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Buffer.h"
#include "carla/Memory.h"
#include "carla/sensor/CompileTimeTypeMap.h"
#include "carla/sensor/RawData.h"

namespace carla {
namespace sensor {

  class SensorData;

  // ===========================================================================
  // -- CompositeSerializer ----------------------------------------------------
  // ===========================================================================

  /// Compile-time map for mapping sensor objects to serializers. The
  /// appropriate serializer is called for each sensor to serialize and
  /// deserialize its data.
  ///
  /// Do not use directly, use the SensorRegistry instantiation.
  template <typename... Items>
  class CompositeSerializer : public CompileTimeTypeMap<Items...> {
    using Super = CompileTimeTypeMap<Items...>;

  public:

    using interpreted_type = SharedPtr<SensorData>;

    /// Serialize the arguments provided into a Buffer by calling to the
    /// serializer registered for the given @a Sensor type.
    template <typename Sensor, typename... Args>
    static Buffer Serialize(Sensor &sensor, Args &&... args);

    /// Deserializes a Buffer by calling the "Deserialize" function of the
    /// serializer that generated the Buffer.
    static interpreted_type Deserialize(Buffer DESERIALIZE_DECL_DATA(data));

  private:

    template <size_t Index, typename Data>
    static interpreted_type Deserialize_impl(Data DESERIALIZE_DECL_DATA(data)) {
      using Serializer = typename Super::template get_by_index<Index>::type;
      return Serializer::Deserialize(DESERIALIZE_FORWARD_DATA(Data, data));
    }

    // This function is equivalent to creating a switch statement with a case
    // for each element in the map, the compiler should be able to optimize it
    // into a jump table. See https://stackoverflow.com/a/46282159/5308925.
    template <typename Data, size_t... Is>
    static interpreted_type Deserialize_impl(size_t i, Data DESERIALIZE_DECL_DATA(data), std::index_sequence<Is...>) {
      interpreted_type result;
      std::initializer_list<int> ({
          (i == Is ? (result = Deserialize_impl<Is>(DESERIALIZE_FORWARD_DATA(Data, data))), 0 : 0)...
      });
      return result;
    }

    template <typename Data>
    static interpreted_type Deserialize(size_t index, Data DESERIALIZE_DECL_DATA(data)) {
      return Deserialize_impl(
          index,
          DESERIALIZE_FORWARD_DATA(Data, data),
          std::make_index_sequence<Super::size()>());
    }
  };

  // ===========================================================================
  // -- CompositeSerializer implementation -------------------------------------
  // ===========================================================================

  template <typename... Items>
  template <typename Sensor, typename... Args>
  inline Buffer CompositeSerializer<Items...>::Serialize(Sensor &sensor, Args &&... args) {
    using TheSensor = typename std::remove_const<Sensor>::type;
    using Serializer = typename Super::template get<TheSensor*>::type;
    return Serializer::Serialize(sensor, std::forward<Args>(args)...);
  }

  template <typename... Items>
  inline typename CompositeSerializer<Items...>::interpreted_type
  CompositeSerializer<Items...>::Deserialize(Buffer DESERIALIZE_DECL_DATA(data)) {
    RawData message{DESERIALIZE_MOVE_DATA(data)};
    size_t index = message.GetSensorTypeId();
    return Deserialize(index, DESERIALIZE_MOVE_DATA(message));
  }

} // namespace sensor
} // namespace carla
