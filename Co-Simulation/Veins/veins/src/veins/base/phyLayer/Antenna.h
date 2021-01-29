//
// Copyright (C) 2016 Alexander Brummer <alexander.brummer@fau.de>
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

#include "veins/base/utils/Coord.h"

namespace veins {

/**
 * @brief The Antenna class is the base class of all antenna models.
 *
 * The purpose of all Antenna classes is to calculate the antenna gain
 * based on the current positions and orientations of the involved nodes.
 *
 * This base Antenna acts as an isotropic antenna, it always returns
 * a gain of 1.0. It is assigned to all nodes if the user does not specify
 * another antenna type.
 *
 * @author Alexander Brummer
 */
class VEINS_API Antenna {
public:
    Antenna(){};
    virtual ~Antenna(){};

    /**
     * Calculates the antenna gain of the represented antenna.
     *
     * In the case of this class, a value of 1.0 is returned always,
     * representing an isotropic radiator.
     *
     * Nevertheless, all Antenna subclasses' getGain() methods have to
     * take the following three parameters as the gain depends on the angle
     * of incidence in general.
     *
     * @param ownPos    - states the position of this antenna
     * @param ownOrient - the direction the antenna/the host is pointing in
     * @param otherPos  - the position of the other antenna which this antenna
     * is sending to or receiving from
     *
     * @return Returns the gain in this specific direction.
     */
    virtual double getGain(Coord ownPos, Coord ownOrient, Coord otherPos);

    virtual double getLastAngle()
    {
        return -1.0;
    };
};

} // namespace veins
