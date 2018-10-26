// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/road/element/RoadInfoVisitor.h"

#include <string>
#include <map>

namespace carla {
namespace road {
namespace element {

  class RoadInfo {
  public:

    virtual ~RoadInfo() = default;

    virtual void AcceptVisitor(RoadInfoVisitor &) = 0;

    // distance from Road's start location
    double d; // [meters]

  protected:

    RoadInfo(double distance = 0) : d(distance) {}
  };

  class RoadGeneralInfo : public RoadInfo {
    private:
      int _junction_id = -1;
      std::vector<std::pair<double, double>> _lanes_offset;

    public:
      void AcceptVisitor(RoadInfoVisitor &v) final {
        v.Visit(*this);
      }

      void SetJunctionId(int junctionId) { _junction_id = junctionId; }
      int GetJunctionId() const { return _junction_id; }

      void SetLanesOffset(double offset, double laneOffset) { _lanes_offset.emplace_back(std::pair<double, double>(offset, laneOffset)); }
      std::vector<std::pair<double, double>> GetLanesOffset() const { return _lanes_offset; }
  };

  class RoadInfoVelocity : public RoadInfo {
  public:

    void AcceptVisitor(RoadInfoVisitor &v) override final {
      v.Visit(*this);
    }

    RoadInfoVelocity(double vel) : velocity(vel) {}
    RoadInfoVelocity(double d, double vel) : RoadInfo(d),
                                             velocity(vel) {}

    double velocity = 0;
  };

  class LaneInfo {
  public:

    int _id;
    double _width;

    std::string _type;
    std::vector<int> _successor;
    std::vector<int> _predecessor;

    LaneInfo()
      : _id(0),
        _width(0.0) {}

    LaneInfo(int id, double width, const std::string &type)
      : _id(id),
        _width(width),
        _type(type) {}
  };

  class RoadInfoLane : public RoadInfo {
  private:

    using lane_t = std::map<int, LaneInfo>;
    lane_t _lanes;

  public:

    void AcceptVisitor(RoadInfoVisitor &v) final {
      v.Visit(*this);
    }

    enum class which_lane_e : int {
      Left,
      Right,
      Both
    };

    void addLaneInfo(int id, double width, const std::string &type) {
      _lanes[id] = LaneInfo(id, width, type);
    }

    int size() const {
      return (int) _lanes.size();
    }

    std::vector<int> getLanesIDs(which_lane_e whichLanes = which_lane_e::Both) const {
      std::vector<int> lanes_id;

      for (lane_t::const_iterator it = _lanes.begin(); it != _lanes.end(); ++it) {
        switch (whichLanes) {
          case which_lane_e::Both: {
            lanes_id.emplace_back(it->first);
          } break;

          case which_lane_e::Left: {
            if (it->first > 0) {
              lanes_id.emplace_back(it->first);
            }
          } break;

          case which_lane_e::Right: {
            if (it->first < 0) {
              lanes_id.emplace_back(it->first);
            }
          } break;
        }
      }

      std::reverse(lanes_id.begin(), lanes_id.end());
      return lanes_id;
    }

    const LaneInfo *getLane(int id) const {
      lane_t::const_iterator it = _lanes.find(id);
      return it == _lanes.end() ? nullptr : &it->second;
    }
  };

} // namespace element
} // namespace road
} // namespace carla
