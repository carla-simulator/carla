// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/multigpu/sensorStreamRegistry.h"

#include "carla/Logging.h"

#include <utility>

namespace carla {
namespace multigpu {

std::optional<SensorStreamRegistry::token_type> SensorStreamRegistry::Resolve(
    carla::streaming::Server &server,
    actor_id_type actor_id,
    stream_id_type primary_stream_id) {
  std::scoped_lock<std::mutex> lock(_mutex);

  auto it = _entries.find(actor_id);
  std::optional<token_type> token;
  if (it != _entries.end()) {
    token = server.FindToken(it->second.stream_id);
    if (!token) {
      log_warning("multigpu: stream ", it->second.stream_id, " of actor ", actor_id, " no longer exists");
      Forget(server, it->second);
      _entries.erase(it);
      it = _entries.end();
    }
  }

  if (it == _entries.end()) {
    auto stream = server.MakeStream();
    const auto stream_id = token_type(stream.token()).get_stream_id();
    token = server.FindToken(stream_id);
    if (!token) {
      return std::nullopt;
    }
    it = _entries.emplace(actor_id, Entry{stream_id, std::move(stream), std::nullopt}).first;
    log_info("multigpu: reserved stream ", stream_id, " for actor ", actor_id, " until it spawns");
  }

  auto &entry = it->second;
  if (entry.primary_stream_id && (*entry.primary_stream_id != primary_stream_id)) {
    server.RemoveStreamAlias(*entry.primary_stream_id);
  }
  entry.primary_stream_id = primary_stream_id;
  server.SetStreamAlias(primary_stream_id, entry.stream_id);

  token->set_stream_id(primary_stream_id);
  return token;
}

std::optional<carla::streaming::Stream> SensorStreamRegistry::Bind(
    carla::streaming::Server &server,
    actor_id_type actor_id,
    stream_id_type sensor_stream_id,
    bool can_adopt) {
  std::scoped_lock<std::mutex> lock(_mutex);
  if (!_bindable) {
    return std::nullopt;
  }

  for (const auto &[other_actor_id, other] : _entries) {
    if ((other_actor_id != actor_id) && (other.stream_id == sensor_stream_id)) {
      log_warning(
          "multigpu: actor ", actor_id, " replayed onto the sensor of actor ", other_actor_id, "; not binding it");
      return std::nullopt;
    }
  }

  auto it = _entries.find(actor_id);
  if (it == _entries.end()) {
    _entries.emplace(actor_id, Entry{sensor_stream_id, std::nullopt, std::nullopt});
    return std::nullopt;
  }

  auto &entry = it->second;
  if (entry.reserved && (entry.stream_id != sensor_stream_id) && can_adopt) {
    return std::exchange(entry.reserved, std::nullopt);
  }

  if (entry.stream_id != sensor_stream_id) {
    if (entry.primary_stream_id) {
      server.SetStreamAlias(*entry.primary_stream_id, sensor_stream_id);
    }
    // Early subscribers get disconnected and reconnect through the alias.
    if (entry.reserved) {
      server.CloseStream(entry.stream_id);
    }
    entry.stream_id = sensor_stream_id;
  }
  entry.reserved.reset();
  return std::nullopt;
}

void SensorStreamRegistry::Unbind(carla::streaming::Server &server, actor_id_type actor_id) {
  std::scoped_lock<std::mutex> lock(_mutex);
  // Destroy events replayed before the new episode opens belong to the
  // previous one, whose actor ids may collide with the current reservations.
  if (!_bindable) {
    return;
  }
  auto it = _entries.find(actor_id);
  if (it == _entries.end()) {
    return;
  }
  Forget(server, it->second);
  _entries.erase(it);
}

SensorStreamRegistry::epoch_type SensorStreamRegistry::BeginEpisodeChange(carla::streaming::Server &server) {
  std::scoped_lock<std::mutex> lock(_mutex);
  for (const auto &item : _entries) {
    Forget(server, item.second);
  }
  _entries.clear();
  _bindable = false;
  return ++_epoch;
}

void SensorStreamRegistry::OpenEpisode(epoch_type epoch) {
  std::scoped_lock<std::mutex> lock(_mutex);
  _bindable = (epoch == _epoch);
}

void SensorStreamRegistry::CloseEpisode() {
  std::scoped_lock<std::mutex> lock(_mutex);
  _bindable = false;
}

bool SensorStreamRegistry::IsEpisodeOpen() {
  std::scoped_lock<std::mutex> lock(_mutex);
  return _bindable;
}

void SensorStreamRegistry::ResetOwnership(carla::streaming::Server &server) {
  std::scoped_lock<std::mutex> lock(_mutex);
  for (auto &item : _entries) {
    auto &entry = item.second;
    if (entry.primary_stream_id) {
      server.RemoveStreamAlias(*entry.primary_stream_id);
      entry.primary_stream_id.reset();
    }
  }
}

bool SensorStreamRegistry::IsOwnedStream(stream_id_type local_stream_id) const {
  std::scoped_lock<std::mutex> lock(_mutex);
  for (const auto &item : _entries) {
    if ((item.second.stream_id == local_stream_id) && item.second.primary_stream_id) {
      return true;
    }
  }
  return false;
}

void SensorStreamRegistry::Forget(carla::streaming::Server &server, const Entry &entry) {
  if (entry.primary_stream_id) {
    server.RemoveStreamAlias(*entry.primary_stream_id);
  }
  if (entry.reserved) {
    server.CloseStream(entry.stream_id);
  }
}

} // namespace multigpu
} // namespace carla
