//
// Copyright (C) 2016 Alexander Brummer <alexander.brummer@fau.de>
// Copyright (C) 2018 Fabian Bronner <fabian.bronner@ccs-labs.org>
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

#include "veins/modules/phy/SampledAntenna1D.h"
#include "veins/base/utils/FWMath.h"

using namespace veins;

SampledAntenna1D::SampledAntenna1D(std::vector<double>& values, std::string offsetType, std::vector<double>& offsetParams, std::string rotationType, std::vector<double>& rotationParams, cRNG* rng)
    : antennaGains(values.size() + 1)
{
    distance = (2 * M_PI) / values.size();

    // instantiate a random number generator for sample offsets if one is specified
    cRandom* offsetGen = nullptr;
    if (offsetType == "uniform") {
        if (!math::almost_equal(offsetParams[0], -offsetParams[1])) {
            throw cRuntimeError("SampledAntenna1D::SampledAntenna1D(): The mean of the random distribution for the samples' offsets has to be 0.");
        }
        offsetGen = new cUniform(rng, offsetParams[0], offsetParams[1]);
    }
    else if (offsetType == "normal") {
        if (!math::almost_equal<double>(offsetParams[0], 0)) {
            throw cRuntimeError("SampledAntenna1D::SampledAntenna1D(): The mean of the random distribution for the samples' offsets has to be 0.");
        }
        offsetGen = new cNormal(rng, offsetParams[0], offsetParams[1]);
    }
    else if (offsetType == "triang") {
        if (!math::almost_equal<double>((offsetParams[0] + offsetParams[1] + offsetParams[2]) / 3, 0)) {
            throw cRuntimeError("SampledAntenna1D::SampledAntenna1D(): The mean of the random distribution for the samples' offsets has to be 0.");
        }
        offsetGen = new cTriang(rng, offsetParams[0], offsetParams[1], offsetParams[2]);
    }

    // determine random rotation of the antenna if specified
    cRandom* rotationGen = nullptr;
    if (rotationType == "uniform") {
        rotationGen = new cUniform(rng, rotationParams[0], rotationParams[1]);
    }
    else if (rotationType == "normal") {
        rotationGen = new cNormal(rng, rotationParams[0], rotationParams[1]);
    }
    else if (rotationType == "triang") {
        rotationGen = new cTriang(rng, rotationParams[0], rotationParams[1], rotationParams[2]);
    }
    rotation = (rotationGen == nullptr) ? 0 : rotationGen->draw();
    if (rotationGen != nullptr) delete rotationGen;

    // transform to rad
    rotation *= (M_PI / 180);

    // copy values and apply offset
    for (unsigned int i = 0; i < values.size(); i++) {
        double offset = 0;
        if (offsetGen != nullptr) {
            offset = offsetGen->draw();
            // transform to rad
            offset *= (M_PI / 180);
        }
        antennaGains[i] = values[i] + offset;
    }
    if (offsetGen != nullptr) delete offsetGen;

    // assign the value of 0 degrees to 360 degrees as well to assure correct interpolation (size allocated already before)
    antennaGains[values.size()] = antennaGains[0];
}

SampledAntenna1D::~SampledAntenna1D()
{
}

double SampledAntenna1D::getGain(Coord ownPos, Coord ownOrient, Coord otherPos)
{
    // get the line of sight vector
    Coord los = otherPos - ownPos;
    // calculate angle using atan2
    double angle = atan2(los.y, los.x) - atan2(ownOrient.y, ownOrient.x);

    // apply possible rotation
    angle -= rotation;

    // make sure angle is within [0, 2*M_PI)
    angle = fmod(angle, 2 * M_PI);
    if (angle < 0) angle += 2 * M_PI;

    // calculate antennaGain
    size_t baseElement = angle / distance;
    double offset = (angle - (baseElement * distance)) / distance;

    // make sure to not address an element out of antennaGains (baseElement == lastElement implies that offset is zero)
    ASSERT((baseElement < antennaGains.size()) && (baseElement != antennaGains.size() - 1 || offset == 0));

    double gainValue = antennaGains[baseElement];
    if (offset > 0) {
        gainValue += offset * (antennaGains[baseElement + 1] - antennaGains[baseElement]);
    }

    return FWMath::dBm2mW(gainValue);
}

double SampledAntenna1D::getLastAngle()
{
    return lastAngle / M_PI * 180.0;
}
