// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/streaming/detail/Types.h"

#include <functional>

namespace carla {
namespace streaming {
namespace detail {

  /// A wrapper to put clients into std::unordered_set.
  template <typename T>
  class HashableClient : public T {
  public:

    template <typename ... Args>
    HashableClient(Args && ... args)
      : T(std::forward<Args>(args) ...) {}

    bool operator==(const HashableClient &rhs) const {
      return T::GetStreamId() == rhs.GetStreamId();
    }
  };

} // namespace detail
} // namespace streaming
} // namespace carla

namespace std {

  // Injecting a hash function for our clients into std namespace so we can
  // directly insert them into std::unordered_set.
  template <typename T>
  struct hash<carla::streaming::detail::HashableClient<T>> {
    using argument_type = carla::streaming::detail::HashableClient<T>;
    using result_type = std::size_t;
    result_type operator()(const argument_type &client) const noexcept {
      return std::hash<carla::streaming::detail::stream_id_type>()(client.GetStreamId());
    }
  };

} // namespace std
