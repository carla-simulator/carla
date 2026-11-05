// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Profiles.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <map>
#include <set>

namespace twinmodel {

  namespace {

    const std::map<std::string, ClassDefaults> &ClassTable() {
      static const std::map<std::string, ClassDefaults> table = {
        {"motorway",       ClassDefaults{3.5, 2, std::nullopt, 2.5,  1.0}},
        {"motorway_link",  ClassDefaults{3.5, 1, std::nullopt, 1.5,  0.75}},
        {"trunk",          ClassDefaults{3.5, 2, std::nullopt, 1.5,  std::nullopt}},
        {"trunk_link",     ClassDefaults{3.5, 1, std::nullopt, 1.0,  std::nullopt}},
        {"primary",        ClassDefaults{3.6, 2, 2.0, std::nullopt, std::nullopt}},
        {"primary_link",   ClassDefaults{3.6, 1, 2.0, std::nullopt, std::nullopt}},
        {"secondary",      ClassDefaults{3.5, 2, 2.0, std::nullopt, std::nullopt}},
        {"secondary_link", ClassDefaults{3.5, 1, 2.0, std::nullopt, std::nullopt}},
        {"tertiary",       ClassDefaults{3.5, 2, 2.0, std::nullopt, std::nullopt}},
        {"tertiary_link",  ClassDefaults{3.5, 1, 2.0, std::nullopt, std::nullopt}},
        {"unclassified",   ClassDefaults{3.5, 2, 2.0, std::nullopt, std::nullopt}},
        {"residential",    ClassDefaults{3.5, 2, 2.0, std::nullopt, std::nullopt}},
        {"living_street",  ClassDefaults{3.5, 2, 2.0, std::nullopt, std::nullopt}},
        {"pedestrian",     ClassDefaults{3.5, 1, 2.0, std::nullopt, std::nullopt}},
        {"service",        ClassDefaults{3.5, 2, std::nullopt, std::nullopt, std::nullopt}},
      };
      return table;
    }

    bool ParseInt(const std::string &text, int &out) {
      try {
        std::size_t used = 0u;
        const int value = std::stoi(text, &used);
        if (used == 0u) {
          return false;
        }
        out = value;
        return true;
      } catch (...) {
        return false;
      }
    }

    bool ParseDouble(const std::string &text, double &out) {
      try {
        std::size_t used = 0u;
        const double value = std::stod(text, &used);
        if (used == 0u) {
          return false;
        }
        out = value;
        return true;
      } catch (...) {
        return false;
      }
    }

  } // namespace

  bool IsDrivable(const std::string &highway) {
    static const std::set<std::string> drivable = {
      "motorway", "motorway_link", "trunk", "trunk_link", "primary", "primary_link",
      "secondary", "secondary_link", "tertiary", "tertiary_link", "unclassified",
      "residential", "living_street", "service"};
    return drivable.count(highway) != 0u;
  }

  const ClassDefaults *FindClassDefaults(const std::string &highway) {
    const auto &table = ClassTable();
    const auto it = table.find(highway);
    return (it == table.end()) ? nullptr : &it->second;
  }

  std::optional<double> ParseMaxSpeed(const std::string &value) {
    if (value.empty() || value == "none" || value == "signals") {
      return std::nullopt;
    }
    double number = 0.0;
    if (!ParseDouble(value, number) || number <= 0.0) {
      return std::nullopt;
    }
    const bool mph = value.find("mph") != std::string::npos;
    return mph ? number * 0.44704 : number / 3.6;
  }

  LaneProfile ResolveProfile(const std::string &highway, const Tags &tags) {
    LaneProfile profile;
    const ClassDefaults *defaults = FindClassDefaults(highway);
    const ClassDefaults fallback;
    const ClassDefaults &base = (defaults != nullptr) ? *defaults : fallback;

    profile.lane_width = base.lane_width;
    profile.sidewalk_width = base.sidewalk_width;
    profile.shoulder = base.shoulder;
    profile.shoulder_inner = base.shoulder_inner;

    const std::string *oneway = FindTag(tags, "oneway");
    profile.oneway = (oneway != nullptr) && (*oneway == "yes" || *oneway == "1" || *oneway == "-1");
    if (highway == "motorway" || highway == "motorway_link") {
      profile.oneway = true;
    }

    int total = base.lanes;
    const std::string *lanes_tag = FindTag(tags, "lanes");
    if (lanes_tag != nullptr) {
      int parsed = 0;
      if (ParseInt(*lanes_tag, parsed) && parsed > 0) {
        total = parsed;
      }
    }

    if (profile.oneway) {
      profile.forward_lanes = std::max(1, total);
      profile.backward_lanes = 0;
    } else {
      profile.forward_lanes = std::max(1, total / 2);
      profile.backward_lanes = std::max(1, total - profile.forward_lanes);
    }

    const std::string *forward_tag = FindTag(tags, "lanes:forward");
    const std::string *backward_tag = FindTag(tags, "lanes:backward");
    int parsed = 0;
    if (forward_tag != nullptr && ParseInt(*forward_tag, parsed) && parsed >= 0) {
      profile.forward_lanes = parsed;
    }
    if (backward_tag != nullptr && ParseInt(*backward_tag, parsed) && parsed >= 0) {
      profile.backward_lanes = parsed;
    }

    const std::string *width_tag = FindTag(tags, "width");
    if (width_tag != nullptr) {
      double width = 0.0;
      const int count = std::max(1, profile.forward_lanes + profile.backward_lanes);
      if (ParseDouble(*width_tag, width) && width > 0.0) {
        profile.lane_width = std::max(2.5, width / count);
      }
    }

    const std::string *maxspeed = FindTag(tags, "maxspeed");
    if (maxspeed != nullptr) {
      profile.speed_limit = ParseMaxSpeed(*maxspeed);
    }

    const std::string *sidewalk = FindTag(tags, "sidewalk");
    if (sidewalk != nullptr && (*sidewalk == "no" || *sidewalk == "none")) {
      profile.sidewalk_width = std::nullopt;
    }
    return profile;
  }

} // namespace twinmodel
