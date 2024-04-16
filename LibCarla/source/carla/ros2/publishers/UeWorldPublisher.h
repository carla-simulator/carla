// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/ROS2NameRegistry.h"
#include "carla/ros2/publishers/CarlaActorListPublisher.h"
#include "carla/ros2/publishers/CarlaStatusPublisher.h"
#include "carla/ros2/publishers/ClockPublisher.h"
#include "carla/ros2/publishers/MapPublisher.h"
#include "carla/ros2/publishers/ObjectPublisher.h"
#include "carla/ros2/publishers/ObjectsPublisher.h"
#include "carla/ros2/publishers/TrafficLightPublisher.h"
#include "carla/ros2/publishers/TrafficLightsPublisher.h"
#include "carla/ros2/publishers/TrafficSignPublisher.h"
#include "carla/ros2/publishers/UePublisherBaseSensor.h"
#include "carla/ros2/publishers/VehiclePublisher.h"
#include "carla/ros2/publishers/WalkerPublisher.h"
#include "carla/ros2/subscribers/AckermannControlSubscriber.h"
#include "carla/ros2/subscribers/CarlaControlSubscriber.h"
#include "carla/ros2/subscribers/CarlaSynchronizationWindowSubscriber.h"
#include "carla/ros2/subscribers/VehicleControlSubscriber.h"
#include "carla/ros2/subscribers/WalkerControlSubscriber.h"
#include "carla/ros2/types/Object.h"
#include "carla/ros2/types/VehicleActorDefinition.h"
#include "carla/rpc/RpcServerInterface.h"
#include "carla/sensor/data/ActorDynamicState.h"
#include "carla/sensor/s11n/EpisodeStateSerializer.h"

namespace carla {
namespace ros2 {

/**
 * The publisher collecting all world related publishing activities that are not explicitly defined as
 * The publisher collecting all world related publishing activities that are not explicitly defined as
 * - clock
 * - transform
 * - sensor
 * - vehicle
 * - traffic_light
 * - traffic_sign
 *
 */
class UeWorldPublisher : public UePublisherBaseSensor {
public:
  UeWorldPublisher(carla::rpc::RpcServerInterface &carla_server, std::shared_ptr<ROS2NameRegistry> name_registry,
                   std::shared_ptr<carla::ros2::types::SensorActorDefinition> sensor_actor_definition);
  virtual ~UeWorldPublisher() = default;

  /**
   * Implements ROS2NameRecord::Init() interface
   */
  bool Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) override;

  /**
   * Implement PublisherInterface::Publish interface
   */
  bool Publish() override;

  /**
   * Implement PublisherInterface::SubscribersConnected() interface
   */
  bool SubscribersConnected() const override {
    return true;
  }

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

  /**
   * Implement actions on actors removed
   */
  void RemoveActor(ActorId actor);

  /**
   * Implement UePublisherBaseSensor::UpdateSensorData()
   */
  void UpdateSensorData(std::shared_ptr<carla::sensor::s11n::SensorHeaderSerializer::Header const> sensor_header,
                        carla::SharedBufferView buffer_view) override;

  void AddVehicleUe(std::shared_ptr<carla::ros2::types::VehicleActorDefinition> vehicle_actor_definition,
                    carla::ros2::types::VehicleControlCallback vehicle_control_callback,
                    carla::ros2::types::VehicleAckermannControlCallback vehicle_ackermann_control_callback);
  void AddWalkerUe(std::shared_ptr<carla::ros2::types::WalkerActorDefinition> walker_actor_definition,
                   carla::ros2::types::WalkerControlCallback walker_control_callback);
  void AddTrafficLightUe(
      std::shared_ptr<carla::ros2::types::TrafficLightActorDefinition> traffic_light_actor_definition);
  void AddTrafficSignUe(std::shared_ptr<carla::ros2::types::TrafficSignActorDefinition> traffic_sign_actor_definition);

  uint64_t CurrentFrame() const {
    return _frame;
  }
  carla::ros2::types::Timestamp const &CurrentTimestamp() const {
    return _timestamp;
  }

  auto GetTransformPublisher() const {
    return _transform_publisher;
  }

private:
  using EpisodeHeaderConst = carla::sensor::s11n::EpisodeStateSerializer::Header const;

  /**
   * @brief provides access to the image header stored at the start of the buffer
   */
  std::shared_ptr<EpisodeHeaderConst> header_view(const carla::SharedBufferView buffer_view) {
    return std::shared_ptr<EpisodeHeaderConst>(buffer_view,
                                               reinterpret_cast<EpisodeHeaderConst *>(buffer_view.get()->data()));
  }

  /**
   * @brief access the buffer data as vector
   */
  std::vector<carla::sensor::data::ActorDynamicState,
              carla::sensor::data::SerializerVectorAllocator<carla::sensor::data::ActorDynamicState>>
  buffer_data_2_vector(const carla::SharedBufferView buffer_view) const {
    return carla::sensor::data::buffer_data_accessed_by_vector<carla::sensor::data::ActorDynamicState>(
        buffer_view, carla::sensor::s11n::EpisodeStateSerializer::header_offset);
  }

  carla::ros2::types::Timestamp _timestamp{};
  uint64_t _frame{0u};
  carla::sensor::s11n::EpisodeStateSerializer::Header _episode_header;
  bool _objects_changed{false};
  std::unordered_map<ActorId, std::shared_ptr<carla::ros2::types::Object>> _objects;

  struct UeVehicle {
    explicit UeVehicle(std::shared_ptr<VehiclePublisher> carla_vehicle_publisher)
      : _vehicle_publisher(carla_vehicle_publisher) {}
    std::shared_ptr<VehiclePublisher> _vehicle_publisher;
    std::shared_ptr<VehicleControlSubscriber> _vehicle_controller;
    std::shared_ptr<AckermannControlSubscriber> _vehicle_ackermann_controller;
  };
  std::unordered_map<ActorId, UeVehicle> _vehicles;

  struct UeWalker {
    explicit UeWalker(std::shared_ptr<WalkerPublisher> carla_walker_publisher)
      : _walker_publisher(carla_walker_publisher) {}
    std::shared_ptr<WalkerPublisher> _walker_publisher;
    std::shared_ptr<WalkerControlSubscriber> _walker_controller;
    carla::ros2::types::WalkerControlCallback _walker_control_callback;
  };
  std::unordered_map<ActorId, UeWalker> _walkers;

  struct UeTrafficLight {
    explicit UeTrafficLight(std::shared_ptr<TrafficLightPublisher> carla_traffic_light_publisher)
      : _traffic_light_publisher(carla_traffic_light_publisher) {}
    std::shared_ptr<TrafficLightPublisher> _traffic_light_publisher;
  };
  std::unordered_map<ActorId, UeTrafficLight> _traffic_lights;

  struct UeTrafficSign {
    explicit UeTrafficSign(std::shared_ptr<TrafficSignPublisher> carla_traffic_sign_publisher)
      : _traffic_sign_publisher(carla_traffic_sign_publisher) {}
    std::shared_ptr<TrafficSignPublisher> _traffic_sign_publisher;
  };
  std::unordered_map<ActorId, UeTrafficSign> _traffic_signs;

  std::shared_ptr<DdsDomainParticipantImpl> _domain_participant_impl;

  carla::rpc::RpcServerInterface &_carla_server;
  std::shared_ptr<ROS2NameRegistry> _name_registry;
  // publisher
  std::shared_ptr<CarlaStatusPublisher> _carla_status_publisher;
  std::shared_ptr<CarlaActorListPublisher> _carla_actor_list_publisher;
  std::shared_ptr<ClockPublisher> _clock_publisher;
  std::shared_ptr<MapPublisher> _map_publisher;
  std::shared_ptr<ObjectsPublisher> _objects_publisher;
  std::shared_ptr<TrafficLightsPublisher> _traffic_lights_publisher;
  // subscriber
  std::shared_ptr<CarlaControlSubscriber> _carla_control_subscriber;
  std::shared_ptr<CarlaSynchronizationWindowSubscriber> _sync_subscriber;

  bool _initialized{false};
};
}  // namespace ros2
}  // namespace carla
