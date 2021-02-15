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
/// @file    GNEPersonPlanFrame.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Jun 2019
///
// The Widget for add PersonPlan elements
/****************************************************************************/
#include <config.h>

#include <netedit/GNEApplicationWindow.h>
#include <netedit/GNENet.h>
#include <netedit/GNEViewNet.h>
#include <netedit/GNEViewParent.h>
#include <netedit/elements/additional/GNEBusStop.h>
#include <netedit/elements/demand/GNEPerson.h>
#include <netedit/elements/demand/GNERouteHandler.h>
#include <netedit/elements/network/GNEEdge.h>
#include <netedit/elements/network/GNELane.h>
#include <utils/gui/div/GLHelper.h>
#include <utils/gui/div/GUIDesigns.h>
#include <utils/gui/globjects/GLIncludes.h>
#include <utils/gui/windows/GUIAppEnum.h>

#include "GNEPersonPlanFrame.h"


// ===========================================================================
// method definitions
// ===========================================================================

// ---------------------------------------------------------------------------
// GNEPersonPlanFrame - methods
// ---------------------------------------------------------------------------

GNEPersonPlanFrame::GNEPersonPlanFrame(FXHorizontalFrame* horizontalFrameParent, GNEViewNet* viewNet) :
    GNEFrame(horizontalFrameParent, viewNet, "PersonPlans") {

    // create person types selector modul
    myPersonSelector = new GNEFrameModuls::DemandElementSelector(this, {GNETagProperties::TagType::PERSON});

    // Create tag selector for person plan
    myPersonPlanTagSelector = new GNEFrameModuls::TagSelector(this, GNETagProperties::TagType::PERSONPLAN);

    // Create person parameters
    myPersonPlanAttributes = new GNEFrameAttributesModuls::AttributesCreator(this);

    // create myPathCreator Modul
    myPathCreator = new GNEFrameModuls::PathCreator(this);

    // Create HierarchicalElementTree modul
    myPersonHierarchy = new GNEFrameModuls::HierarchicalElementTree(this);

    // set PersonPlan tag type in tag selector
    myPersonPlanTagSelector->setCurrentTagType(GNETagProperties::TagType::PERSONPLAN);
}


GNEPersonPlanFrame::~GNEPersonPlanFrame() {}


void
GNEPersonPlanFrame::show() {
    // get persons maps
    const std::map<std::string, GNEDemandElement*>& persons = myViewNet->getNet()->getAttributeCarriers()->getDemandElements().at(SUMO_TAG_PERSON);
    const std::map<std::string, GNEDemandElement*>& personFlows = myViewNet->getNet()->getAttributeCarriers()->getDemandElements().at(SUMO_TAG_PERSONFLOW);
    // Only show moduls if there is at least one person
    if ((persons.size() > 0) || (personFlows.size() > 0)) {
        // show person selector
        myPersonSelector->showDemandElementSelector();
        // refresh person plan tag selector
        myPersonPlanTagSelector->refreshTagProperties();
        // set first person as demand element (this will call demandElementSelected() function)
        if (persons.size() > 0) {
            myPersonSelector->setDemandElement(persons.begin()->second);
        } else {
            myPersonSelector->setDemandElement(personFlows.begin()->second);
        }
    } else {
        // hide all moduls except helpCreation
        myPersonSelector->hideDemandElementSelector();
        myPersonPlanTagSelector->hideTagSelector();
        myPersonPlanAttributes->hideAttributesCreatorModul();
        myPathCreator->hidePathCreatorModul();
        myPersonHierarchy->hideHierarchicalElementTree();
    }
    // show frame
    GNEFrame::show();
}


void
GNEPersonPlanFrame::hide() {
    // reset candidate edges
    for (const auto& edge : myViewNet->getNet()->getAttributeCarriers()->getEdges()) {
        edge.second->resetCandidateFlags();
    }
    // hide frame
    GNEFrame::hide();
}


bool
GNEPersonPlanFrame::addPersonPlanElement(const GNEViewNetHelper::ObjectsUnderCursor& objectsUnderCursor, const GNEViewNetHelper::KeyPressed& keyPressed) {
    // first check if person selected is valid
    if (myPersonSelector->getCurrentDemandElement() == nullptr) {
        myViewNet->setStatusBarText("Current selected person isn't valid.");
        return false;
    }
    // finally check that person plan selected is valid
    if (myPersonPlanTagSelector->getCurrentTagProperties().getTag() == SUMO_TAG_NOTHING) {
        myViewNet->setStatusBarText("Current selected person plan isn't valid.");
        return false;
    }
    // Obtain current person plan tag (only for improve code legibility)
    SumoXMLTag personPlanTag = myPersonPlanTagSelector->getCurrentTagProperties().getTag();
    // declare flags for requierements
    const bool requireBusStop = ((personPlanTag == GNE_TAG_PERSONTRIP_EDGE_BUSSTOP) || (personPlanTag == GNE_TAG_WALK_EDGE_BUSSTOP) ||
                                 (personPlanTag == GNE_TAG_RIDE_EDGE_BUSSTOP) || (personPlanTag == GNE_TAG_PERSONSTOP_BUSSTOP));
    const bool requireEdge = ((personPlanTag == GNE_TAG_PERSONTRIP_EDGE_EDGE) || (personPlanTag == GNE_TAG_WALK_EDGES) ||
                              (personPlanTag == GNE_TAG_WALK_EDGE_EDGE) || (personPlanTag == GNE_TAG_RIDE_EDGE_EDGE));
    // continue depending of tag
    if ((personPlanTag == GNE_TAG_WALK_ROUTE) && objectsUnderCursor.getDemandElementFront() && (objectsUnderCursor.getDemandElementFront()->getTagProperty().getTag() == SUMO_TAG_ROUTE)) {
        return myPathCreator->addRoute(objectsUnderCursor.getDemandElementFront(), keyPressed.shiftKeyPressed(), keyPressed.controlKeyPressed());
    } else if (requireBusStop && objectsUnderCursor.getAdditionalFront() && (objectsUnderCursor.getAdditionalFront()->getTagProperty().getTag() == SUMO_TAG_BUS_STOP)) {
        return myPathCreator->addStoppingPlace(objectsUnderCursor.getAdditionalFront(), keyPressed.shiftKeyPressed(), keyPressed.controlKeyPressed());
    } else if (requireEdge && objectsUnderCursor.getEdgeFront()) {
        return myPathCreator->addEdge(objectsUnderCursor.getEdgeFront(), keyPressed.shiftKeyPressed(), keyPressed.controlKeyPressed());
    } else {
        return false;
    }
}


GNEFrameModuls::PathCreator*
GNEPersonPlanFrame::getPathCreator() const {
    return myPathCreator;
}

// ===========================================================================
// protected
// ===========================================================================

void
GNEPersonPlanFrame::tagSelected() {
    // first check if person is valid
    if (myPersonPlanTagSelector->getCurrentTagProperties().getTag() != SUMO_TAG_NOTHING) {
        // Obtain current person plan tag (only for improve code legibility)
        SumoXMLTag personPlanTag = myPersonPlanTagSelector->getCurrentTagProperties().getTag();
        // show person attributes
        myPersonPlanAttributes->showAttributesCreatorModul(myPersonPlanTagSelector->getCurrentTagProperties(), {});
        // get previous person plan
        GNEEdge* previousEdge = myPersonSelector->getPersonPlanPreviousEdge();
        // set path creator mode depending if previousEdge exist
        if (previousEdge) {
            // set path creator mode
            myPathCreator->showPathCreatorModul(personPlanTag, true, false);
            // add previous edge
            myPathCreator->addEdge(previousEdge, false, false);
        } else {
            // set path creator mode
            myPathCreator->showPathCreatorModul(personPlanTag, false, false);
        }
        // show person hierarchy
        myPersonHierarchy->showHierarchicalElementTree(myPersonSelector->getCurrentDemandElement());
    } else {
        // hide moduls if tag selecte isn't valid
        myPersonPlanAttributes->hideAttributesCreatorModul();
        myPathCreator->hidePathCreatorModul();
        myPersonHierarchy->hideHierarchicalElementTree();
    }
}


void
GNEPersonPlanFrame::demandElementSelected() {
    // check if a valid person was selected
    if (myPersonSelector->getCurrentDemandElement()) {
        // show person plan tag selector
        myPersonPlanTagSelector->showTagSelector();
        // now check if person plan selected is valid
        if (myPersonPlanTagSelector->getCurrentTagProperties().getTag() != SUMO_TAG_NOTHING) {
            // call tag selected
            tagSelected();
        } else {
            myPersonPlanAttributes->hideAttributesCreatorModul();
            myPathCreator->hidePathCreatorModul();
            myPersonHierarchy->hideHierarchicalElementTree();
        }
    } else {
        // hide moduls if person selected isn't valid
        myPersonPlanTagSelector->hideTagSelector();
        myPersonPlanAttributes->hideAttributesCreatorModul();
        myPathCreator->hidePathCreatorModul();
        myPersonHierarchy->hideHierarchicalElementTree();
    }
}


void
GNEPersonPlanFrame::createPath() {
    // first check that all attributes are valid
    if (!myPersonPlanAttributes->areValuesValid()) {
        myViewNet->setStatusBarText("Invalid " + myPersonPlanTagSelector->getCurrentTagProperties().getTagStr() + " parameters.");
    } else {
        // check if person plan can be created
        if (GNERouteHandler::buildPersonPlan(
                    myPersonPlanTagSelector->getCurrentTagProperties().getTag(),
                    myPersonSelector->getCurrentDemandElement(),
                    myPersonPlanAttributes,
                    myPathCreator)) {
            // refresh HierarchicalElementTree
            myPersonHierarchy->refreshHierarchicalElementTree();
            // abort path creation
            myPathCreator->abortPathCreation();
            // refresh using tagSelected
            tagSelected();
            // refresh personPlan attributes
            myPersonPlanAttributes->refreshRows();
        }
    }
}

/****************************************************************************/
