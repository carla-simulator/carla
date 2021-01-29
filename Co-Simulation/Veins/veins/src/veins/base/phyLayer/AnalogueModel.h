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

#include <memory>
#include <vector>

#include "veins/veins.h"

#include "veins/base/utils/AntennaPosition.h"
#include "veins/base/utils/Coord.h"
#include "veins/modules/utility/HasLogProxy.h"

namespace veins {

class AirFrame;
class Signal;

/**
 * @brief Interface for the analogue models of the physical layer.
 *
 * An analogue model is a filter responsible for changing
 * the attenuation value of a Signal to simulate things like
 * shadowing, fading, pathloss or obstacles.
 *
 * @ingroup analogueModels
 */
class VEINS_API AnalogueModel : public HasLogProxy {

public:
    AnalogueModel(cComponent* owner)
        : HasLogProxy(owner)
    {
    }

    virtual ~AnalogueModel()
    {
    }

    /**
     * @brief Has to be overriden by every implementation.
     *
     * Filters a specified AirFrame's Signal by adding an attenuation
     * over time to the Signal.
     *
     * @param signal        The signal to filter.
     */
    virtual void filterSignal(Signal* signal) = 0;

    /**
     * If the model never increases the power level of any signal given to filterSignal, it returns true here.
     * This allows optimized signal handling.
     */
    virtual bool neverIncreasesPower()
    {
        return false;
    }
};

using AnalogueModelList = std::vector<std::unique_ptr<AnalogueModel>>;

} // namespace veins
