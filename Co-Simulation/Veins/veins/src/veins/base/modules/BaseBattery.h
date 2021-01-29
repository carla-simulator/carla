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

#pragma once

#include "veins/veins.h"

#include "veins/base/modules/BaseModule.h"
#include "veins/base/utils/HostState.h"

namespace veins {

/**
 * @brief Defines the amount of power drawn by a device from
 * a power source.
 *
 * Used as generic amount parameter for BaseBatteries "draw"-method.
 *
 * Can be either an instantaneous draw of a certain energy amount
 * in mWs (type=ENERGY) or a draw of a certain current in mA over
 * time (type=CURRENT).
 *
 * Can be sub-classed for more complex power draws.
 *
 * @ingroup baseModules
 * @ingroup power
 */
class VEINS_API DrawAmount {
public:
    /** @brief The type of the amount to draw.*/
    enum PowerType {
        CURRENT, /** @brief Current in mA over time. */
        ENERGY /** @brief Single fixed energy draw in mWs */
    };

protected:
    /** @brief Stores the type of the amount.*/
    int type;

    /** @brief Stores the actual amount.*/
    double value;

public:
    /** @brief Initializes with passed type and value.*/
    DrawAmount(int type = CURRENT, double value = 0)
        : type(type)
        , value(value)
    {
    }
    virtual ~DrawAmount()
    {
    }

    /** @brief Returns the type of power drawn as PowerType. */
    virtual int getType() const
    {
        return type;
    }
    /** @brief Returns the actual amount of power drawn. */
    virtual double getValue() const
    {
        return value;
    }

    /** @brief Sets the type of power drawn. */
    virtual void setType(int t)
    {
        type = t;
    }
    /** @brief Sets the actual amount of power drawn. */
    virtual void setValue(double v)
    {
        value = v;
    }
};

/**
 * @brief Base class for any power source.
 *
 * See "SimpleBattery" for an example implementation.
 *
 * @ingroup baseModules
 * @ingroup power
 * @see SimpleBattery
 */
class VEINS_API BaseBattery : public BaseModule {
public:
    BaseBattery()
        : BaseModule()
    {
    }
    BaseBattery(unsigned stacksize)
        : BaseModule(stacksize)
    {
    }
    /**
     * @brief Registers a power draining device with this battery.
     *
     * Takes the name of the device as well as a number of accounts
     * the devices draws power for (like rx, tx, idle for a radio device).
     *
     * Returns an ID by which the device can identify itself to the
     * battery.
     *
     * Has to be implemented by actual battery implementations.
     */
    virtual int registerDevice(const std::string& name, int numAccounts) = 0;

    /**
     * @brief Draws power from the battery.
     *
     * The actual amount and type of power drawn is defined by the passed
     * DrawAmount parameter. Can be an fixed single amount or an amount
     * drawn over time.
     * The drainID identifies the device which drains the power.
     * "Account" identifies the account the power is drawn from. It is
     * used for statistical evaluation only to see which activity of a
     * device has used how much power. It does not affect functionality.
     */
    virtual void draw(int drainID, DrawAmount& amount, int account) = 0;

    /**
     * @name State-of-charge interface
     *
     * @brief Other host modules should use these interfaces to obtain
     *  the state-of-charge of the battery.  Do NOT use BatteryState
     *  interfaces, which should be used only by Battery Stats modules.
     */
    /*@{*/
    /** @brief get voltage (future support for non-voltage regulated h/w */
    virtual double getVoltage() const = 0;
    /** @brief current state of charge of the battery, relative to its
     * rated nominal capacity [0..1]
     */
    virtual double estimateResidualRelative() const = 0;
    /** @brief current state of charge of the battery (mW-s) */
    virtual double estimateResidualAbs() const = 0;
    /** @brief Current state of the battery. */
    virtual HostState::States getState() const = 0;
    /*@}*/
};

} // namespace veins
