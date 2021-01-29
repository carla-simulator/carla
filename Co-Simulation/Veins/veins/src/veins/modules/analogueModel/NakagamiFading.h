//
// Copyright (C) 2015 David Eckhoff <david.eckhoff@fau.de>
//                    Christoph Sommer <sommer@ccs-labs.org>
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

#include "veins/base/phyLayer/AnalogueModel.h"
#include "veins/base/modules/BaseWorldUtility.h"
#include "veins/base/messages/AirFrame_m.h"

namespace veins {

/**
 * @brief
 * A simple model to account for fast fading using the Nakagami Distribution.
 *
 * See the Veins website <a href="http://veins.car2x.org/"> for a tutorial, documentation, and publications </a>.
 *
 * An in-depth description of the model is available at:
 * Todo: add paper
 *
 * @author David Eckhoff, Christoph Sommer
 *
 * @ingroup analogueModels
 */
class VEINS_API NakagamiFading : public AnalogueModel {

public:
    NakagamiFading(cComponent* owner, bool constM, double m)
        : AnalogueModel(owner)
        , constM(constM)
        , m(m)
    {
    }

    ~NakagamiFading() override
    {
    }

    void filterSignal(Signal* signal) override;

protected:
    /** @brief Whether to use a constant m or a m based on distance */
    bool constM;

    /** @brief The value of the coefficient m */
    double m;
};

} // namespace veins
