// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/road/element/LaneMarking.h"

#include "carla/Exception.h"
#include "carla/StringUtil.h"
#include "carla/road/element/RoadInfoMarkRecord.h"

namespace carla {
namespace road {
namespace element {

  static LaneMarking::Type GetType(std::string str) {
    StringUtil::ToLower(str);
    if (str == "broken") {
      return LaneMarking::Type::Broken;
    } else if (str == "solid") {
      return LaneMarking::Type::Solid;
    } else if (str == "solid solid") {
      return LaneMarking::Type::SolidSolid;
    } else if (str == "solid broken") {
      return LaneMarking::Type::SolidBroken;
    } else if (str == "broken solid") {
      return LaneMarking::Type::BrokenSolid;
    } else if (str == "broken broken") {
      return LaneMarking::Type::BrokenBroken;
    } else if (str == "botts dots") {
      return LaneMarking::Type::BottsDots;
    } else if (str == "grass") {
      return LaneMarking::Type::Grass;
    } else if (str == "curb") {
      return LaneMarking::Type::Curb;
    } else if (str == "none") {
      return LaneMarking::Type::None;
    } else {
      return LaneMarking::Type::Other;
    }
  }

  static LaneMarking::Color GetColor(std::string str) {
    StringUtil::ToLower(str);
    if (str == "standard") {
      return LaneMarking::Color::Standard;
    } else if (str == "blue") {
      return LaneMarking::Color::Blue;
    } else if (str == "green") {
      return LaneMarking::Color::Green;
    } else if (str == "red") {
      return LaneMarking::Color::Red;
    } else if (str == "white") {
      return LaneMarking::Color::White;
    } else if (str == "yellow") {
      return LaneMarking::Color::Yellow;
    } else {
      return LaneMarking::Color::Other;
    }
  }

  static LaneMarking::LaneChange GetLaneChange(RoadInfoMarkRecord::LaneChange lane_change) {
    switch (lane_change) {
      case RoadInfoMarkRecord::LaneChange::Increase:
        return LaneMarking::LaneChange::Right;
      case RoadInfoMarkRecord::LaneChange::Decrease:
        return LaneMarking::LaneChange::Left;
      case RoadInfoMarkRecord::LaneChange::Both:
        return LaneMarking::LaneChange::Both;
      default:
        return LaneMarking::LaneChange::None;
    }
  }

  LaneMarking::LaneMarking(const RoadInfoMarkRecord &info)
    : type(GetType(info.GetType())),
      color(GetColor(info.GetColor())),
      lane_change(GetLaneChange(info.GetLaneChange())),
      width(info.GetWidth()) {}

} // namespace element
} // namespace road
} // namespace carla
