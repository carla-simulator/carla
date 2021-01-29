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
 * BatteryAccess.h
 *
 *  Created on: Aug 26, 2009
 *      Author: karl
 */

#pragma once

#include "veins/veins.h"

#include "veins/base/modules/BaseModule.h"
#include "veins/base/modules/BaseBattery.h"

namespace veins {

/**
 * @brief Extends BaseModule by several methods which provide
 * access to the battery module.
 *
 * @ingroup power
 * @ingroup baseModules
 * @author Karl Wessel
 */
class VEINS_API BatteryAccess : public BaseModule {
protected:
    /** @brief Stores pointer to the battery module. */
    BaseBattery* battery;

    /** @brief This devices id for the battery module. */
    int deviceID;

protected:
    /**
     * @brief Registers this module as a device with the battery module.
     *
     * If no battery module is available than nothing happens.
     */
    void registerWithBattery(const std::string& name, int numAccounts);

    /**
     * @brief Draws the amount defined by the passed DrawAmount from the
     * battery on account of the passed account.
     *
     * If no battery module is available than nothing happens.
     */
    void draw(DrawAmount& amount, int account);

    /**
     * @brief Draws the passed amount of current (in mA) over time from the
     * battery on account of the passed account.
     *
     * If no battery module is available than nothing happens.
     */
    void drawCurrent(double amount, int account);

    /**
     * @brief Draws the passed amount of energy (in mWs) from the
     * battery on account of the passed account.
     *
     * If no battery module is available than nothing happens.
     */
    void drawEnergy(double amount, int account);

public:
    BatteryAccess();
    BatteryAccess(unsigned stacksize);
};

} // namespace veins
