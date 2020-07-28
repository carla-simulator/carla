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
/// @file    NBNodeShapeComputer.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// This class computes shapes of junctions
/****************************************************************************/
#include <config.h>

#include <algorithm>
#include <iterator>
#include <utils/geom/PositionVector.h>
#include <utils/options/OptionsCont.h>
#include <utils/geom/GeomHelper.h>
#include <utils/common/StdDefs.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/ToString.h>
#include <utils/iodevices/OutputDevice.h>
#include "NBNode.h"
#include "NBNodeShapeComputer.h"

//#define DEBUG_NODE_SHAPE
//#define DEBUG_SMOOTH_CORNERS
//#define DEBUG_RADIUS
#define DEBUGCOND (myNode.getID() == "C")


#define EXT 100.0
#define EXT2 10.0

// foot- and bicycle paths as well as pure service roads should not get larget junctions
// railways also do have have junctions with sharp turns so can be excluded
const SVCPermissions NBNodeShapeComputer::SVC_LARGE_TURN(
        SVCAll & ~(SVC_BICYCLE | SVC_PEDESTRIAN | SVC_DELIVERY | SVC_RAIL_CLASSES));

// ===========================================================================
// method definitions
// ===========================================================================
NBNodeShapeComputer::NBNodeShapeComputer(const NBNode& node) :
    myNode(node),
    myRadius(node.getRadius()) {
}


NBNodeShapeComputer::~NBNodeShapeComputer() {}


PositionVector
NBNodeShapeComputer::compute() {
    PositionVector ret;
    // check whether the node is a dead end node or a node where only turning is possible
    //  in this case, we will use "computeNodeShapeSmall"
    bool singleDirection = false;
    if (myNode.getEdges().size() == 1) {
        singleDirection = true;
    }
    if (myNode.getEdges().size() == 2 && myNode.getIncomingEdges().size() == 1) {
        if (myNode.getIncomingEdges()[0]->isTurningDirectionAt(myNode.getOutgoingEdges()[0])) {
            singleDirection = true;
        }
    }
#ifdef DEBUG_NODE_SHAPE
    if (DEBUGCOND) {
        // annotate edges edges to make their ordering visible
        int i = 0;
        for (NBEdge* e : myNode.getEdges()) {
            e->setStreetName(toString(i));
            i++;
        }
    }
#endif
    if (singleDirection) {
        return computeNodeShapeSmall();
    }
    // check whether the node is a just something like a geometry
    //  node (one in and one out or two in and two out, pair-wise continuations)
    // also in this case "computeNodeShapeSmall" is used
    bool geometryLike = myNode.isSimpleContinuation(true, true);
    if (geometryLike) {
        // additionally, the angle between the edges must not be larger than 45 degrees
        //  (otherwise, we will try to compute the shape in a different way)
        const EdgeVector& incoming = myNode.getIncomingEdges();
        const EdgeVector& outgoing = myNode.getOutgoingEdges();
        double maxAngle = 0.;
        for (EdgeVector::const_iterator i = incoming.begin(); i != incoming.end(); ++i) {
            double ia = (*i)->getAngleAtNode(&myNode);
            for (EdgeVector::const_iterator j = outgoing.begin(); j != outgoing.end(); ++j) {
                double oa = (*j)->getAngleAtNode(&myNode);
                double ad = GeomHelper::getMinAngleDiff(ia, oa);
                if (22.5 >= ad) {
                    maxAngle = MAX2(ad, maxAngle);
                }
            }
        }
        if (maxAngle > 22.5) {
            return computeNodeShapeSmall();
        }
    }

    //
    ret = computeNodeShapeDefault(geometryLike);
    // fail fall-back: use "computeNodeShapeSmall"
    if (ret.size() < 3) {
        ret = computeNodeShapeSmall();
    }
    return ret;
}


void
computeSameEnd(PositionVector& l1, PositionVector& l2) {
    assert(l1[0].distanceTo2D(l1[1]) >= EXT);
    assert(l2[0].distanceTo2D(l2[1]) >= EXT);
    PositionVector tmp;
    tmp.push_back(PositionVector::positionAtOffset2D(l1[0], l1[1], EXT));
    tmp.push_back(l1[1]);
    tmp[1].sub(tmp[0]);
    tmp[1].set(-tmp[1].y(), tmp[1].x());
    tmp[1].add(tmp[0]);
    tmp.extrapolate2D(EXT);
    if (l2.intersects(tmp[0], tmp[1])) {
        const double offset = l2.intersectsAtLengths2D(tmp)[0];
        if (l2.length2D() - offset > POSITION_EPS) {
            PositionVector tl2 = l2.getSubpart2D(offset, l2.length2D());
            tl2.extrapolate2D(EXT);
            l2.erase(l2.begin(), l2.begin() + (l2.size() - tl2.size()));
            l2[0] = tl2[0];
        }
    }
}


PositionVector
NBNodeShapeComputer::computeNodeShapeDefault(bool simpleContinuation) {
    // if we have less than two edges, we can not compute the node's shape this way
    if (myNode.getEdges().size() < 2) {
        return PositionVector();
    }
    // magic values
    const OptionsCont& oc = OptionsCont::getOptions();
    const double defaultRadius = getDefaultRadius(oc);
    const bool useDefaultRadius = myNode.getRadius() == NBNode::UNSPECIFIED_RADIUS || myNode.getRadius() == defaultRadius;
    myRadius = (useDefaultRadius ? defaultRadius : myNode.getRadius());
    const double smallRadius = oc.getFloat("junctions.small-radius");
    const int cornerDetail = oc.getInt("junctions.corner-detail");
    const double sCurveStretch = oc.getFloat("junctions.scurve-stretch");
    const bool rectangularCut = oc.getBool("rectangular-lane-cut");
    const bool openDriveOutput = oc.isSet("opendrive-output");

    // Extend geometries to move the stop line forward.
    // In OpenDrive the junction starts whenever the geometry changes. Stop
    // line information is not given or ambiguous (sign positions at most)
    // In SUMO, stop lines are where the junction starts. This is computed
    // heuristically from intersecting the junctions roads geometries.
    const double advanceStopLine = oc.exists("opendrive-files") && oc.isSet("opendrive-files") ? oc.getFloat("opendrive.advance-stopline") : 0;


#ifdef DEBUG_NODE_SHAPE
    if (DEBUGCOND) {
        std::cout << "\ncomputeNodeShapeDefault node " << myNode.getID() << " simple=" << simpleContinuation << " useDefaultRadius=" << useDefaultRadius << " radius=" << myRadius << "\n";
    }
#endif

    // initialise
    EdgeVector::const_iterator i;
    // edges located in the value-vector have the same direction as the key edge
    std::map<NBEdge*, std::set<NBEdge*> > same;
    // the counter-clockwise boundary of the edge regarding possible same-direction edges
    GeomsMap geomsCCW;
    // the clockwise boundary of the edge regarding possible same-direction edges
    GeomsMap geomsCW;
    EdgeVector usedEdges = myNode.getEdges();
    computeEdgeBoundaries(usedEdges, geomsCCW, geomsCW);

    // check which edges are parallel
    joinSameDirectionEdges(usedEdges, same);
    // compute unique direction list
    EdgeVector newAll = computeUniqueDirectionList(usedEdges, same, geomsCCW, geomsCW);
    // if we have only two "directions", let's not compute the geometry using this method
    if (newAll.size() < 2) {
        return PositionVector();
    }

    // All geoms are outoing from myNode.
    // for every direction in newAll we compute the offset at which the
    // intersection ends and the edge starts. This value is saved in 'distances'
    // If the geometries need to be extended to get an intersection, this is
    // recorded in 'myExtended'
    std::map<NBEdge*, double> distances;
    std::map<NBEdge*, bool> myExtended;

    for (i = newAll.begin(); i != newAll.end(); ++i) {
        EdgeVector::const_iterator cwi = i;
        EdgeVector::const_iterator ccwi = i;
        double ccad;
        double cad;
        initNeighbors(newAll, i, geomsCW, geomsCCW, cwi, ccwi, cad, ccad);
        assert(geomsCCW.find(*i) != geomsCCW.end());
        assert(geomsCW.find(*ccwi) != geomsCW.end());
        assert(geomsCW.find(*cwi) != geomsCW.end());

        // there are only 2 directions and they are almost parallel
        if (*cwi == *ccwi &&
                (
                    // no change in lane numbers, even low angles still give a good intersection
                    (simpleContinuation && fabs(ccad - cad) < (double) 0.1)
                    // lane numbers change, a direct intersection could be far away from the node position
                    // so we use a larger threshold
                    || (!simpleContinuation && fabs(ccad - cad) < DEG2RAD(22.5)))
           ) {
            // compute the mean position between both edges ends ...
            Position p;
            if (myExtended.find(*ccwi) != myExtended.end()) {
                p = geomsCCW[*ccwi][0];
                p.add(geomsCW[*ccwi][0]);
                p.mul(0.5);
#ifdef DEBUG_NODE_SHAPE
                if (DEBUGCOND) {
                    std::cout << " extended: p=" << p << " angle=" << (ccad - cad) << "\n";
                }
#endif
            } else {
                p = geomsCCW[*ccwi][0];
                p.add(geomsCW[*ccwi][0]);
                p.add(geomsCCW[*i][0]);
                p.add(geomsCW[*i][0]);
                p.mul(0.25);
#ifdef DEBUG_NODE_SHAPE
                if (DEBUGCOND) {
                    std::cout << " unextended: p=" << p << " angle=" << (ccad - cad) << "\n";
                }
#endif
            }
            // ... compute the distance to this point ...
            double dist = MAX2(
                              geomsCCW[*i].nearest_offset_to_point2D(p),
                              geomsCW[*i].nearest_offset_to_point2D(p));
            if (dist < 0) {
                // ok, we have the problem that even the extrapolated geometry
                //  does not reach the point
                // in this case, the geometry has to be extenden... too bad ...
                // ... let's append the mean position to the geometry
                PositionVector g = (*i)->getGeometry();
                if (myNode.hasIncoming(*i)) {
                    g.push_back_noDoublePos(p);
                } else {
                    g.push_front_noDoublePos(p);
                }
                (*i)->setGeometry(g);
                // and rebuild previous information
                geomsCCW[*i] = (*i)->getCCWBoundaryLine(myNode);
                geomsCCW[*i].extrapolate(EXT);
                geomsCW[*i] = (*i)->getCWBoundaryLine(myNode);
                geomsCW[*i].extrapolate(EXT);
                // the distance is now = zero (the point we have appended)
                distances[*i] = EXT;
                myExtended[*i] = true;
#ifdef DEBUG_NODE_SHAPE
                if (DEBUGCOND) {
                    std::cout << " extending (dist=" << dist << ")\n";
                }
#endif
            } else {
                if (!simpleContinuation) {
                    dist += myRadius;
                } else {
                    // if the angles change, junction should have some size to avoid degenerate shape
                    double radius2 = fabs(ccad - cad) * (*i)->getNumLanes();
                    if (radius2 > NUMERICAL_EPS || openDriveOutput) {
                        radius2 = MAX2(0.15, radius2);
                    }
                    dist += radius2;
#ifdef DEBUG_NODE_SHAPE
                    if (DEBUGCOND) {
                        std::cout << " using radius=" << fabs(ccad - cad) * (*i)->getNumLanes() << " ccad=" << ccad << " cad=" << cad << "\n";
                    }
#endif
                }
                distances[*i] = dist;
            }

        } else {
            // the angles are different enough to compute the intersection of
            // the outer boundaries directly (or there are more than 2 directions). The "nearer" neighbar causes the furthest distance
            const bool ccwCloser = ccad < cad;
            const bool cwLargeTurn = needsLargeTurn(*i, *cwi, same);
            const bool ccwLargeTurn = needsLargeTurn(*i, *ccwi, same);
            const bool neighLargeTurn = ccwCloser ? ccwLargeTurn : cwLargeTurn;
            const bool neigh2LargeTurn =  ccwCloser ? cwLargeTurn : ccwLargeTurn;
            // the border facing the closer neighbor
            const PositionVector& currGeom = ccwCloser ? geomsCCW[*i] : geomsCW[*i];
            // the border facing the far neighbor
            const PositionVector& currGeom2 = ccwCloser ? geomsCW[*i] : geomsCCW[*i];
            // the border of the closer neighbor
            const PositionVector& neighGeom = ccwCloser ? geomsCW[*ccwi] : geomsCCW[*cwi];
            // the border of the far neighbor
            const PositionVector& neighGeom2 = ccwCloser ? geomsCCW[*cwi] : geomsCW[*ccwi];
#ifdef DEBUG_NODE_SHAPE
            if (DEBUGCOND) {
                std::cout << " i=" << (*i)->getID() << " neigh=" << (*ccwi)->getID() << " neigh2=" << (*cwi)->getID() << "\n";
                std::cout << "    ccwCloser=" << ccwCloser
                          << "\n      currGeom=" << currGeom << " neighGeom=" << neighGeom
                          << "\n      currGeom2=" << currGeom2 << " neighGeom2=" << neighGeom2
                          << "\n";
            }
#endif
            if (!simpleContinuation) {
                if (currGeom.intersects(neighGeom)) {
                    distances[*i] = (neighLargeTurn ? myRadius : smallRadius) + closestIntersection(currGeom, neighGeom, EXT);
#ifdef DEBUG_NODE_SHAPE
                    if (DEBUGCOND) {
                        std::cout << "   neigh intersects dist=" << distances[*i] << " currGeom=" << currGeom << " neighGeom=" << neighGeom << "\n";
                    }
#endif
                    if (*cwi != *ccwi && currGeom2.intersects(neighGeom2)) {
                        // also use the second intersection point
                        // but prevent very large node shapes
                        const double farAngleDist = ccwCloser ? cad : ccad;
                        double a1 = distances[*i];
                        double a2 = (neigh2LargeTurn ? myRadius : smallRadius) + closestIntersection(currGeom2, neighGeom2, EXT);
#ifdef DEBUG_NODE_SHAPE
                        if (DEBUGCOND) {
                            std::cout << "      neigh2 also intersects a1=" << a1 << " a2=" << a2 << " ccad=" << RAD2DEG(ccad) << " cad=" << RAD2DEG(cad) << " dist[cwi]=" << distances[*cwi] << " dist[ccwi]=" << distances[*ccwi] << " farAngleDist=" << RAD2DEG(farAngleDist) << " currGeom2=" << currGeom2 << " neighGeom2=" << neighGeom2 << "\n";
                        }
#endif
                        //if (RAD2DEG(farAngleDist) < 175) {
                        //    distances[*i] = MAX2(a1, MIN2(a2, a1 + 180 - RAD2DEG(farAngleDist)));
                        //}
                        if (a2 <= EXT) {
                            distances[*i] = MAX2(a1, a2);
                        } else if (ccad > DEG2RAD(90. + 45.) && cad > DEG2RAD(90. + 45.)) {
                            // do nothing.
                        } else if (farAngleDist < DEG2RAD(135) || (fabs(RAD2DEG(farAngleDist) - 180) > 1 && fabs(a2 - a1) < 10)) {
                            distances[*i] = MAX2(a1, a2);
                        }
#ifdef DEBUG_NODE_SHAPE
                        if (DEBUGCOND) {
                            std::cout << "   a1=" << a1 << " a2=" << a2 << " dist=" << distances[*i] << "\n";
                        }
#endif
                    }
                } else {
                    if (*cwi != *ccwi && currGeom2.intersects(neighGeom2)) {
                        distances[*i] = (neigh2LargeTurn ? myRadius : smallRadius) + currGeom2.intersectsAtLengths2D(neighGeom2)[0];
#ifdef DEBUG_NODE_SHAPE
                        if (DEBUGCOND) {
                            std::cout << "   neigh2 intersects dist=" << distances[*i] << " currGeom2=" << currGeom2 << " neighGeom2=" << neighGeom2 << "\n";
                        }
#endif
                    } else {
                        distances[*i] = EXT + myRadius;
#ifdef DEBUG_NODE_SHAPE
                        if (DEBUGCOND) {
                            std::cout << "   no intersects dist=" << distances[*i]  << " currGeom=" << currGeom << " neighGeom=" << neighGeom << " currGeom2=" << currGeom2 << " neighGeom2=" << neighGeom2 << "\n";
                        }
#endif
                    }
                }
            } else {
                if (currGeom.intersects(neighGeom)) {
                    distances[*i] = currGeom.intersectsAtLengths2D(neighGeom)[0];
                } else {
                    distances[*i] = (double) EXT;
                }
            }
        }
        if (useDefaultRadius && sCurveStretch > 0) {
            double sCurveWidth = myNode.getDisplacementError();
            if (sCurveWidth > 0) {
                const double sCurveRadius = myRadius + sCurveWidth / SUMO_const_laneWidth * sCurveStretch * pow((*i)->getSpeed(), 2 + sCurveStretch) / 1000;
                const double stretch = EXT + sCurveRadius - distances[*i];
                if (stretch > 0) {
                    distances[*i] += stretch;
                    // fixate extended geometry for repeated computation
                    const double shorten = distances[*i] - EXT;
                    (*i)->shortenGeometryAtNode(&myNode, shorten);
                    for (std::set<NBEdge*>::iterator k = same[*i].begin(); k != same[*i].end(); ++k) {
                        (*k)->shortenGeometryAtNode(&myNode, shorten);
                    }
#ifdef DEBUG_NODE_SHAPE
                    if (DEBUGCOND) {
                        std::cout << "   stretching junction: sCurveWidth=" << sCurveWidth << " sCurveRadius=" << sCurveRadius << " stretch=" << stretch << " dist=" << distances[*i]  << "\n";
                    }
#endif
                }
            }
        }
    }

    for (i = newAll.begin(); i != newAll.end(); ++i) {
        if (distances.find(*i) == distances.end()) {
            assert(false);
            distances[*i] = EXT;
        }
    }
    // prevent inverted node shapes
    // (may happen with near-parallel edges)
    const double minDistSum = 2 * (EXT + myRadius);
    for (i = newAll.begin(); i != newAll.end(); ++i) {
        if (distances[*i] < EXT && (*i)->hasDefaultGeometryEndpointAtNode(&myNode)) {
            for (EdgeVector::const_iterator j = newAll.begin(); j != newAll.end(); ++j) {
                if (distances[*j] > EXT && (*j)->hasDefaultGeometryEndpointAtNode(&myNode) && distances[*i] + distances[*j] < minDistSum) {
                    const double angleDiff = fabs(NBHelpers::relAngle((*i)->getAngleAtNode(&myNode), (*j)->getAngleAtNode(&myNode)));
                    if (angleDiff > 160 || angleDiff < 20) {
#ifdef DEBUG_NODE_SHAPE
                        if (DEBUGCOND) {
                            std::cout << "   increasing dist for i=" << (*i)->getID() << " because of j=" << (*j)->getID() << " jDist=" << distances[*j]
                                      << "  oldI=" << distances[*i] << " newI=" << minDistSum - distances[*j]
                                      << " angleDiff=" << angleDiff
                                      << " geomI=" << (*i)->getGeometry() << " geomJ=" << (*j)->getGeometry() << "\n";
                        }
#endif
                        distances[*i] = minDistSum - distances[*j];
                    }
                }
            }
        }
    }


    // build
    PositionVector ret;
    for (i = newAll.begin(); i != newAll.end(); ++i) {
        const PositionVector& ccwBound = geomsCCW[*i];
        const PositionVector& cwBound = geomsCW[*i];
        //double offset = MIN3(distances[*i], cwBound.length2D() - POSITION_EPS, ccwBound.length2D() - POSITION_EPS);
        double offset = distances[*i];
        if (!(*i)->hasDefaultGeometryEndpointAtNode(&myNode)) {
            // for non geometry-endpoints, only shorten but never extend the geometry
            if (advanceStopLine > 0 && offset < EXT) {
#ifdef DEBUG_NODE_SHAPE
                std::cout << " i=" << (*i)->getID() << " offset=" << offset << " advanceStopLine=" << advanceStopLine << "\n";
#endif
                // fixate extended geometry for repeated computation
                (*i)->extendGeometryAtNode(&myNode, advanceStopLine);
                for (std::set<NBEdge*>::iterator k = same[*i].begin(); k != same[*i].end(); ++k) {
                    (*k)->extendGeometryAtNode(&myNode, advanceStopLine);
                }
            }
            offset = MAX2(EXT - advanceStopLine, offset);
        }
        if (offset == -1) {
            WRITE_WARNING("Fixing offset for edge '" + (*i)->getID() + "' at node '" + myNode.getID() + ".");
            offset = (double) - .1;
        }
        Position p = ccwBound.positionAtOffset2D(offset);
        p.setz(myNode.getPosition().z());
        if (i != newAll.begin()) {
            ret.append(getSmoothCorner(geomsCW[*(i - 1)], ccwBound, ret[-1], p, cornerDetail));
        }
        ret.push_back_noDoublePos(p);
        //
        Position p2 = cwBound.positionAtOffset2D(offset);
        p2.setz(myNode.getPosition().z());
        ret.push_back_noDoublePos(p2);
#ifdef DEBUG_NODE_SHAPE
        if (DEBUGCOND) {
            std::cout << "   build stopLine for i=" << (*i)->getID() << " offset=" << offset << " dist=" << distances[*i] << " cwLength=" << cwBound.length2D() << " ccwLength=" << ccwBound.length2D() << " p=" << p << " p2=" << p2 << " ccwBound=" <<  ccwBound << " cwBound=" << cwBound << "\n";
        }
#endif
        (*i)->setNodeBorder(&myNode, p, p2, rectangularCut);
        for (std::set<NBEdge*>::iterator k = same[*i].begin(); k != same[*i].end(); ++k) {
            (*k)->setNodeBorder(&myNode, p, p2, rectangularCut);
        }
    }
    // final curve segment
    ret.append(getSmoothCorner(geomsCW[*(newAll.end() - 1)], geomsCCW[*newAll.begin()], ret[-1], ret[0], cornerDetail));
#ifdef DEBUG_NODE_SHAPE
    if (DEBUGCOND) {
        std::cout << " final shape=" << ret << "\n";
    }
#endif
    return ret;
}


double
NBNodeShapeComputer::closestIntersection(const PositionVector& geom1, const PositionVector& geom2, double offset) {
    std::vector<double> intersections = geom1.intersectsAtLengths2D(geom2);
    double result = intersections[0];
    for (std::vector<double>::iterator it = intersections.begin() + 1; it != intersections.end(); ++it) {
        if (fabs(*it - offset) < fabs(result - offset)) {
            result = *it;
        }
    }
    return result;
}

bool
NBNodeShapeComputer::needsLargeTurn(NBEdge* e1, NBEdge* e2,
        std::map<NBEdge*, std::set<NBEdge*> >& same) const {
    const SVCPermissions p1 = e1->getPermissions();
    const SVCPermissions p2 = e2->getPermissions();
    if ((p1 & p2 & SVC_LARGE_TURN) != 0) {
        // note: would could also check whether there is actually a connection
        // between those edges
        return true;
    }
    // maybe edges in the same direction need a large turn
    for (NBEdge* e2s : same[e2]) {
        if ((p1 & e2s->getPermissions() & SVC_LARGE_TURN) != 0
                && (e1->getToNode() == e2s->getFromNode() || e2s->getToNode() == e1->getFromNode())) {
            return true;
        }
        for (NBEdge* e1s : same[e1]) {
            if ((e2s->getPermissions() & e1s->getPermissions() & SVC_LARGE_TURN) != 0
                    && (e2s->getToNode() == e1s->getFromNode() || e1s->getToNode() == e2s->getFromNode())) {
                return true;
            }
        }
    }
    for (NBEdge* e1s : same[e1]) {
        if ((p2 & e1s->getPermissions() & SVC_LARGE_TURN) != 0
                && (e2->getToNode() == e1s->getFromNode() || e1s->getToNode() == e2->getFromNode())) {
            return true;
        }
    }
    //std::cout << " e1=" << e1->getID() << " e2=" << e2->getID() << " sameE1=" << toString(same[e1]) << " sameE2=" << toString(same[e2]) << "\n";
    return false;
}

PositionVector
NBNodeShapeComputer::getSmoothCorner(PositionVector begShape, PositionVector endShape,
                                     const Position& begPoint, const Position& endPoint, int cornerDetail) {
    PositionVector ret;
    if (cornerDetail > 0) {
        PositionVector begShape2 = begShape.reverse().getSubpart2D(EXT2, begShape.length());
        const double begSplit = begShape2.nearest_offset_to_point2D(begPoint, false);
#ifdef DEBUG_SMOOTH_CORNERS
        if (DEBUGCOND) {
            std::cout << " begLength=" << begShape2.length2D() << " begSplit=" << begSplit << "\n";
        }
#endif
        if (begSplit > POSITION_EPS && begSplit < begShape2.length2D() - POSITION_EPS) {
            begShape2 = begShape2.splitAt(begSplit, true).first;
        } else {
            return ret;
        }
        PositionVector endShape2 = endShape.getSubpart(0, endShape.length() - EXT2);
        const double endSplit = endShape2.nearest_offset_to_point2D(endPoint, false);
#ifdef DEBUG_SMOOTH_CORNERS
        if (DEBUGCOND) {
            std::cout << " endLength=" << endShape2.length2D() << " endSplit=" << endSplit << "\n";
        }
#endif
        if (endSplit > POSITION_EPS && endSplit < endShape2.length2D() - POSITION_EPS) {
            endShape2 = endShape2.splitAt(endSplit, true).second;
        } else {
            return ret;
        }
        // flatten z to junction z level
        begShape2 = begShape2.interpolateZ(myNode.getPosition().z(), myNode.getPosition().z());
        endShape2 = endShape2.interpolateZ(myNode.getPosition().z(), myNode.getPosition().z());
#ifdef DEBUG_SMOOTH_CORNERS
        if (DEBUGCOND) {
            std::cout << "getSmoothCorner begPoint=" << begPoint << " endPoint=" << endPoint
                      << " begShape=" << begShape << " endShape=" << endShape
                      << " begShape2=" << begShape2 << " endShape2=" << endShape2
                      << "\n";
        }
#endif
        if (begShape2.size() < 2 || endShape2.size() < 2) {
            return ret;
        }
        const double angle = GeomHelper::angleDiff(begShape2.angleAt2D(-2), endShape2.angleAt2D(0));
        NBNode* recordError = nullptr;
#ifdef DEBUG_SMOOTH_CORNERS
        if (DEBUGCOND) {
            std::cout << "   angle=" << RAD2DEG(angle) << "\n";
        }
        recordError = const_cast<NBNode*>(&myNode);
#endif
        // fill highly acute corners
        //if (fabs(angle) > DEG2RAD(135)) {
        //    return ret;
        //}
        PositionVector curve = myNode.computeSmoothShape(begShape2, endShape2, cornerDetail + 2, false, 25, 25, recordError, NBNode::AVOID_WIDE_LEFT_TURN);
        //PositionVector curve = myNode.computeSmoothShape(begShape2, endShape2, cornerDetail + 2, false, 25, 25, recordError, 0);
        const double curvature = curve.length2D() / MAX2(NUMERICAL_EPS, begPoint.distanceTo2D(endPoint));
#ifdef DEBUG_SMOOTH_CORNERS
        if (DEBUGCOND) {
            std::cout << "   curve=" << curve << " curveLength=" << curve.length2D() << " dist=" << begPoint.distanceTo2D(endPoint) << " curvature=" << curvature << "\n";
        }
#endif
        if (curvature > 2 && angle > DEG2RAD(85)) {
            // simplify dubious inside corner shape
            return ret;
        }
        if (curve.size() > 2) {
            curve.erase(curve.begin());
            curve.pop_back();
            ret = curve;
        }
    }
    return ret;
}

void
NBNodeShapeComputer::computeEdgeBoundaries(const EdgeVector& edges,
        GeomsMap& geomsCCW,
        GeomsMap& geomsCW) {
    // compute boundary lines and extend it by EXT m
    for (NBEdge* edge : edges) {
        // store current edge's boundary as current ccw/cw boundary
        try {
            geomsCCW[edge] = edge->getCCWBoundaryLine(myNode);
        } catch (InvalidArgument& e) {
            WRITE_WARNING("While computing intersection geometry at junction '" + myNode.getID() + "': " + std::string(e.what()));
            geomsCCW[edge] = edge->getGeometry();
        }
        try {
            geomsCW[edge] = edge->getCWBoundaryLine(myNode);
        } catch (InvalidArgument& e) {
            WRITE_WARNING("While computing intersection geometry at junction '" + myNode.getID() + "': " + std::string(e.what()));
            geomsCW[edge] = edge->getGeometry();
        }
        // ensure the boundary is valid
        if (geomsCCW[edge].length2D() < NUMERICAL_EPS) {
            geomsCCW[edge] = edge->getGeometry();
        }
        if (geomsCW[edge].length2D() < NUMERICAL_EPS) {
            geomsCW[edge] = edge->getGeometry();
        }
        // extend the boundary by extroplating it by EXT m
        geomsCCW[edge].extrapolate2D(EXT, true);
        geomsCW[edge].extrapolate2D(EXT, true);
        geomsCCW[edge].extrapolate(EXT2, false, true);
        geomsCW[edge].extrapolate(EXT2, false, true);
    }
}

void
NBNodeShapeComputer::joinSameDirectionEdges(const EdgeVector& edges, std::map<NBEdge*, std::set<NBEdge*> >& same) {
    // compute same (edges where an intersection doesn't work well
    // (always check an edge and its cw neightbor)
    // distance to look ahead for a misleading angle
    const double angleChangeLookahead = 35;
    EdgeSet foundOpposite;
    for (EdgeVector::const_iterator i = edges.begin(); i != edges.end(); i++) {
        EdgeVector::const_iterator j;
        if (i == edges.end() - 1) {
            j = edges.begin();
        } else {
            j = i + 1;
        }
        const bool incoming = (*i)->getToNode() == &myNode;
        const bool incoming2 = (*j)->getToNode() == &myNode;
        const Position positionAtNode = (*i)->getGeometry()[incoming ? -1 : 0];
        const Position positionAtNode2 = (*j)->getGeometry()[incoming2 ? -1 : 0];
        const PositionVector g1 = incoming ? (*i)->getCCWBoundaryLine(myNode) : (*i)->getCWBoundaryLine(myNode);
        const PositionVector g2 = incoming ? (*j)->getCCWBoundaryLine(myNode) : (*j)->getCWBoundaryLine(myNode);
        const double angle1further = (g1.size() > 2 && g1[0].distanceTo2D(g1[1]) < angleChangeLookahead ?
                                      g1.angleAt2D(1) : g1.angleAt2D(0));
        const double angle2further = (g2.size() > 2 && g2[0].distanceTo2D(g2[1]) < angleChangeLookahead ?
                                      g2.angleAt2D(1) : g2.angleAt2D(0));
        const double angleDiff = GeomHelper::angleDiff(g1.angleAt2D(0), g2.angleAt2D(0));
        const double angleDiffFurther = GeomHelper::angleDiff(angle1further, angle2further);
        const bool ambiguousGeometry = ((angleDiff > 0 && angleDiffFurther < 0) || (angleDiff < 0 && angleDiffFurther > 0));
        const bool differentDirs = (incoming != incoming2);
        //if (ambiguousGeometry) {
        //    @todo: this warning would be helpful in many cases. However, if angle and angleFurther jump between 179 and -179 it is misleading
        //    WRITE_WARNING("Ambigous angles at junction '" + myNode.getID() + "' for edges '" + (*i)->getID() + "' and '" + (*j)->getID() + "'.");
        //}
#ifdef DEBUG_NODE_SHAPE
        if (DEBUGCOND) {
            std::cout << "   checkSameDirection " << (*i)->getID() << " " << (*j)->getID()
                      << " diffDirs=" << differentDirs
                      << " isOpposite=" << (differentDirs && foundOpposite.count(*i) == 0)
                      << " angleDiff=" << angleDiff
                      << " ambiguousGeometry=" << ambiguousGeometry
                      << " badIntersect=" << badIntersection(*i, *j, EXT)
                      << "\n";

        }
#endif
        if (fabs(angleDiff) < DEG2RAD(20)) {
            const bool isOpposite = differentDirs && foundOpposite.count(*i) == 0;
            if (isOpposite) {
                foundOpposite.insert(*i);
                foundOpposite.insert(*j);
            }
            if (isOpposite || ambiguousGeometry || badIntersection(*i, *j, EXT)) {
                // maintain equivalence relation for all members of the equivalence class
                for (std::set<NBEdge*>::iterator k = same[*i].begin(); k != same[*i].end(); ++k) {
                    if (*j != *k) {
                        same[*k].insert(*j);
                        same[*j].insert(*k);
                    }
                }
                for (std::set<NBEdge*>::iterator k = same[*j].begin(); k != same[*j].end(); ++k) {
                    if (*i != *k) {
                        same[*k].insert(*i);
                        same[*i].insert(*k);
                    }
                }
                same[*i].insert(*j);
                same[*j].insert(*i);
#ifdef DEBUG_NODE_SHAPE
                if (DEBUGCOND) {
                    std::cout << "   joinedSameDirectionEdges " << (*i)->getID() << "   " << (*j)->getID() << " isOpposite=" << isOpposite << " ambiguousGeometry=" << ambiguousGeometry << "\n";
                }
#endif
            }
        }
    }
}


bool
NBNodeShapeComputer::badIntersection(const NBEdge* e1, const NBEdge* e2, double distance) {
    // check whether the two edges are on top of each other. In that case they should be joined
    // also, if they never touch along their common length
    const double commonLength = MIN3(distance, e1->getGeometry().length(), e2->getGeometry().length());
    PositionVector geom1 = e1->getGeometry();
    PositionVector geom2 = e2->getGeometry();
    // shift to make geom the centerline of the edge regardless of spreadtype
    if (e1->getLaneSpreadFunction() == LaneSpreadFunction::RIGHT) {
        geom1.move2side(e1->getTotalWidth() / 2);
    }
    if (e2->getLaneSpreadFunction() == LaneSpreadFunction::RIGHT) {
        geom2.move2side(e2->getTotalWidth() / 2);
    }
    // always let geometry start at myNode
    if (e1->getToNode() == &myNode) {
        geom1 = geom1.reverse();
    }
    if (e2->getToNode() == &myNode) {
        geom2 = geom2.reverse();
    }
    geom1 = geom1.getSubpart2D(0, commonLength);
    geom2 = geom2.getSubpart2D(0, commonLength);
    double endAngleDiff = 0;
    if (geom1.size() >= 2 && geom2.size() >= 2) {
        endAngleDiff = fabs(RAD2DEG(GeomHelper::angleDiff(
                                        geom1.angleAt2D((int)geom1.size() - 2),
                                        geom2.angleAt2D((int)geom2.size() - 2))));
    }
    const double minDistanceThreshold = (e1->getTotalWidth() + e2->getTotalWidth()) / 2 + POSITION_EPS;
    std::vector<double> distances = geom1.distances(geom2, true);
    const double minDist = VectorHelper<double>::minValue(distances);
    const double maxDist = VectorHelper<double>::maxValue(distances);
    const bool curvingTowards = geom1[0].distanceTo2D(geom2[0]) > minDistanceThreshold && minDist < minDistanceThreshold;
    const bool onTop = (maxDist - POSITION_EPS < minDistanceThreshold) && endAngleDiff < 30;
    geom1.extrapolate2D(EXT);
    geom2.extrapolate2D(EXT);
    Position intersect = geom1.intersectionPosition2D(geom2);
    const bool intersects = intersect != Position::INVALID && geom1.distance2D(intersect) < POSITION_EPS;
#ifdef DEBUG_NODE_SHAPE
    if (DEBUGCOND) {
        std::cout << "    badIntersect: onTop=" << onTop << " curveTo=" << curvingTowards << " intersects=" << intersects
                  << " endAngleDiff=" << endAngleDiff
                  << " geom1=" << geom1 << " geom2=" << geom2
                  << " distances=" << toString(distances) << " minDist=" << minDist << " maxDist=" << maxDist << " thresh=" << minDistanceThreshold
                  << " intersectPos=" << intersect
                  << "\n";
    }
#endif
    return onTop || curvingTowards || !intersects;
}


EdgeVector
NBNodeShapeComputer::computeUniqueDirectionList(
    const EdgeVector& all,
    std::map<NBEdge*, std::set<NBEdge*> >& same,
    GeomsMap& geomsCCW,
    GeomsMap& geomsCW) {
    // store relationships
    EdgeVector newAll = all;
    for (NBEdge* e1 : all) {
        // determine which of the edges marks the outer boundary
        auto e2NewAll = std::find(newAll.begin(), newAll.end(), e1);
#ifdef DEBUG_NODE_SHAPE
        if (DEBUGCOND) std::cout << "computeUniqueDirectionList e1=" << e1->getID()
                                     << " deleted=" << (e2NewAll == newAll.end())
                                     << " same=" << joinNamedToStringSorting(same[e1], ',') << "\n";
#endif
        if (e2NewAll == newAll.end()) {
            continue;
        }
        auto e1It = std::find(all.begin(), all.end(), e1);
        auto bestCCW = e1It;
        auto bestCW = e1It;
        bool changed = true;
        while (changed) {
            changed = false;
            for (NBEdge* e2 : same[e1]) {
#ifdef DEBUG_NODE_SHAPE
                if (DEBUGCOND) {
                    std::cout << "  e2=" << e2->getID() << "\n";
                }
#endif
                auto e2It = std::find(all.begin(), all.end(), e2);
                if (e2It + 1 == bestCCW || (e2It == (all.end() - 1) && bestCCW == all.begin())) {
                    bestCCW = e2It;
                    changed = true;
#ifdef DEBUG_NODE_SHAPE
                    if (DEBUGCOND) {
                        std::cout << "    bestCCW=" << e2->getID() << "\n";
                    }
#endif
                } else if (bestCW + 1 == e2It || (bestCW == (all.end() - 1) && e2It == all.begin())) {
                    bestCW = e2It;
                    changed = true;
#ifdef DEBUG_NODE_SHAPE
                    if (DEBUGCOND) {
                        std::cout << "    bestCW=" << e2->getID() << "\n";
                    }
#endif
                }
            }
        }
        if (bestCW != e1It) {
            geomsCW[e1] = geomsCW[*bestCW];
            computeSameEnd(geomsCW[e1], geomsCCW[e1]);
        }
        if (bestCCW != e1It) {
            geomsCCW[e1] = geomsCCW[*bestCCW];
            computeSameEnd(geomsCW[e1], geomsCCW[e1]);
        }
        // clean up
        for (NBEdge* e2 : same[e1]) {
            auto e2NewAll = std::find(newAll.begin(), newAll.end(), e2);
            if (e2NewAll != newAll.end()) {
                newAll.erase(e2NewAll);
            }
        }
    }
#ifdef DEBUG_NODE_SHAPE
    if (DEBUGCOND) {
        std::cout << "  newAll:\n";
        for (NBEdge* e : newAll) {
            std::cout << "    " << e->getID() << " geomCCW=" << geomsCCW[e] << " geomsCW=" << geomsCW[e] << "\n";
        }
    }
#endif
    return newAll;
}


void
NBNodeShapeComputer::initNeighbors(const EdgeVector& edges, const EdgeVector::const_iterator& current,
                                   GeomsMap& geomsCW,
                                   GeomsMap& geomsCCW,
                                   EdgeVector::const_iterator& cwi,
                                   EdgeVector::const_iterator& ccwi,
                                   double& cad,
                                   double& ccad) {
    const double twoPI = (double)(2 * M_PI);
    cwi = current;
    cwi++;
    if (cwi == edges.end()) {
        std::advance(cwi, -((int)edges.size())); // set to edges.begin();
    }
    ccwi = current;
    if (ccwi == edges.begin()) {
        std::advance(ccwi, edges.size() - 1); // set to edges.end() - 1;
    } else {
        ccwi--;
    }

    const double angleCurCCW = geomsCCW[*current].angleAt2D(0);
    const double angleCurCW = geomsCW[*current].angleAt2D(0);
    const double angleCCW = geomsCW[*ccwi].angleAt2D(0);
    const double angleCW = geomsCCW[*cwi].angleAt2D(0);
    ccad = angleCCW - angleCurCCW;
    while (ccad < 0.) {
        ccad += twoPI;
    }
    cad = angleCurCW - angleCW;
    while (cad < 0.) {
        cad += twoPI;
    }
}



PositionVector
NBNodeShapeComputer::computeNodeShapeSmall() {
#ifdef DEBUG_NODE_SHAPE
    if (DEBUGCOND) {
        std::cout << "computeNodeShapeSmall node=" << myNode.getID() << "\n";
    }
#endif
    PositionVector ret;
    for (NBEdge* e : myNode.getEdges()) {
        // compute crossing with normal
        PositionVector edgebound1 = e->getCCWBoundaryLine(myNode).getSubpartByIndex(0, 2);
        PositionVector edgebound2 = e->getCWBoundaryLine(myNode).getSubpartByIndex(0, 2);
        Position delta = edgebound1[1] - edgebound1[0];
        delta.set(-delta.y(), delta.x()); // rotate 90 degrees
        PositionVector cross(myNode.getPosition(), myNode.getPosition() + delta);
        cross.extrapolate2D(500);
        edgebound1.extrapolate2D(500);
        edgebound2.extrapolate2D(500);
        if (cross.intersects(edgebound1)) {
            Position np = cross.intersectionPosition2D(edgebound1);
            np.set(np.x(), np.y(), myNode.getPosition().z());
            ret.push_back_noDoublePos(np);
        }
        if (cross.intersects(edgebound2)) {
            Position np = cross.intersectionPosition2D(edgebound2);
            np.set(np.x(), np.y(), myNode.getPosition().z());
            ret.push_back_noDoublePos(np);
        }
        e->resetNodeBorder(&myNode);
    }
    return ret;
}


double
NBNodeShapeComputer::getDefaultRadius(const OptionsCont& oc) {
    // look for incoming/outgoing edge pairs that do not go straight and allow wide vehicles
    // (connection information is not available yet)
    // @TODO compute the radius for each pair of neighboring edge intersections in computeNodeShapeDefault rather than use the maximum
    const double radius = oc.getFloat("default.junctions.radius");
    const double smallRadius = oc.getFloat("junctions.small-radius");
    double maxRightAngle = 0; // rad
    double extraWidthRight = 0; // m
    double maxLeftAngle = 0; // rad
    double extraWidthLeft = 0; // m
    int laneDelta = 0;
    for (NBEdge* in : myNode.getIncomingEdges()) {
        int wideLanesIn = 0;
        for (int i = 0; i < in->getNumLanes(); i++) {
            if ((in->getPermissions(i) & SVC_LARGE_TURN) != 0) {
                wideLanesIn++;
            }
        }
        for (NBEdge* out : myNode.getOutgoingEdges()) {
            if ((in->getPermissions() & out->getPermissions() & SVC_LARGE_TURN) != 0) {
                if (myNode.getDirection(in, out) == LinkDirection::TURN) {
                    continue;
                };
                const double angle = GeomHelper::angleDiff(
                                         in->getGeometry().angleAt2D(-2),
                                         out->getGeometry().angleAt2D(0));
                if (angle < 0) {
                    if (maxRightAngle < -angle) {
                        maxRightAngle = -angle;
                        extraWidthRight = MAX2(getExtraWidth(in, SVC_LARGE_TURN), getExtraWidth(out, SVC_LARGE_TURN));
                    }
                } else {
                    if (maxLeftAngle < angle) {
                        maxLeftAngle = angle;
                        // all edges clockwise between in and out count as extra width
                        extraWidthLeft = 0;
                        EdgeVector::const_iterator pIn = std::find(myNode.getEdges().begin(), myNode.getEdges().end(), in);
                        NBContHelper::nextCW(myNode.getEdges(), pIn);
                        while (*pIn != out) {
                            extraWidthLeft += (*pIn)->getTotalWidth();
//#ifdef DEBUG_RADIUS
//                            if (DEBUGCOND) {
//                                std::cout << "   in=" << in->getID() << " out=" << out->getID() << " extra=" << (*pIn)->getID() << " extraWidthLeft=" << extraWidthLeft << "\n";
//                            }
//#endif
                            NBContHelper::nextCW(myNode.getEdges(), pIn);
                        }
                    }
                }
                int wideLanesOut = 0;
                for (int i = 0; i < out->getNumLanes(); i++) {
                    if ((out->getPermissions(i) & SVC_LARGE_TURN) != 0) {
                        wideLanesOut++;
                    }
                }
                laneDelta = MAX2(laneDelta, abs(wideLanesOut - wideLanesIn));
            }
        }
    }
    // changing the number of wide-vehicle lanes on a straight segment requires a larger junction to allow for smooth driving
    // otherwise we can reduce the radius according to the angle
    double result = radius;
    // left turns are assumed to cross additional edges and thus du not determine the required radius in most cases
    double maxTurnAngle = maxRightAngle;
    double extraWidth = extraWidthRight;
    if (maxRightAngle < DEG2RAD(5)) {
        maxTurnAngle = maxLeftAngle;
        extraWidth = extraWidthLeft;
    }
    if (laneDelta == 0 || maxTurnAngle >= DEG2RAD(30) || myNode.isConstantWidthTransition()) {
        // subtract radius gained from extra lanes
        // do not increase radius for turns that are sharper than a right angle
        result = MAX2(smallRadius, radius * tan(0.5 * MIN2(0.5 * M_PI, maxTurnAngle)) - extraWidth);
    }
#ifdef DEBUG_RADIUS
    if (DEBUGCOND) {
        std::cout << "getDefaultRadius n=" << myNode.getID() << " laneDelta=" << laneDelta
                  << " rightA=" << RAD2DEG(maxRightAngle)
                  << " leftA=" << RAD2DEG(maxLeftAngle)
                  << " maxA=" << RAD2DEG(maxTurnAngle)
                  << " extraWidth=" << extraWidth
                  << " result=" << result << "\n";
    }
#endif
    return result;
}


double
NBNodeShapeComputer::getExtraWidth(const NBEdge* e, SVCPermissions exclude) {
    double result = 0;
    int lane = 0;
    while (lane < e->getNumLanes() && e->getPermissions(lane) == 0) {
        // ignore forbidden lanes out the outside
        lane++;
    }
    while (lane < e->getNumLanes() && (e->getPermissions(lane) & exclude) == 0) {
        result += e->getLaneWidth(lane);
        lane++;
    }
    return result;
}


/****************************************************************************/
