//
// Copyright (C) 2010-2018 Christoph Sommer <sommer@ccs-labs.org>
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

#include <sstream>
#include <map>
#include <set>

#include "veins/modules/obstacle/ObstacleControl.h"
#include "veins/base/modules/BaseWorldUtility.h"

using veins::ObstacleControl;

Define_Module(veins::ObstacleControl);

namespace {

veins::BBoxLookup rebuildBBoxLookup(const std::vector<std::unique_ptr<veins::Obstacle>>& obstacleOwner, int gridCellSize = 250)
{
    std::vector<veins::Obstacle*> obstaclePointers;
    obstaclePointers.reserve(obstacleOwner.size());
    std::transform(obstacleOwner.begin(), obstacleOwner.end(), std::back_inserter(obstaclePointers), [](const std::unique_ptr<veins::Obstacle>& obstacle) { return obstacle.get(); });
    auto playgroundSize = veins::FindModule<veins::BaseWorldUtility*>::findGlobalModule()->getPgs();
    auto bboxFunction = [](veins::Obstacle* o) { return veins::BBoxLookup::Box{{o->getBboxP1().x, o->getBboxP1().y}, {o->getBboxP2().x, o->getBboxP2().y}}; };
    return veins::BBoxLookup(obstaclePointers, bboxFunction, playgroundSize->x, playgroundSize->y, gridCellSize);
}

} // anonymous namespace

ObstacleControl::~ObstacleControl()
{
}

void ObstacleControl::initialize(int stage)
{
    if (stage == 1) {
        obstacleOwner.clear();
        cacheEntries.clear();
        isBboxLookupDirty = true;

        annotations = AnnotationManagerAccess().getIfExists();
        if (annotations) annotationGroup = annotations->createGroup("obstacles");

        obstaclesXml = par("obstacles");
        gridCellSize = par("gridCellSize");
        if (gridCellSize < 1) {
            throw cRuntimeError("gridCellSize was %d, but must be a positive integer number", gridCellSize);
        }

        addFromXml(obstaclesXml);
    }
}

void ObstacleControl::finish()
{
    obstacleOwner.clear();
}

void ObstacleControl::handleMessage(cMessage* msg)
{
    if (msg->isSelfMessage()) {
        handleSelfMsg(msg);
        return;
    }
    throw cRuntimeError("ObstacleControl doesn't handle messages from other modules");
}

void ObstacleControl::handleSelfMsg(cMessage* msg)
{
    throw cRuntimeError("ObstacleControl doesn't handle self-messages");
}

void ObstacleControl::addFromXml(cXMLElement* xml)
{
    std::string rootTag = xml->getTagName();
    if (rootTag != "obstacles") {
        throw cRuntimeError("Obstacle definition root tag was \"%s\", but expected \"obstacles\"", rootTag.c_str());
    }

    cXMLElementList list = xml->getChildren();
    for (cXMLElementList::const_iterator i = list.begin(); i != list.end(); ++i) {
        cXMLElement* e = *i;

        std::string tag = e->getTagName();

        if (tag == "type") {
            // <type id="building" db-per-cut="9" db-per-meter="0.4" />

            ASSERT(e->getAttribute("id"));
            std::string id = e->getAttribute("id");
            ASSERT(e->getAttribute("db-per-cut"));
            std::string perCutParS = e->getAttribute("db-per-cut");
            double perCutPar = strtod(perCutParS.c_str(), nullptr);
            ASSERT(e->getAttribute("db-per-meter"));
            std::string perMeterParS = e->getAttribute("db-per-meter");
            double perMeterPar = strtod(perMeterParS.c_str(), nullptr);

            perCut[id] = perCutPar;
            perMeter[id] = perMeterPar;
        }
        else if (tag == "poly") {

            // <poly id="building#0" type="building" color="#F00" shape="16,0 8,13.8564 -8,13.8564 -16,0 -8,-13.8564 8,-13.8564" />
            ASSERT(e->getAttribute("id"));
            std::string id = e->getAttribute("id");
            ASSERT(e->getAttribute("type"));
            std::string type = e->getAttribute("type");
            ASSERT(e->getAttribute("color"));
            std::string color = e->getAttribute("color");
            ASSERT(e->getAttribute("shape"));
            std::string shape = e->getAttribute("shape");

            Obstacle obs(id, type, getAttenuationPerCut(type), getAttenuationPerMeter(type));
            std::vector<Coord> sh;
            cStringTokenizer st(shape.c_str());
            while (st.hasMoreTokens()) {
                std::string xy = st.nextToken();
                std::vector<double> xya = cStringTokenizer(xy.c_str(), ",").asDoubleVector();
                ASSERT(xya.size() == 2);
                sh.push_back(Coord(xya[0], xya[1]));
            }
            obs.setShape(sh);
            add(obs);
        }
        else {
            throw cRuntimeError("Found unknown tag in obstacle definition: \"%s\"", tag.c_str());
        }
    }
}

void ObstacleControl::addFromTypeAndShape(std::string id, std::string typeId, std::vector<Coord> shape)
{
    if (!isTypeSupported(typeId)) {
        throw cRuntimeError("Unsupported obstacle type: \"%s\"", typeId.c_str());
    }
    Obstacle obs(id, typeId, getAttenuationPerCut(typeId), getAttenuationPerMeter(typeId));
    obs.setShape(shape);
    add(obs);
}

void ObstacleControl::add(Obstacle obstacle)
{
    Obstacle* o = new Obstacle(obstacle);
    obstacleOwner.emplace_back(o);

    // visualize using AnnotationManager
    if (annotations) o->visualRepresentation = annotations->drawPolygon(o->getShape(), "red", annotationGroup);

    cacheEntries.clear();
    isBboxLookupDirty = true;
}

void ObstacleControl::erase(const Obstacle* obstacle)
{
    if (annotations && obstacle->visualRepresentation) annotations->erase(obstacle->visualRepresentation);
    for (auto itOwner = obstacleOwner.begin(); itOwner != obstacleOwner.end(); ++itOwner) {
        // find owning pointer and remove it to deallocate obstacle
        if (itOwner->get() == obstacle) {
            obstacleOwner.erase(itOwner);
            break;
        }
    }

    cacheEntries.clear();
    isBboxLookupDirty = true;
}

std::vector<std::pair<veins::Obstacle*, std::vector<double>>> ObstacleControl::getIntersections(const Coord& senderPos, const Coord& receiverPos) const
{
    std::vector<std::pair<Obstacle*, std::vector<double>>> allIntersections;

    // rebuild bounding box lookup structure if dirty (new obstacles added recently)
    if (isBboxLookupDirty) {
        bboxLookup = rebuildBBoxLookup(obstacleOwner);
        isBboxLookupDirty = false;
    }

    auto candidateObstacles = bboxLookup.findOverlapping({senderPos.x, senderPos.y}, {receiverPos.x, receiverPos.y});

    // remove duplicates
    sort(candidateObstacles.begin(), candidateObstacles.end());
    candidateObstacles.erase(unique(candidateObstacles.begin(), candidateObstacles.end()), candidateObstacles.end());

    for (Obstacle* o : candidateObstacles) {
        // if obstacles has neither borders nor matter: bail.
        if (o->getShape().size() < 2) continue;
        auto foundIntersections = o->getIntersections(senderPos, receiverPos);
        if (!foundIntersections.empty() || o->containsPoint(senderPos) || o->containsPoint(receiverPos)) {
            allIntersections.emplace_back(o, foundIntersections);
        }
    }
    return allIntersections;
}

double ObstacleControl::calculateAttenuation(const Coord& senderPos, const Coord& receiverPos) const
{
    Enter_Method_Silent();

    if ((perCut.size() == 0) || (perMeter.size() == 0)) {
        throw cRuntimeError("Unable to use SimpleObstacleShadowing: No obstacle types have been configured");
    }
    if (obstacleOwner.size() == 0) {
        throw cRuntimeError("Unable to use SimpleObstacleShadowing: No obstacles have been added");
    }

    // return cached result, if available
    CacheKey cacheKey(senderPos, receiverPos);
    CacheEntries::const_iterator cacheEntryIter = cacheEntries.find(cacheKey);
    if (cacheEntryIter != cacheEntries.end()) {
        return cacheEntryIter->second;
    }

    // get intersections
    auto intersections = getIntersections(senderPos, receiverPos);

    double factor = 1;
    for (auto i = intersections.begin(); i != intersections.end(); ++i) {
        auto o = i->first;
        auto intersectAt = i->second;

        // if beam interacts with neither borders nor matter: bail.
        bool senderInside = o->containsPoint(senderPos);
        bool receiverInside = o->containsPoint(receiverPos);
        if ((intersectAt.size() == 0) && !senderInside && !receiverInside) continue;

        // remember number of cuts before messing with intersection points
        double numCuts = intersectAt.size();

        // for distance calculation, make sure every other pair of points marks transition through matter and void, respectively.
        if (senderInside) intersectAt.insert(intersectAt.begin(), 0);
        if (receiverInside) intersectAt.push_back(1);
        ASSERT((intersectAt.size() % 2) == 0);

        // sum up distances in matter.
        double fractionInObstacle = 0;
        for (auto i = intersectAt.begin(); i != intersectAt.end();) {
            double p1 = *(i++);
            double p2 = *(i++);
            fractionInObstacle += (p2 - p1);
        }

        // calculate attenuation
        double totalDistance = senderPos.distance(receiverPos);
        double attenuation = (o->getAttenuationPerCut() * numCuts) + (o->getAttenuationPerMeter() * fractionInObstacle * totalDistance);
        factor *= pow(10.0, -attenuation / 10.0);

        // bail if attenuation is already extremely high
        if (factor < 1e-30) break;
    }

    // cache result
    if (cacheEntries.size() >= 1000) cacheEntries.clear();
    cacheEntries[cacheKey] = factor;

    return factor;
}

double ObstacleControl::getAttenuationPerCut(std::string type)
{
    if (perCut.find(type) != perCut.end())
        return perCut[type];
    else {
        throw cRuntimeError("Obstacle type %s unknown", type.c_str());
        return -1;
    }
}

double ObstacleControl::getAttenuationPerMeter(std::string type)
{
    if (perMeter.find(type) != perMeter.end())
        return perMeter[type];
    else {
        throw cRuntimeError("Obstacle type %s unknown", type.c_str());
        return -1;
    }
}

bool ObstacleControl::isTypeSupported(std::string type)
{
    // the type of obstacle is supported if there are attenuation values for borders and interior
    return (perCut.find(type) != perCut.end()) && (perMeter.find(type) != perMeter.end());
}
