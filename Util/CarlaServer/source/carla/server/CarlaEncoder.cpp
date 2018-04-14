// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/server/CarlaEncoder.h"

#include <cstring>

#include "carla/ArrayView.h"
#include "carla/Debug.h"
#include "carla/Logging.h"
#include "carla/server/CarlaSceneDescription.h"
#include "carla/server/RequestNewEpisode.h"

#include "carla/server/carla_server.pb.h"

namespace cs = carla_server;

namespace carla {
namespace server {

  static auto start_spots(const carla_scene_description &values) {
    return array_view::make_const(values.player_start_spots, values.number_of_player_start_spots);
  }

  static auto sensors(const carla_scene_description &values) {
    return array_view::make_const(values.sensors, values.number_of_sensors);
  }

  static auto agents(const carla_measurements &values) {
    return array_view::make_const(values.non_player_agents, values.number_of_non_player_agents);
  }

  static void Set(cs::Vector3D *lhs, const carla_vector3d &rhs) {
    DEBUG_ASSERT(lhs != nullptr);
    lhs->set_x(rhs.x);
    lhs->set_y(rhs.y);
    lhs->set_z(rhs.z);
  }

  static void Set(cs::Rotation3D *lhs, const carla_rotation3d &rhs) {
    DEBUG_ASSERT(lhs != nullptr);
    lhs->set_pitch(rhs.pitch);
    lhs->set_roll(rhs.roll);
    lhs->set_yaw(rhs.yaw);
  }

  static void Set(cs::Transform *lhs, const carla_transform &rhs) {
    DEBUG_ASSERT(lhs != nullptr);
    Set(lhs->mutable_location(), rhs.location);
    Set(lhs->mutable_orientation(), rhs.orientation);
    Set(lhs->mutable_rotation(), rhs.rotation);
  }

  static void Set(cs::BoundingBox *lhs, const carla_bounding_box &rhs) {
    DEBUG_ASSERT(lhs != nullptr);
    Set(lhs->mutable_transform(), rhs.transform);
    Set(lhs->mutable_extent(), rhs.extent);
  }

  static void Set(cs::Sensor *lhs, const carla_sensor_definition &rhs) {
    DEBUG_ASSERT(lhs != nullptr);
    lhs->set_id(rhs.id);
    lhs->set_name(std::string(rhs.name));
    lhs->set_type([&](){
      switch (rhs.type) {
        case CARLA_SERVER_CAMERA:             return cs::Sensor::CAMERA;
        case CARLA_SERVER_LIDAR_RAY_CAST:     return cs::Sensor::LIDAR_RAY_CAST;
        default:                              return cs::Sensor::UNKNOWN;
      }
    }());
  }

  static void Set(cs::Control *lhs, const carla_control &rhs) {
    DEBUG_ASSERT(lhs != nullptr);
    lhs->set_steer(rhs.steer);
    lhs->set_throttle(rhs.throttle);
    lhs->set_brake(rhs.brake);
    lhs->set_hand_brake(rhs.hand_brake);
    lhs->set_reverse(rhs.reverse);
  }

  static void SetVehicle(cs::Vehicle *lhs, const carla_agent &rhs) {
    DEBUG_ASSERT(lhs != nullptr);
    Set(lhs->mutable_transform(), rhs.transform);
    Set(lhs->mutable_bounding_box(), rhs.bounding_box);
    lhs->set_forward_speed(rhs.forward_speed);
  }

  static void SetPedestrian(cs::Pedestrian *lhs, const carla_agent &rhs) {
    DEBUG_ASSERT(lhs != nullptr);
    Set(lhs->mutable_transform(), rhs.transform);
    Set(lhs->mutable_bounding_box(), rhs.bounding_box);
    lhs->set_forward_speed(rhs.forward_speed);
  }

  static void SetSpeedLimitSign(cs::SpeedLimitSign *lhs, const carla_agent &rhs) {
    DEBUG_ASSERT(lhs != nullptr);
    Set(lhs->mutable_transform(), rhs.transform);
    lhs->set_speed_limit(rhs.forward_speed);
  }

  static void SetTrafficLight(cs::TrafficLight *lhs, const carla_agent &rhs, cs::TrafficLight::State state) {
    DEBUG_ASSERT(lhs != nullptr);
    Set(lhs->mutable_transform(), rhs.transform);
    lhs->set_state(state);
  }

  static void Set(cs::Agent *lhs, const carla_agent &rhs) {
    DEBUG_ASSERT(lhs != nullptr);
    lhs->set_id(rhs.id);
    switch (rhs.type) {
      case CARLA_SERVER_AGENT_VEHICLE:
        return SetVehicle(lhs->mutable_vehicle(), rhs);
      case CARLA_SERVER_AGENT_PEDESTRIAN:
        return SetPedestrian(lhs->mutable_pedestrian(), rhs);
      case CARLA_SERVER_AGENT_SPEEDLIMITSIGN:
        return SetSpeedLimitSign(lhs->mutable_speed_limit_sign(), rhs);
      case CARLA_SERVER_AGENT_TRAFFICLIGHT_GREEN:
        return SetTrafficLight(lhs->mutable_traffic_light(), rhs, cs::TrafficLight::GREEN);
      case CARLA_SERVER_AGENT_TRAFFICLIGHT_YELLOW:
        return SetTrafficLight(lhs->mutable_traffic_light(), rhs, cs::TrafficLight::YELLOW);
      case CARLA_SERVER_AGENT_TRAFFICLIGHT_RED:
        return SetTrafficLight(lhs->mutable_traffic_light(), rhs, cs::TrafficLight::RED);
      default:
        log_error("invalid agent type");
    }
  }

  std::string CarlaEncoder::Encode(const carla_scene_description &values) {
    auto *message = _protobuf.CreateMessage<cs::SceneDescription>();
    DEBUG_ASSERT(message != nullptr);
    message->set_map_name(std::string(values.map_name));
    for (auto &spot : start_spots(values)) {
      Set(message->add_player_start_spots(), spot);
    }
    for (auto &sensor : sensors(values)) {
      Set(message->add_sensors(), sensor);
    }
    return Protobuf::Encode(*message);
  }

  std::string CarlaEncoder::Encode(const CarlaSceneDescription &values) {
    return values.pop_scene();
  }

  std::string CarlaEncoder::Encode(const carla_episode_ready &values) {
    auto *message = _protobuf.CreateMessage<cs::EpisodeReady>();
    DEBUG_ASSERT(message != nullptr);
    message->set_ready(values.ready);
    return Protobuf::Encode(*message);
  }

  std::string CarlaEncoder::Encode(const carla_measurements &values) {
    static thread_local auto *message = _protobuf.CreateMessage<cs::Measurements>();
    DEBUG_ASSERT(message != nullptr);
    message->set_frame_number(values.frame_number);
    message->set_platform_timestamp(values.platform_timestamp);
    message->set_game_timestamp(values.game_timestamp);
    // Player measurements.
    auto *player = message->mutable_player_measurements();
    DEBUG_ASSERT(player != nullptr);
    Set(player->mutable_transform(), values.player_measurements.transform);
    Set(player->mutable_bounding_box(), values.player_measurements.bounding_box);
    Set(player->mutable_acceleration(), values.player_measurements.acceleration);
    player->set_forward_speed(values.player_measurements.forward_speed);
    player->set_collision_vehicles(values.player_measurements.collision_vehicles);
    player->set_collision_pedestrians(values.player_measurements.collision_pedestrians);
    player->set_collision_other(values.player_measurements.collision_other);
    player->set_intersection_otherlane(values.player_measurements.intersection_otherlane);
    player->set_intersection_offroad(values.player_measurements.intersection_offroad);
    Set(player->mutable_autopilot_control(), values.player_measurements.autopilot_control);
    // Non-player agents.
    message->clear_non_player_agents(); // we need to clear as we cache the message.
    for (auto &agent : agents(values)) {
      Set(message->add_non_player_agents(), agent);
    }
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
      values.player_start_spot_index = message->player_start_spot_index();
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
