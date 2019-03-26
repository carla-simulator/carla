// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/road/element/WaypointInformationTypes.h"
#include "carla/road/element/RoadInfoMarkRecord.h"

namespace carla {
namespace road {
namespace element {

  WaypointInfoRoadMark::WaypointInfoRoadMark(const RoadInfoMarkRecord &info) {
    const auto &t = info.GetType();
    const auto &c = info.GetColor();
    const auto &lc = info.GetLaneChange();

    if (t == "broken") {
      type = LaneMarking::Broken;
    } else if (t == "solid") {
      type = LaneMarking::Solid;
    } else if (t == "solid solid") {
      type = LaneMarking::SolidSolid;
    } else if (t == "solid broken") {
      type = LaneMarking::SolidBroken;
    } else if (t == "broken solid") {
      type = LaneMarking::BrokenSolid;
    } else if (t == "broken broken") {
      type = LaneMarking::BrokenBroken;
    } else if (t == "botts dots") {
      type = LaneMarking::BottsDots;
    } else if (t == "grass") {
      type = LaneMarking::Grass;
    } else if (t == "curb") {
      type = LaneMarking::Curb;
    } else if (t == "none") {
      type = LaneMarking::None;
    } else {
      type = LaneMarking::Other;
    }

    if (c == "standard") {
      color = WaypointInfoRoadMark::Color::Standard;
    } else if (c == "blue") {
      color = WaypointInfoRoadMark::Color::Blue;
    } else if (c == "green") {
      color = WaypointInfoRoadMark::Color::Green;
    } else if (c == "red") {
      color = WaypointInfoRoadMark::Color::Red;
    } else if (c == "white") {
      color = WaypointInfoRoadMark::Color::White;
    } else if (c == "yellow") {
      color = WaypointInfoRoadMark::Color::Yellow;
    } else {
      color = WaypointInfoRoadMark::Color::Other;
    }

    if (lc == RoadInfoMarkRecord::LaneChange::Increase) {
      lane_change = WaypointInfoRoadMark::LaneChange::Increase;
    } else if (lc == RoadInfoMarkRecord::LaneChange::Decrease) {
      lane_change = WaypointInfoRoadMark::LaneChange::Decrease;
    } else if (lc == RoadInfoMarkRecord::LaneChange::Both) {
      lane_change = WaypointInfoRoadMark::LaneChange::Both;
    } else if (lc == RoadInfoMarkRecord::LaneChange::None) {
      lane_change = WaypointInfoRoadMark::LaneChange::None;
    }

    width = info.GetWidth();
  }

} // namespace element
} // namespace road
} // namespace carla
