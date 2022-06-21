// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "GraphTypes.h"

#include <vector>

namespace MapGen {

  class CARLA_API CityAreaDescription : private NonCopyable
  {
  public:

    explicit CityAreaDescription(const GraphFace &Face) : _face(&Face) {}

    void Add(const GraphNode &Node) {
      _nodes.emplace_back(&Node);
    }

    const GraphFace &GetFace() const {
      return *_face;
    }

    const GraphNode &GetNodeAt(size_t i) const {
      return *_nodes[i];
    }

    size_t NodeCount() const {
      return _nodes.size();
    }

  private:

    const GraphFace *_face;

    std::vector<const GraphNode *> _nodes;
  };

} // namespace MapGen
