// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"

#include <boost/variant.hpp>

#include <tuple>

namespace clmdep_msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
namespace adaptor {

  // ===========================================================================
  // -- Adaptors for boost::variant --------------------------------------------
  // ===========================================================================

  template<typename... Ts>
  struct convert<boost::variant<Ts...>> {

    const clmdep_msgpack::object &operator()(
        const clmdep_msgpack::object &o,
        boost::variant<Ts...> &v) const {
      if (o.type != clmdep_msgpack::type::ARRAY) {
        throw clmdep_msgpack::type_error();
      }
      if (o.via.array.size != 2) {
        throw clmdep_msgpack::type_error();
      }
      const auto index = o.via.array.ptr[0].as<int>();
      copy_to_variant(index, o, v, std::make_index_sequence<sizeof...(Ts)>());
      return o;
    }

  private:

    template <size_t I>
    static void copy_to_variant_impl(
        const clmdep_msgpack::object &o,
        boost::variant<Ts...> &v) {
      /// @todo Workaround for finding the type.
      auto dummy = std::get<I>(std::tuple<Ts...>{});
      using T = decltype(dummy);
      v = o.via.array.ptr[1].as<T>();
    }

    template <size_t... Is>
    static void copy_to_variant(
        const size_t index,
        const clmdep_msgpack::object &o,
        boost::variant<Ts...> &v,
        std::index_sequence<Is...>) {
      std::initializer_list<int> ({
        (index == Is ? copy_to_variant_impl<Is>(o, v), 0 : 0)...
      });
    }
  };

  template<typename... Ts>
  struct pack<boost::variant<Ts...>> {
    template <typename Stream>
    packer<Stream> &operator()(
        clmdep_msgpack::packer<Stream> &o,
        const boost::variant<Ts...> &v) const {
      o.pack_array(2);
      o.pack(static_cast<int>(v.which()));
      boost::apply_visitor([&](const auto &value) { o.pack(value); }, v);
      return o;
    }
  };

  template<typename... Ts>
  struct object_with_zone<boost::variant<Ts...>> {
    void operator()(
        clmdep_msgpack::object::with_zone &o,
        const boost::variant<Ts...> &v) const {
      o.type = type::ARRAY;
      o.via.array.size = 2;
      o.via.array.ptr = static_cast<clmdep_msgpack::object*>(o.zone.allocate_align(
          sizeof(clmdep_msgpack::object) * o.via.array.size,
          MSGPACK_ZONE_ALIGNOF(clmdep_msgpack::object)));
      o.via.array.ptr[0] = clmdep_msgpack::object(static_cast<int>(v.which()), o.zone);
      boost::apply_visitor([&](const auto &value) {
        o.via.array.ptr[1] = clmdep_msgpack::object(value, o.zone);
      }, v);
    }
  };

} // namespace adaptor
} // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
} // namespace msgpack
