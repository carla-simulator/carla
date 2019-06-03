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
  struct AccelerationRestriction;
  class LaneSegment;
  class Object;
  class OccupiedRegion;
  using OccupiedRegionVector = std::vector<::ad_rss::world::OccupiedRegion>;
  class RssDynamics;
  struct Velocity;
}  // namespace world
}  // namespace ad_rss

namespace carla {
namespace rss {

  namespace cc = carla::client;
  namespace cg = carla::geom;

  class RssCheck {
  public:

    RssCheck();

    ~RssCheck();

    bool checkObjects(cc::Timestamp const & timestamp, cc::World &world,
    carla::SharedPtr<cc::ActorList> const & vehicles,
    carla::SharedPtr<cc::Actor> const & carlaEgoVehicle,
    carla::SharedPtr<cc::Map> const & map, ::ad_rss::state::ProperResponse &response,
    ::ad_rss::world::AccelerationRestriction &restriction,
    ::ad_rss::world::Velocity &egoVelocity, bool visualizeResults = false);

    const ::ad_rss::world::RssDynamics &getEgoVehicleDynamics() const;
    void setEgoVehicleDynamics(const ::ad_rss::world::RssDynamics &egoDynamics);
    const ::ad_rss::world::RssDynamics &getOtherVehicleDynamics() const;
    void setOtherVehicleDynamics(const ::ad_rss::world::RssDynamics &otherVehicleDynamics);

  private:

    std::shared_ptr<::ad_rss::world::RssDynamics> _egoVehicleDynamics;
    std::shared_ptr<::ad_rss::world::RssDynamics> _otherVehicleDynamics;

    std::shared_ptr<::ad_rss::core::RssSituationExtraction> _rssSituationExtraction;
    std::shared_ptr<::ad_rss::core::RssSituationChecking> _rssSituationChecking;
    std::shared_ptr<::ad_rss::core::RssResponseResolving> _rssResponseResolving;

    uint64_t calculateLaneSegmentId(const carla::road::Lane &lane) const;

    void calculateLatLonVelocities(const cg::Transform &laneLocation,
    const cg::Vector3D &velocity,
    ::ad_rss::world::Object &rssObject, bool inverseDirection) const;

    void calculateOccupiedRegions(const std::array<cg::Location, 4u> &bounds,
    ::ad_rss::world::OccupiedRegionVector &occupiedRegions, const carla::road::Map &map,
    const bool drivingInRoadDirection,
    const cg::Transform &vehicleTransform) const;

    void convertAndSetLaneSegmentId(const carla::road::Lane &lane,
    ::ad_rss::world::LaneSegment &laneSegment) const;

    std::array<cg::Location, 4u> getVehicleBounds(const cc::Vehicle &vehicle) const;

    void initVehicle(::ad_rss::world::Object &vehicle) const;
    void initEgoVehicleDynamics(::ad_rss::world::RssDynamics &dynamics) const;
    void initOtherVehicleDynamics(::ad_rss::world::RssDynamics &dynamics) const;

    void visualizeRssResults(::ad_rss::state::RssStateSnapshot stateSnapshot,
    const cg::Location &egoVehicleLocation,
    const cg::Transform &egoVehicleTransform, cc::World &world) const;
  };

}  // namespace rss
}  // namespace carla
