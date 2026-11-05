// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <map>
#include <optional>
#include <string>
#include <vector>

#include "Model.h"

namespace twinmodel {

  struct OsmNode {
    long long id = 0;
    double lat = 0.0;
    double lon = 0.0;
    Tags tags;
  };

  struct OsmWay {
    long long id = 0;
    std::vector<long long> nodes;
    Tags tags;
  };

  struct OsmMember {
    std::string type;
    long long ref = 0;
    std::string role;
  };

  struct OsmRelation {
    long long id = 0;
    std::vector<OsmMember> members;
    Tags tags;
  };

  struct BoundingBox {
    double south = 0.0;
    double west = 0.0;
    double north = 0.0;
    double east = 0.0;
  };

  struct OsmData {
    std::map<long long, OsmNode> nodes;
    std::vector<OsmWay> ways;
    std::vector<OsmRelation> relations;
    std::optional<BoundingBox> bbox;

    /// Extent of the nodes themselves, used when the file carries no <bounds>
    /// or the bounds are smaller than the ways crossing them.
    BoundingBox NodeExtent() const;
  };

  /// Parse an OpenStreetMap XML document. Throws std::runtime_error when the
  /// document cannot be parsed or carries no nodes.
  OsmData ParseOsmXml(const std::string &xml);

} // namespace twinmodel
