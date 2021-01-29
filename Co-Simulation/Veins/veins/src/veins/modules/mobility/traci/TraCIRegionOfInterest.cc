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

#include <algorithm>
#include <sstream>

#include "veins/veins.h"

#include "veins/modules/mobility/traci/TraCIRegionOfInterest.h"

namespace veins {

TraCIRegionOfInterest::TraCIRegionOfInterest()
{
}

void TraCIRegionOfInterest::addRoads(const std::string& roads)
{
    std::istringstream roadsStream(roads);
    std::string road;
    while (std::getline(roadsStream, road, ' ')) {
        roiRoads.insert(road);
    }
}

void TraCIRegionOfInterest::addRectangles(const std::string& rects)
{
    std::istringstream rectsStream(rects);
    std::string rect;
    while (std::getline(rectsStream, rect, ' ')) {
        std::istringstream rectStream(rect);
        double x1;
        rectStream >> x1;
        char c1;
        rectStream >> c1;
        double y1;
        rectStream >> y1;
        char c2;
        rectStream >> c2;
        double x2;
        rectStream >> x2;
        char c3;
        rectStream >> c3;
        double y2;
        rectStream >> y2;
        if (rectStream.good()) {
            throw cRuntimeError("Parsing ROI rectangle failed");
        }
        roiRects.push_back(std::pair<TraCICoord, TraCICoord>(TraCICoord(x1, y1), TraCICoord(x2, y2)));
    }
}

void TraCIRegionOfInterest::clear()
{
    roiRoads.clear();
    roiRects.clear();
}

bool TraCIRegionOfInterest::onAnyRectangle(const TraCICoord& position) const
{
    struct RectangleTest {
        RectangleTest(const TraCICoord& position_)
            : position(position_)
        {
        }

        bool operator()(const std::pair<TraCICoord, TraCICoord>& rect) const
        {
            return (position.x >= rect.first.x && position.y >= rect.first.y) && (position.x <= rect.second.x && position.y <= rect.second.y);
        }

        const TraCICoord& position;
    };

    std::list<std::pair<TraCICoord, TraCICoord>>::const_iterator found = std::find_if(roiRects.begin(), roiRects.end(), RectangleTest(position));
    return found != roiRects.end();
}

bool TraCIRegionOfInterest::partOfRoads(const std::string& road) const
{
    return roiRoads.count(road) > 0;
}

bool TraCIRegionOfInterest::hasConstraints() const
{
    return !roiRoads.empty() || !roiRects.empty();
}

const std::list<std::pair<TraCICoord, TraCICoord>>& TraCIRegionOfInterest::getRectangles() const
{
    return roiRects;
}

} // namespace veins
