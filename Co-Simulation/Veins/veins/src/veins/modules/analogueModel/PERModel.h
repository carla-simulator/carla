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

#include "veins/base/phyLayer/AnalogueModel.h"

using veins::AirFrame;

namespace veins {

/**
 * @brief This class applies a parameterized packet error rate
 * to incoming packets. This allows the user to easily
 * study the robustness of its system to packet loss.
 *
 * @ingroup analogueModels
 *
 * @author Jérôme Rousselot <jerome.rousselot@csem.ch>
 */
class VEINS_API PERModel : public AnalogueModel {
protected:
    double packetErrorRate;

public:
    /** @brief The PERModel constructor takes as argument the packet error rate to apply (must be between 0 and 1). */
    PERModel(cComponent* owner, double per)
        : AnalogueModel(owner)
        , packetErrorRate(per)
    {
        ASSERT(per <= 1 && per >= 0);
    }

    void filterSignal(Signal*) override;
};

} // namespace veins
