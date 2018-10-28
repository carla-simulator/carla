// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"

#include <tuple>

namespace carla {
namespace rpc {

  /// @todo Workaround for packing boost::variant; it uses tuple instead so it's
  /// quite inefficient memory-wise.
  template <typename... Ts>
  class Variant {
  private:

    size_t _index = 0u;

    std::tuple<Ts...> _tuple;

    template <typename T, typename Tuple>
    struct IndexFromType;

    template <typename T, typename... Types>
    struct IndexFromType<T, std::tuple<T, Types...>> {
      static constexpr size_t value = 0u;
    };

    template <typename T, typename U, typename... Types>
    struct IndexFromType<T, std::tuple<U, Types...>> {
      static constexpr size_t value = 1u + IndexFromType<T, std::tuple<Types...>>::value;
    };

    template <typename VisitorT, size_t... Is>
    void ApplyVisitorImpl(VisitorT &visitor, std::index_sequence<Is...>) const {
      std::initializer_list<int> ({ (_index == Is ? visitor(std::get<Is>(_tuple)), 0 : 0)... });
    }

  public:

    Variant() = default;

    template <typename ObjT>
    Variant(ObjT &&rhs) {
      (*this) = std::forward<ObjT>(rhs);
    }

    template <typename ObjT>
    Variant &operator=(ObjT &&rhs) {
      constexpr auto index = IndexFromType<typename std::decay<ObjT>::type, decltype(_tuple)>::value;
      _index = index;
      std::get<index>(_tuple) = std::forward<ObjT>(rhs);
      return *this;
    }

    template <typename VisitorT>
    void ApplyVisitor(VisitorT visitor) const {
      return ApplyVisitorImpl(visitor, std::make_index_sequence<sizeof...(Ts)>());
    }

    MSGPACK_DEFINE_ARRAY(_index, _tuple);
  };

} // namespace rpc
} // namespace carla
