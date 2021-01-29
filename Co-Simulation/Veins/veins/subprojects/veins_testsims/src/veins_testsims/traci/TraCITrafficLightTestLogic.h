//
// Copyright (C) 2015-2018 Dominik Buse <dbuse@mail.uni-paderborn.de>
// Copyright (C) 2018 Tobias Hardes <hardes@ccs-labs.org>
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

#include "veins/modules/world/traci/trafficLight/logics/TraCITrafficLightAbstractLogic.h"
#include "veins/base/utils/FindModule.h"
#include "veins/modules/world/traci/trafficLight/TraCITrafficLightInterface.h"

namespace veins {
class TraCITrafficLightTestLogic : public TraCITrafficLightAbstractLogic {
private:
    cMessage* changeProgramm;
    unsigned int currentIndex = 0;

public:
    using signalScheme = std::string;
    virtual void startChangingProgramAt(simtime_t t);

protected:
    virtual void initialize();
    virtual void handleMessage(cMessage* msg);
    virtual void handleApplMsg(cMessage* msg);
    virtual void handleTlIfMsg(TraCITrafficLightMessage* tlMsg);
    virtual void handlePossibleSwitch();
};

class TraCITrafficLightTestLogicAccess {
public:
    TraCITrafficLightTestLogic* get(cModule* host)
    {
        TraCITrafficLightTestLogic* traci = FindModule<TraCITrafficLightTestLogic*>::findSubModule(host);
        ASSERT(traci);
        return traci;
    };
};

} // namespace veins
