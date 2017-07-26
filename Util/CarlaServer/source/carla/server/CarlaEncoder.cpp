// CARLA, Copyright (C) 2017 Computer Vision Center (CVC) Project Settings.

#include "carla/server/CarlaEncoder.h"

#include <cstring>

#include "carla/Debug.h"
#include "carla/Logging.h"

#include "carla/server/carla_server.pb.h"

namespace cs = carla_server;

namespace carla {
namespace server {

  std::string CarlaEncoder::Encode(const carla_scene_description &values) {
    auto *message = _protobuf.CreateMessage<cs::SceneDescription>();
    DEBUG_ASSERT(message != nullptr);
    message->set_player_start_locations(
        values.player_start_locations,
        sizeof(carla_vector3d) * values.number_of_player_start_locations);
    return Protobuf::Encode(*message);
  }

  std::string CarlaEncoder::Encode(const carla_episode_ready &values) {
    auto *message = _protobuf.CreateMessage<cs::EpisodeReady>();
    DEBUG_ASSERT(message != nullptr);
    message->set_ready(values.ready);
    return Protobuf::Encode(*message);
  }

  static void SetVector3D(cs::Vector3D *lhs, const carla_vector3d &rhs) {
    DEBUG_ASSERT(lhs != nullptr);
    lhs->set_x(rhs.x);
    lhs->set_y(rhs.y);
    lhs->set_z(rhs.z);
  }

  std::string CarlaEncoder::Encode(const carla_measurements &values) {
    static thread_local auto *message = _protobuf.CreateMessage<cs::Measurements>();
    DEBUG_ASSERT(message != nullptr);
    message->set_platform_timestamp(values.platform_timestamp);
    message->set_game_timestamp(values.game_timestamp);
    auto *player = message->mutable_player_measurements();
    DEBUG_ASSERT(player != nullptr);
    SetVector3D(player->mutable_location(), values.player_measurements.location);
    SetVector3D(player->mutable_orientation(), values.player_measurements.orientation);
    SetVector3D(player->mutable_acceleration(), values.player_measurements.acceleration);
    player->set_forward_speed(values.player_measurements.forward_speed);
    player->set_collision_vehicles(values.player_measurements.collision_vehicles);
    player->set_collision_pedestrians(values.player_measurements.collision_pedestrians);
    player->set_collision_other(values.player_measurements.collision_other);
    player->set_intersection_otherlane(values.player_measurements.intersection_otherlane);
    player->set_intersection_offroad(values.player_measurements.intersection_offroad);
    return Protobuf::Encode(*message);
  }

  bool CarlaEncoder::Decode(const std::string &str, RequestNewEpisode &values) {
    auto *message = _protobuf.CreateMessage<cs::RequestNewEpisode>();
    DEBUG_ASSERT(message != nullptr);
    message->ParseFromString(str);
    if (message->IsInitialized()) {
      const std::string &file = message->ini_file();
      auto data = std::make_unique<char[]>(file.size());
      std::memcpy(data.get(), file.c_str(), file.size());
      values.data = std::move(data);
      values.values.ini_file = values.data.get();
      values.values.ini_file_length = file.size();
      return true;
    } else {
      log_error("invalid protobuf message: request new episode");
      return false;
    }
  }

  bool CarlaEncoder::Decode(const std::string &str, carla_episode_start &values) {
    auto *message = _protobuf.CreateMessage<cs::EpisodeStart>();
    DEBUG_ASSERT(message != nullptr);
    message->ParseFromString(str);
    if (message->IsInitialized()) {
      values.player_start_location_index = message->player_start_location_index();
      return true;
    } else {
      log_error("invalid protobuf message: episode start");
      return false;
    }
  }

  bool CarlaEncoder::Decode(const std::string &str, carla_control &values) {
    static thread_local auto *message = _protobuf.CreateMessage<cs::Control>();
    DEBUG_ASSERT(message != nullptr);
    message->ParseFromString(str);
    if (message->IsInitialized()) {
      values.steer = message->steer();
      values.throttle = message->throttle();
      values.brake = message->brake();
      values.hand_brake = message->hand_brake();
      values.reverse = message->reverse();
      return true;
    } else {
      log_error("invalid protobuf message: control");
      return false;
    }
  }

} // namespace server
} // namespace carla
