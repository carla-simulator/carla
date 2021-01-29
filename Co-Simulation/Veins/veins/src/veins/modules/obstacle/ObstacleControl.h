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

#include <memory>

#include "veins/veins.h"

#include "veins/base/utils/Coord.h"
#include "veins/modules/obstacle/Obstacle.h"
#include "veins/modules/world/annotations/AnnotationManager.h"
#include "veins/modules/utility/BBoxLookup.h"

namespace veins {

/**
 * ObstacleControl models obstacles that block radio transmissions.
 *
 * Each Obstacle is a polygon.
 * Transmissions that cross one of the polygon's lines will have
 * their receive power set to zero.
 */
class VEINS_API ObstacleControl : public cSimpleModule {
public:
    ~ObstacleControl() override;
    void initialize(int stage) override;
    int numInitStages() const override
    {
        return 2;
    }
    void finish() override;
    void handleMessage(cMessage* msg) override;
    void handleSelfMsg(cMessage* msg);

    void addFromXml(cXMLElement* xml);
    void addFromTypeAndShape(std::string id, std::string typeId, std::vector<Coord> shape);
    void add(Obstacle obstacle);
    void erase(const Obstacle* obstacle);
    bool isTypeSupported(std::string type);
    double getAttenuationPerCut(std::string type);
    double getAttenuationPerMeter(std::string type);

    /**
     * get hit obstacles (along with a list of points (in [0, 1]) along the line between sender and receiver where the beam intersects with the respective obstacle) as well as any obstacle that contains the sender or receiver (with a list of potentially 0 points)
     */
    std::vector<std::pair<Obstacle*, std::vector<double>>> getIntersections(const Coord& senderPos, const Coord& receiverPos) const;

    /**
     * calculate additional attenuation by obstacles, return multiplicative factor
     */
    double calculateAttenuation(const Coord& senderPos, const Coord& receiverPos) const;

protected:
    struct CacheKey {
        const Coord senderPos;
        const Coord receiverPos;

        CacheKey(const Coord& senderPos, const Coord& receiverPos)
            : senderPos(senderPos)
            , receiverPos(receiverPos)
        {
        }

        bool operator<(const CacheKey& o) const
        {
            if (senderPos.x < o.senderPos.x) return true;
            if (senderPos.x > o.senderPos.x) return false;
            if (senderPos.y < o.senderPos.y) return true;
            if (senderPos.y > o.senderPos.y) return false;
            if (receiverPos.x < o.receiverPos.x) return true;
            if (receiverPos.x > o.receiverPos.x) return false;
            if (receiverPos.y < o.receiverPos.y) return true;
            if (receiverPos.y > o.receiverPos.y) return false;
            return false;
        }
    };

    typedef std::map<CacheKey, double> CacheEntries;

    cXMLElement* obstaclesXml; /**< obstacles to add at startup */
    int gridCellSize = 250; /**< size of square grid tiles for obstacle store */

    std::vector<std::unique_ptr<Obstacle>> obstacleOwner;
    AnnotationManager* annotations;
    AnnotationManager::Group* annotationGroup;
    std::map<std::string, double> perCut;
    std::map<std::string, double> perMeter;
    mutable CacheEntries cacheEntries;
    mutable BBoxLookup bboxLookup;
    mutable bool isBboxLookupDirty = true;
};

class VEINS_API ObstacleControlAccess {
public:
    ObstacleControlAccess()
    {
    }

    ObstacleControl* getIfExists()
    {
        return dynamic_cast<ObstacleControl*>(getSimulation()->getModuleByPath("obstacles"));
    }
};

} // namespace veins
