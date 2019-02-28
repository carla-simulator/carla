// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/road/element/RoadInfo.h"

#include <string>

namespace carla {
namespace road {
namespace element {

  // Each lane within a road cross section can be provided with several road
  // markentries. The road mark information defines the style of the line at the
  // lane’s outer border. For left lanes, this is the left border, for right
  // lanes the right one. The style of the line separating left and right lanes
  // is determined by the road mark entry for lane zero (i.e. the center lane)
  class RoadInfoMarkRecord : public RoadInfo {
  public:

    void AcceptVisitor(RoadInfoVisitor &v) final {
      v.Visit(*this);
    }

    /// Can be used as flags
    enum class LaneChange : uint8_t {
      None     = 0x00, //00
      Increase = 0x01, //01
      Decrease = 0x02, //10
      Both     = 0x03  //11
    };

    RoadInfoMarkRecord(double d, int lane_id)
      : RoadInfo(d),
        _lane_id(lane_id),
        _type(""),
        _weight(""),
        _color("white"),
        _material("standard"),
        _width(0.15),
        _lane_change(LaneChange::None),
        _height(0.0) {}

    RoadInfoMarkRecord(
        double d,
        int lane_id,
        std::string type,
        std::string weight,
        std::string color,
        std::string material,
        double width,
        LaneChange lane_change,
        double height)
      : RoadInfo(d),
        _lane_id(lane_id),
        _type(type),
        _weight(weight),
        _color(color),
        _material(material),
        _width(width),
        _lane_change(lane_change),
        _height(height) {}

    int GetLaneId() const {
      return _lane_id;
    }

    const std::string &GetType() const {
      return _type;
    }

    const std::string &GetWeight() const {
      return _weight;
    }

    const std::string &GetColor() const {
      return _color;
    }

    const std::string &GetMaterial() const {
      return _material;
    }

    double GetWidth() const {
      return _width;
    }

    LaneChange GetLaneChange() const {
      return _lane_change;
    }

    double GetHeight() const {
      return _height;
    }

  private:

    using signed_id = int;

    signed_id _lane_id = 0;

    std::string _type;       // Type of the road mark
    std::string _weight;     // Weight of the road mark
    std::string _color;      // Color of the road mark
    std::string _material;   // Material of the road mark (identifiers to be
                             // defined, use "standard" for the moment
    double _width;           // Width of the road mark –optional
    LaneChange _lane_change; // Allow a lane change in the indicated direction
                             // taking into account that lanes are numbered in
                             // ascending order from right to left. If the
                             // attributeis missing, “both” is assumedto be
                             // valid.
    double _height;          // Physical distance of top edge of road mark from
                             // reference plane of the lane
  };

} // namespace element
} // namespace road
} // namespace carla
