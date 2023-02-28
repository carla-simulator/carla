// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/road/element/RoadInfo.h"
#include "carla/road/element/RoadInfoMarkTypeLine.h"
#include <string>
#include <vector>
#include <memory>

namespace carla {
namespace road {
namespace element {

  /// Each lane within a road cross section can be provided with several road
  /// markentries. The road mark information defines the style of the line at
  /// the lane’s outer border. For left lanes, this is the left border, for
  /// right lanes the right one. The style of the line separating left and right
  /// lanes is determined by the road mark entry for lane zero (i.e. the center
  /// lane)
  class RoadInfoMarkRecord final : public RoadInfo {
  public:

    /// Can be used as flags
    enum class LaneChange : uint8_t {
      None     = 0x00, //00
      Increase = 0x01, //01
      Decrease = 0x02, //10
      Both     = 0x03  //11
    };

    RoadInfoMarkRecord(
        double s,
        int road_mark_id)
      : RoadInfo(s),
        _road_mark_id(road_mark_id),
        _type(""),
        _weight(""),
        _color("white"),
        _material("standard"),
        _width(0.15),
        _lane_change(LaneChange::None),
        _height(0.0),
        _type_name(""),
        _type_width(0.0) {}

    RoadInfoMarkRecord(
        double s,
        int road_mark_id,
        std::string type,
        std::string weight,
        std::string color,
        std::string material,
        double width,
        LaneChange lane_change,
        double height,
        std::string type_name,
        double type_width)
      : RoadInfo(s),
        _road_mark_id(road_mark_id),
        _type(type),
        _weight(weight),
        _color(color),
        _material(material),
        _width(width),
        _lane_change(lane_change),
        _height(height),
        _type_name(type_name),
        _type_width(type_width) {}

    void AcceptVisitor(RoadInfoVisitor &v) final {
      v.Visit(*this);
    }

    /// Unique identifer for the road mark.
    int GetRoadMarkId() const {
      return _road_mark_id;
    }

    /// Type of the road mark.
    const std::string &GetType() const {
      return _type;
    }

    /// Weight of the road mark.
    const std::string &GetWeight() const {
      return _weight;
    }

    /// Color of the road mark.
    const std::string &GetColor() const {
      return _color;
    }

    /// Material of the road mark (identifiers to be defined, use "standard" for
    /// the moment.
    const std::string &GetMaterial() const {
      return _material;
    }

    /// Width of the road mark –optional.
    double GetWidth() const {
      return _width;
    }

    /// Allow a lane change in the indicated direction taking into account that
    /// lanes are numbered in ascending order from right to left. If the
    /// attribute is missing, “both” is assumed to be valid.
    LaneChange GetLaneChange() const {
      return _lane_change;
    }

    /// Physical distance of top edge of road mark from reference plane of the
    /// lane.
    double GetHeight() const {
      return _height;
    }

    /// Name of the road mark type if it has one.
    const std::string &GetTypeName() const {
      return _type_name;
    }

    /// Width of the road mark type if it has one.
    double GetTypeWidth() const {
      return _type_width;
    }

    std::vector<std::unique_ptr<RoadInfoMarkTypeLine>> &GetLines() {
      return _lines;
    }

  private:

    const int _road_mark_id;

    const std::string _type;

    const std::string _weight;

    const std::string _color;

    const std::string _material;

    const double _width;

    const LaneChange _lane_change;

    const double _height;

    const std::string _type_name;

    const double _type_width;

    std::vector<std::unique_ptr<RoadInfoMarkTypeLine>> _lines;
  };

} // namespace element
} // namespace road
} // namespace carla
