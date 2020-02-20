// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/NonCopyable.h"
#include "carla/geom/Transform.h"
#include "carla/road/MapData.h"
#include "carla/geom/Mesh.h"
#include "carla/road/RoadTypes.h"
#include "carla/road/element/LaneMarking.h"
#include "carla/road/element/RoadInfoMarkRecord.h"
#include "carla/road/element/Waypoint.h"
#include "carla/geom/Rtree.h"

#include <boost/optional.hpp>

#include <vector>

namespace carla {
namespace road {

  class Map : private MovableNonCopyable {
  public:

    using Waypoint = element::Waypoint;

    /// ========================================================================
    /// -- Constructor ---------------------------------------------------------
    /// ========================================================================

    Map(MapData m) : _data(std::move(m)) {
      CreateRtree();
    }

    /// ========================================================================
    /// -- Georeference --------------------------------------------------------
    /// ========================================================================

    const geom::GeoLocation &GetGeoReference() const {
      return _data.GetGeoReference();
    }

    /// ========================================================================
    /// -- Geometry ------------------------------------------------------------
    /// ========================================================================

    boost::optional<element::Waypoint> GetClosestWaypointOnRoad(
        const geom::Location &location,
        uint32_t lane_type = static_cast<uint32_t>(Lane::LaneType::Driving)) const;

    boost::optional<element::Waypoint> GetWaypoint(
        const geom::Location &location,
        uint32_t lane_type = static_cast<uint32_t>(Lane::LaneType::Driving)) const;

    boost::optional<element::Waypoint> GetWaypoint(
        RoadId road_id,
        LaneId lane_id,
        float s) const;

    geom::Transform ComputeTransform(Waypoint waypoint) const;

    /// ========================================================================
    /// -- Road information ----------------------------------------------------
    /// ========================================================================

    const Lane &GetLane(Waypoint waypoint) const;

    Lane::LaneType GetLaneType(Waypoint waypoint) const;

    double GetLaneWidth(Waypoint waypoint) const;

    JuncId GetJunctionId(RoadId road_id) const;

    bool IsJunction(RoadId road_id) const;

    std::pair<const element::RoadInfoMarkRecord *, const element::RoadInfoMarkRecord *>
        GetMarkRecord(Waypoint waypoint) const;

    std::vector<element::LaneMarking> CalculateCrossedLanes(
        const geom::Location &origin,
        const geom::Location &destination) const;

    std::vector<geom::Location> GetAllCrosswalkZones() const;

    /// Data structure for the signal search
    struct SignalSearchData {
      const element::RoadInfoSignal *signal;
      Waypoint waypoint;
      double accumulated_s = 0;
    };

    /// Searches signals from an initial waypoint until the defined distance.
    std::vector<SignalSearchData> GetSignalsInDistance(
        Waypoint waypoint, double distance, bool stop_at_junction = false) const;

    /// ========================================================================
    /// -- Waypoint generation -------------------------------------------------
    /// ========================================================================

    /// Return the list of waypoints placed at the entrance of each drivable
    /// successor lane; i.e., the list of each waypoint in the next road segment
    /// that a vehicle could drive from @a waypoint.
    std::vector<Waypoint> GetSuccessors(Waypoint waypoint) const;
    std::vector<Waypoint> GetPredecessors(Waypoint waypoint) const;

    /// Return the list of waypoints at @a distance such that a vehicle at @a
    /// waypoint could drive to.
    std::vector<Waypoint> GetNext(Waypoint waypoint, double distance) const;
    /// Return the list of waypoints at @a distance in the reversed direction
    /// that a vehicle at @a waypoint could drive to.
    std::vector<Waypoint> GetPrevious(Waypoint waypoint, double distance) const;

    /// Return a waypoint at the lane of @a waypoint's right lane.
    boost::optional<Waypoint> GetRight(Waypoint waypoint) const;

    /// Return a waypoint at the lane of @a waypoint's left lane.
    boost::optional<Waypoint> GetLeft(Waypoint waypoint) const;

    /// Generate all the waypoints in @a map separated by @a approx_distance.
    std::vector<Waypoint> GenerateWaypoints(double approx_distance) const;

    /// Generate waypoints on each @a lane at the start of each @a road
    std::vector<Waypoint> GenerateWaypointsOnRoadEntries(Lane::LaneType lane_type = Lane::LaneType::Driving) const;

    /// Generate the minimum set of waypoints that define the topology of @a
    /// map. The waypoints are placed at the entrance of each lane.
    std::vector<std::pair<Waypoint, Waypoint>> GenerateTopology() const;

    /// Generate waypoints of the junction
    std::vector<std::pair<Waypoint, Waypoint>> GetJunctionWaypoints(JuncId id, Lane::LaneType lane_type) const;

    Junction* GetJunction(JuncId id);

    const Junction* GetJunction(JuncId id) const;

    /// Buids a mesh based on the OpenDRIVE
    geom::Mesh GenerateGeometry(double distance) const;

    const std::unordered_map<SignId, std::unique_ptr<Signal>>& GetSignals() const {
      return _data.GetSignals();
    }

    const std::unordered_map<ContId, std::unique_ptr<Controller>>& GetControllers() const {
      return _data.GetControllers();
    }

#ifdef LIBCARLA_WITH_GTEST
    MapData &GetMap() {
      return _data;
    }
#endif // LIBCARLA_WITH_GTEST

private:

    friend MapBuilder;
    MapData _data;

    using Rtree = geom::SegmentCloudRtree<Waypoint>;
    Rtree _rtree;

    void CreateRtree();

    /// Helper Functions for constructing the rtree element list
    void AddElementToRtree(
        std::vector<Rtree::TreeElement> &rtree_elements,
        geom::Transform &current_transform,
        geom::Transform &next_transform,
        Waypoint &current_waypoint,
        Waypoint &next_waypoint);

    void AddElementToRtreeAndUpdateTransforms(
        std::vector<Rtree::TreeElement> &rtree_elements,
        geom::Transform &current_transform,
        Waypoint &current_waypoint,
        Waypoint &next_waypoint);
  };

} // namespace road
} // namespace carla
