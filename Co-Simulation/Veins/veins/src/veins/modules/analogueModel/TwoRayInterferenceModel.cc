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

#include "veins/modules/analogueModel/TwoRayInterferenceModel.h"
#include "veins/base/messages/AirFrame_m.h"

using namespace veins;

void TwoRayInterferenceModel::filterSignal(Signal* signal)
{
    auto senderPos = signal->getSenderPoa().pos.getPositionAt();
    auto receiverPos = signal->getReceiverPoa().pos.getPositionAt();

    const Coord senderPos2D(senderPos.x, senderPos.y);
    const Coord receiverPos2D(receiverPos.x, receiverPos.y);

    ASSERT(senderPos.z > 0); // make sure send antenna is above ground
    ASSERT(receiverPos.z > 0); // make sure receive antenna is above ground

    double d = senderPos2D.distance(receiverPos2D);
    double ht = senderPos.z, hr = receiverPos.z;

    EV_TRACE << "(ht, hr) = (" << ht << ", " << hr << ")" << endl;

    double d_dir = sqrt(pow(d, 2) + pow((ht - hr), 2)); // direct distance
    double d_ref = sqrt(pow(d, 2) + pow((ht + hr), 2)); // distance via ground reflection
    double sin_theta = (ht + hr) / d_ref;
    double cos_theta = d / d_ref;

    double gamma = (sin_theta - sqrt(epsilon_r - pow(cos_theta, 2))) / (sin_theta + sqrt(epsilon_r - pow(cos_theta, 2)));

    Signal attenuation(signal->getSpectrum());
    for (uint16_t i = 0; i < signal->getNumValues(); i++) {
        double freq = signal->getSpectrum().freqAt(i);
        double lambda = BaseWorldUtility::speedOfLight() / freq;
        double phi = (2 * M_PI / lambda * (d_dir - d_ref));
        double att = pow(4 * M_PI * (d / lambda) * 1 / (sqrt((pow((1 + gamma * cos(phi)), 2) + pow(gamma, 2) * pow(sin(phi), 2)))), 2);

        EV_TRACE << "Add attenuation for (freq, lambda, phi, gamma, att) = (" << freq << ", " << lambda << ", " << phi << ", " << gamma << ", " << (1 / att) << ", " << FWMath::mW2dBm(att) << ")" << endl;

        attenuation.at(i) = 1 / att;
    }
    *signal *= attenuation;
}
