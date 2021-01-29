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

#include "veins/veins.h"

#include "veins/base/utils/FindModule.h"
#include "veins/modules/world/traci/trafficLight/logics/TraCITrafficLightAbstractLogic.h"
#include "veins/modules/world/traci/trafficLight/TraCITrafficLightInterface.h"

namespace veins {
class VEINS_API TraCITrafficLightSimpleLogic : public TraCITrafficLightAbstractLogic {

public:
    using signalScheme = std::string;

protected:
    void handleApplMsg(cMessage* msg) override;
    void handleTlIfMsg(TraCITrafficLightMessage* tlMsg) override;
    void handlePossibleSwitch() override;
};

class VEINS_API TraCITrafficLightSimpleLogicAccess {
public:
    TraCITrafficLightSimpleLogic* get(cModule* host)
    {
        TraCITrafficLightSimpleLogic* traci = FindModule<TraCITrafficLightSimpleLogic*>::findSubModule(host);
        ASSERT(traci);
        return traci;
    };
};

} // namespace veins
