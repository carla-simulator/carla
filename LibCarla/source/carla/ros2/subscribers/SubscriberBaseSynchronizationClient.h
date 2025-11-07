// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>
#include <vector>

#include "carla/ros2/subscribers/SubscriberBase.h"
#include "carla/rpc/RpcServerInterface.h"
#include "carla/rpc/ServerSynchronizationTypes.h"

namespace carla {
namespace ros2 {


template <typename MESSAGE_TYPE>
class SubscriberBaseSynchronizationClient : public SubscriberBase<MESSAGE_TYPE> {
public:
  explicit SubscriberBaseSynchronizationClient(ROS2NameRecord &parent, carla::rpc::RpcServerInterface &carla_server) :
    SubscriberBase<MESSAGE_TYPE>(parent), 
    _carla_server(carla_server) {
  }

  virtual ~SubscriberBaseSynchronizationClient() {
    if ( !_synchronization_client_id.empty() ) {
      for (auto [publisher_guid, participant] : _carla_synchronization_window_participants) {
        carla::log_debug("~SubscriberBaseSynchronizationClient[", _synchronization_client_id, "]:: disconnect publisher (",
                        publisher_guid, ", ", participant, ")");
        _carla_server.call_deregister_synchronization_participant(_synchronization_client_id, participant);
      }
    }
    _carla_synchronization_window_participants.clear();
  }

  /**
   * Implements SubscriberBase::PublisherConnected()
   */
  void PublisherConnected(std::string const &publisher_guid) override {
    if ( _synchronization_client_id.empty() ) {
      _synchronization_client_id = ThisAsSynchronizationClient();
    }
    auto carla_synchronization_window_participant = _carla_server.call_register_synchronization_participant(ThisAsSynchronizationClient()).Get();
    _carla_synchronization_window_participants.insert({publisher_guid, carla_synchronization_window_participant});
    carla::log_debug("SubscriberBaseSynchronizationClient[", ThisAsSynchronizationClient(), "]::PublisherConnected(",
                      publisher_guid, ", ", carla_synchronization_window_participant, ")");
  }

  /**
   * Implements SubscriberBase::PublisherDisconnected()
   */
  void PublisherDisconnected(std::string const &publisher_guid) override {
    auto carla_synchronization_window_participant = GetSynchronizationParticipant(publisher_guid);
    carla::log_debug("SubscriberBaseSynchronizationClient[", ThisAsSynchronizationClient(), "]::PublisherDisconnected(",
                      publisher_guid, ", ", carla_synchronization_window_participant, ")");
    _carla_server.call_deregister_synchronization_participant(ThisAsSynchronizationClient(),
                                                              carla_synchronization_window_participant);
    _carla_synchronization_window_participants.erase(publisher_guid);
  }


protected:

  virtual carla::rpc::synchronization_client_id_type ThisAsSynchronizationClient() = 0;

  carla::rpc::synchronization_participant_id_type GetSynchronizationParticipant(std::string const &participant_guid) {
    auto find_result = _carla_synchronization_window_participants.find(participant_guid);
    if ( find_result != _carla_synchronization_window_participants.end() ) {
      return find_result->second;
    }
    carla::log_error("SubscriberBaseSynchronizationClient[", ThisAsSynchronizationClient(), "]::GetSynchronizationParticipant(",
                      participant_guid, ") participant not found.");
    return carla::rpc::ALL_PARTICIPANTS;
  }

  carla::rpc::RpcServerInterface &_carla_server;

private:
  std::map<std::string , carla::rpc::synchronization_participant_id_type>
      _carla_synchronization_window_participants;
  carla::rpc::synchronization_client_id_type _synchronization_client_id;
};
}  // namespace ros2
}  // namespace carla
