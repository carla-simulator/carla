// Copyright (c) 2023 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/BufferView.h"
#include "carla/ros2/ROS2NameRegistry.h"
#include "carla/ros2/ROS2Session.h"
#include "carla/ros2/types/SensorActorDefinition.h"
#include "carla/ros2/types/TrafficLightActorDefinition.h"
#include "carla/ros2/types/TrafficSignActorDefinition.h"
#include "carla/ros2/types/VehicleActorDefinition.h"
#include "carla/ros2/types/WalkerActorDefinition.h"
#include "carla/rpc/RpcServerInterface.h"
#include "carla/streaming/detail/Message.h"

#include <list>
#include <memory>
#include <unordered_map>

namespace carla {
namespace ros2 {

class DdsDomainParticipantImpl;
class UePublisherBaseSensor;
class TransformPublisher;
class UeWorldPublisher;
class ServiceInterface;

class ROS2 {
public:
  // deleting copy constructor for singleton
  ROS2(const ROS2& obj) = delete;
  ~ROS2() = default;

  static std::shared_ptr<ROS2> GetInstance();

  // starting/stopping
  void Enable(carla::rpc::RpcServerInterface* carla_server,
              carla::streaming::detail::stream_id_type const world_observer_stream_id);
  bool IsEnabled() {
    return _enabled;
  }
  void NotifyInitGame();
  void NotifyBeginEpisode();
  void NotifyEndEpisode();
  void NotifyEndGame();
  void Disable();

  void AttachActors(ActorId const child, ActorId const parent);

  void AddVehicleUe(std::shared_ptr<carla::ros2::types::VehicleActorDefinition> vehicle_actor_definition,
                    carla::ros2::types::VehicleControlCallback vehicle_control_callback,
                    carla::ros2::types::VehicleAckermannControlCallback vehicle_ackermann_control_callback);
  void AddWalkerUe(std::shared_ptr<carla::ros2::types::WalkerActorDefinition> walker_actor_definition,
                   carla::ros2::types::WalkerControlCallback walker_control_callback);
  void AddTrafficLightUe(
      std::shared_ptr<carla::ros2::types::TrafficLightActorDefinition> traffic_light_actor_definition);
  void AddTrafficSignUe(std::shared_ptr<carla::ros2::types::TrafficSignActorDefinition> traffic_sign_actor_definition);
  bool AddSensorUe(std::shared_ptr<carla::ros2::types::SensorActorDefinition> sensor_actor_definition);
  void RemoveActor(ActorId const actor);

  void ProcessDataFromUeSensor(carla::streaming::detail::stream_id_type const stream_id,
                               std::shared_ptr<const carla::streaming::detail::Message> message);

  /**
   * Implement actions before each tick
   */
  void PreTickAction();

  /**
   * Process incoming messages
   */
  void ProcessMessages();

  /**
   * Implement actions after each tick
   */
  void PostTickAction();

  uint64_t CurrentFrame() const;
  carla::ros2::types::Timestamp const& CurrentTimestamp() const;

  std::shared_ptr<ROS2NameRegistry> GetNameRegistry() {
    return _name_registry;
  }

private:
  bool _enabled{false};
  carla::rpc::RpcServerInterface* _carla_server{nullptr};
  std::shared_ptr<ROS2NameRegistry> _name_registry{nullptr};
  std::shared_ptr<carla::streaming::detail::Dispatcher> _dispatcher;
  std::shared_ptr<DdsDomainParticipantImpl> _domain_participant_impl;
  std::shared_ptr<carla::ros2::types::SensorActorDefinition> _world_observer_sensor_actor_definition;

  struct UeSensor {
    UeSensor(std::shared_ptr<carla::ros2::types::SensorActorDefinition> sensor_actor_definition_)
      : sensor_actor_definition(sensor_actor_definition_) {}
    std::shared_ptr<carla::ros2::types::SensorActorDefinition> sensor_actor_definition;
    bool publisher_expected{true};
    std::shared_ptr<UePublisherBaseSensor> publisher;
    std::shared_ptr<ROS2Session> session;
  };
  std::unordered_map<carla::streaming::detail::stream_id_type, UeSensor> _ue_sensors;
  std::shared_ptr<TransformPublisher> _transform_publisher;

  std::shared_ptr<UeWorldPublisher> _world_publisher;

  std::list<std::shared_ptr<carla::ros2::ServiceInterface>> _services;

  void CreateSensorUePublisher(UeSensor& sensor);

  // sigleton
  ROS2(){};
};

}  // namespace ros2
}  // namespace carla
