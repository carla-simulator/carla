//
// Copyright (C) 2006-2018 Christoph Sommer <sommer@ccs-labs.org>
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

#include <vector>

#include "veins/veins.h"

#include "veins/base/utils/Coord.h"
#include "veins/modules/world/annotations/AnnotationManager.h"

namespace veins {

/**
 * stores information about an Obstacle for ObstacleControl
 */
class VEINS_API Obstacle {
public:
    using Coords = std::vector<Coord>;

    Obstacle(std::string id, std::string type, double attenuationPerCut, double attenuationPerMeter);

    void setShape(Coords shape);
    const Coords& getShape() const;
    const Coord getBboxP1() const;
    const Coord getBboxP2() const;
    bool containsPoint(Coord Point) const;

    std::string getType() const;
    std::string getId() const;
    double getAttenuationPerCut() const;
    double getAttenuationPerMeter() const;

    /**
     * get a list of points (in [0, 1]) along the line between sender and receiver where the beam intersects with this obstacle
     */
    std::vector<double> getIntersections(const Coord& senderPos, const Coord& receiverPos) const;

    AnnotationManager::Annotation* visualRepresentation;

protected:
    std::string id;
    std::string type;
    double attenuationPerCut; /**< in dB. attenuation per exterior border of obstacle */
    double attenuationPerMeter; /**< in dB / m. to account for attenuation caused by interior of obstacle */
    Coords coords;
    Coord bboxP1;
    Coord bboxP2;
};

} // namespace veins
