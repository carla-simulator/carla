// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/sensor/CompileTimeTypeMap.h"

namespace carla {
namespace sensor {

  template <typename... Items>
  class CompositeSerializer : public CompileTimeTypeMap<Items...> {
    using Super = CompileTimeTypeMap<Items...>;

  public:

    using interpreted_type = size_t; /// @todo

    template <typename Sensor, typename... Args>
    static auto Serialize(Sensor &sensor, Args &&... args) {
      using TheSensor = typename std::remove_const<Sensor>::type;
      using Serializer = typename Super::template get<TheSensor*>::type;
      return Serializer::Serialize(sensor, std::forward<Args>(args)...);
    }

    template <size_t Index, typename Data>
    static interpreted_type Deserialize(Data &&data) {
      using Serializer = typename Super::template get_by_index<Index>::type;
      return Serializer::Deserialize(std::forward<Data>(data));
    }

  private:

    template <typename Data, size_t... Is>
    static interpreted_type Deserialize_impl(size_t i, Data &&data, std::index_sequence<Is...>) {
      // This function is equivalent to creating a switch statement with a case
      // for each element in the map, the compiler should be able to optimize it
      // into a jump table. See https://stackoverflow.com/a/46282159/5308925.
      interpreted_type result;
      std::initializer_list<int> ({
          (i == Is ? (result = Deserialize<Is>(std::forward<Data>(data))), 0 : 0)...
      });
      return result;
    }

  public:

    template <typename Data>
    static interpreted_type Deserialize(size_t index, Data &&data) {
      return Deserialize_impl(
          index,
          std::forward<Data>(data),
          std::make_index_sequence<Super::size()>());
    }
  };

} // namespace sensor
} // namespace carla
