// Copyright (c) 2019 Intel Corporation
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/rss/RssSensor.h"

#include "carla/Logging.h"
#include "carla/client/detail/Simulator.h"

#include "carla/client/ActorList.h"
#include "carla/rss/RssCheck.h"
#include "carla/sensor/data/RssResponse.h"

#include <ad_rss/state/ProperResponse.hpp>
#include <ad_rss/world/AccelerationRestriction.hpp>
#include <ad_rss/world/Velocity.hpp>

#include <exception>

namespace carla {
namespace client {

  namespace cc = carla::client;
  namespace csd = carla::sensor::data;

  RssSensor::~RssSensor() = default;

  void RssSensor::Listen(CallbackFunctionType callback) {
    if (_is_listening) {
      log_error(GetDisplayId(), ": already listening");
      return;
    }

    if (GetParent() == nullptr) {
      throw_exception(std::runtime_error(GetDisplayId() + ": not attached to vehicle"));
      return;
    }

    _map = GetWorld().GetMap();
    DEBUG_ASSERT(_map != nullptr);

    mRssCheck = std::make_shared<::carla::rss::RssCheck>();

    auto self = boost::static_pointer_cast<RssSensor>(shared_from_this());

    log_debug(GetDisplayId(), ": subscribing to tick event");
    GetEpisode().Lock()->RegisterOnTickEvent(
    [cb = std::move(callback), weak_self = WeakPtr<RssSensor>(self)](const auto &snapshot) {
      auto self = weak_self.lock();
      if (self != nullptr) {
        auto data = self->TickRssSensor(snapshot.GetTimestamp());
        if (data != nullptr) {
          cb(std::move(data));
        }
      }
    });

    _is_listening = true;
  }

  SharedPtr<sensor::SensorData> RssSensor::TickRssSensor(const Timestamp &timestamp) {
    try {
      bool result = false;
      ::ad_rss::state::ProperResponse response;
      ::ad_rss::world::AccelerationRestriction accelerationRestriction;
      ::ad_rss::world::Velocity egoVelocity;
      if (_processing_lock.try_lock()) {
        cc::World world = GetWorld();
        SharedPtr<cc::ActorList> actors = world.GetActors();
        SharedPtr<cc::ActorList> vehicles = actors->Filter("vehicle.*");

        // check all object<->ego pairs with RSS and calculate proper response
        result =
        mRssCheck->checkObjects(timestamp, world, vehicles, GetParent(), _map, response,
            accelerationRestriction,
            egoVelocity, visualize_results);
        _processing_lock.unlock();

        csd::LongitudinalResponse longitudinalResponse;
        csd::LateralResponse lateralResponseRight;
        csd::LateralResponse lateralResponseLeft;

        switch (response.longitudinalResponse) {
          case ::ad_rss::state::LongitudinalResponse::None:
            longitudinalResponse = csd::LongitudinalResponse::None;
            break;
          case ::ad_rss::state::LongitudinalResponse::BrakeMinCorrect:
            longitudinalResponse = csd::LongitudinalResponse::BrakeMinCorrect;
            break;
          case ::ad_rss::state::LongitudinalResponse::BrakeMin:
            longitudinalResponse = csd::LongitudinalResponse::BrakeMin;
            break;
        }
        switch (response.lateralResponseRight) {
          case ::ad_rss::state::LateralResponse::None:
            lateralResponseRight = csd::LateralResponse::None;
            break;
          case ::ad_rss::state::LateralResponse::BrakeMin:
            lateralResponseRight = csd::LateralResponse::BrakeMin;
            break;
        }
        switch (response.lateralResponseLeft) {
          case ::ad_rss::state::LateralResponse::None:
            lateralResponseLeft = csd::LateralResponse::None;
            break;
          case ::ad_rss::state::LateralResponse::BrakeMin:
            lateralResponseLeft = csd::LateralResponse::BrakeMin;
            break;
        }
        return MakeShared<sensor::data::RssResponse>(timestamp.frame, timestamp.elapsed_seconds,
        GetTransform(),
        result, longitudinalResponse, lateralResponseRight,
        lateralResponseLeft, accelerationRestriction, egoVelocity);
      } else {
        return nullptr;
      }
    } catch (const std::exception &e) {
      std::cout << e.what() << std::endl;
      _processing_lock.unlock();
      return nullptr;
    }
  }

  void RssSensor::Stop() {
    _is_listening = false;
  }

  const ::ad_rss::world::RssDynamics &RssSensor::GetEgoVehicleDynamics() const {
    return mRssCheck->getEgoVehicleDynamics();
  }

  void RssSensor::SetEgoVehicleDynamics(const ::ad_rss::world::RssDynamics &dynamics) {
    mRssCheck->setEgoVehicleDynamics(dynamics);
  }

  const ::ad_rss::world::RssDynamics &RssSensor::GetOtherVehicleDynamics() const {
    return mRssCheck->getOtherVehicleDynamics();
  }

  void RssSensor::SetOtherVehicleDynamics(const ::ad_rss::world::RssDynamics &dynamics) {
    mRssCheck->setOtherVehicleDynamics(dynamics);
  }

}  // namespace client
}  // namespace carla
