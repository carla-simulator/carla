// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <type_traits>
#include <utility>

namespace carla {
namespace sensor {

namespace detail {

  /// Private implementation of the CompileTimeTypeMap.
  template <size_t Size, typename...>
  struct CompileTimeTypeMapImpl;

  /// Specialization for an empty map, it is retrieved when a key cannot be
  /// found.
  template <size_t Size>
  struct CompileTimeTypeMapImpl<Size> {

    template <typename InKey>
    struct get {
      using type = void;
      static constexpr size_t index = Size;
    };

    template <size_t Index>
    struct get_by_index {
      using type = void;
      using key = void;
    };
  };

  template <size_t Size, typename Key, typename Value, typename... Rest>
  struct CompileTimeTypeMapImpl<Size, std::pair<Key, Value>, Rest...> {

    static constexpr size_t current_index() {
      return Size - 1u - sizeof...(Rest);
    }

    // Recursively call this struct until the InKey matches an element.
    template <typename InKey>
    struct get {
      using type = typename std::conditional<
          std::is_same<InKey, Key>::value,
          Value,
          typename CompileTimeTypeMapImpl<Size, Rest...>::template get<InKey>::type
        >::type;
      static constexpr size_t index =
          std::is_same<InKey, Key>::value ?
              current_index() :
              CompileTimeTypeMapImpl<Size, Rest...>::template get<InKey>::index;
    };

    // Recursively call this struct until the Index matches an element.
    template <size_t Index>
    struct get_by_index {
      using type = typename std::conditional<
          Index == current_index(),
          Value,
          typename CompileTimeTypeMapImpl<Size, Rest...>::template get_by_index<Index>::type
        >::type;

      using key = typename std::conditional<
          Index == current_index(),
          Key,
          typename CompileTimeTypeMapImpl<Size, Rest...>::template get_by_index<Index>::key
        >::type;
    };
  };

} // namespace detail

  /// A compile time structure for mapping two types. Lookup elements by Key or
  /// by Index.
  ///
  /// Example usage:
  ///
  ///     using MyMap = CompileTimeTypeMap<std::pair<A, B>, std::pair<C, D>>;
  ///     using type_B = MyMap::get<A>::type;
  ///     constexpr size_t index_B = MyMap::get<A>::index;
  ///     using type_B_too = MyMap::get_by_index<index_B>::type;
  ///
  template <typename... Items>
  struct CompileTimeTypeMap {

    static constexpr size_t size() {
      return sizeof...(Items);
    }

    template <typename InKey>
    using get = typename detail::CompileTimeTypeMapImpl<sizeof...(Items), Items...>::template get<InKey>;

    template <size_t Index>
    using get_by_index = typename detail::CompileTimeTypeMapImpl<sizeof...(Items), Items...>::template get_by_index<Index>;
  };

} // namespace sensor
} // namespace carla
