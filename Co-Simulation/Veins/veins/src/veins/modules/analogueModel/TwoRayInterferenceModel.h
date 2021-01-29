//
// Copyright (C) 2011 Stefan Joerer <stefan.joerer@uibk.ac.at>
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

namespace veins {

using veins::AirFrame;

/**
 * @brief
 * Extended version of Two-Ray Ground path loss model.
 *
 * See the Veins website <a href="http://veins.car2x.org/"> for a tutorial, documentation, and publications </a>.
 *
 * An in-depth description of the model is available at:
 * Christoph Sommer and Falko Dressler, "Using the Right Two-Ray Model? A Measurement based Evaluation of PHY Models in VANETs," Proceedings of 17th ACM International Conference on Mobile Computing and Networking (MobiCom 2011), Poster Session, Las Vegas, NV, September 2011.
 *
 * @author Stefan Joerer
 *
 * @ingroup analogueModels
 */
class VEINS_API TwoRayInterferenceModel : public AnalogueModel {

public:
    TwoRayInterferenceModel(cComponent* owner, double dielectricConstant)
        : AnalogueModel(owner)
        , epsilon_r(dielectricConstant)
    {
    }

    ~TwoRayInterferenceModel() override
    {
    }

    void filterSignal(Signal* signal) override;

protected:
    /** @brief stores the dielectric constant used for calculation */
    double epsilon_r;
};

} // namespace veins
