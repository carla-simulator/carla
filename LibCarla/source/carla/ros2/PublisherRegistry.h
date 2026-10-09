// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <cstdint>
#include <initializer_list>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace carla {
namespace ros2 {

/// Per-actor ROS 2 naming and publisher cache, shared by the game thread
/// (registration, CPU sensors) and the camera readback threads.
///
/// One leaf mutex guards every map: a lookup reads the registration, parent
/// chain and topic override together with the publisher maps, and the hot path
/// is a single hash lookup. The mutex is never held while a publisher is
/// constructed, destroyed, written or published, and no callback runs under it.
/// Callers keep the returned shared_ptr, so an erase on another thread cannot
/// destroy a publisher that is still in use.
template <typename SensorPublisherT, typename CameraPublisherT, typename TransformPublisherT>
class PublisherRegistry {
public:
  using stream_id_type = std::uint32_t;

  struct Registration {
    std::string ros_name;
    std::string frame_id;
    bool publish_tf{true};
  };

  struct Naming {
    std::string base_topic;
    std::string frame_id;
    bool topic_override{false};
  };

  struct TransformTarget {
    std::shared_ptr<TransformPublisherT> publisher;
    std::string parent_frame;
    std::string frame_id;
  };

  void Register(void *actor, Registration registration) {
    std::lock_guard<std::mutex> lock(_mutex);
    _registrations.insert_or_assign(actor, std::move(registration));
  }

  void Unregister(void *actor) {
    Erased erased;
    std::lock_guard<std::mutex> lock(_mutex);
    erased = TakePublishers(actor);
    _actor_parents.erase(actor);
    _registrations.erase(actor);
  }

  void Release(void *actor) {
    Erased erased;
    std::lock_guard<std::mutex> lock(_mutex);
    erased = TakePublishers(actor);
  }

  void SetTopicOverride(void *actor, std::string ros_topic_name) {
    std::lock_guard<std::mutex> lock(_mutex);
    _topic_overrides.insert_or_assign(actor, std::move(ros_topic_name));
  }

  /// Also drops the actor's publishers, so the next sample re-creates them
  /// under the default naming.
  void RemoveTopicOverride(void *actor) {
    Erased erased;
    std::lock_guard<std::mutex> lock(_mutex);
    _topic_overrides.erase(actor);
    erased = TakePublishers(actor);
  }

  std::string GetTopicOverride(void *actor) const {
    std::lock_guard<std::mutex> lock(_mutex);
    auto it = _topic_overrides.find(actor);
    return it != _topic_overrides.end() ? it->second : std::string{};
  }

  void AddParent(void *actor, void *parent) {
    std::lock_guard<std::mutex> lock(_mutex);
    _actor_parents[actor].push_back(parent);
  }

  std::string BaseTopicName(void *actor) const {
    std::lock_guard<std::mutex> lock(_mutex);
    return BaseTopicNameLocked(actor);
  }

  void Clear() {
    SensorMap publishers;
    CameraMap cameras;
    TransformMap transforms;
    std::lock_guard<std::mutex> lock(_mutex);
    publishers.swap(_publishers);
    cameras.swap(_camera_publishers);
    transforms.swap(_transforms);
    _registrations.clear();
    _actor_parents.clear();
    _topic_overrides.clear();
    for (auto &item : _in_flight) {
      item.second.released = true;
    }
  }

  /// @a make receives the actor's Naming after every `prefix__` placeholder in
  /// @a prefixes was resolved with @a id.
  template <typename Factory>
  std::shared_ptr<SensorPublisherT> GetOrCreateSensor(
      void *actor, stream_id_type id, std::initializer_list<const char *> prefixes, Factory &&make) {
    return GetOrCreate(_publishers, actor, id, prefixes, std::forward<Factory>(make));
  }

  template <typename Factory>
  std::shared_ptr<CameraPublisherT> GetOrCreateCamera(
      void *actor, stream_id_type id, const char *prefix, Factory &&make) {
    return GetOrCreate(_camera_publishers, actor, id, {prefix}, std::forward<Factory>(make));
  }

  /// Empty publisher when none is cached and the actor is unregistered or
  /// opted out of TF. parent_frame is "map" for a top-level actor.
  template <typename Factory>
  TransformTarget GetOrCreateTransform(void *actor, Factory &&make) {
    TransformTarget target;
    {
      std::lock_guard<std::mutex> lock(_mutex);
      auto registration = _registrations.find(actor);
      auto it = _transforms.find(actor);
      if (it == _transforms.end() &&
          (registration == _registrations.end() || !registration->second.publish_tf)) {
        return target;
      }
      if (registration != _registrations.end()) {
        target.frame_id = registration->second.frame_id;
      }
      const std::string chain = ParentChainLocked(actor);
      target.parent_frame = chain.empty() ? std::string{"map"} : chain;
      if (it != _transforms.end()) {
        target.publisher = it->second;
        return target;
      }
      ++_in_flight[actor].count;
    }
    target.publisher = Finish(_transforms, actor, make());
    return target;
  }

private:
  using SensorMap = std::unordered_map<void *, std::shared_ptr<SensorPublisherT>>;
  using CameraMap = std::unordered_map<void *, std::shared_ptr<CameraPublisherT>>;
  using TransformMap = std::unordered_map<void *, std::shared_ptr<TransformPublisherT>>;

  /// Publishers taken out under the lock and destroyed after it is released.
  struct Erased {
    std::shared_ptr<SensorPublisherT> sensor;
    std::shared_ptr<CameraPublisherT> camera;
    std::shared_ptr<TransformPublisherT> transform;
  };

  template <typename Map>
  static typename Map::mapped_type Take(Map &map, void *actor) {
    typename Map::mapped_type taken;
    auto it = map.find(actor);
    if (it != map.end()) {
      taken = std::move(it->second);
      map.erase(it);
    }
    return taken;
  }

  Erased TakePublishers(void *actor) {
    auto in_flight = _in_flight.find(actor);
    if (in_flight != _in_flight.end()) {
      in_flight->second.released = true;
    }
    return Erased{Take(_publishers, actor), Take(_camera_publishers, actor), Take(_transforms, actor)};
  }

  template <typename Map, typename Factory>
  typename Map::mapped_type GetOrCreate(
      Map &map, void *actor, stream_id_type id,
      std::initializer_list<const char *> prefixes, Factory &&make) {
    Naming naming;
    {
      std::lock_guard<std::mutex> lock(_mutex);
      auto it = map.find(actor);
      if (it != map.end()) {
        return it->second;
      }
      for (const char *prefix : prefixes) {
        ResolveAutoStreamSuffixLocked(actor, prefix, id);
      }
      naming.base_topic = BaseTopicNameLocked(actor);
      auto registration = _registrations.find(actor);
      if (registration != _registrations.end()) {
        naming.frame_id = registration->second.frame_id;
      }
      auto topic_override = _topic_overrides.find(actor);
      naming.topic_override = topic_override != _topic_overrides.end() && !topic_override->second.empty();
      ++_in_flight[actor].count;
    }
    return Finish(map, actor, make(naming));
  }

  /// A publisher built while the actor's publishers were released is used for
  /// this sample only: caching it would revive what was just released. When
  /// another thread cached one first, that one wins and ours is dropped
  /// after the lock is released.
  template <typename Map>
  typename Map::mapped_type Finish(Map &map, void *actor, typename Map::mapped_type created) {
    std::lock_guard<std::mutex> lock(_mutex);
    auto in_flight = _in_flight.find(actor);
    const bool released = in_flight == _in_flight.end() || in_flight->second.released;
    if (in_flight != _in_flight.end() && --in_flight->second.count == 0u) {
      _in_flight.erase(in_flight);
    }
    if (created == nullptr || released) {
      return created;
    }
    return map.try_emplace(actor, created).first->second;
  }

  void ResolveAutoStreamSuffixLocked(void *actor, const std::string &prefix, stream_id_type id) {
    auto it = _registrations.find(actor);
    if (it == _registrations.end()) {
      return;
    }
    const std::string placeholder = prefix + "__";
    if (it->second.ros_name != placeholder) {
      return;
    }
    std::string resolved = prefix + std::to_string(id);
    it->second.ros_name = resolved;
    if (it->second.frame_id == placeholder) {
      it->second.frame_id = std::move(resolved);
    }
  }

  std::string RosNameLocked(void *actor) const {
    auto it = _registrations.find(actor);
    return it != _registrations.end() ? it->second.ros_name : std::string{};
  }

  std::string ParentChainLocked(void *actor) const {
    auto it = _actor_parents.find(actor);
    if (it == _actor_parents.end()) {
      return std::string{};
    }
    const std::string current_actor_name = RosNameLocked(actor);
    std::string parent_name;
    for (auto *parent : it->second) {
      const std::string name = RosNameLocked(parent);
      if (name.empty() || name == current_actor_name) {
        continue;
      }
      parent_name = name + '/' + parent_name;
    }
    if (!parent_name.empty() && parent_name.back() == '/') {
      parent_name.pop_back();
    }
    return parent_name;
  }

  /// `rt/carla/[parent/]ros_name`, or `rt[/]<override>` when the actor has a
  /// non-empty ros_topic_name; empty if the actor is not registered.
  std::string BaseTopicNameLocked(void *actor) const {
    auto override_it = _topic_overrides.find(actor);
    if (override_it != _topic_overrides.end() && !override_it->second.empty()) {
      const std::string &custom = override_it->second;
      std::string base_topic_name = "rt";
      if (custom.front() != '/') {
        base_topic_name += '/';
      }
      base_topic_name += custom;
      return base_topic_name;
    }
    const std::string ros_name = RosNameLocked(actor);
    if (ros_name.empty()) {
      return std::string{};
    }
    const std::string parent_chain = ParentChainLocked(actor);
    std::string base_topic_name = "rt/carla/";
    if (!parent_chain.empty()) {
      base_topic_name += parent_chain + "/";
    }
    base_topic_name += ros_name;
    return base_topic_name;
  }

  struct InFlight {
    std::uint32_t count{0u};
    bool released{false};
  };

  mutable std::mutex _mutex;
  std::unordered_map<void *, InFlight> _in_flight;
  std::unordered_map<void *, Registration> _registrations;
  std::unordered_map<void *, std::string> _topic_overrides;
  std::unordered_map<void *, std::vector<void *>> _actor_parents;
  SensorMap _publishers;
  CameraMap _camera_publishers;
  TransformMap _transforms;
};

}  // namespace ros2
}  // namespace carla
