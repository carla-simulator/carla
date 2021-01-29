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
/// @file    GNECrossingFrame.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Oct 2016
///
// The Widget for add Crossing elements
/****************************************************************************/
#include <config.h>

#include <utils/gui/windows/GUIAppEnum.h>
#include <utils/gui/div/GUIDesigns.h>
#include <netedit/changes/GNEChange_Crossing.h>
#include <netedit/GNENet.h>
#include <netedit/GNEViewNet.h>
#include <netedit/elements/network/GNEJunction.h>
#include <netedit/elements/network/GNEEdge.h>
#include <netedit/elements/network/GNELane.h>
#include <netedit/GNEUndoList.h>

#include "GNECrossingFrame.h"


// ===========================================================================
// FOX callback mapping
// ===========================================================================

FXDEFMAP(GNECrossingFrame::EdgesSelector) EdgesSelectorMap[] = {
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_ADDITIONALFRAME_USESELECTED,        GNECrossingFrame::EdgesSelector::onCmdUseSelectedEdges),
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_ADDITIONALFRAME_CLEARSELECTION,     GNECrossingFrame::EdgesSelector::onCmdClearSelection),
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_ADDITIONALFRAME_INVERTSELECTION,    GNECrossingFrame::EdgesSelector::onCmdInvertSelection),
};

FXDEFMAP(GNECrossingFrame::CrossingParameters) CrossingParametersMap[] = {
    FXMAPFUNC(SEL_COMMAND, MID_GNE_SET_ATTRIBUTE,   GNECrossingFrame::CrossingParameters::onCmdSetAttribute),
    FXMAPFUNC(SEL_COMMAND, MID_HELP,                GNECrossingFrame::CrossingParameters::onCmdHelp),
};

FXDEFMAP(GNECrossingFrame::CreateCrossing) CreateCrossingMap[] = {
    FXMAPFUNC(SEL_COMMAND, MID_GNE_CREATE,  GNECrossingFrame::CreateCrossing::onCmdCreateCrossing),
};

// Object implementation
FXIMPLEMENT(GNECrossingFrame::EdgesSelector,        FXGroupBox,     EdgesSelectorMap,       ARRAYNUMBER(EdgesSelectorMap))
FXIMPLEMENT(GNECrossingFrame::CrossingParameters,   FXGroupBox,     CrossingParametersMap,  ARRAYNUMBER(CrossingParametersMap))
FXIMPLEMENT(GNECrossingFrame::CreateCrossing,       FXGroupBox,     CreateCrossingMap,      ARRAYNUMBER(CreateCrossingMap))


// ===========================================================================
// method definitions
// ===========================================================================

// ---------------------------------------------------------------------------
// GNECrossingFrame::CurrentJunction - methods
// ---------------------------------------------------------------------------

GNECrossingFrame::CurrentJunction::CurrentJunction(GNECrossingFrame* crossingFrameParent) :
    FXGroupBox(crossingFrameParent->myContentFrame, "Junction", GUIDesignGroupBoxFrame) {
    // create junction label
    myCurrentJunctionLabel = new FXLabel(this, "No junction selected", 0, GUIDesignLabelLeft);
}


GNECrossingFrame::CurrentJunction::~CurrentJunction() {}


void
GNECrossingFrame::CurrentJunction::updateCurrentJunctionLabel(const std::string& junctionID) {
    if (junctionID.empty()) {
        myCurrentJunctionLabel->setText("No junction selected");
    } else {
        myCurrentJunctionLabel->setText((std::string("Current Junction: ") + junctionID).c_str());
    }
}

// ---------------------------------------------------------------------------
// GNECrossingFrame::EdgesSelector - methods
// ---------------------------------------------------------------------------

GNECrossingFrame::EdgesSelector::EdgesSelector(GNECrossingFrame* crossingFrameParent) :
    FXGroupBox(crossingFrameParent->myContentFrame, ("selection of " + toString(SUMO_TAG_EDGE) + "s").c_str(), GUIDesignGroupBoxFrame),
    myCrossingFrameParent(crossingFrameParent),
    myCurrentJunction(nullptr) {

    // Create button for selected edges
    myUseSelectedEdges = new FXButton(this, ("Use selected " + toString(SUMO_TAG_EDGE) + "s").c_str(), nullptr, this, MID_GNE_ADDITIONALFRAME_USESELECTED, GUIDesignButton);

    // Create button for clear selection
    myClearEdgesSelection = new FXButton(this, ("Clear " + toString(SUMO_TAG_EDGE) + "s").c_str(), nullptr, this, MID_GNE_ADDITIONALFRAME_CLEARSELECTION, GUIDesignButton);

    // Create button for invert selection
    myInvertEdgesSelection = new FXButton(this, ("Invert " + toString(SUMO_TAG_EDGE) + "s").c_str(), nullptr, this, MID_GNE_ADDITIONALFRAME_INVERTSELECTION, GUIDesignButton);
}


GNECrossingFrame::EdgesSelector::~EdgesSelector() {}


GNEJunction*
GNECrossingFrame::EdgesSelector::getCurrentJunction() const {
    return myCurrentJunction;
}


void
GNECrossingFrame::EdgesSelector::enableEdgeSelector(GNEJunction* currentJunction) {
    // restore color of all lanes of edge candidates
    restoreEdgeColors();
    // Set current junction
    myCurrentJunction = currentJunction;
    // Update view net to show the new colors
    myCrossingFrameParent->getViewNet()->updateViewNet();
    // check if use selected eges must be enabled
    myUseSelectedEdges->disable();
    for (const auto& edge : myCurrentJunction->getChildEdges()) {
        if (edge->isAttributeCarrierSelected()) {
            myUseSelectedEdges->enable();
        }
    }
    // Enable rest of elements
    myClearEdgesSelection->enable();
    myInvertEdgesSelection->enable();
}


void
GNECrossingFrame::EdgesSelector::disableEdgeSelector() {
    // disable current junction
    myCurrentJunction = nullptr;
    // disable all elements of the EdgesSelector
    myUseSelectedEdges->disable();
    myClearEdgesSelection->disable();
    myInvertEdgesSelection->disable();
    // Disable crossing parameters
    myCrossingFrameParent->myCrossingParameters->disableCrossingParameters();
}


void
GNECrossingFrame::EdgesSelector::restoreEdgeColors() {
    if (myCurrentJunction != nullptr) {
        // restore color of all lanes of edge candidates
        for (const auto& edge : myCurrentJunction->getChildEdges()) {
            edge->resetCandidateFlags();
        }
        // Update view net to show the new colors
        myCrossingFrameParent->getViewNet()->updateViewNet();
        myCurrentJunction = nullptr;
    }
}


long
GNECrossingFrame::EdgesSelector::onCmdUseSelectedEdges(FXObject*, FXSelector, void*) {
    myCrossingFrameParent->myCrossingParameters->useSelectedEdges(myCurrentJunction);
    return 1;
}


long
GNECrossingFrame::EdgesSelector::onCmdClearSelection(FXObject*, FXSelector, void*) {
    myCrossingFrameParent->myCrossingParameters->clearEdges();
    return 1;
}


long
GNECrossingFrame::EdgesSelector::onCmdInvertSelection(FXObject*, FXSelector, void*) {
    myCrossingFrameParent->myCrossingParameters->invertEdges(myCurrentJunction);
    return 1;
}

// ---------------------------------------------------------------------------
// GNECrossingFrame::NeteditAttributes- methods
// ---------------------------------------------------------------------------

GNECrossingFrame::CrossingParameters::CrossingParameters(GNECrossingFrame* crossingFrameParent) :
    FXGroupBox(crossingFrameParent->myContentFrame, "Crossing parameters", GUIDesignGroupBoxFrame),
    myCrossingFrameParent(crossingFrameParent),
    myCurrentParametersValid(true) {
    FXHorizontalFrame* crossingParameter = nullptr;
    // create label and string textField for edges
    crossingParameter = new FXHorizontalFrame(this, GUIDesignAuxiliarHorizontalFrame);
    myCrossingEdgesLabel = new FXLabel(crossingParameter, toString(SUMO_ATTR_EDGES).c_str(), nullptr, GUIDesignLabelAttribute);
    myCrossingEdges = new FXTextField(crossingParameter, GUIDesignTextFieldNCol, this, MID_GNE_SET_ATTRIBUTE, GUIDesignTextField);
    myCrossingEdgesLabel->disable();
    myCrossingEdges->disable();
    // create label and checkbox for Priority
    crossingParameter = new FXHorizontalFrame(this, GUIDesignAuxiliarHorizontalFrame);
    myCrossingPriorityLabel = new FXLabel(crossingParameter, toString(SUMO_ATTR_PRIORITY).c_str(), nullptr, GUIDesignLabelAttribute);
    myCrossingPriorityCheckButton = new FXCheckButton(crossingParameter, "", this, MID_GNE_SET_ATTRIBUTE, GUIDesignCheckButton);
    myCrossingPriorityLabel->disable();
    myCrossingPriorityCheckButton->disable();
    // create label and textfield for width
    crossingParameter = new FXHorizontalFrame(this, GUIDesignAuxiliarHorizontalFrame);
    myCrossingWidthLabel = new FXLabel(crossingParameter, toString(SUMO_ATTR_WIDTH).c_str(), nullptr, GUIDesignLabelAttribute);
    myCrossingWidth = new FXTextField(crossingParameter, GUIDesignTextFieldNCol, this, MID_GNE_SET_ATTRIBUTE, GUIDesignTextField);
    myCrossingWidthLabel->disable();
    myCrossingWidth->disable();
    // Create help button
    myHelpCrossingAttribute = new FXButton(this, "Help", nullptr, this, MID_HELP, GUIDesignButtonRectangular);
    myHelpCrossingAttribute->disable();
}


GNECrossingFrame::CrossingParameters::~CrossingParameters() {}


void
GNECrossingFrame::CrossingParameters::enableCrossingParameters(bool hasTLS) {
    // obtain Tag Values
    const auto& tagProperties = GNEAttributeCarrier::getTagProperties(SUMO_TAG_CROSSING);
    // Enable all elements of the crossing frames
    myCrossingEdgesLabel->enable();
    myCrossingEdges->enable();
    myCrossingPriorityLabel->enable();
    // only enable priority check button if junction's crossing doesn't have TLS
    if (hasTLS) {
        myCrossingPriorityCheckButton->disable();
    } else {
        myCrossingPriorityCheckButton->enable();
    }
    myCrossingWidthLabel->enable();
    myCrossingWidth->enable();
    myHelpCrossingAttribute->enable();
    // set values of parameters
    onCmdSetAttribute(nullptr, 0, nullptr);
    // Crossings placed in junctinos with TLS always has priority
    if (hasTLS) {
        myCrossingPriorityCheckButton->setCheck(TRUE);
    } else {
        myCrossingPriorityCheckButton->setCheck(GNEAttributeCarrier::parse<bool>(tagProperties.getDefaultValue(SUMO_ATTR_PRIORITY)));
    }
    myCrossingWidth->setText(tagProperties.getDefaultValue(SUMO_ATTR_WIDTH).c_str());
    myCrossingWidth->setTextColor(FXRGB(0, 0, 0));
}


void
GNECrossingFrame::CrossingParameters::disableCrossingParameters() {
    // clear all values of parameters
    myCrossingEdges->setText("");
    myCrossingPriorityCheckButton->setCheck(false);
    myCrossingPriorityCheckButton->setText("false");
    myCrossingWidth->setText("");
    // Disable all elements of the crossing frames
    myCrossingEdgesLabel->disable();
    myCrossingEdges->disable();
    myCrossingPriorityLabel->disable();
    myCrossingPriorityCheckButton->disable();
    myCrossingWidthLabel->disable();
    myCrossingWidth->disable();
    myHelpCrossingAttribute->disable();
    myCrossingFrameParent->myCreateCrossing->setCreateCrossingButton(false);
}


bool
GNECrossingFrame::CrossingParameters::isCrossingParametersEnabled() const {
    return myCrossingEdgesLabel->isEnabled();
}


void
GNECrossingFrame::CrossingParameters::markEdge(GNEEdge* edge) {
    GNEJunction* currentJunction = myCrossingFrameParent->myEdgeSelector->getCurrentJunction();
    if (currentJunction != nullptr) {
        // Check if edge belongs to junction's edge
        if (std::find(currentJunction->getChildEdges().begin(), currentJunction->getChildEdges().end(), edge) != currentJunction->getChildEdges().end()) {
            // Update text field with the new edge
            std::vector<std::string> crossingEdges = GNEAttributeCarrier::parse<std::vector<std::string> > (myCrossingEdges->getText().text());
            // Check if new edge must be added or removed
            std::vector<std::string>::iterator itFinder = std::find(crossingEdges.begin(), crossingEdges.end(), edge->getID());
            if (itFinder == crossingEdges.end()) {
                crossingEdges.push_back(edge->getID());
            } else {
                crossingEdges.erase(itFinder);
            }
            myCrossingEdges->setText(joinToString(crossingEdges, " ").c_str());
        }
        // Update colors and attributes
        onCmdSetAttribute(nullptr, 0, nullptr);
    }
}


void
GNECrossingFrame::CrossingParameters::clearEdges() {
    myCrossingEdges->setText("");
    // Update colors and attributes
    onCmdSetAttribute(nullptr, 0, nullptr);
}


void
GNECrossingFrame::CrossingParameters::invertEdges(GNEJunction* parentJunction) {
    std::vector<std::string> crossingEdges;
    for (auto i : parentJunction->getChildEdges()) {
        if (std::find(myCurrentSelectedEdges.begin(), myCurrentSelectedEdges.end(), i) == myCurrentSelectedEdges.end()) {
            crossingEdges.push_back(i->getID());
        }
    }
    myCrossingEdges->setText(joinToString(crossingEdges, " ").c_str());
    // Update colors and attributes
    onCmdSetAttribute(nullptr, 0, nullptr);
}


void
GNECrossingFrame::CrossingParameters::useSelectedEdges(GNEJunction* parentJunction) {
    std::vector<std::string> crossingEdges;
    for (auto i : parentJunction->getChildEdges()) {
        if (i->isAttributeCarrierSelected()) {
            crossingEdges.push_back(i->getID());
        }
    }
    myCrossingEdges->setText(joinToString(crossingEdges, " ").c_str());
    // Update colors and attributes
    onCmdSetAttribute(nullptr, 0, nullptr);
}


std::vector<NBEdge*>
GNECrossingFrame::CrossingParameters::getCrossingEdges() const {
    std::vector<NBEdge*> NBEdgeVector;
    // Iterate over myCurrentSelectedEdges
    for (auto i : myCurrentSelectedEdges) {
        NBEdgeVector.push_back(i->getNBEdge());
    }
    return NBEdgeVector;
}


bool
GNECrossingFrame::CrossingParameters::getCrossingPriority() const {
    if (myCrossingPriorityCheckButton->getCheck()) {
        return true;
    } else {
        return false;
    }
}


bool
GNECrossingFrame::CrossingParameters::isCurrentParametersValid() const {
    return myCurrentParametersValid;
}


double
GNECrossingFrame::CrossingParameters::getCrossingWidth() const {
    return GNEAttributeCarrier::parse<double>(myCrossingWidth->getText().text());
}


long
GNECrossingFrame::CrossingParameters::onCmdSetAttribute(FXObject*, FXSelector, void*) {
    myCurrentParametersValid = true;
    // get string vector with the edges
    std::vector<std::string> crossingEdges = GNEAttributeCarrier::parse<std::vector<std::string> > (myCrossingEdges->getText().text());
    // Clear selected edges
    myCurrentSelectedEdges.clear();
    // iterate over vector of edge IDs
    for (auto i : crossingEdges) {
        GNEEdge* edge = myCrossingFrameParent->getViewNet()->getNet()->retrieveEdge(i, false);
        GNEJunction* currentJunction = myCrossingFrameParent->myEdgeSelector->getCurrentJunction();
        // Check that edge exists and belongs to Junction
        if (edge == nullptr) {
            myCurrentParametersValid = false;
        } else if (std::find(currentJunction->getChildEdges().begin(), currentJunction->getChildEdges().end(), edge) == currentJunction->getChildEdges().end()) {
            myCurrentParametersValid = false;
        } else {
            // select or unselected edge
            auto itFinder = std::find(myCurrentSelectedEdges.begin(), myCurrentSelectedEdges.end(), edge);
            if (itFinder == myCurrentSelectedEdges.end()) {
                myCurrentSelectedEdges.push_back(edge);
            } else {
                myCurrentSelectedEdges.erase(itFinder);
            }
        }
    }
    // change color of textfield dependig of myCurrentParametersValid
    if (myCurrentParametersValid) {
        myCrossingEdges->setTextColor(FXRGB(0, 0, 0));
        myCrossingEdges->killFocus();
    } else {
        myCrossingEdges->setTextColor(FXRGB(255, 0, 0));
        myCurrentParametersValid = false;
    }
    // Update edge colors
    for (const auto& edge : myCrossingFrameParent->myEdgeSelector->getCurrentJunction()->getChildEdges()) {
        // restore colors
        edge->resetCandidateFlags();
        // set selected or candidate color
        if (std::find(myCurrentSelectedEdges.begin(), myCurrentSelectedEdges.end(), edge) != myCurrentSelectedEdges.end()) {
            edge->setTargetCandidate(true);
        } else {
            edge->setPossibleCandidate(true);
        }
    }
    // Update view net
    myCrossingFrameParent->getViewNet()->updateViewNet();

    // Check that at least there are a selected edge
    if (crossingEdges.empty()) {
        myCurrentParametersValid = false;
    }

    // change label of crossing priority
    if (myCrossingPriorityCheckButton->getCheck()) {
        myCrossingPriorityCheckButton->setText("true");
    } else {
        myCrossingPriorityCheckButton->setText("false");
    }

    // Check width
    if (GNEAttributeCarrier::canParse<double>(myCrossingWidth->getText().text()) &&
            GNEAttributeCarrier::parse<double>(myCrossingWidth->getText().text()) > 0) {
        myCrossingWidth->setTextColor(FXRGB(0, 0, 0));
        myCrossingWidth->killFocus();
    } else {
        myCrossingWidth->setTextColor(FXRGB(255, 0, 0));
        myCurrentParametersValid = false;
    }

    // Enable or disable create crossing button depending of the current parameters
    myCrossingFrameParent->myCreateCrossing->setCreateCrossingButton(myCurrentParametersValid);
    return 0;
}


long
GNECrossingFrame::CrossingParameters::onCmdHelp(FXObject*, FXSelector, void*) {
    myCrossingFrameParent->openHelpAttributesDialog(GNEAttributeCarrier::getTagProperties(SUMO_TAG_CROSSING));
    return 1;
}

// ---------------------------------------------------------------------------
// GNECrossingFrame::CreateCrossing - methods
// ---------------------------------------------------------------------------

GNECrossingFrame::CreateCrossing::CreateCrossing(GNECrossingFrame* crossingFrameParent) :
    FXGroupBox(crossingFrameParent->myContentFrame, "Create", GUIDesignGroupBoxFrame),
    myCrossingFrameParent(crossingFrameParent) {
    // Create groupbox for create crossings
    myCreateCrossingButton = new FXButton(this, "Create crossing", 0, this, MID_GNE_CREATE, GUIDesignButton);
    myCreateCrossingButton->disable();
}


GNECrossingFrame::CreateCrossing::~CreateCrossing() {}


long
GNECrossingFrame::CreateCrossing::onCmdCreateCrossing(FXObject*, FXSelector, void*) {
    // First check that current parameters are valid
    if (myCrossingFrameParent->myCrossingParameters->isCurrentParametersValid()) {
        // iterate over junction's crossing to find duplicated crossings
        if (myCrossingFrameParent->myEdgeSelector->getCurrentJunction()->getNBNode()->checkCrossingDuplicated(myCrossingFrameParent->myCrossingParameters->getCrossingEdges()) == false) {
            // create new crossing
            myCrossingFrameParent->myViewNet->getUndoList()->add(new GNEChange_Crossing(myCrossingFrameParent->myEdgeSelector->getCurrentJunction(),
                    myCrossingFrameParent->myCrossingParameters->getCrossingEdges(),
                    myCrossingFrameParent->myCrossingParameters->getCrossingWidth(),
                    myCrossingFrameParent->myCrossingParameters->getCrossingPriority(),
                    -1, -1,
                    PositionVector::EMPTY,
                    false, true), true);
            // clear selected edges
            myCrossingFrameParent->myEdgeSelector->onCmdClearSelection(0, 0, 0);
        } else {
            WRITE_WARNING("There is already another crossing with the same edges in the junction; Duplicated crossing aren't allowed.");
        }
    }
    return 1;
}


void
GNECrossingFrame::CreateCrossing::setCreateCrossingButton(bool value) {
    if (value) {
        myCreateCrossingButton->enable();
    } else {
        myCreateCrossingButton->disable();
    }
}

// ---------------------------------------------------------------------------
// GNECrossingFrame - methods
// ---------------------------------------------------------------------------

GNECrossingFrame::GNECrossingFrame(FXHorizontalFrame* horizontalFrameParent, GNEViewNet* viewNet) :
    GNEFrame(horizontalFrameParent, viewNet, "Crossings") {
    // create CurrentJunction modul
    myCurrentJunction = new CurrentJunction(this);

    // Create edge Selector modul
    myEdgeSelector = new EdgesSelector(this);

    // Create CrossingParameters modul
    myCrossingParameters = new CrossingParameters(this);

    // create CreateCrossing modul
    myCreateCrossing = new CreateCrossing(this);

    // Create groupbox and labels for legends
    FXGroupBox* groupBoxLegend = new FXGroupBox(myContentFrame, "", GUIDesignGroupBoxFrame);
    FXLabel* colorCandidateLabel = new FXLabel(groupBoxLegend, " Candidate", 0, GUIDesignLabelLeft);
    colorCandidateLabel->setBackColor(MFXUtils::getFXColor(viewNet->getVisualisationSettings().candidateColorSettings.possible));
    colorCandidateLabel->setTextColor(MFXUtils::getFXColor(RGBColor::WHITE));
    FXLabel* colorSelectedLabel = new FXLabel(groupBoxLegend, " Selected", 0, GUIDesignLabelLeft);
    colorSelectedLabel->setBackColor(MFXUtils::getFXColor(viewNet->getVisualisationSettings().candidateColorSettings.target));

    // disable edge selector
    myEdgeSelector->disableEdgeSelector();
}


GNECrossingFrame::~GNECrossingFrame() {
}


void
GNECrossingFrame::hide() {
    // restore color of all lanes of edge candidates
    myEdgeSelector->restoreEdgeColors();
    // hide frame
    GNEFrame::hide();
}


void
GNECrossingFrame::addCrossing(const GNEViewNetHelper::ObjectsUnderCursor& objectsUnderCursor) {
    // If current element is a junction
    if (objectsUnderCursor.getJunctionFront()) {
        // change label
        myCurrentJunction->updateCurrentJunctionLabel(objectsUnderCursor.getJunctionFront()->getID());
        // Enable edge selector and crossing parameters
        myEdgeSelector->enableEdgeSelector(objectsUnderCursor.getJunctionFront());
        myCrossingParameters->enableCrossingParameters(objectsUnderCursor.getJunctionFront()->getNBNode()->isTLControlled());
        // clears selected edges
        myCrossingParameters->clearEdges();
    } else if (objectsUnderCursor.getEdgeFront()) {
        // mark edge
        myCrossingParameters->markEdge(objectsUnderCursor.getEdgeFront());
    } else {
        // set default label
        myCurrentJunction->updateCurrentJunctionLabel("");
        // restore  color of all lanes of edge candidates
        myEdgeSelector->restoreEdgeColors();
        // Disable edge selector
        myEdgeSelector->disableEdgeSelector();
    }
    // always update view after an operation
    myViewNet->updateViewNet();
}


void
GNECrossingFrame::createCrossingHotkey() {
    if (myEdgeSelector->getCurrentJunction()) {
        // simply call onCmdCreateCrossing of CreateCrossing modul
        myCreateCrossing->onCmdCreateCrossing(0, 0, 0);
    }
}


/****************************************************************************/
