// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/detail/EpisodeProxy.h"

#include "carla/Exception.h"
#include "carla/client/detail/Simulator.h"

#include <exception>

namespace carla {
namespace client {
namespace detail {

  static EpisodeProxyPointerType::Shared Load(EpisodeProxyPointerType::Strong ptr) {
    return ptr.load();
  }

  static EpisodeProxyPointerType::Shared Load(EpisodeProxyPointerType::Weak ptr) {
    return ptr.lock();
  }

  template <typename T>
  EpisodeProxyImpl<T>::EpisodeProxyImpl(SharedPtrType simulator)
    : _episode_id(simulator != nullptr ? simulator->GetCurrentEpisodeId() : 0u),
      _simulator(std::move(simulator)) {}

  template <typename T>
  typename EpisodeProxyImpl<T>::SharedPtrType EpisodeProxyImpl<T>::TryLock() const noexcept {
    auto ptr = Load(_simulator);
    const bool is_valid = (ptr != nullptr) && (_episode_id == ptr->GetCurrentEpisodeId());
    return is_valid ? ptr : nullptr;
  }

  template <typename T>
  typename EpisodeProxyImpl<T>::SharedPtrType EpisodeProxyImpl<T>::Lock() const {
    auto ptr = Load(_simulator);
    if (ptr == nullptr) {
      throw_exception(std::runtime_error(
          "trying to operate on a destroyed actor; an actor's function "
          "was called, but the actor is already destroyed."));
    }
    return ptr;
  }

  template <typename T>
  void EpisodeProxyImpl<T>::Clear() noexcept {
    _simulator.reset();
  }

  template class EpisodeProxyImpl<EpisodeProxyPointerType::Strong>;

  template class EpisodeProxyImpl<EpisodeProxyPointerType::Weak>;

} // namespace detail
} // namespace client
} // namespace carla
