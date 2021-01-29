//
// Copyright (C) 2015 Raphael Riebl <raphael.riebl@thi.de>
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

#include <list>
#include <set>
#include <string>
#include <utility>

#include "veins/modules/mobility/traci/TraCICoord.h"

namespace veins {

/**
 * Can return whether a given position lies within the simulation's region of interest.
 * Modules are destroyed and re-created as managed vehicles leave and re-enter the ROI
 */
class VEINS_API TraCIRegionOfInterest {
public:
    enum ConstraintResult {
        NONE, // when no constraints exist
        SATISFY,
        BREAK
    };

    TraCIRegionOfInterest();

    /**
     * Add roads to contraints
     * @param roads given as road ids separated by spaces
     */
    void addRoads(const std::string& roads);

    /**
     * Add rectangles to constraints
     * @param rects given as x1,y1-x2,y2 pairs separated by spaces
     */
    void addRectangles(const std::string& rects);

    /**
     * Remove all constraints
     */
    void clear();

    /**
     * Check if position lies on any ROI rectangle
     * @param pos Position to check
     * @return true if on any rectangle
     */
    bool onAnyRectangle(const TraCICoord& pos) const;

    /**
     * Check if a given road is part of interest roads
     * @param road_id
     * @return true if part of ROI roads
     */
    bool partOfRoads(const std::string& road_id) const;

    /**
     * Check if any constraints are defined
     * @return true if constraints exist
     */
    bool hasConstraints() const;

    const std::list<std::pair<TraCICoord, TraCICoord>>& getRectangles() const;

private:
    std::set<std::string> roiRoads; /**< which roads (e.g. "hwy1 hwy2") are considered to consitute the region of interest, if not empty */
    std::list<std::pair<TraCICoord, TraCICoord>> roiRects; /**< which rectangles (e.g. "0,0-10,10 20,20-30,30) are considered to consitute the region of interest, if not empty */
};

} // namespace veins
