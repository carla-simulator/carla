/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2001-2020 German Aerospace Center (DLR) and others.
// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// https://www.eclipse.org/legal/epl-2.0/
// This Source Code may also be made available under the following Secondary
// Licenses when the conditions for such availability set forth in the Eclipse
// Public License 2.0 are satisfied: GNU General Public License, version 2
// or later which is available at
// https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
// SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later
/****************************************************************************/
/// @file    NBPTStopCont.cpp
/// @author  Gregor Laemmel
/// @date    Tue, 20 Mar 2017
///
// Container for pt stops during the netbuilding process
/****************************************************************************/


#include <utils/common/MsgHandler.h>
#include <utils/geom/Boundary.h>
#include <utils/options/OptionsCont.h>
#include <microsim/MSLane.h>
#include "NBPTStopCont.h"
#include "NBEdgeCont.h"
#include "NBEdge.h"
#include "NBNode.h"
#include <utils/geom/Position.h>


NBPTStopCont::~NBPTStopCont() {
    for (auto& myPTStop : myPTStops) {
        delete myPTStop.second;
    }
    myPTStops.clear();
}


bool
NBPTStopCont::insert(NBPTStop* ptStop) {
    std::string id = ptStop->getID();
    auto i = myPTStops.find(id);
    if (i != myPTStops.end()) {
        return false;
    }
    myPTStops[id] = ptStop;
    return true;
}


NBPTStop*
NBPTStopCont::get(std::string id) {
    if (myPTStops.find(id) != myPTStops.end()) {
        return myPTStops.find(id)->second;
    }
    return nullptr;
}


void
NBPTStopCont::localizePTStops(NBEdgeCont& cont) {
    std::vector<NBPTStop*> reverseStops;
    //first pass localize pt stop at correct side of the street; create stop for opposite side if needed
    for (auto& myPTStop : myPTStops) {

        NBPTStop* stop = myPTStop.second;

        bool multipleStopPositions = stop->getIsMultipleStopPositions();
        bool platformsDefined = !stop->getPlatformCands().empty();
        if (!platformsDefined) {
            //create pt stop for reverse edge if edge exists
            NBPTStop* reverseStop = getReverseStop(stop, cont);
            if (reverseStop != nullptr) {
                reverseStops.push_back(reverseStop);
            }
        } else if (multipleStopPositions) {
            //create pt stop for closest platform at corresponding edge
            assignPTStopToEdgeOfClosestPlatform(stop, cont);

        } else {
            //create pt stop for each side of the street where a platform is defined (create additional pt stop as needed)
            NBPTStop* additionalStop = assignAndCreatNewPTStopAsNeeded(stop, cont);
            if (additionalStop != nullptr) {
                reverseStops.push_back(additionalStop);
            }
        }
    }

    //insrt new stops if any
    for (auto& reverseStop : reverseStops) {
        insert(reverseStop);
    }
}


void NBPTStopCont::assignLanes(NBEdgeCont& cont) {
    //scnd pass set correct lane
    for (auto i = myPTStops.begin(); i != myPTStops.end();) {
        NBPTStop* stop = i->second;

        if (!stop->findLaneAndComputeBusStopExtent(cont)) {
            WRITE_WARNING("Could not find corresponding edge or compatible lane for pt stop '" + i->first
                          + "' (" + i->second->getName() + "). Thus, it will be removed!");
            EdgeVector edgeVector = cont.getGeneratedFrom((*i).second->getOrigEdgeId());
            //std::cout << edgeVector.size() << std::endl;
            myPTStops.erase(i++);
        } else {
            i++;
        }
    }
}


int
NBPTStopCont::generateBidiStops(NBEdgeCont& ec) {
    //scnd pass set correct lane
    std::vector<NBPTStop*> toAdd;
    for (auto i = myPTStops.begin(); i != myPTStops.end(); i++) {
        NBPTStop* stop = i->second;
        NBEdge* edge = ec.getByID(stop->getEdgeId());
        if (edge != nullptr && edge->isBidiRail()) {
            NBEdge* bidiEdge = edge->getTurnDestination(true);
            assert(bidiEdge != 0);
            const std::string id = getReverseID(stop->getID());
            if (myPTStops.count(id) > 0) {
                if (myPTStops[id]->getEdgeId() != bidiEdge->getID()) {
                    WRITE_WARNING("Could not create reverse-direction stop for superposed edge '" + bidiEdge->getID()
                                  + "' (origStop '" + i->first + "'). Stop id '" + id
                                  + "' already in use by stop on edge '" + myPTStops[id]->getEdgeId() + "'.");
                }
                continue;
            }
            NBPTStop* bidiStop = new NBPTStop(id,
                                              stop->getPosition(),
                                              bidiEdge->getID(),
                                              stop->getOrigEdgeId(),
                                              stop->getLength(),
                                              stop->getName(),
                                              stop->getPermissions());
            if (bidiStop->findLaneAndComputeBusStopExtent(ec)) {
                toAdd.push_back(bidiStop);
                stop->setBidiStop(bidiStop);
                bidiStop->setBidiStop(stop);
            } else {
                // should not happen
                assert(false);
            }
        }
    }
    for (NBPTStop* newStop : toAdd) {
        myPTStops[newStop->getID()] = newStop;
    }
    if (toAdd.size() > 0) {
        WRITE_MESSAGE("Added " + toString(toAdd.size()) + " stops for superposed rail edges.");
    }
    return (int)toAdd.size();
}


NBPTStop*
NBPTStopCont::getReverseStop(NBPTStop* pStop, NBEdgeCont& cont) {
    std::string edgeId = pStop->getEdgeId();
    NBEdge* edge = cont.getByID(edgeId);
    NBEdge* reverse = NBPTStopCont::getReverseEdge(edge);
    if (reverse != nullptr) {
        const std::string reverseID = getReverseID(pStop->getID());
        if (myPTStops.count(reverseID) == 0) {
            return new NBPTStop(reverseID, pStop->getPosition(), reverse->getID(), reverse->getID(),
                                pStop->getLength(), pStop->getName(), pStop->getPermissions());
        } else {
            return myPTStops[reverseID];
        }
    }
    return nullptr;
}


NBPTStop*
NBPTStopCont::assignAndCreatNewPTStopAsNeeded(NBPTStop* pStop, NBEdgeCont& cont) {
    std::string edgeId = pStop->getEdgeId();
    NBEdge* edge = cont.getByID(edgeId);
    bool rightOfEdge = false;
    bool leftOfEdge = false;
    const NBPTPlatform* left = nullptr;
    for (const NBPTPlatform& platform : pStop->getPlatformCands()) {
        double crossProd = computeCrossProductEdgePosition(edge, platform.getPos());
        //TODO consider driving on the left!!! [GL May '17]
        if (crossProd > 0) {
            leftOfEdge = true;
            left = &platform;
        } else {
            rightOfEdge = true;
            pStop->setMyPTStopLength(platform.getLength());
        }
    }

    if (leftOfEdge && rightOfEdge) {
        NBPTStop* leftStop = getReverseStop(pStop, cont);
        leftStop->setMyPTStopLength(left->getLength());
        return leftStop;
    } else if (leftOfEdge) {
        NBEdge* reverse = getReverseEdge(edge);
        if (reverse != nullptr) {
            pStop->setEdgeId(reverse->getID(), cont);
            pStop->setMyPTStopLength(left->getLength());
        }
    }

    return nullptr;
}


void
NBPTStopCont::assignPTStopToEdgeOfClosestPlatform(NBPTStop* pStop, NBEdgeCont& cont) {
    std::string edgeId = pStop->getEdgeId();
    NBEdge* edge = cont.getByID(edgeId);
    NBEdge* reverse = NBPTStopCont::getReverseEdge(edge);
    const NBPTPlatform* closestPlatform = getClosestPlatformToPTStopPosition(pStop);
    pStop->setMyPTStopLength(closestPlatform->getLength());
    if (reverse != nullptr) {

        //TODO make isLeft in PositionVector static [GL May '17]
//        if (PositionVector::isLeft(edge->getFromNode()->getPosition(),edge->getToNode()->getPosition(),closestPlatform)){
//
//        }
        double crossProd = computeCrossProductEdgePosition(edge, closestPlatform->getPos());

        //TODO consider driving on the left!!! [GL May '17]
        if (crossProd > 0) { //pt stop is on the left of the orig edge
            pStop->setEdgeId(reverse->getID(), cont);
        }
    }
}


double
NBPTStopCont::computeCrossProductEdgePosition(const NBEdge* edge, const Position& closestPlatform) const {
    PositionVector geom = edge->getGeometry();
    int idxTmp = geom.indexOfClosest(closestPlatform);
    double offset = geom.nearest_offset_to_point2D(closestPlatform, true);
    double offset2 = geom.offsetAtIndex2D(idxTmp);
    int idx1, idx2;
    if (offset2 < offset) {
        idx1 = idxTmp;
        idx2 = idx1 + 1;
    } else {
        idx2 = idxTmp;
        idx1 = idxTmp - 1;
    }
    if (idx1 < 0 || idx1 >= (int) geom.size() || idx2 < 0 || idx2 >= (int) geom.size()) {
        WRITE_WARNING("Could not determine cross product");
        return 0;
    }
    Position p1 = geom[idx1];
    Position p2 = geom[idx2];

    double x0 = p1.x();
    double y0 = p1.y();
    double x1 = p2.x();
    double y1 = p2.y();
    double x2 = closestPlatform.x();
    double y2 = closestPlatform.y();
    double crossProd = (x1 - x0) * (y2 - y0) - (y1 - y0) * (x2 - x0);
    return crossProd;
}


const NBPTPlatform*
NBPTStopCont::getClosestPlatformToPTStopPosition(NBPTStop* pStop) {
    Position stopPosition = pStop->getPosition();
    const NBPTPlatform* closest = nullptr;
    double minSqrDist = std::numeric_limits<double>::max();
    for (const NBPTPlatform& platform : pStop->getPlatformCands()) {
        double sqrDist = stopPosition.distanceSquaredTo2D(platform.getPos());
        if (sqrDist < minSqrDist) {
            minSqrDist = sqrDist;
            closest = &platform;
        }
    }
    return closest;
}

//static functions

NBEdge*
NBPTStopCont::getReverseEdge(NBEdge* edge) {
    if (edge != nullptr) {
        for (auto it = edge->getToNode()->getOutgoingEdges().begin();
                it != edge->getToNode()->getOutgoingEdges().end();
                it++) {
            if ((*it)->getToNode() == edge->getFromNode()) {
                return (*it);
            }
        }
    }
    return nullptr;
}


int
NBPTStopCont::cleanupDeleted(NBEdgeCont& cont) {
    int numDeleted = 0;
    for (auto i = myPTStops.begin(); i != myPTStops.end();) {
        if (cont.getByID((*i).second->getEdgeId()) == nullptr) {
            WRITE_WARNING("Removing pt stop:" + (*i).first + " on non existing edge: " + (*i).second->getEdgeId());
            myPTStops.erase(i++);
            numDeleted++;
        } else {
            i++;
        }
    }
    return numDeleted;
}


void
NBPTStopCont::addEdges2Keep(const OptionsCont& oc, std::set<std::string>& into) {
    if (oc.isSet("ptstop-output")) {
        for (auto stop : myPTStops) {
            into.insert(stop.second->getEdgeId());
        }
    }
}


void
NBPTStopCont::postprocess(std::set<std::string>& usedStops) {
    for (auto i = myPTStops.begin(); i != myPTStops.end();) {
        if (usedStops.find(i->second->getID()) == usedStops.end()) {
            myPTStops.erase(i++);
        } else {
            i++;
        }
    }
}

std::string
NBPTStopCont::getReverseID(const std::string& id) {
    return id.size() > 0 && id[0] == '-' ? id.substr(1) : "-" + id;
}

void
NBPTStopCont::alignIdSigns() {
    PTStopsCont stops = myPTStops;
    for (auto& i : stops) {
        const std::string& stopId = i.second->getID();
        if (i.second->getEdgeId() == "") {
            continue;
        }
        const char edgeSign = i.second->getEdgeId().at(0);
        const char stopSign = stopId.at(0);
        if (edgeSign != stopSign && (edgeSign == '-' || stopSign == '-')) {
            i.second->setMyPTStopId(getReverseID(stopId));
            myPTStops.erase(stopId);
            myPTStops[i.second->getID()] = i.second;
        }
    }
}


void
NBPTStopCont::findAccessEdgesForRailStops(NBEdgeCont& cont, double maxRadius, int maxCount, double accessFactor) {
    NamedRTree r;
    for (auto edge : cont) {
        const Boundary& bound = edge.second->getGeometry().getBoxBoundary();
        float min[2] = { static_cast<float>(bound.xmin()), static_cast<float>(bound.ymin()) };
        float max[2] = { static_cast<float>(bound.xmax()), static_cast<float>(bound.ymax()) };
        r.Insert(min, max, edge.second);
    }
    for (auto& ptStop : myPTStops) {
        const std::string& stopEdgeID = ptStop.second->getEdgeId();
        NBEdge* stopEdge = cont.getByID(stopEdgeID);
        //std::cout << "findAccessEdgesForRailStops edge=" << stopEdgeID << " exists=" << (stopEdge != 0) << "\n";
        if (stopEdge != nullptr && (stopEdge->getPermissions() & SVC_PEDESTRIAN) == 0) {
            //if (stopEdge != 0 && isRailway(stopEdge->getPermissions())) {
            std::set<const Named*> edges;
            Named::StoringVisitor visitor(edges);
            const Position& pos = ptStop.second->getPosition();
            float min[2] = {static_cast<float>(pos.x() - maxRadius), static_cast<float>(pos.y() - maxRadius)};
            float max[2] = {static_cast<float>(pos.x() + maxRadius), static_cast<float>(pos.y() + maxRadius)};
            r.Search(min, max, visitor);
            std::vector<NBEdge*> edgCants;
            for (const Named* namedEdge : edges) {
                NBEdge* e = const_cast<NBEdge*>(dynamic_cast<const NBEdge*>(namedEdge));
                edgCants.push_back(e);
            }
            std::sort(edgCants.begin(), edgCants.end(), [pos](NBEdge * a, NBEdge * b) {
                return a->getLaneShape(0).distance2D(pos, false) < b->getLaneShape(0).distance2D(pos, false);
            });
            int cnt = 0;
            for (auto edge : edgCants) {
                int laneIdx = 0;
                for (auto lane : edge->getLanes()) {
                    if ((lane.permissions & SVC_PEDESTRIAN) != 0) {
                        double offset = lane.shape.nearest_offset_to_point2D(pos, false);
                        double finalLength = edge->getFinalLength();
                        double laneLength = lane.shape.length();
                        double accessLength = pos.distanceTo2D(lane.shape.positionAtOffset2D(offset)) * accessFactor;
                        ptStop.second->addAccess(edge->getLaneID(laneIdx), offset * finalLength / laneLength, accessLength);
                        cnt++;
                        break;
                    }
                    laneIdx++;
                }
                if (cnt == maxCount) {
                    break;
                }
            }
        }
    }
}


NBPTStop*
NBPTStopCont::findStop(const std::string& origEdgeID, Position pos, double threshold) const {
    for (auto& item : myPTStops) {
        if (item.second->getOrigEdgeId() == origEdgeID &&
                item.second->getPosition().distanceTo2D(pos) < threshold) {
            return item.second;
        }
    }
    return nullptr;
}


/****************************************************************************/
