//
// Copyright (C) 2007 Technische Universitaet Berlin (TUB), Germany, Telecommunication Networks Group
// Copyright (C) 2007 Technische Universiteit Delft (TUD), Netherlands
// Copyright (C) 2007 Universitaet Paderborn (UPB), Germany
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

/*
 * BatteryAccess.cc
 *
 *  Created on: Aug 26, 2009
 *      Author: Karl Wessel
 */

#include "veins/base/modules/BatteryAccess.h"

#include "veins/base/utils/FindModule.h"

using veins::BatteryAccess;

BatteryAccess::BatteryAccess()
    : BaseModule()
    , battery(nullptr)
{
}

BatteryAccess::BatteryAccess(unsigned stacksize)
    : BaseModule(stacksize)
    , battery(nullptr)
{
}

void BatteryAccess::registerWithBattery(const std::string& name, int numAccounts)
{
    battery = FindModule<BaseBattery*>::findSubModule(findHost());

    if (!battery) {
        throw cRuntimeError("No battery module defined!");
    }
    else {
        deviceID = battery->registerDevice(name, numAccounts);
    }
}

void BatteryAccess::draw(DrawAmount& amount, int account)
{
    if (!battery) return;

    battery->draw(deviceID, amount, account);
}

void BatteryAccess::drawCurrent(double amount, int account)
{
    if (!battery) return;

    DrawAmount val(DrawAmount::CURRENT, amount);
    battery->draw(deviceID, val, account);
}

void BatteryAccess::drawEnergy(double amount, int account)
{
    if (!battery) return;

    DrawAmount val(DrawAmount::ENERGY, amount);
    battery->draw(deviceID, val, account);
}
