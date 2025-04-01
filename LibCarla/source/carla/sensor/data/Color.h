// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/rpc/Color.h"
#include "carla/rpc/FloatColor.h"
#if __has_include(<boost/gil/pixel.hpp>)
#include <boost/gil/pixel.hpp>
#endif
#include <cstdint>

namespace carla::sensor::data {

  #pragma pack(push, 1)
  namespace detail
  {
    template <typename T, std::size_t N>
    struct GenericColorData
    {
      union
      {
        T components[N];
        struct { T x, y, z, w; };
        struct { T r, g, b, a; };
      };
    };
    
    template <typename T>
    struct GenericColorData<T, 1>
    {
      union
      {
        T components[1];
        struct { T x; };
        struct { T r; };
      };
    };
    
    template <typename T>
    struct GenericColorData<T, 2>
    {
      union
      {
        T components[2];
        struct { T x, y; };
        struct { T r, g; };
      };
    };
    
    template <typename T>
    struct GenericColorData<T, 3>
    {
      union
      {
        T components[3];
        struct { T x, y, z; };
        struct { T r, g, b; };
      };
    };
  }
  
  template <typename T, std::size_t N = 3>
  class GenericColor :
    public detail::GenericColorData<T, N>
  {
    using Base = detail::GenericColorData<T, N>;
  public:

    static constexpr std::size_t ChannelCount = N;

    GenericColor() = default;
    GenericColor(const GenericColor&) = default;
    GenericColor& operator=(const GenericColor&) = default;
    ~GenericColor() = default;

    // Per-component initializer. Allows partial initialization (so you can initialize RG and leave B to 0).
    template <typename... U>
    requires (
      sizeof...(U) <= ChannelCount &&
      (std::is_convertible_v<U, T> && ...))
    constexpr GenericColor(U&&... components) :
      Base{{ T(std::forward<U>(components))... }}
    {
      constexpr auto M = sizeof...(U);
      if constexpr (N != M)
        for (auto i = M; i < N; ++i)
          Base::components[i] = T(0);
    }

    static constexpr auto Black()
    {
      return GenericColor();
    }

    // RPC Conversion:

    rpc::Color ToRPCColor() const
    {
      return rpc::Color(
        static_cast<uint8_t>(N > 0 ? Base::components[0] : T(0)),
        static_cast<uint8_t>(N > 1 ? Base::components[1] : T(0)),
        static_cast<uint8_t>(N > 2 ? Base::components[2] : T(0)));
    }

    operator rpc::Color() const
    {
      return ToRPCColor();
    }

    rpc::FloatColor ToRPCFloatColor(bool DefaultOpaque = false) const
    {
      return rpc::FloatColor(
        static_cast<float>(N > 0 ? Base::components[0] : T(0)),
        static_cast<float>(N > 1 ? Base::components[1] : T(0)),
        static_cast<float>(N > 2 ? Base::components[2] : T(0)),
        static_cast<float>(N > 3 ? Base::components[3] : T(DefaultOpaque ? 1 : 0)));
    }

    operator rpc::FloatColor() const
    {
      return ToRPCFloatColor();
    }

    // MSGPack Helpers:
    
    template <typename Packer>
    void msgpack_pack(Packer& pk) const
    {
      return msgpack_pack(pk, std::make_index_sequence<N>());
    }

    void msgpack_unpack(clmdep_msgpack::object const& o)
    { 
      return msgpack_unpack(o, std::make_index_sequence<N>());
    }

    template <typename MSGPACK_OBJECT>
    void msgpack_object(MSGPACK_OBJECT* o, clmdep_msgpack::zone& z) const
    { 
      return msgpack_object(o, z, std::make_index_sequence<N>());
    }
  
    template <typename Packer, std::size_t... Indices>
    void msgpack_pack(
      Packer& pk,
      [[maybe_unused]] std::index_sequence<Indices...>) const
    {
      clmdep_msgpack::type::make_define_array(Base::components[Indices]...).msgpack_pack(pk);
    }
  
    template <std::size_t... Indices>
    void msgpack_unpack(
      clmdep_msgpack::object const& o,
      [[maybe_unused]] std::index_sequence<Indices...>)
    { 
      clmdep_msgpack::type::make_define_array(Base::components[Indices]...).msgpack_unpack(o);
    }
  
    template <typename MSGPACK_OBJECT, std::size_t... Indices>
    void msgpack_object(
      MSGPACK_OBJECT* o,
      clmdep_msgpack::zone& z,
      [[maybe_unused]] std::index_sequence<Indices...>) const
    { 
      clmdep_msgpack::type::make_define_array(Base::components[Indices]...).msgpack_object(o, z);
    }
  };
#pragma pack(pop)

  template <typename L, typename R, std::size_t N, std::size_t M>
  constexpr bool operator==(GenericColor<L, N> l, GenericColor<R, M> r)
  {
    if constexpr (N == M)
      return std::equal(std::begin(l), std::end(r), std::begin(r));
    else
      return false;
  }
  
  template <typename L, typename R, std::size_t N, std::size_t M>
  constexpr bool operator!=(GenericColor<L, N> l, GenericColor<R, M> r)
  {
    return !(l == r);
  }
  

  using Color = GenericColor<uint8_t, 4>;
  static_assert(sizeof(Color) == sizeof(uint32_t), "Invalid color size!");

  using VectorColor = GenericColor<float, 3>;
  static_assert(sizeof(VectorColor) == sizeof(float) * 3);

  using OpticalFlowPixel = GenericColor<float, 2>;

} // namespace carla::sensor::data
