// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/AtomicSharedPtr.h"

#include <cstdint>

namespace carla {
namespace client {
namespace detail {

  class Simulator;

  struct EpisodeProxyPointerType {
    using Shared = std::shared_ptr<Simulator>;
    using Strong = AtomicSharedPtr<Simulator>;
    using Weak = std::weak_ptr<Simulator>;
  };

  /// Provides access to the Simulator during a given episode. After the episode
  /// is ended any access to the simulator throws an std::runtime_error.
  template <typename PointerT>
  class EpisodeProxyImpl {
  public:

    using SharedPtrType = EpisodeProxyPointerType::Shared;

    EpisodeProxyImpl() = default;

    EpisodeProxyImpl(SharedPtrType simulator);

    template <typename T>
    EpisodeProxyImpl(EpisodeProxyImpl<T> other)
      : _episode_id(other._episode_id),
        _simulator(other._simulator) {}

    auto GetId() const noexcept {
      return _episode_id;
    }

    SharedPtrType TryLock() const noexcept;

    /// Same as TryLock but never return nullptr.
    ///
    /// @throw std::runtime_error if episode is gone.
    SharedPtrType Lock() const;

    bool IsValid() const noexcept {
      return TryLock() != nullptr;
    }

    void Clear() noexcept;

  private:

    template <typename T>
    friend class EpisodeProxyImpl;

    uint64_t _episode_id;

    PointerT _simulator;
  };

  using EpisodeProxy = EpisodeProxyImpl<EpisodeProxyPointerType::Strong>;

  using WeakEpisodeProxy = EpisodeProxyImpl<EpisodeProxyPointerType::Weak>;

} // namespace detail
} // namespace client
} // namespace carla
