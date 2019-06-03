// Copyright (c) 2019 Intel Corporation
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>
#include "carla/client/ActorList.h"
#include "carla/client/Vehicle.h"
#include "carla/road/Map.h"

namespace ad_rss {
namespace state {
struct ProperResponse;
struct RssStateSnapshot;
}  // namespace state
namespace core {
class RssSituationChecking;
class RssSituationExtraction;
class RssResponseResolving;
}  // namespace core
namespace lane {
struct VehicleState;
}
namespace world {
class LaneSegment;
class Object;
class OccupiedRegion;
using OccupiedRegionVector = std::vector<::ad_rss::world::OccupiedRegion>;
class RssDynamics;
}  // namespace world
}  // namespace ad_rss

namespace carla {
namespace rss {

class RssCheck {
 public:
  RssCheck();

  ~RssCheck();

  bool checkObjects(carla::client::Timestamp const &timestamp, carla::client::World &world,
                    carla::SharedPtr<carla::client::ActorList> const &vehicles,
                    carla::SharedPtr<carla::client::Actor> const &carlaEgoVehicle,
                    carla::SharedPtr<carla::client::Map> const &map, ::ad_rss::state::ProperResponse &response,
                    bool visualizeResults = false);

 private:
  std::shared_ptr<::ad_rss::core::RssSituationExtraction> mRssSituationExtraction;
  std::shared_ptr<::ad_rss::core::RssSituationChecking> mRssSituationChecking;
  std::shared_ptr<::ad_rss::core::RssResponseResolving> mRssResponseResolving;

  uint64_t calculateLaneSegmentId(const carla::road::Lane &lane) const;

  void calculateLatLonVelocities(const carla::geom::Transform &laneLocation, const carla::geom::Vector3D &velocity,
                                 ::ad_rss::world::Object &rssObject, bool inverseDirection) const;

  void calculateOccupiedRegions(const std::array<carla::geom::Location, 4u> &bounds,
                                ::ad_rss::world::OccupiedRegionVector &occupiedRegions, const carla::road::Map &map,
                                const bool drivingInRoadDirection,
                                const carla::geom::Transform &vehicleTransform) const;

  void convertAndSetLaneSegmentId(const carla::road::Lane &lane, ::ad_rss::world::LaneSegment &laneSegment) const;

  std::array<carla::geom::Location, 4u> getVehicleBounds(const carla::client::Vehicle &vehicle) const;

  void initVehicle(::ad_rss::world::Object &vehicle, ::ad_rss::world::RssDynamics &vehicleDynamics) const;

  void visualizeRssResults(::ad_rss::state::RssStateSnapshot stateSnapshot,
                           const carla::geom::Location &egoVehicleLocation,
                           const carla::geom::Transform &egoVehicleTransform, carla::client::World &world) const;
};

}  // namespace rss
}  // namespace carla
