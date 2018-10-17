// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "RoadSegment.h"
#include <string>
#include <vector>
#include <map>

namespace carla {
namespace road {
namespace element {
  class RoadInfo {
  public:

    RoadInfo() {}
    RoadInfo(double distance) : d(distance) {}
    virtual ~RoadInfo() = default;

    // distance from Road's start location
    double d = 0; // [meters]
  };

  class LaneInfo
  {
    public:
      int _id;
      double _width;

      std::string _type;
      std::vector<int> _successor;
      std::vector<int> _predecessor;

      LaneInfo() :
        _id(0), _width(0.0) {}

      LaneInfo(int id, double width, const std::string &type) :
        _id(id), _width(width), _type(type) {}
  };

  class RoadInfoLane : public RoadInfo {
  private:
    using lane_t = std::map<int, LaneInfo>;
    lane_t  _lanes;

  public:

    enum class which_lane_e : int {
      Left,
      Right,
      Both
    };

    inline void addLaneInfo(int id, double width, const std::string &type) {
      _lanes[id] = LaneInfo(id, width, type);
    }

    inline int size() {
      return (int) _lanes.size();
    }

    std::vector<int> getLanesIDs(which_lane_e whichLanes = which_lane_e::Both) {
      std::vector<int> lanesId;

      for(lane_t::iterator it = _lanes.begin(); it != _lanes.end(); ++it) {
        switch(whichLanes) {
          case which_lane_e::Both: {
            lanesId.push_back(it->first);
          } break;

          case which_lane_e::Left: {
            if(it->first > 0) {
              lanesId.push_back(it->first);
            }
          } break;

          case which_lane_e::Right: {
            if(it->first < 0) {
              lanesId.push_back(it->first);
            }
          } break;
        }
      }
    }

    const LaneInfo *getLane(int id) {
      lane_t::iterator it = _lanes.find(id);
      return it == _lanes.end() ? nullptr : &it->second;
    }
  };
} // namespace element
} // namespace road
} // namespace carla
