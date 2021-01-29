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

#include "veins/modules/messages/TraCITrafficLightMessage_m.h"

namespace veins {
using omnetpp::cMessage;
using omnetpp::cSimpleModule;

/**
 * Base class to simplify implementation of traffic light logics
 *
 * already provides multiplexing of different message types to message handlers and a
 * special handler to be executed right before the TraCI server performs a phase switch
 */
class VEINS_API TraCITrafficLightAbstractLogic : public cSimpleModule {
public:
    TraCITrafficLightAbstractLogic();
    ~TraCITrafficLightAbstractLogic() override;

protected:
    cMessage* switchTimer;

    void initialize() override;
    void handleMessage(cMessage* msg) override;
    virtual void handleSelfMsg(cMessage* msg);
    virtual void handleApplMsg(cMessage* msg) = 0;
    virtual void handleTlIfMsg(TraCITrafficLightMessage* tlMsg) = 0;
    virtual void handlePossibleSwitch() = 0;
};

} // namespace veins
