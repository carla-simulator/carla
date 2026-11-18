// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "OsmIngest.h"

#include <limits>
#include <stdexcept>

#include <third-party/pugixml/pugixml.hpp>

namespace twinmodel {

  namespace {

    Tags ReadTags(const pugi::xml_node &node) {
      Tags tags;
      for (pugi::xml_node tag = node.child("tag"); tag; tag = tag.next_sibling("tag")) {
        const char *key = tag.attribute("k").value();
        const char *value = tag.attribute("v").value();
        if (key != nullptr && key[0] != '\0') {
          tags.emplace(key, value != nullptr ? value : "");
        }
      }
      return tags;
    }

  } // namespace

  BoundingBox OsmData::NodeExtent() const {
    BoundingBox extent;
    extent.south = std::numeric_limits<double>::max();
    extent.west = std::numeric_limits<double>::max();
    extent.north = std::numeric_limits<double>::lowest();
    extent.east = std::numeric_limits<double>::lowest();
    for (const auto &entry : nodes) {
      extent.south = std::min(extent.south, entry.second.lat);
      extent.north = std::max(extent.north, entry.second.lat);
      extent.west = std::min(extent.west, entry.second.lon);
      extent.east = std::max(extent.east, entry.second.lon);
    }
    return extent;
  }

  OsmData ParseOsmXml(const std::string &xml) {
    pugi::xml_document document;
    const pugi::xml_parse_result result = document.load_buffer(xml.data(), xml.size());
    if (!result) {
      throw std::runtime_error(std::string("osm: could not parse the document: ") +
                               result.description());
    }
    const pugi::xml_node root = document.child("osm");
    if (!root) {
      throw std::runtime_error("osm: the document has no <osm> element");
    }

    OsmData data;
    for (pugi::xml_node node = root.child("node"); node; node = node.next_sibling("node")) {
      OsmNode parsed;
      parsed.id = node.attribute("id").as_llong();
      parsed.lat = node.attribute("lat").as_double();
      parsed.lon = node.attribute("lon").as_double();
      parsed.tags = ReadTags(node);
      data.nodes.emplace(parsed.id, std::move(parsed));
    }
    if (data.nodes.empty()) {
      throw std::runtime_error("osm: the document carries no nodes");
    }

    for (pugi::xml_node way = root.child("way"); way; way = way.next_sibling("way")) {
      OsmWay parsed;
      parsed.id = way.attribute("id").as_llong();
      for (pugi::xml_node ref = way.child("nd"); ref; ref = ref.next_sibling("nd")) {
        const long long id = ref.attribute("ref").as_llong();
        if (data.nodes.count(id) != 0u) {
          parsed.nodes.push_back(id);
        }
      }
      parsed.tags = ReadTags(way);
      if (parsed.nodes.size() >= 2u) {
        data.ways.push_back(std::move(parsed));
      }
    }

    for (pugi::xml_node rel = root.child("relation"); rel; rel = rel.next_sibling("relation")) {
      OsmRelation parsed;
      parsed.id = rel.attribute("id").as_llong();
      for (pugi::xml_node member = rel.child("member"); member;
           member = member.next_sibling("member")) {
        OsmMember entry;
        entry.type = member.attribute("type").value();
        entry.ref = member.attribute("ref").as_llong();
        entry.role = member.attribute("role").value();
        parsed.members.push_back(std::move(entry));
      }
      parsed.tags = ReadTags(rel);
      data.relations.push_back(std::move(parsed));
    }

    const pugi::xml_node bounds = root.child("bounds");
    if (bounds) {
      BoundingBox box;
      box.south = bounds.attribute("minlat").as_double();
      box.west = bounds.attribute("minlon").as_double();
      box.north = bounds.attribute("maxlat").as_double();
      box.east = bounds.attribute("maxlon").as_double();
      data.bbox = box;
    }
    return data;
  }

} // namespace twinmodel
