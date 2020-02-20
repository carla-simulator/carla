// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/BoundingBox.h"
#include "carla/NonCopyable.h"
#include "carla/road/RoadTypes.h"

#include <unordered_map>
#include <vector>
#include <string>

namespace carla {
namespace road {

  class MapBuilder;

  class Junction : private MovableNonCopyable {
  public:

    struct LaneLink {
      LaneId from;
      LaneId to;
    };

    struct Connection {
      ConId id;
      RoadId incoming_road;
      RoadId connecting_road;
      std::vector<LaneLink> lane_links;

      void AddLaneLink(LaneId from, LaneId to) {
        lane_links.push_back(LaneLink{from, to});
      }

      Connection(ConId id, RoadId incoming_road, RoadId connecting_road)
        : id(id),
          incoming_road(incoming_road),
          connecting_road(connecting_road),
          lane_links() {}
    };

    Junction(const JuncId id, const std::string name)
      : _id(id),
        _name(name),
        _connections() {}

    JuncId GetId() const {
      return _id;
    }

    Connection *GetConnection(ConId id) {
      const auto search = _connections.find(id);
      if (search != _connections.end()) {
        return &search->second;
      }
      return nullptr;
    }

    std::unordered_map<ConId, Connection> &GetConnections() {
      return _connections;
    }

    std::unordered_map<ConId, Connection> GetConnections() const {
      return _connections;
    }

    carla::geom::BoundingBox GetBoundingBox() const{
      return _bounding_box;
    }

  private:

    friend MapBuilder;

    JuncId _id;

    std::string _name;

    std::unordered_map<ConId, Connection> _connections;

    std::set<ContId> _controllers;

    carla::geom::BoundingBox _bounding_box;
  };

} // road
} // carla
