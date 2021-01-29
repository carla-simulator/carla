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
/// @file    GNEPathElements.cpp
/// @author  Pablo Alvarez Lopez
/// @date    May 2020
///
// A abstract class for representation of element paths
/****************************************************************************/
#include <config.h>

#include <netedit/GNENet.h>

#include "GNEPathElements.h"


// ===========================================================================
// member method definitions
// ===========================================================================

// ---------------------------------------------------------------------------
// GNEPathElements::PathElement - methods
// ---------------------------------------------------------------------------

GNEPathElements::PathElement::PathElement(GNELane* lane) :
    myLane(lane),
    myNextLane(nullptr) {
}


void
GNEPathElements::PathElement::updateNextLane(GNELane* lane) {
    myNextLane = lane;
}


GNEJunction*
GNEPathElements::PathElement::getJunction() const {
    if (myNextLane) {
        return myLane->getParentEdge()->getParentJunctions().back();
    } else {
        return nullptr;
    }
}


GNELane*
GNEPathElements::PathElement::getLane() const {
    return myLane;
}


GNELane*
GNEPathElements::PathElement::getNextLane() const {
    return myNextLane;
}


GNEPathElements::PathElement::PathElement():
    myLane(nullptr),
    myNextLane(nullptr) {
}

// ---------------------------------------------------------------------------
// GNEPathElements - methods
// ---------------------------------------------------------------------------

GNEPathElements::GNEPathElements(GNEAdditional* additional) :
    myAdditionalElement(additional),
    myDemandElement(nullptr),
    myGenericData(nullptr) {
}


GNEPathElements::GNEPathElements(GNEDemandElement* demandElement) :
    myAdditionalElement(nullptr),
    myDemandElement(demandElement),
    myGenericData(nullptr) {
}


GNEPathElements::GNEPathElements(GNEGenericData* genericData) :
    myAdditionalElement(nullptr),
    myDemandElement(nullptr),
    myGenericData(genericData) {
}


GNEPathElements::~GNEPathElements() {}


const std::vector<GNEPathElements::PathElement>&
GNEPathElements::getPath() const {
    return myPathElements;
}


void
GNEPathElements::drawLanePathChildren(const GUIVisualizationSettings& s, const GNELane* lane, const double offset) const {
    // additionals
    if (myAdditionalElement) {
        for (const auto& pathElement : myPathElements) {
            if (pathElement.getLane() == lane) {
                myAdditionalElement->drawPartialGL(s, lane, offset);
            }
        }
    }
    // demand elements
    if (myDemandElement) {
        for (const auto& pathElement : myPathElements) {
            if (pathElement.getLane() == lane) {
                myDemandElement->drawPartialGL(s, lane, offset);
            }
        }
    }
    // generic datas (only in supermode Data)
    if (myGenericData && myGenericData->isGenericDataVisible()) {
        for (const auto& pathElement : myPathElements) {
            if (pathElement.getLane() == lane) {
                myGenericData->drawPartialGL(s, lane, offset);
            }
        }
    }
}


void
GNEPathElements::drawJunctionPathChildren(const GUIVisualizationSettings& s, const GNEJunction* junction, const double offset) const {
    // additionals
    if (myAdditionalElement) {
        for (const auto& pathElement : myPathElements) {
            if (pathElement.getJunction() == junction) {
                myAdditionalElement->drawPartialGL(s, pathElement.getLane(), pathElement.getNextLane(), offset);
            }
        }
    }
    // demand elements
    if (myDemandElement) {
        for (const auto& pathElement : myPathElements) {
            if (pathElement.getJunction() == junction) {
                myDemandElement->drawPartialGL(s, pathElement.getLane(), pathElement.getNextLane(), offset);
            }
        }
    }
    // generic datas
    if (myGenericData && myGenericData->isGenericDataVisible()) {
        for (const auto& pathElement : myPathElements) {
            if (pathElement.getJunction() == junction) {
                myGenericData->drawPartialGL(s, pathElement.getLane(), pathElement.getNextLane(), offset);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// GNEPathElements - protected methods
// ---------------------------------------------------------------------------

void
GNEPathElements::calculatePathLanes(SUMOVehicleClass vClass, const bool allowedVClass, GNELane* fromLane, GNELane* toLane, const std::vector<GNEEdge*>& /* viaEdges */) {
    // check if from and to lane are valid
    if (fromLane && toLane) {
        // remove path elements from lanes and junctions
        removeElements();
        // get from-via-to edges
        const std::vector<GNEEdge*> edges = calculateFromViaToEdges(fromLane, toLane, edges);
        // calculate path
        const std::vector<GNEEdge*> path = myDemandElement->getNet()->getPathCalculator()->calculatePath(vClass, edges);
        // set new path lanes
        myPathElements.clear();
        // check if path was sucesfully calculated
        if (path.size() > 0) {
            for (int i = 0; i < (int)path.size(); i++) {
                if (i == 0) {
                    myPathElements.push_back(fromLane);
                } else if (i == (int)path.size()) {
                    myPathElements.push_back(toLane);
                } else if (allowedVClass) {
                    myPathElements.push_back(path.at(i)->getLaneByAllowedVClass(vClass));
                } else {
                    myPathElements.push_back(path.at(i)->getLaneByDisallowedVClass(vClass));
                }
            }
        } else {
            myPathElements = {fromLane, toLane};
        }
        // update path elements
        updatePathElements();
        // add path elements in lanes and junctions
        addElements();
    }
}


void
GNEPathElements::calculateConsecutivePathLanes(SUMOVehicleClass vClass, const bool allowedVClass, const std::vector<GNEEdge*>& edges) {
    // remove path elements from lanes and junctions
    removeElements();
    // set new paht lanes
    myPathElements.clear();
    // use edges as path elements
    for (const auto& edge : edges) {
        if (allowedVClass) {
            myPathElements.push_back(edge->getLaneByAllowedVClass(vClass));
        } else {
            myPathElements.push_back(edge->getLaneByDisallowedVClass(vClass));
        }
    }
    // update path elements
    updatePathElements();
    // add path elements in lanes and junctions
    addElements();
}


void
GNEPathElements::calculateConsecutivePathLanes(const std::vector<GNELane*>& lanes) {
    // remove path elements from lanes and junctions
    removeElements();
    // set new route lanes
    myPathElements.clear();
    // use edges as path elements
    for (const auto& lane : lanes) {
        myPathElements.push_back(lane);
    }
    // update path elements
    updatePathElements();
    // add path elements in lanes and junctions
    addElements();
}


void
GNEPathElements::resetPathLanes(SUMOVehicleClass vClass, const bool allowedVClass, GNELane* fromLane, GNELane* toLane, const std::vector<GNEEdge*>& /* viaEdges */) {
    // check if from and to lane are valid
    if (fromLane && toLane) {
        // remove path elements from lanes and junctions
        removeElements();
        // get from-via-to edges
        const std::vector<GNEEdge*> edges = calculateFromViaToEdges(fromLane, toLane, edges);
        // set new route lanes
        myPathElements.clear();
        // use edges as path elements
        if (edges.size() > 0) {
            for (int i = 0; i < (int)edges.size(); i++) {
                if (i == 0) {
                    myPathElements.push_back(fromLane);
                } else if (i == (int)edges.size()) {
                    myPathElements.push_back(toLane);
                } else if (allowedVClass) {
                    myPathElements.push_back(edges.at(i)->getLaneByAllowedVClass(vClass));
                } else {
                    myPathElements.push_back(edges.at(i)->getLaneByDisallowedVClass(vClass));
                }
            }
        } else {
            myPathElements = {fromLane, toLane};
        }
        // update path elements
        updatePathElements();
        // add path elements in lanes and junctions
        addElements();
    }
}


void
GNEPathElements::calculateGenericDataLanePath(const std::vector<GNEEdge*>& edges) {
    // only for demand elements
    if (myGenericData) {
        // remove path elements from lanes and junctions
        removeElements();
        // clear path
        myPathElements.clear();
        // iterate over edge lanes and add it
        for (const auto& edge : edges) {
            for (const auto& lane : edge->getLanes()) {
                myPathElements.push_back(lane);
            }
        }
        // update path elements
        updatePathElements();
        // add path elements in lanes and junctions
        addElements();
    }
}


void
GNEPathElements::addElements() {
    // additionals
    if (myAdditionalElement) {
        // add demandElement into parent lanes
        for (const auto& pathElement : myPathElements) {
            pathElement.getLane()->addPathAdditionalElement(myAdditionalElement);
            if (pathElement.getJunction()) {
                pathElement.getJunction()->addPathAdditionalElement(myAdditionalElement);
            }
        }
    }
    // demand elements
    if (myDemandElement) {
        // add demandElement into parent lanes
        for (const auto& pathElement : myPathElements) {
            pathElement.getLane()->addPathDemandElement(myDemandElement);
            if (pathElement.getJunction()) {
                pathElement.getJunction()->addPathDemandElement(myDemandElement);
            }
        }
    }
    // demand elements
    if (myGenericData) {
        // add genericData into parent lanes
        for (const auto& pathElement : myPathElements) {
            pathElement.getLane()->addPathGenericData(myGenericData);
            if (pathElement.getJunction()) {
                pathElement.getJunction()->addPathGenericData(myGenericData);
            }
        }
    }
}


void
GNEPathElements::removeElements() {
    // additionals
    if (myAdditionalElement) {
        // remove demandElement from parent lanes
        for (const auto& pathElement : myPathElements) {
            pathElement.getLane()->removePathAdditionalElement(myAdditionalElement);
            if (pathElement.getJunction()) {
                pathElement.getJunction()->removePathAdditionalElement(myAdditionalElement);
            }
        }
    }
    // demand elements
    if (myDemandElement) {
        // remove demandElement from parent lanes
        for (const auto& pathElement : myPathElements) {
            pathElement.getLane()->removePathDemandElement(myDemandElement);
            if (pathElement.getJunction()) {
                pathElement.getJunction()->removePathDemandElement(myDemandElement);
            }
        }
    }
    // generic datas
    if (myGenericData) {
        // remove genericData from parent lanes
        for (const auto& pathElement : myPathElements) {
            pathElement.getLane()->removePathGenericData(myGenericData);
            if (pathElement.getJunction()) {
                pathElement.getJunction()->removePathGenericData(myGenericData);
            }
        }
    }
}


void
GNEPathElements::updatePathElements() {
    // update next lanes
    for (auto i = myPathElements.begin(); i != (myPathElements.end() - 1); i++) {
        i->updateNextLane((i + 1)->getLane());
    }
}


const std::vector<GNEEdge*>
GNEPathElements::calculateFromViaToEdges(GNELane* fromLane, GNELane* toLane, const std::vector<GNEEdge*>& viaEdges) {
    // declare a edge vector
    std::vector<GNEEdge*> edges;
    // add from-via-edge lanes
    edges.push_back(fromLane->getParentEdge());
    for (const auto& edge : viaEdges) {
        edges.push_back(edge);
    }
    edges.push_back(toLane->getParentEdge());
    // remove consecutive (adjacent) duplicates
    edges.erase(std::unique(edges.begin(), edges.end()), edges.end());
    // return edges
    return edges;
}


GNEPathElements::GNEPathElements() :
    myAdditionalElement(nullptr),
    myDemandElement(nullptr),
    myGenericData(nullptr) {
}

/****************************************************************************/
