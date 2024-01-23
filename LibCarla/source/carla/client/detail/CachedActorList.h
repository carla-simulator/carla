// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/NonCopyable.h"
#include "carla/rpc/Actor.h"

#include <boost/iterator/transform_iterator.hpp>

#include <algorithm>
#include <iterator>
#include <mutex>

namespace carla {
namespace client {
namespace detail {

  // ===========================================================================
  // -- CachedActorList --------------------------------------------------------
  // ===========================================================================

  /// Keeps a list of actor descriptions to avoid requesting each time the
  /// descriptions to the server.
  ///
  /// @todo Dead actors are never removed from the list.
  class CachedActorList : private MovableNonCopyable {
  public:

    /// Inserts an actor into the list.
    void Insert(rpc::Actor actor);

    /// Inserts a @a range containing actors.
    template <typename RangeT>
    void InsertRange(RangeT range);

    /// Return the actor ids present in @a range that haven't been added to this
    /// list.
    template <typename RangeT>
    std::vector<ActorId> GetMissingIds(const RangeT &range) const;

    /// Retrieve the actor matching @a id, or empty optional if actor is not
    /// cached.
    std::optional<rpc::Actor> GetActorById(ActorId id) const;

    /// Retrieve the actors matching the ids in @a range.
    template <typename RangeT>
    std::vector<rpc::Actor> GetActorsById(const RangeT &range) const;

    void Clear();

  private:

    mutable std::mutex _mutex;

    std::unordered_map<ActorId, rpc::Actor> _actors;
  };

  // ===========================================================================
  // -- CachedActorList implementation -----------------------------------------
  // ===========================================================================

  inline void CachedActorList::Insert(rpc::Actor actor) {
    std::scoped_lock<std::mutex> lock(_mutex);
    auto id = actor.id;
    _actors.emplace(id, std::move(actor));
  }

  template <typename RangeT>
  inline void CachedActorList::InsertRange(RangeT range) {
    auto make_a_pair = [](rpc::Actor actor) {
      auto id = actor.id;
      return std::make_pair(id, std::move(actor));
    };
    auto make_iterator = [&make_a_pair](auto it) {
      return boost::make_transform_iterator(std::make_move_iterator(it), make_a_pair);
    };
    std::scoped_lock<std::mutex> lock(_mutex);
    _actors.insert(make_iterator(std::begin(range)), make_iterator(std::end(range)));
  }

  template <typename RangeT>
  inline std::vector<ActorId> CachedActorList::GetMissingIds(const RangeT &range) const {
    std::vector<ActorId> result;
    result.reserve(range.size());
    std::scoped_lock<std::mutex> lock(_mutex);
    std::copy_if(std::begin(range), std::end(range), std::back_inserter(result), [this](auto id) {
      return _actors.find(id) == _actors.end();
    });
    return result;
  }

  inline std::optional<rpc::Actor> CachedActorList::GetActorById(ActorId id) const {
    std::scoped_lock<std::mutex> lock(_mutex);
    auto it = _actors.find(id);
    if (it != _actors.end()) {
      return it->second;
    }
    return std::nullopt;
  }

  template <typename RangeT>
  inline std::vector<rpc::Actor> CachedActorList::GetActorsById(const RangeT &range) const {
    std::vector<rpc::Actor> result;
    result.reserve(range.size());
    std::scoped_lock<std::mutex> lock(_mutex);
    for (auto &&id : range) {
      auto it = _actors.find(id);
      if (it != _actors.end()) {
        result.emplace_back(it->second);
      }
    }
    return result;
  }

  inline void CachedActorList::Clear() {
    std::scoped_lock<std::mutex> lock(_mutex);
    _actors.clear();
  }

} // namespace detail
} // namespace client
} // namespace carla
