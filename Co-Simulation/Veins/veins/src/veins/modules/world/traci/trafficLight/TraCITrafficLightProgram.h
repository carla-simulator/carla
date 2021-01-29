//
// Copyright (C) 2015-2018 Dominik Buse <dbuse@mail.uni-paderborn.de>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#pragma once

#include <string>
#include <vector>
#include <map>

#include "veins/veins.h"

using omnetpp::simtime_t;

namespace veins {
class VEINS_API TraCITrafficLightProgram {
public:
    struct Phase {
        simtime_t duration;
        std::string state;
        simtime_t minDuration;
        simtime_t maxDuration;
        std::vector<int32_t> next;
        std::string name;

        bool isGreenPhase() const;
    };
    struct Logic {
        std::string id;
        int32_t currentPhase;
        std::vector<Phase> phases;
        int32_t type; // currently unused, just 0
        int32_t parameter; // currently unused, just 0
    };

    TraCITrafficLightProgram(std::string id = "");

    void addLogic(const Logic& logic);
    TraCITrafficLightProgram::Logic getLogic(const std::string& lid) const;
    bool hasLogic(const std::string& lid) const;

private:
    std::string id;
    std::map<std::string, TraCITrafficLightProgram::Logic> logics;
};

struct VEINS_API TraCITrafficLightLink {
    std::string incoming;
    std::string outgoing;
    std::string internal;
};

} // namespace veins
