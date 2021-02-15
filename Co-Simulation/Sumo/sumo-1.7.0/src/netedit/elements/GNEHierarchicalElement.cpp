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
/// @file    GNEHierarchicalElement.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Jul 2020
///
// A abstract class for representation of hierarchical elements
/****************************************************************************/
#include <config.h>

#include <netedit/GNENet.h>
#include <netedit/GNEViewNet.h>


#include "GNEHierarchicalElement.h"

// ===========================================================================
// member method definitions
// ===========================================================================

// ---------------------------------------------------------------------------
// GNEHierarchicalElement - methods
// ---------------------------------------------------------------------------

GNEHierarchicalElement::GNEHierarchicalElement(GNENet* net, SumoXMLTag tag,
        const std::vector<GNEJunction*>& parentJunctions,
        const std::vector<GNEEdge*>& parentEdges,
        const std::vector<GNELane*>& parentLanes,
        const std::vector<GNEAdditional*>& parentAdditionals,
        const std::vector<GNEShape*>& parentShapes,
        const std::vector<GNETAZElement*>& parentTAZElements,
        const std::vector<GNEDemandElement*>& ParentDemandElements,
        const std::vector<GNEGenericData*>& parentGenericDatas) :
    GNEAttributeCarrier(tag, net),
    myHierarchicalConnections(this),
    myHierarchicalContainer(parentJunctions, parentEdges, parentLanes, parentAdditionals, parentShapes, parentTAZElements, ParentDemandElements, parentGenericDatas) {
}


GNEHierarchicalElement::~GNEHierarchicalElement() {}


const GNEHierarchicalContainer&
GNEHierarchicalElement::getHierarchicalContainer() const {
    return myHierarchicalContainer;
}


void
GNEHierarchicalElement::restoreHierarchicalContainer(const GNEHierarchicalContainer& container) {
    myHierarchicalContainer = container;
}


std::vector<GNEHierarchicalElement*>
GNEHierarchicalElement::getAllHierarchicalElements() const {
    // declare result
    std::vector<GNEHierarchicalElement*> result;
    // reserve
    result.reserve(myHierarchicalContainer.getContainerSize());
    // add parent elements
    for (const auto& element : getParentJunctions()) {
        result.push_back(element);
    }
    for (const auto& element : getParentEdges()) {
        result.push_back(element);
    }
    for (const auto& element : getParentLanes()) {
        result.push_back(element);
    }
    for (const auto& element : getParentAdditionals()) {
        result.push_back(element);
    }
    for (const auto& element : getParentShapes()) {
        result.push_back(element);
    }
    for (const auto& element : getParentTAZElements()) {
        result.push_back(element);
    }
    for (const auto& element : getParentDemandElements()) {
        result.push_back(element);
    }
    for (const auto& element : getParentGenericDatas()) {
        result.push_back(element);
    }
    // add child elements
    for (const auto& element : getChildJunctions()) {
        result.push_back(element);
    }
    for (const auto& element : getChildEdges()) {
        result.push_back(element);
    }
    for (const auto& element : getChildLanes()) {
        result.push_back(element);
    }
    for (const auto& element : getChildAdditionals()) {
        result.push_back(element);
    }
    for (const auto& element : getChildShapes()) {
        result.push_back(element);
    }
    for (const auto& element : getChildTAZElements()) {
        result.push_back(element);
    }
    for (const auto& element : getChildDemandElements()) {
        result.push_back(element);
    }
    for (const auto& element : getChildGenericDatas()) {
        result.push_back(element);
    }
    return result;
}


const std::vector<GNEJunction*>&
GNEHierarchicalElement::getParentJunctions() const {
    return myHierarchicalContainer.getParents<std::vector<GNEJunction*> >();
}


const std::vector<GNEEdge*>&
GNEHierarchicalElement::getParentEdges() const {
    return myHierarchicalContainer.getParents<std::vector<GNEEdge*> >();
}


const std::vector<GNELane*>&
GNEHierarchicalElement::getParentLanes() const {
    return myHierarchicalContainer.getParents<std::vector<GNELane*> >();
}


const std::vector<GNEAdditional*>&
GNEHierarchicalElement::getParentAdditionals() const {
    return myHierarchicalContainer.getParents<std::vector<GNEAdditional*> >();
}


const std::vector<GNEShape*>&
GNEHierarchicalElement::getParentShapes() const {
    return myHierarchicalContainer.getParents<std::vector<GNEShape*> >();
}


const std::vector<GNETAZElement*>&
GNEHierarchicalElement::getParentTAZElements() const {
    return myHierarchicalContainer.getParents<std::vector<GNETAZElement*> >();
}


const std::vector<GNEDemandElement*>&
GNEHierarchicalElement::getParentDemandElements() const {
    return myHierarchicalContainer.getParents<std::vector<GNEDemandElement*> >();
}


const std::vector<GNEGenericData*>&
GNEHierarchicalElement::getParentGenericDatas() const {
    return myHierarchicalContainer.getParents<std::vector<GNEGenericData*> >();
}


const std::vector<GNEJunction*>&
GNEHierarchicalElement::getChildJunctions() const {
    return myHierarchicalContainer.getChildren<std::vector<GNEJunction*> >();
}


const std::vector<GNEEdge*>&
GNEHierarchicalElement::getChildEdges() const {
    return myHierarchicalContainer.getChildren<std::vector<GNEEdge*> >();
}


const std::vector<GNELane*>&
GNEHierarchicalElement::getChildLanes() const {
    return myHierarchicalContainer.getChildren<std::vector<GNELane*> >();
}


const std::vector<GNEAdditional*>&
GNEHierarchicalElement::getChildAdditionals() const {
    return myHierarchicalContainer.getChildren<std::vector<GNEAdditional*> >();
}


const std::vector<GNEShape*>&
GNEHierarchicalElement::getChildShapes() const {
    return myHierarchicalContainer.getChildren<std::vector<GNEShape*> >();
}


const std::vector<GNETAZElement*>&
GNEHierarchicalElement::getChildTAZElements() const {
    return myHierarchicalContainer.getChildren<std::vector<GNETAZElement*> >();
}


const std::vector<GNEDemandElement*>&
GNEHierarchicalElement::getChildDemandElements() const {
    return myHierarchicalContainer.getChildren<std::vector<GNEDemandElement*> >();
}


const std::vector<GNEGenericData*>&
GNEHierarchicalElement::getChildGenericDatas() const {
    return myHierarchicalContainer.getChildren<std::vector<GNEGenericData*> >();
}


template<> void
GNEHierarchicalElement::addParentElement(GNEJunction* element) {
    // add parent element into container
    myHierarchicalContainer.addParentElement(this, element);
}


template<> void
GNEHierarchicalElement::addParentElement(GNEEdge* element) {
    // add parent element into container
    myHierarchicalContainer.addParentElement(this, element);
}


template<> void
GNEHierarchicalElement::addParentElement(GNELane* element) {
    // add parent element into container
    myHierarchicalContainer.addParentElement(this, element);
}


template<> void
GNEHierarchicalElement::addParentElement(GNEAdditional* element) {
    // add parent element into container
    myHierarchicalContainer.addParentElement(this, element);
    // update connections geometry
    myHierarchicalConnections.update();
}


template<> void
GNEHierarchicalElement::addParentElement(GNEShape* element) {
    // add parent element into container
    myHierarchicalContainer.addParentElement(this, element);
}


template<> void
GNEHierarchicalElement::addParentElement(GNETAZElement* element) {
    // add parent element into container
    myHierarchicalContainer.addParentElement(this, element);
}


template<> void
GNEHierarchicalElement::addParentElement(GNEDemandElement* element) {
    // add parent element into container
    myHierarchicalContainer.addParentElement(this, element);
}


template<> void
GNEHierarchicalElement::addParentElement(GNEGenericData* element) {
    // add parent element into container
    myHierarchicalContainer.addParentElement(this, element);
}


template<> void
GNEHierarchicalElement::removeParentElement(GNEJunction* element) {
    // remove parent element from container
    myHierarchicalContainer.removeParentElement(this, element);
}


template<> void
GNEHierarchicalElement::removeParentElement(GNEEdge* element) {
    // remove parent element from container
    myHierarchicalContainer.removeParentElement(this, element);
}


template<> void
GNEHierarchicalElement::removeParentElement(GNELane* element) {
    // remove parent element from container
    myHierarchicalContainer.removeParentElement(this, element);
}


template<> void
GNEHierarchicalElement::removeParentElement(GNEAdditional* element) {
    // remove parent element from container
    myHierarchicalContainer.removeParentElement(this, element);
    // update connections geometry
    myHierarchicalConnections.update();
}


template<> void
GNEHierarchicalElement::removeParentElement(GNEShape* element) {
    // remove parent element from container
    myHierarchicalContainer.removeParentElement(this, element);
}


template<> void
GNEHierarchicalElement::removeParentElement(GNETAZElement* element) {
    // remove parent element from container
    myHierarchicalContainer.removeParentElement(this, element);
}


template<> void
GNEHierarchicalElement::removeParentElement(GNEDemandElement* element) {
    // remove parent element from container
    myHierarchicalContainer.removeParentElement(this, element);
}


template<> void
GNEHierarchicalElement::removeParentElement(GNEGenericData* element) {
    // remove parent element from container
    myHierarchicalContainer.removeParentElement(this, element);
}


template<> void
GNEHierarchicalElement::addChildElement(GNEJunction* element) {
    // add child element into container
    myHierarchicalContainer.addChildElement(this, element);
}


template<> void
GNEHierarchicalElement::addChildElement(GNEEdge* element) {
    // add child element into container
    myHierarchicalContainer.addChildElement(this, element);
}


template<> void
GNEHierarchicalElement::addChildElement(GNELane* element) {
    // add child element into container
    myHierarchicalContainer.addChildElement(this, element);
}


template<> void
GNEHierarchicalElement::addChildElement(GNEAdditional* element) {
    // add child element into container
    myHierarchicalContainer.addChildElement(this, element);
    // update connections geometry
    if (element->getTagProperty().isSlave()) {
        myHierarchicalConnections.update();
    }
}


template<> void
GNEHierarchicalElement::addChildElement(GNEShape* element) {
    // add child element into container
    myHierarchicalContainer.addChildElement(this, element);
}


template<> void
GNEHierarchicalElement::addChildElement(GNETAZElement* element) {
    // add child element into container
    myHierarchicalContainer.addChildElement(this, element);
}


template<> void
GNEHierarchicalElement::addChildElement(GNEDemandElement* element) {
    // add child element into container
    myHierarchicalContainer.addChildElement(this, element);
}


template<> void
GNEHierarchicalElement::addChildElement(GNEGenericData* element) {
    // add child element into container
    myHierarchicalContainer.addChildElement(this, element);
}


template<> void
GNEHierarchicalElement::removeChildElement(GNEJunction* element) {
    // remove child element from container
    myHierarchicalContainer.removeChildElement(this, element);
}


template<> void
GNEHierarchicalElement::removeChildElement(GNEEdge* element) {
    // remove child element from container
    myHierarchicalContainer.removeChildElement(this, element);
}


template<> void
GNEHierarchicalElement::removeChildElement(GNELane* element) {
    // remove child element from container
    myHierarchicalContainer.removeChildElement(this, element);
}


template<> void
GNEHierarchicalElement::removeChildElement(GNEAdditional* element) {
    // remove child element from container
    myHierarchicalContainer.removeChildElement(this, element);
    // update connections geometry
    if (element->getTagProperty().isSlave()) {
        myHierarchicalConnections.update();
    }
}


template<> void
GNEHierarchicalElement::removeChildElement(GNEShape* element) {
    // remove child element from container
    myHierarchicalContainer.removeChildElement(this, element);
}


template<> void
GNEHierarchicalElement::removeChildElement(GNETAZElement* element) {
    // remove child element from container
    myHierarchicalContainer.removeChildElement(this, element);
}


template<> void
GNEHierarchicalElement::removeChildElement(GNEDemandElement* element) {
    // remove child element from container
    myHierarchicalContainer.removeChildElement(this, element);
}


template<> void
GNEHierarchicalElement::removeChildElement(GNEGenericData* element) {
    // remove child element from container
    myHierarchicalContainer.removeChildElement(this, element);
}


std::string
GNEHierarchicalElement::getNewListOfParents(const GNENetworkElement* currentElement, const GNENetworkElement* newNextElement) const {
    std::vector<std::string> solution;
    if ((currentElement->getTagProperty().getTag() == SUMO_TAG_EDGE) && (newNextElement->getTagProperty().getTag() == SUMO_TAG_EDGE)) {
        // reserve solution
        solution.reserve(getParentEdges().size());
        // iterate over edges
        for (const auto& edge : getParentEdges()) {
            // add edge ID
            solution.push_back(edge->getID());
            // if current edge is the current element, then insert newNextElement ID
            if (edge == currentElement) {
                solution.push_back(newNextElement->getID());
            }
        }
    } else if ((currentElement->getTagProperty().getTag() == SUMO_TAG_LANE) && (newNextElement->getTagProperty().getTag() == SUMO_TAG_LANE)) {
        // reserve solution
        solution.reserve(getParentLanes().size());
        // iterate over lanes
        for (const auto& lane : getParentLanes()) {
            // add lane ID
            solution.push_back(lane->getID());
            // if current lane is the current element, then insert newNextElement ID
            if (lane == currentElement) {
                solution.push_back(newNextElement->getID());
            }
        }
    }
    // remove consecutive (adjacent) duplicates
    solution.erase(std::unique(solution.begin(), solution.end()), solution.end());
    // return solution
    return toString(solution);
}


void
GNEHierarchicalElement::updateHierarchicalConnections() {
    myHierarchicalConnections.update();
}


void
GNEHierarchicalElement::drawHierarchicalConnections(const GUIVisualizationSettings& s, const GNEAttributeCarrier* AC, const double exaggeration) const {
    // first check if connections can be drawn
    if (!s.drawForPositionSelection && !s.drawForRectangleSelection && (exaggeration > 0)) {
        myHierarchicalConnections.drawConnection(s, AC, exaggeration);
        // check if we have to draw dotted inspect contour
        if (s.drawDottedContour() || (AC->getNet()->getViewNet()->getInspectedAttributeCarrier() == AC)) {
            myHierarchicalConnections.drawDottedConnection(true, s, exaggeration);
        }
        // check if we have to draw dotted fronto contour
        if (s.drawDottedContour() || (AC->getNet()->getViewNet()->getFrontAttributeCarrier() == AC)) {
            myHierarchicalConnections.drawDottedConnection(false, s, exaggeration);
        }
    }
}


bool
GNEHierarchicalElement::checkChildAdditionalsOverlapping() const {
    // declare a vector to keep sorted children
    std::vector<std::pair<std::pair<double, double>, GNEAdditional*> > sortedChildren;
    // iterate over child additional
    for (const auto& additional : getChildAdditionals()) {
        sortedChildren.push_back(std::make_pair(std::make_pair(0., 0.), additional));
        // set begin/start attribute
        if (additional->getTagProperty().hasAttribute(SUMO_ATTR_TIME) && GNEAttributeCarrier::canParse<double>(additional->getAttribute(SUMO_ATTR_TIME))) {
            sortedChildren.back().first.first = additional->getAttributeDouble(SUMO_ATTR_TIME);
        } else if (additional->getTagProperty().hasAttribute(SUMO_ATTR_BEGIN) && GNEAttributeCarrier::canParse<double>(additional->getAttribute(SUMO_ATTR_BEGIN))) {
            sortedChildren.back().first.first = additional->getAttributeDouble(SUMO_ATTR_BEGIN);
        }
        // set end attribute
        if (additional->getTagProperty().hasAttribute(SUMO_ATTR_END) && GNEAttributeCarrier::canParse<double>(additional->getAttribute(SUMO_ATTR_END))) {
            sortedChildren.back().first.second = additional->getAttributeDouble(SUMO_ATTR_END);
        } else {
            sortedChildren.back().first.second = sortedChildren.back().first.first;
        }
    }
    // sort children
    std::sort(sortedChildren.begin(), sortedChildren.end());
    // make sure that number of sorted children is the same as the child additional
    if (sortedChildren.size() == getChildAdditionals().size()) {
        if (sortedChildren.size() <= 1) {
            return true;
        } else {
            // check overlapping
            for (int i = 0; i < (int)sortedChildren.size() - 1; i++) {
                if (sortedChildren.at(i).first.second > sortedChildren.at(i + 1).first.first) {
                    return false;
                }
            }
        }
        return true;
    } else {
        throw ProcessError("Some child additional were lost during sorting");
    }
}


bool
GNEHierarchicalElement::checkChildDemandElementsOverlapping() const {
    return true;
}


void
GNEHierarchicalElement::updateParentAdditional() {
    // by default nothing to do
}

/****************************************************************************/
