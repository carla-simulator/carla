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

#pragma once

#include "veins/base/phyLayer/Antenna.h"
#include <vector>

namespace veins {

/**
 * @brief
 * This class represents an antenna whose gain is calculated from given samples in the horizontal plane.
 * The respective gain is therefore dependent on the azimuth angle.
 * The user has to provide the samples, which are assumed to be distributed equidistantly.
 * As the power is assumed to be relative to an isotropic radiator, the values have to be given in dBi.
 * The values are stored in a mapping automatically supporting linear interpolation between samples.
 * Optional randomness in terms of sample offsets and antenna rotation is supported.
 *
 * * An example antenna.xml for this Antenna can be the following:
 * @verbatim
    <?xml version="1.0" encoding="UTF-8"?>
    <root>
        <Antenna type="SampledAntenna1D" id="antenna1">
            <!-- Write the samples in the value attribute, separated by spaces. The values will be -->
            <!-- distributed equidistantly, e.g. 4 values will be placed at 0°, 90°, 180° and 270° -->
            <parameter name="samples" type="string" value="3 -3 3 -3"/>

            <!-- Options for random offsets are as follows. -->
            <!-- The mean of the given distribution has to be 0 (so that the overall offset is close to 0dBi) -->
            <!-- <parameter name="random-offsets" type="string" value="uniform a b"/> -->
            <!-- <parameter name="random-offsets" type="string" value="normal mean stddev"/> -->
            <!-- <parameter name="random-offsets" type="string" value="triang a b c"/> -->
            <parameter name="random-offsets" type="string" value="uniform -0.01 0.01"/>

            <!-- Options for random rotation of the antennas are the same, but mean doesn't have to be 0. -->
            <parameter name="random-rotation" type="string" value="uniform -1 1"/>
        </Antenna>
    </root>
   @endverbatim
 *
 *
 * @author Alexander Brummer
 *
 *
 * @see Antenna
 * @see BasePhyLayer
 */
class VEINS_API SampledAntenna1D : public Antenna {
public:
    /**
     * @brief Constructor for the sampled antenna.
     *
     * @param values            - contains the samples representing the antenna
     * @param offsetType        - name of random distribution to use for the random offset of the samples
     * @param offsetParams      - contains the parameters for the offset random distribution
     * @param rotationType      - name of random distribution to use for the random rotation of the whole antenna
     * @param rotationParams    - contains the parameters for the rotation random distribution
     * @param rng               - pointer to the random number generator to use
     */
    SampledAntenna1D(std::vector<double>& values, std::string offsetType, std::vector<double>& offsetParams, std::string rotationType, std::vector<double>& rotationParams, cRNG* rng);

    /**
     * @brief Destructor of the sampled antenna.
     *
     * Deletes the mapping used for storing the antenna samples.
     */
    ~SampledAntenna1D() override;

    /**
     * @brief Calculates this antenna's gain based on the direction the signal is coming from/sent in.
     *
     * @param ownPos        - coordinates of this antenna
     * @param ownOrient     - states the direction the antenna (i.e. the car) is pointing at
     * @param otherPos      - coordinates of the other antenna which this antenna is currently communicating with
     * @return Returns the gain this antenna achieves depending on the computed direction.
     * If the angle is within two samples, linear interpolation is applied.
     */
    double getGain(Coord ownPos, Coord ownOrient, Coord otherPos) override;

    double getLastAngle() override;

private:
    /**
     * @brief Used to store the antenna's samples.
     */
    std::vector<double> antennaGains;
    double distance;

    /**
     * @brief An optional random rotation of the antenna is stored in this field and applied every time
     * the gain has to be calculated.
     */
    double rotation;

    double lastAngle;
};

} // namespace veins
