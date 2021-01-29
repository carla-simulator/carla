//
// Copyright (C) 2006-2017 Christoph Sommer <sommer@ccs-labs.org>
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

#include "veins_inet/veins_inet.h"

#include "veins/modules/mobility/traci/TraCIScenarioManagerLaunchd.h"

namespace veins {

/**
 * @brief
 * Creates and manages network nodes corresponding to cars.
 *
 * See the Veins website <a href="http://veins.car2x.org/"> for a tutorial, documentation, and publications </a>.
 *
 * @author Christoph Sommer
 *
 */
class VEINS_INET_API VeinsInetManager : public TraCIScenarioManagerLaunchd {
public:
    virtual ~VeinsInetManager();
    virtual void preInitializeModule(cModule* mod, const std::string& nodeId, const Coord& position, const std::string& road_id, double speed, Heading heading, VehicleSignalSet signals) override;
    virtual void updateModulePosition(cModule* mod, const Coord& p, const std::string& edge, double speed, Heading heading, VehicleSignalSet signals) override;

protected:
};

class VEINS_INET_API VeinsInetManagerAccess {
public:
    VeinsInetManager* get()
    {
        return FindModule<VeinsInetManager*>::findGlobalModule();
    };
};

} // namespace veins
