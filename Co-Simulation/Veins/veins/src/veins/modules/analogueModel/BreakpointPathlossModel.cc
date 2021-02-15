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

#include "veins/modules/analogueModel/BreakpointPathlossModel.h"

#include "veins/base/messages/AirFrame_m.h"

using namespace veins;
using veins::AirFrame;

void BreakpointPathlossModel::filterSignal(Signal* signal)
{
    auto senderPos = signal->getSenderPoa().pos.getPositionAt();
    auto receiverPos = signal->getReceiverPoa().pos.getPositionAt();

    /** Calculate the distance factor */
    double distance = useTorus ? receiverPos.sqrTorusDist(senderPos, playgroundSize) : receiverPos.sqrdist(senderPos);
    distance = sqrt(distance);
    EV_TRACE << "distance is: " << distance << endl;

    if (distance <= 1.0) {
        // attenuation is negligible
        return;
    }

    double attenuation = 1;
    // PL(d) = PL0 + 10 alpha log10 (d/d0)
    // 10 ^ { PL(d)/10 } = 10 ^{PL0 + 10 alpha log10 (d/d0)}/10
    // 10 ^ { PL(d)/10 } = 10 ^ PL0/10 * 10 ^ { 10 log10 (d/d0)^alpha }/10
    // 10 ^ { PL(d)/10 } = 10 ^ PL0/10 * 10 ^ { log10 (d/d0)^alpha }
    // 10 ^ { PL(d)/10 } = 10 ^ PL0/10 * (d/d0)^alpha
    if (distance < breakpointDistance) {
        attenuation = attenuation * PL01_real;
        attenuation = attenuation * pow(distance, alpha1);
    }
    else {
        attenuation = attenuation * PL02_real;
        attenuation = attenuation * pow(distance / breakpointDistance, alpha2);
    }
    attenuation = 1 / attenuation;
    EV_TRACE << "attenuation is: " << attenuation << endl;

    pathlosses.record(10 * log10(attenuation)); // in dB

    *signal *= attenuation;
}
