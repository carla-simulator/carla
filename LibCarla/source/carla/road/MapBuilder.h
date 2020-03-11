// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/road/Map.h"
#include "carla/road/element/RoadInfoCrosswalk.h"
#include "carla/road/element/RoadInfoSignal.h"

#include <boost/optional.hpp>

#include <map>

namespace carla {
namespace road {

  class MapBuilder {
  public:

    boost::optional<Map> Build();

    // called from road parser
    carla::road::Road *AddRoad(
        const RoadId road_id,
        const std::string name,
        const double length,
        const JuncId junction_id,
        const RoadId predecessor,
        const RoadId successor);

    carla::road::LaneSection *AddRoadSection(
        carla::road::Road *road,
        const SectionId id,
        const double s);

    carla::road::Lane *AddRoadSectionLane(
        carla::road::LaneSection *section,
        const LaneId lane_id,
        const uint32_t lane_type,
        const bool lane_level,
        const LaneId predecessor,
        const LaneId successor);

    // called from geometry parser
    void AddRoadGeometryLine(
        carla::road::Road *road,
        const double s,
        const double x,
        const double y,
        const double hdg,
        const double length);

    void AddRoadGeometryArc(
        carla::road::Road *road,
        const double s,
        const double x,
        const double y,
        const double hdg,
        const double length,
        const double curvature);

    void AddRoadGeometrySpiral(
        carla::road::Road *road,
        const double s,
        const double x,
        const double y,
        const double hdg,
        const double length,
        const double curvStart,
        const double curvEnd);

    void AddRoadGeometryPoly3(
        carla::road::Road *road,
        const double s,
        const double x,
        const double y,
        const double hdg,
        const double length,
        const double a,
        const double b,
        const double c,
        const double d);

    void AddRoadGeometryParamPoly3(
        carla::road::Road *road,
        const double s,
        const double x,
        const double y,
        const double hdg,
        const double length,
        const double aU,
        const double bU,
        const double cU,
        const double dU,
        const double aV,
        const double bV,
        const double cV,
        const double dV,
        const std::string p_range);

    // called from profiles parser
    void AddRoadElevationProfile(
        Road *road,
        const double s,
        const double a,
        const double b,
        const double c,
        const double d);

    void AddRoadObjectCrosswalk(
        Road *road,
        const std::string name,
        const double s,
        const double t,
        const double zOffset,
        const double hdg,
        const double pitch,
        const double roll,
        const std::string orientation,
        const double width,
        const double length,
        const std::vector<road::element::CrosswalkPoint> points);

    // void AddRoadLateralSuperElevation(
    //     Road* road,
    //     const double s,
    //     const double a,
    //     const double b,
    //     const double c,
    //     const double d);

    // void AddRoadLateralCrossfall(
    //     Road* road,
    //     const double s,
    //     const double a,
    //     const double b,
    //     const double c,
    //     const double d,
    //     const std::string side);

    // void AddRoadLateralShape(
    //     Road* road,
    //     const double s,
    //     const double a,
    //     const double b,
    //     const double c,
    //     const double d,
    //     const double t);

    // Signal methods
    element::RoadInfoSignal* AddSignal(
        Road* road,
        const SignId signal_id,
        const double s,
        const double t,
        const std::string name,
        const std::string dynamic,
        const std::string orientation,
        const double zOffset,
        const std::string country,
        const std::string type,
        const std::string subtype,
        const double value,
        const std::string unit,
        const double height,
        const double width,
        const std::string text,
        const double hOffset,
        const double pitch,
        const double roll);

    element::RoadInfoSignal* AddSignalReference(
        Road* road,
        const SignId signal_id,
        const double s_position,
        const double t_position,
        const std::string signal_reference_orientation);

    void AddValidityToSignalReference(
        element::RoadInfoSignal* signal_reference,
        const LaneId from_lane,
        const LaneId to_lane);

    void AddDependencyToSignal(
        const SignId signal_id,
        const std::string dependency_id,
        const std::string dependency_type);

    // called from junction parser
    void AddJunction(
        const JuncId id,
        const std::string name);

    void AddConnection(
        const JuncId junction_id,
        const ConId connection_id,
        const RoadId incoming_road,
        const RoadId connecting_road);

    void AddLaneLink(
        const JuncId junction_id,
        const ConId connection_id,
        const LaneId from,
        const LaneId to);

    void AddJunctionController(
        const JuncId junction_id,
        std::set<ContId>&& controllers);

    void AddRoadSection(
        const RoadId road_id,
        const SectionId section_index,
        const double s,
        const double a,
        const double b,
        const double c,
        const double d);

    void SetRoadLaneLink(
        const RoadId road_id,
        const SectionId section_index,
        const LaneId lane_id,
        const Lane::LaneType lane_type,
        const bool lane_level,
        const LaneId predecessor,
        const LaneId successor);

    // called from lane parser
    void CreateLaneAccess(
        Lane *lane,
        const double s,
        const std::string restriction);

    void CreateLaneBorder(
        Lane *lane,
        const double s,
        const double a,
        const double b,
        const double c,
        const double d);

    void CreateLaneHeight(
        Lane *lane,
        const double s,
        const double inner,
        const double outer);

    void CreateLaneMaterial(
        Lane *lane,
        const double s,
        const std::string surface,
        const double friction,
        const double roughness);

    void CreateSectionOffset(
        Road *road,
        const double s,
        const double a,
        const double b,
        const double c,
        const double d);

    void CreateLaneRule(
        Lane *lane,
        const double s,
        const std::string value);

    void CreateLaneVisibility(
        Lane *lane,
        const double s,
        const double forward,
        const double back,
        const double left,
        const double right);

    void CreateLaneWidth(
        Lane *lane,
        const double s,
        const double a,
        const double b,
        const double c,
        const double d);

    void CreateRoadMark(
        Lane *lane,
        const int road_mark_id,
        const double s,
        const std::string type,
        const std::string weight,
        const std::string color,
        const std::string material,
        const double width,
        const std::string lane_change,
        const double height,
        const std::string type_name,
        const double type_width);

    void CreateRoadMarkTypeLine(
        Lane *lane,
        const int road_mark_id,
        const double length,
        const double space,
        const double tOffset,
        const double s,
        const std::string rule,
        const double width);

    void CreateRoadSpeed(
        Road *road,
        const double s,
        const std::string type,
        const double max,
        const std::string unit);

    void CreateLaneSpeed(
        Lane *lane,
        const double s,
        const double max,
        const std::string unit);

    Road *GetRoad(
        const RoadId road_id);

    Lane *GetLane(
        const RoadId road_id,
        const LaneId lane_id,
        const double s);

    // Called from ControllerParser
    void CreateController(
        const ContId controller_id,
        const std::string controller_name,
        const uint32_t controller_sequence,
        const std::set<road::SignId>&& signals
        );



    void SetGeoReference(const geom::GeoLocation &geo_reference) {
      _map_data._geo_reference = geo_reference;
    }

  private:

    MapData _map_data;

    /// Create the pointers between RoadSegments based on the ids.
    void CreatePointersBetweenRoadSegments();

    /// Create the bounding boxes of each junction
    void CreateJunctionBoundingBoxes(Map &map);

    /// Solves the signal references in the road
    void SolveSignalReferencesAndTransforms();

    /// Solve the references between Controllers and Juntions
    void SolveControllerAndJuntionReferences();

    /// Return the pointer to a lane object.
    Lane *GetEdgeLanePointer(RoadId road_id, bool from_start, LaneId lane_id);

    /// Return a list of pointers to all lanes from a lane (using road and
    /// junction info).
    std::vector<Lane *> GetLaneNext(
        RoadId road_id,
        SectionId section_id,
        LaneId lane_id);

    std::vector<std::pair<RoadId, LaneId>> GetJunctionLanes(
        JuncId junction_id,
        RoadId road_id,
        LaneId lane_id);

    /// Map to temporary store all the road and lane infos until the map is
    /// built, so they can be added all together.
    std::unordered_map<Road *, std::vector<std::unique_ptr<element::RoadInfo>>>
        _temp_road_info_container;

    std::unordered_map<Lane *, std::vector<std::unique_ptr<element::RoadInfo>>>
        _temp_lane_info_container;

    std::unordered_map<SignId, std::unique_ptr<Signal>>
        _temp_signal_container;

    std::vector<element::RoadInfoSignal*> _temp_signal_reference_container;

  };

} // namespace road
} // namespace carla
