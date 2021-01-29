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

#include <cstdlib>

#include "veins/veins.h"

#include "veins/base/phyLayer/AnalogueModel.h"
#include "veins/base/modules/BaseWorldUtility.h"

namespace veins {

using veins::AirFrame;

class SimplePathlossModel;

/**
 * @brief Basic implementation of a SimplePathlossModel
 *
 * An example config.xml for this AnalogueModel can be the following:
 * @verbatim
    <AnalogueModel type="SimplePathlossModel">
        <!-- Environment parameter of the pathloss formula
             If ommited default value is 3.5-->
        <parameter name="alpha" type="double" value="3.5"/>
    </AnalogueModel>
   @endverbatim
 *
 * @ingroup analogueModels
 */
class VEINS_API SimplePathlossModel : public AnalogueModel {
protected:
    /** @brief Path loss coefficient. **/
    double pathLossAlphaHalf;

    /** @brief Information needed about the playground */
    const bool useTorus;

    /** @brief The size of the playground.*/
    const Coord& playgroundSize;

public:
    /**
     * @brief Initializes the analogue model. playgroundSize
     * need to be valid as long as this instance exists.
     *
     * The constructor needs some specific knowledge in order to create
     * its mapping properly:
     *
     * @param owner pointer to the cComponent that owns this AnalogueModel
     * @param alpha the coefficient alpha (specified e.g. in config.xml and
     *                 passed in constructor call)
     * @param useTorus information about the playground the host is moving in
     * @param playgroundSize information about the playground the host is
     *                          moving in
     */
    SimplePathlossModel(cComponent* owner, double alpha, bool useTorus, const Coord& playgroundSize)
        : AnalogueModel(owner)
        , pathLossAlphaHalf(alpha * 0.5)
        , useTorus(useTorus)
        , playgroundSize(playgroundSize)
    {
    }

    /**
     * @brief Filters a specified AirFrame's Signal by adding an attenuation
     * over time to the Signal.
     */
    void filterSignal(Signal*) override;

    bool neverIncreasesPower() override
    {
        return true;
    }
};

} // namespace veins
