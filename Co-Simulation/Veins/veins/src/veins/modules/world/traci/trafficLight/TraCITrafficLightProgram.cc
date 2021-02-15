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

#include "veins/modules/world/traci/trafficLight/TraCITrafficLightProgram.h"

using veins::TraCITrafficLightProgram;

bool TraCITrafficLightProgram::Phase::isGreenPhase() const
{
    // implementation taken from SUMO MSPhaseDefinition.cc
    if (state.find_first_of("gG") == std::string::npos) {
        return false;
    }
    if (state.find_first_of("yY") != std::string::npos) {
        return false;
    }
    return true;
}

TraCITrafficLightProgram::TraCITrafficLightProgram(std::string id)
    : id(id)
    , logics()
{
}

void TraCITrafficLightProgram::addLogic(const Logic& logic)
{
    logics[logic.id] = logic;
}

TraCITrafficLightProgram::Logic TraCITrafficLightProgram::getLogic(const std::string& lid) const
{
    return logics.at(lid);
}

bool TraCITrafficLightProgram::hasLogic(const std::string& lid) const
{
    return logics.find(lid) != logics.end();
}
