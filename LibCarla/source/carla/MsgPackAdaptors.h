// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Exception.h"
#include "carla/MsgPack.h"

#include <optional>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4583)
#pragma warning(disable:4582)
#include <variant>
#pragma warning(pop)
#else
#include <variant>
#endif

#include <tuple>

namespace clmdep_msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
namespace adaptor {

  // ===========================================================================
  // -- Adaptors for std::optional -------------------------------------------
  // ===========================================================================

  template<typename T>
  struct convert<std::optional<T>> {
    const clmdep_msgpack::object &operator()(
        const clmdep_msgpack::object &o,
        std::optional<T> &v) const {
      if (o.type != clmdep_msgpack::type::ARRAY) {
        ::carla::throw_exception(clmdep_msgpack::type_error());
      }
      if (o.via.array.size == 1) {
        v.reset();
      } else if (o.via.array.size == 2) {
        v = o.via.array.ptr[1].as<T>();
      } else {
        ::carla::throw_exception(clmdep_msgpack::type_error());
      }
      return o;
    }
  };

  template<typename T>
  struct pack<std::optional<T>> {
    template <typename Stream>
    packer<Stream> &operator()(
        clmdep_msgpack::packer<Stream> &o,
        const std::optional<T> &v) const {
      if (v.has_value()) {
        o.pack_array(2);
        o.pack(true);
        o.pack(*v);
      } else {
        o.pack_array(1);
        o.pack(false);
      }
      return o;
    }
  };

  template<typename T>
  struct object_with_zone<std::optional<T>> {
    void operator()(
        clmdep_msgpack::object::with_zone &o,
        const std::optional<T> &v) const {
      o.type = type::ARRAY;
      if (v.has_value()) {
        o.via.array.size = 2;
        o.via.array.ptr = static_cast<clmdep_msgpack::object*>(o.zone.allocate_align(
            sizeof(clmdep_msgpack::object) * o.via.array.size,
            MSGPACK_ZONE_ALIGNOF(clmdep_msgpack::object)));
        o.via.array.ptr[0] = clmdep_msgpack::object(true, o.zone);
        o.via.array.ptr[1] = clmdep_msgpack::object(*v, o.zone);
      } else {
        o.via.array.size = 1;
        o.via.array.ptr = static_cast<clmdep_msgpack::object*>(o.zone.allocate_align(
            sizeof(clmdep_msgpack::object) * o.via.array.size,
            MSGPACK_ZONE_ALIGNOF(clmdep_msgpack::object)));
        o.via.array.ptr[0] = clmdep_msgpack::object(false, o.zone);
      }
    }
  };

  // ===========================================================================
  // -- Adaptors for std::variant ----------------------------------
  // ===========================================================================

  template<typename... Ts>
  struct convert<std::variant<Ts...>> {

    const clmdep_msgpack::object &operator()(
        const clmdep_msgpack::object &o,
        std::variant<Ts...> &v) const {
      if (o.type != clmdep_msgpack::type::ARRAY) {
        ::carla::throw_exception(clmdep_msgpack::type_error());
      }
      if (o.via.array.size != 2) {
        ::carla::throw_exception(clmdep_msgpack::type_error());
      }
      const auto index = o.via.array.ptr[0].as<uint64_t>();
      copy_to_variant(index, o, v, std::make_index_sequence<sizeof...(Ts)>());
      return o;
    }

  private:

    template <uint64_t I>
    static void copy_to_variant_impl(
        const clmdep_msgpack::object &o,
        std::variant<Ts...> &v) {
      /// @todo Workaround for finding the type.
      auto dummy = std::get<I>(std::tuple<Ts...>{});
      using T = decltype(dummy);
      v = o.via.array.ptr[1].as<T>();
    }

    template <uint64_t... Is>
    static void copy_to_variant(
        const uint64_t index,
        const clmdep_msgpack::object &o,
        std::variant<Ts...> &v,
        std::index_sequence<Is...>) {
      std::initializer_list<int> ({
        (index == Is ? copy_to_variant_impl<Is>(o, v), 0 : 0)...
      });
    }
  };

  template<typename... Ts>
  struct pack<std::variant<Ts...>> {
    template <typename Stream>
    packer<Stream> &operator()(
        clmdep_msgpack::packer<Stream> &o,
        const std::variant<Ts...> &v) const {
      o.pack_array(2);
      o.pack(static_cast<uint64_t>(v.index()));
      std::visit([&](const auto &value) { o.pack(value); }, v);
      return o;
    }
  };

  template<typename... Ts>
  struct object_with_zone<std::variant<Ts...>> {
    void operator()(
        clmdep_msgpack::object::with_zone &o,
        const std::variant<Ts...> &v) const {
      o.type = type::ARRAY;
      o.via.array.size = 2;
      o.via.array.ptr = static_cast<clmdep_msgpack::object*>(o.zone.allocate_align(
          sizeof(clmdep_msgpack::object) * o.via.array.size,
          MSGPACK_ZONE_ALIGNOF(clmdep_msgpack::object)));
      o.via.array.ptr[0] = clmdep_msgpack::object(static_cast<uint64_t>(v.index()), o.zone);
      std::visit([&](const auto &value) {
        o.via.array.ptr[1] = clmdep_msgpack::object(value, o.zone);
      }, v);
    }
  };

} // namespace adaptor
} // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
} // namespace msgpack
