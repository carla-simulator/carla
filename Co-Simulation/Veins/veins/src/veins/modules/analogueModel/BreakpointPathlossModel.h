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

using veins::AirFrame;

namespace veins {

/**
 * @brief Basic implementation of a BreakpointPathlossModel.
 * This class can be used to implement the ieee802154 path loss model.
 *
 * @ingroup analogueModels
 */
class VEINS_API BreakpointPathlossModel : public AnalogueModel {
protected:
    //    /** @brief Model to use for distances below breakpoint distance */
    //    SimplePathlossModel closeRangeModel;
    //    /** @brief Model to use for distances larger than the breakpoint distance */
    //    SimplePathlossModel farRangeModel;

    /** @brief initial path loss in dB */
    double PL01, PL02;
    /** @brief initial path loss */
    double PL01_real, PL02_real;

    /** @brief pathloss exponents */
    double alpha1, alpha2;

    /** @brief Breakpoint distance squared. */
    double breakpointDistance;

    /** @brief Information needed about the playground */
    const bool useTorus;

    /** @brief The size of the playground.*/
    const Coord& playgroundSize;

    /** logs computed pathlosses. */
    cOutVector pathlosses;

public:
    /**
     * @brief Initializes the analogue model. playgroundSize
     * need to be valid as long as this instance exists.
     */
    BreakpointPathlossModel(cComponent* owner, double L01, double L02, double alpha1, double alpha2, double breakpointDistance, bool useTorus, const Coord& playgroundSize)
        : AnalogueModel(owner)
        , PL01(L01)
        , PL02(L02)
        , alpha1(alpha1)
        , alpha2(alpha2)
        , breakpointDistance(breakpointDistance)
        , useTorus(useTorus)
        , playgroundSize(playgroundSize)
    {
        PL01_real = pow(10, PL01 / 10);
        PL02_real = pow(10, PL02 / 10);
        pathlosses.setName("pathlosses");
    }

    /**
     * @brief Filters a specified AirFrame's Signal by adding an attenuation
     * over time to the Signal.
     */
    void filterSignal(Signal*) override;

    virtual bool isActiveAtDestination()
    {
        return true;
    }

    virtual bool isActiveAtOrigin()
    {
        return false;
    }
};

} // namespace veins
