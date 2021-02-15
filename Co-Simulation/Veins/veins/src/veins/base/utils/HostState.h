//
// Copyright (C) 2007 Technische Universitaet Berlin (TUB), Germany, Telecommunication Networks Group
// Copyright (C) 2007 Technische Universiteit Delft (TUD), Netherlands
// Copyright (C) 2007 Universitaet Paderborn (UPB), Germany
// Copyright (C) 2009 Swedish Institute of Computer Science
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

// Original author: Laura Marie Feeney

#pragma once

#include "veins/veins.h"

namespace veins {

/**
 * @brief HostState is published by the battery to announce host failure
 *
 * HostState information is published by the Battery to announce Host
 * failure due to battery depletion.  (Default state is ON, OFF is not
 * used; existing modules basically ignore everything but FAIL.)
 *
 * All modules that generate messages and collect statistics should
 * subscribe to this notification.  Should eventually be generalized
 * to handle more general HostState (e.g. stochastic failure,
 * restart).
 *
 * @ingroup power
 */
class VEINS_API HostState : public cObject {
public:
    /**
     * @brief Possible host states.
     */
    enum States {
        ACTIVE, /** Host is active and fully working*/
        FAILED, /** Host is not active because of some failure
                 * but might able to be restarted*/
        BROKEN, /** Host is not active because of some failure
                 * and won't be able to be restarted */
        SLEEP, /** Host is not active (sleeping) */
        OFF /** Host is not active (shut down) */
    };
    // we could make a nice 'info' field here, to allow us to specify
    // the cause of failure (e.g. battery, stochastic hardware failure)

private:
    /**
     * @brief Host state.
     */
    States state;

public:
    /**
     * @brief Constructor taking a state.
     */
    HostState(States state = ACTIVE)
        : state(state)
    {
    }

    /** @brief Returns the host state */
    States get() const
    {
        return state;
    }
    /** @brief Sets the host state */
    void set(States s)
    {
        state = s;
    }

    /**
     * @brief Returns information about the current state.
     */
    std::string info() const override
    {
        std::ostringstream ost;
        switch (state) {
        case ACTIVE:
            ost << "ACTIVE";
            break;
        case FAILED:
            ost << "FAILED";
            break;
        case BROKEN:
            ost << "BROKEN";
            break;
        case SLEEP:
            ost << "SLEEP";
            break;
        case OFF:
            ost << "OFF";
            break;
        default:
            ost << "Unknown";
            break;
        }
        return ost.str();
    }
};

} // namespace veins
