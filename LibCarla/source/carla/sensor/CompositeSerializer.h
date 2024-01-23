// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
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
    static interpreted_type Deserialize(Buffer &&data);

  private:

    template <size_t Index, typename Data>
    static interpreted_type Deserialize_impl(Data &&data) {
      using Serializer = typename Super::template get_by_index<Index>::type;
      return Serializer::Deserialize(std::forward<Data>(data));
    }

    template <typename Data, size_t... Is>
    static interpreted_type Deserialize_impl(size_t i, Data &&data, std::index_sequence<Is...>) {
      // This function is equivalent to creating a switch statement with a case
      // for each element in the map, the compiler should be able to optimize it
      // into a jump table. See https://stackoverflow.com/a/46282159/5308925.
      interpreted_type result;
      std::initializer_list<int> ({
          (i == Is ? (result = Deserialize_impl<Is>(std::forward<Data>(data))), 0 : 0)...
      });
      return result;
    }

    template <typename Data>
    static interpreted_type Deserialize(size_t index, Data &&data) {
      return Deserialize_impl(
          index,
          std::forward<Data>(data),
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
  CompositeSerializer<Items...>::Deserialize(Buffer &&data) {
    RawData message{std::move(data)};
    size_t index = message.GetSensorTypeId();
    return Deserialize(index, std::move(message));
  }

} // namespace sensor
} // namespace carla
