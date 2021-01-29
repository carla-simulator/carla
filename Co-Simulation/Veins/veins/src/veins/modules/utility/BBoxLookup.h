//
// Copyright (C) 2019 Dominik S. Buse <buse@ccs-labs.org>
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

#include <algorithm>
#include <functional>
#include <vector>

#include "veins/veins.h"

namespace veins {

class Obstacle;

/**
 * Fast grid-based spatial datastructure to find obstacles (geometric shapes) in a bounding box.
 *
 * Stores bounding boxes for a set of obstacles and allows searching for them via another bounding box.
 *
 * Only considers a 2-dimensional plane (x and y coordinates).
 *
 * In principle, any kind (or implementation) of a obstacle/shape/polygon is possible.
 * There only has to be a function to derive a bounding box for a given obstacle.
 * Obstacle instances are stored as pointers, so the lifetime of the obstacle instances is not managed by this class.
 */
class VEINS_API BBoxLookup {
public:
    struct Point {
        double x;
        double y;
    };
    // array of stucts approach
    // cache line size: 64byte = 8 x 8-byte double = 2 bboxes per line
    // bbox coordinates are inherently local, if i check x1, i likely also check the other
    struct Box {
        Point p1;
        Point p2;
    };
    struct BBoxCell {
        size_t index; /**< index of the first element of this cell in bboxes */
        size_t count; /**< number of elements in this cell; index + number = index of last element */
    };

    BBoxLookup() = default;
    BBoxLookup(const std::vector<Obstacle*>& obstacles, std::function<BBoxLookup::Box(Obstacle*)> makeBBox, double scenarioX, double scenarioY, int cellSize = 250);

    /**
     * Return all obstacles which have their bounding box touched by the transmission from sender to receiver.
     *
     * The obstacles itself may not actually overlap with transmission (false positives are possible).
     */
    std::vector<Obstacle*> findOverlapping(Point sender, Point receiver) const;

private:
    // NOTE: obstacles may occur multiple times in bboxes/obstacleLookup (if they are in multiple cells)
    std::vector<Box> bboxes; /**< ALL bboxes in one chunck of contiguos memory, ordered by cells */
    std::vector<Obstacle*> obstacleLookup; /**< bboxes[i] belongs to instance in obstacleLookup[i] */
    std::vector<BBoxCell> bboxCells; /**< flattened matrix of X * Y BBoxCell instances */
    int cellSize = 0;
    size_t numCols = 0; /**< X BBoxCell instances in a row */
    size_t numRows = 0; /**< Y BBoxCell instances in a column */
};

} // namespace veins
