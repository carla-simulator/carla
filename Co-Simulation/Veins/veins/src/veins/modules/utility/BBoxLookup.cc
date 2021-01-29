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

#include <cmath>

#include "veins/modules/utility/BBoxLookup.h"

namespace {

using Point = veins::BBoxLookup::Point;
using Box = veins::BBoxLookup::Box;

/**
 * Helper structure representing a wireless ray from a sender to a receiver.
 *
 * Contains pre-computed values to speed up calls to intersect with the same ray but different boxes.
 */
struct Ray {
    Point origin;
    Point destination;
    Point direction;
    Point invDirection;
    struct {
        size_t x;
        size_t y;
    } sign;
    double length;
};

/**
 * Return a Ray struct for fast intersection tests from sender to receiver.
 */
Ray makeRay(const Point& sender, const Point& receiver)
{
    const double dir_x = receiver.x - sender.x;
    const double dir_y = receiver.y - sender.y;
    Ray ray;
    ray.origin = sender;
    ray.destination = receiver;
    ray.length = std::sqrt(dir_x * dir_x + dir_y * dir_y);
    ray.direction.x = dir_x / ray.length;
    ray.direction.y = dir_y / ray.length;
    ray.invDirection.x = 1.0 / ray.direction.x;
    ray.invDirection.y = 1.0 / ray.direction.y;
    ray.sign.x = ray.invDirection.x < 0;
    ray.sign.y = ray.invDirection.y < 0;
    return ray;
}
/**
 * Return whether ray intersects with box.
 *
 * Based on:
 * Amy Williams, Steve Barrus, R. Keith Morley & Peter Shirley (2005) An Efficient and Robust Ray-Box Intersection Algorithm, Journal of Graphics Tools, 10:1, 49-54, DOI: 10.1080/2151237X.2005.10129188
 */
bool intersects(const Ray& ray, const Box& box)
{
    const double x[2]{box.p1.x, box.p2.x};
    const double y[2]{box.p1.y, box.p2.y};
    double tmin = (x[ray.sign.x] - ray.origin.x) * ray.invDirection.x;
    double tmax = (x[1 - ray.sign.x] - ray.origin.x) * ray.invDirection.x;
    double tymin = (y[ray.sign.y] - ray.origin.y) * ray.invDirection.y;
    double tymax = (y[1 - ray.sign.y] - ray.origin.y) * ray.invDirection.y;

    if ((tmin > tymax) || (tymin > tmax)) return false;
    if (tymin > tmin) tmin = tymin;
    if (tymax < tmax) tmax = tymax;
    return (tmin < ray.length) && (tmax > 0);
}

} // anonymous namespace

namespace veins {

BBoxLookup::BBoxLookup(const std::vector<Obstacle*>& obstacles, std::function<BBoxLookup::Box(Obstacle*)> makeBBox, double scenarioX, double scenarioY, int cellSize)
    : bboxes()
    , obstacleLookup()
    , bboxCells()
    , cellSize(cellSize)
    , numCols(std::floor(scenarioX / cellSize) + 1)
    , numRows(std::floor(scenarioY / cellSize) + 1)
{
    // phase 1: build unordered collection of cells
    // initialize proto-cells (cells in non-contiguos memory)
    ASSERT(scenarioX > 0);
    ASSERT(scenarioY > 0);
    ASSERT(numCols * cellSize >= scenarioX);
    ASSERT(numRows * cellSize >= scenarioY);
    const size_t numCells = numCols * numRows;
    std::vector<std::vector<BBoxLookup::Box>> protoCells(numCells);
    std::vector<std::vector<Obstacle*>> protoLookup(numCells);
    // fill protoCells with boundingBoxes
    size_t numEntries = 0;
    for (const auto obstaclePtr : obstacles) {
        auto bbox = makeBBox(obstaclePtr);
        const size_t fromCol = std::max(0, int(bbox.p1.x / cellSize));
        const size_t toCol = std::max(0, int(bbox.p2.x / cellSize));
        const size_t fromRow = std::max(0, int(bbox.p1.y / cellSize));
        const size_t toRow = std::max(0, int(bbox.p2.y / cellSize));
        for (size_t row = fromRow; row <= toRow; ++row) {
            for (size_t col = fromCol; col <= toCol; ++col) {
                const size_t cellIndex = col + row * numCols;
                protoCells[cellIndex].push_back(bbox);
                protoLookup[cellIndex].push_back(obstaclePtr);
                ++numEntries;
                ASSERT(protoCells[cellIndex].size() == protoLookup[cellIndex].size());
            }
        }
    }

    // phase 2: derive read-only data structure with fast lookup
    bboxes.reserve(numEntries);
    obstacleLookup.reserve(numEntries);
    bboxCells.reserve(numCells);
    size_t index = 0;
    for (size_t row = 0; row < numRows; ++row) {
        for (size_t col = 0; col < numCols; ++col) {
            const size_t cellIndex = col + row * numCols;
            auto& currentCell = protoCells.at(cellIndex);
            auto& currentLookup = protoLookup.at(cellIndex);
            ASSERT(currentCell.size() == currentLookup.size());
            const size_t count = currentCell.size();
            // copy over bboxes and obstacle lookups (in strict order)
            for (size_t entryIndex = 0; entryIndex < count; ++entryIndex) {
                bboxes.push_back(currentCell.at(entryIndex));
                obstacleLookup.push_back(currentLookup.at(entryIndex));
            }
            // create lookup table for this cell
            bboxCells.push_back({index, count});
            // forward index to begin of next cell
            index += count;
            ASSERT(bboxes.size() == index);
        }
    }
    ASSERT(bboxes.size() == numEntries);
    ASSERT(bboxes.size() == obstacleLookup.size());
}

std::vector<Obstacle*> BBoxLookup::findOverlapping(Point sender, Point receiver) const
{
    std::vector<Obstacle*> overlappingObstacles;
    const Box bbox{
        {std::min(sender.x, receiver.x), std::min(sender.y, receiver.y)},
        {std::max(sender.x, receiver.x), std::max(sender.y, receiver.y)},
    };

    // determine coordinates for all cells touched by bbox
    const size_t firstCol = std::max(0, int(bbox.p1.x / cellSize));
    const size_t lastCol = std::max(0, int(bbox.p2.x / cellSize));
    const size_t firstRow = std::max(0, int(bbox.p1.y / cellSize));
    const size_t lastRow = std::max(0, int(bbox.p2.y / cellSize));
    ASSERT(lastCol < numCols && lastRow < numRows);
    // precompute transmission ray properties
    const Ray ray = makeRay(sender, receiver);
    // iterate over cells
    for (size_t row = firstRow; row <= lastRow; ++row) {
        for (size_t col = firstCol; col <= lastCol; ++col) {
            // skip cell if ray does not intersect with the cell.
            const Box cellBox = {{static_cast<double>(col * cellSize), static_cast<double>(row * cellSize)}, {static_cast<double>((col + 1) * cellSize), static_cast<double>((row + 1) * cellSize)}};
            if (!intersects(ray, cellBox)) continue;
            // derive cell for current cell coordinates
            const size_t cellIndex = col + row * numCols;
            const BBoxCell& cell = bboxCells.at(cellIndex);
            // iterate over bboxes in each cell
            for (size_t bboxIndex = cell.index; bboxIndex < cell.index + cell.count; ++bboxIndex) {
                const Box& current = bboxes.at(bboxIndex);
                // check for overlap with bbox (fast rejection)
                if (current.p2.x < bbox.p1.x) continue;
                if (current.p1.x > bbox.p2.x) continue;
                if (current.p2.y < bbox.p1.y) continue;
                if (current.p1.y > bbox.p2.y) continue;
                // derive corresponding obstacle
                if (!intersects(ray, current)) continue;
                overlappingObstacles.push_back(obstacleLookup.at(bboxIndex));
            }
        }
    }
    return overlappingObstacles;
}

} // namespace veins
