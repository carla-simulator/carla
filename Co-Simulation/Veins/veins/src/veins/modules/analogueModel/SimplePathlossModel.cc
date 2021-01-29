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

#include "veins/modules/analogueModel/SimplePathlossModel.h"

#include "veins/base/messages/AirFrame_m.h"

using namespace veins;

using veins::AirFrame;

void SimplePathlossModel::filterSignal(Signal* signal)
{
    auto senderPos = signal->getSenderPoa().pos.getPositionAt();
    auto receiverPos = signal->getReceiverPoa().pos.getPositionAt();

    /** Calculate the distance factor */
    double sqrDistance = useTorus ? receiverPos.sqrTorusDist(senderPos, playgroundSize) : receiverPos.sqrdist(senderPos);

    EV_TRACE << "sqrdistance is: " << sqrDistance << endl;

    if (sqrDistance <= 1.0) {
        // attenuation is negligible
        return;
    }

    // the part of the attenuation only depending on the distance
    double distFactor = pow(sqrDistance, -pathLossAlphaHalf) / (16.0 * M_PI * M_PI);
    EV_TRACE << "distance factor is: " << distFactor << endl;

    Signal attenuation(signal->getSpectrum());
    for (uint16_t i = 0; i < signal->getNumValues(); i++) {
        double wavelength = BaseWorldUtility::speedOfLight() / signal->getSpectrum().freqAt(i);
        attenuation.at(i) = (wavelength * wavelength) * distFactor;
    }
    *signal *= attenuation;
}
