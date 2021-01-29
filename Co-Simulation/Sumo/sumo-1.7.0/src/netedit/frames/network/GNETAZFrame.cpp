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
/// @file    GNETAZFrame.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Oct 2018
///
// The Widget for add TAZ elements
/****************************************************************************/
#include <config.h>

#include <utils/gui/windows/GUIAppEnum.h>
#include <utils/gui/div/GUIDesigns.h>
#include <utils/xml/SUMOSAXAttributesImpl_Cached.h>
#include <netedit/GNENet.h>
#include <netedit/GNEViewNet.h>
#include <netedit/changes/GNEChange_TAZElement.h>
#include <netedit/elements/network/GNEEdge.h>
#include <netedit/elements/network/GNELane.h>
#include <netedit/elements/additional/GNETAZElement.h>
#include <netedit/elements/additional/GNETAZ.h>
#include <netedit/elements/additional/GNETAZSourceSink.h>
#include <netedit/elements/additional/GNEAdditionalHandler.h>
#include <netedit/GNEUndoList.h>

#include "GNETAZFrame.h"


// ===========================================================================
// FOX callback mapping
// ===========================================================================

FXDEFMAP(GNETAZFrame::TAZParameters) TAZParametersMap[] = {
    FXMAPFUNC(SEL_COMMAND, MID_GNE_SET_ATTRIBUTE_DIALOG,    GNETAZFrame::TAZParameters::onCmdSetColorAttribute),
    FXMAPFUNC(SEL_COMMAND, MID_GNE_SET_ATTRIBUTE,           GNETAZFrame::TAZParameters::onCmdSetAttribute),
    FXMAPFUNC(SEL_COMMAND, MID_HELP,                        GNETAZFrame::TAZParameters::onCmdHelp),
};

FXDEFMAP(GNETAZFrame::TAZSaveChanges) TAZSaveChangesMap[] = {
    FXMAPFUNC(SEL_COMMAND,  MID_OK,         GNETAZFrame::TAZSaveChanges::onCmdSaveChanges),
    FXMAPFUNC(SEL_COMMAND,  MID_CANCEL,     GNETAZFrame::TAZSaveChanges::onCmdCancelChanges),
};

FXDEFMAP(GNETAZFrame::TAZChildDefaultParameters) TAZChildDefaultParametersMap[] = {
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_SET_ATTRIBUTE,  GNETAZFrame::TAZChildDefaultParameters::onCmdSetDefaultValues),
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_SELECT,         GNETAZFrame::TAZChildDefaultParameters::onCmdUseSelectedEdges),
};

FXDEFMAP(GNETAZFrame::TAZSelectionStatistics) TAZSelectionStatisticsMap[] = {
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_SET_ATTRIBUTE,  GNETAZFrame::TAZSelectionStatistics::onCmdSetNewValues),
};

FXDEFMAP(GNETAZFrame::TAZEdgesGraphic) TAZEdgesGraphicMap[] = {
    FXMAPFUNC(SEL_COMMAND,  MID_CHOOSEN_OPERATION,  GNETAZFrame::TAZEdgesGraphic::onCmdChoosenBy),
};

// Object implementation
FXIMPLEMENT(GNETAZFrame::TAZParameters,             FXGroupBox,     TAZParametersMap,               ARRAYNUMBER(TAZParametersMap))
FXIMPLEMENT(GNETAZFrame::TAZSaveChanges,            FXGroupBox,     TAZSaveChangesMap,              ARRAYNUMBER(TAZSaveChangesMap))
FXIMPLEMENT(GNETAZFrame::TAZChildDefaultParameters, FXGroupBox,     TAZChildDefaultParametersMap,   ARRAYNUMBER(TAZChildDefaultParametersMap))
FXIMPLEMENT(GNETAZFrame::TAZSelectionStatistics,    FXGroupBox,     TAZSelectionStatisticsMap,      ARRAYNUMBER(TAZSelectionStatisticsMap))
FXIMPLEMENT(GNETAZFrame::TAZEdgesGraphic,           FXGroupBox,     TAZEdgesGraphicMap,             ARRAYNUMBER(TAZEdgesGraphicMap))


// ===========================================================================
// method definitions
// ===========================================================================

// ---------------------------------------------------------------------------
// GNETAZFrame::TAZCurrent - methods
// ---------------------------------------------------------------------------

GNETAZFrame::TAZCurrent::TAZEdge::TAZEdge(TAZCurrent* TAZCurrentParent, GNEEdge* _edge, GNETAZSourceSink* _TAZSource, GNETAZSourceSink* _TAZSink) :
    edge(_edge),
    TAZSource(_TAZSource),
    TAZSink(_TAZSink),
    sourceColor(0),
    sinkColor(0),
    sourcePlusSinkColor(0),
    sourceMinusSinkColor(0),
    myTAZCurrentParent(TAZCurrentParent)
{ }


GNETAZFrame::TAZCurrent::TAZEdge::~TAZEdge() {}


void
GNETAZFrame::TAZCurrent::TAZEdge::updateColors() {
    sourceColor = GNEAttributeCarrier::parse<int>(TAZSource->getAttribute(GNE_ATTR_TAZCOLOR));
    sinkColor = GNEAttributeCarrier::parse<int>(TAZSink->getAttribute(GNE_ATTR_TAZCOLOR));
    // Obtain Source+Sink needs more steps. First obtain Source+Sink Weight
    double sourcePlusSinkWeight = TAZSource->getDepartWeight() + TAZSink->getDepartWeight();
    // avoid division between zero
    if ((myTAZCurrentParent->myMaxSourcePlusSinkWeight - myTAZCurrentParent->myMinSourcePlusSinkWeight) == 0) {
        sourcePlusSinkColor = 0;
    } else {
        // calculate percentage relative to the max and min Source+Sink weight
        double percentage = (sourcePlusSinkWeight - myTAZCurrentParent->myMinSourcePlusSinkWeight) /
                            (myTAZCurrentParent->myMaxSourcePlusSinkWeight - myTAZCurrentParent->myMinSourcePlusSinkWeight);
        // convert percentage to a value between [0-9] (because we have only 10 colors)
        if (percentage >= 1) {
            sourcePlusSinkColor = 9;
        } else if (percentage < 0) {
            sourcePlusSinkColor = 0;
        } else {
            sourcePlusSinkColor = (int)(percentage * 10);
        }
    }
    // Obtain Source+Sink needs more steps. First obtain Source-Sink Weight
    double sourceMinusSinkWeight =  TAZSource->getDepartWeight() - TAZSink->getDepartWeight();
    // avoid division between zero
    if ((myTAZCurrentParent->myMaxSourceMinusSinkWeight - myTAZCurrentParent->myMinSourceMinusSinkWeight) == 0) {
        sourceMinusSinkColor = 0;
    } else {
        // calculate percentage relative to the max and min Source-Sink weight
        double percentage = (sourceMinusSinkWeight - myTAZCurrentParent->myMinSourceMinusSinkWeight) /
                            (myTAZCurrentParent->myMaxSourceMinusSinkWeight - myTAZCurrentParent->myMinSourceMinusSinkWeight);
        // convert percentage to a value between [0-9] (because we have only 10 colors)
        if (percentage >= 1) {
            sourceMinusSinkColor = 9;
        } else if (percentage < 0) {
            sourceMinusSinkColor = 0;
        } else {
            sourceMinusSinkColor = (int)(percentage * 10);
        }
    }
}


GNETAZFrame::TAZCurrent::TAZCurrent(GNETAZFrame* TAZFrameParent) :
    FXGroupBox(TAZFrameParent->myContentFrame, "TAZ", GUIDesignGroupBoxFrame),
    myTAZFrameParent(TAZFrameParent),
    myEditedTAZ(nullptr),
    myMaxSourcePlusSinkWeight(0),
    myMinSourcePlusSinkWeight(-1),
    myMaxSourceMinusSinkWeight(0),
    myMinSourceMinusSinkWeight(-1) {
    // create TAZ label
    myTAZCurrentLabel = new FXLabel(this, "No TAZ selected", 0, GUIDesignLabelLeft);
}


GNETAZFrame::TAZCurrent::~TAZCurrent() {}


void
GNETAZFrame::TAZCurrent::setTAZ(GNETAZ* editedTAZ) {
    // set new current TAZ
    myEditedTAZ = editedTAZ;
    // update label and moduls
    if (myEditedTAZ != nullptr) {
        myTAZCurrentLabel->setText(("Current TAZ: " + myEditedTAZ->getID()).c_str());
        // obtain a copy of all edges of the net (to avoid slowdown during manipulations)
        myNetEdges = myTAZFrameParent->myViewNet->getNet()->retrieveEdges();
        // obtain a copy of all SELECTED edges of the net (to avoid slowdown during manipulations)
        mySelectedEdges = myTAZFrameParent->myViewNet->getNet()->retrieveEdges(true);
        // resfresh TAZ Edges
        refreshTAZEdges();
        // hide TAZ parameters
        myTAZFrameParent->myTAZParameters->hideTAZParametersModul();
        // hide Netedit parameters
        myTAZFrameParent->myNeteditAttributes->hideNeteditAttributesModul();
        // hide drawing shape
        myTAZFrameParent->myDrawingShape->hideDrawingShape();
        // show edge common parameters
        myTAZFrameParent->myTAZCommonStatistics->showTAZCommonStatisticsModul();
        // show save TAZ Edges
        myTAZFrameParent->myTAZSaveChanges->showTAZSaveChangesModul();
        // show edge common parameters
        myTAZFrameParent->myTAZChildDefaultParameters->showTAZChildDefaultParametersModul();
        // show Edges graphics
        myTAZFrameParent->myTAZEdgesGraphic->showTAZEdgesGraphicModul();
    } else {
        // show TAZ parameters
        myTAZFrameParent->myTAZParameters->showTAZParametersModul();
        // show Netedit parameters
        myTAZFrameParent->myNeteditAttributes->showNeteditAttributesModul(GNEAttributeCarrier::getTagProperties(SUMO_TAG_TAZ));
        // show drawing shape
        myTAZFrameParent->myDrawingShape->showDrawingShape();
        // hide edge common parameters
        myTAZFrameParent->myTAZCommonStatistics->hideTAZCommonStatisticsModul();
        // hide edge common parameters
        myTAZFrameParent->myTAZChildDefaultParameters->hideTAZChildDefaultParametersModul();
        // hide Edges graphics
        myTAZFrameParent->myTAZEdgesGraphic->hideTAZEdgesGraphicModul();
        // hide save TAZ Edges
        myTAZFrameParent->myTAZSaveChanges->hideTAZSaveChangesModul();
        // restore label
        myTAZCurrentLabel->setText("No TAZ selected");
        // clear net edges (always the last step due hideTAZEdgesGraphicModul() function)
        myNetEdges.clear();
        // clear selected edges
        mySelectedEdges.clear();
        // reset all weight values
        myMaxSourcePlusSinkWeight = 0;
        myMinSourcePlusSinkWeight = -1;
        myMaxSourceMinusSinkWeight = 0;
        myMinSourceMinusSinkWeight = -1;
    }
}


GNETAZ*
GNETAZFrame::TAZCurrent::getTAZ() const {
    return myEditedTAZ;
}


bool
GNETAZFrame::TAZCurrent::isTAZEdge(GNEEdge* edge) const {
    // simply iterate over edges and check edge parameter
    for (const auto& i : myTAZEdges) {
        if (i.edge == edge) {
            return true;
        }
    }
    // not found, then return false
    return false;
}


const std::vector<GNEEdge*>&
GNETAZFrame::TAZCurrent::getNetEdges() const {
    return myNetEdges;
}


const std::vector<GNEEdge*>&
GNETAZFrame::TAZCurrent::getSelectedEdges() const {
    return mySelectedEdges;
}


const std::vector<GNETAZFrame::TAZCurrent::TAZEdge>&
GNETAZFrame::TAZCurrent::getTAZEdges() const {
    return myTAZEdges;
}


void
GNETAZFrame::TAZCurrent::refreshTAZEdges() {
    // clear all curren TAZEdges
    myTAZEdges.clear();
    // clear weight values
    myMaxSourcePlusSinkWeight = 0;
    myMinSourcePlusSinkWeight = -1;
    myMaxSourceMinusSinkWeight = 0;
    myMinSourceMinusSinkWeight = -1;
    // only refresh if we're editing an TAZ
    if (myEditedTAZ) {
        // iterate over child additional and create TAZEdges
        for (const auto& TAZElement : myEditedTAZ->getChildTAZElements()) {
            addTAZChild(dynamic_cast<GNETAZSourceSink*>(TAZElement));
        }
        // update colors after add all edges
        for (auto& i : myTAZEdges) {
            i.updateColors();
        }
        // update edge colors
        myTAZFrameParent->myTAZEdgesGraphic->updateEdgeColors();
    }
}


void
GNETAZFrame::TAZCurrent::addTAZChild(GNETAZSourceSink* sourceSink) {
    // first make sure that additional is an TAZ Source or Sink
    if (sourceSink && ((sourceSink->getTagProperty().getTag() == SUMO_TAG_TAZSOURCE) || (sourceSink->getTagProperty().getTag() == SUMO_TAG_TAZSINK))) {
        GNEEdge* edge = myTAZFrameParent->myViewNet->getNet()->retrieveEdge(sourceSink->getAttribute(SUMO_ATTR_EDGE));
        // first check if TAZEdge has to be created
        bool createTAZEdge = true;
        for (auto& i : myTAZEdges) {
            if (i.edge == edge) {
                createTAZEdge = false;
                // update TAZ Source or Sink
                if (sourceSink->getTagProperty().getTag() == SUMO_TAG_TAZSOURCE) {
                    i.TAZSource = sourceSink;
                } else {
                    i.TAZSink = sourceSink;
                }
            }
        }
        // check if additional has to be created
        if (createTAZEdge) {
            if (sourceSink->getTagProperty().getTag() == SUMO_TAG_TAZSOURCE) {
                myTAZEdges.push_back(TAZEdge(this, edge, sourceSink, nullptr));
            } else {
                myTAZEdges.push_back(TAZEdge(this, edge, nullptr, sourceSink));
            }
        }
        // recalculate weights
        myMaxSourcePlusSinkWeight = 0;
        myMinSourcePlusSinkWeight = -1;
        myMaxSourceMinusSinkWeight = 0;
        myMinSourceMinusSinkWeight = -1;
        for (const auto& i : myTAZEdges) {
            // make sure that both TAZ Source and Sink exist
            if (i.TAZSource && i.TAZSink) {
                // obtain source plus sink
                double sourcePlusSink = i.TAZSource->getDepartWeight() + i.TAZSink->getDepartWeight();
                // check myMaxSourcePlusSinkWeight
                if (sourcePlusSink > myMaxSourcePlusSinkWeight) {
                    myMaxSourcePlusSinkWeight = sourcePlusSink;
                }
                // check myMinSourcePlusSinkWeight
                if ((myMinSourcePlusSinkWeight == -1) || (sourcePlusSink < myMinSourcePlusSinkWeight)) {
                    myMinSourcePlusSinkWeight = sourcePlusSink;
                }
                // obtain source minus sink
                double sourceMinusSink = i.TAZSource->getDepartWeight() - i.TAZSink->getDepartWeight();
                // use valor absolute
                if (sourceMinusSink < 0) {
                    sourceMinusSink *= -1;
                }
                // check myMaxSourcePlusSinkWeight
                if (sourceMinusSink > myMaxSourceMinusSinkWeight) {
                    myMaxSourceMinusSinkWeight = sourceMinusSink;
                }
                // check myMinSourcePlusSinkWeight
                if ((myMinSourceMinusSinkWeight == -1) || (sourceMinusSink < myMinSourceMinusSinkWeight)) {
                    myMinSourceMinusSinkWeight = sourceMinusSink;
                }
            }
        }
    } else {
        throw ProcessError("Invalid TAZ Child");
    }
}

// ---------------------------------------------------------------------------
// GNETAZFrame::TAZCommonStatistics - methods
// ---------------------------------------------------------------------------

GNETAZFrame::TAZCommonStatistics::TAZCommonStatistics(GNETAZFrame* TAZFrameParent) :
    FXGroupBox(TAZFrameParent->myContentFrame, "TAZ Statistics", GUIDesignGroupBoxFrame),
    myTAZFrameParent(TAZFrameParent) {
    // create label for statistics
    myStatisticsLabel = new FXLabel(this, "Statistics", 0, GUIDesignLabelFrameInformation);
}


GNETAZFrame::TAZCommonStatistics::~TAZCommonStatistics() {}


void
GNETAZFrame::TAZCommonStatistics::showTAZCommonStatisticsModul() {
    // always update statistics after show
    updateStatistics();
    show();
}


void
GNETAZFrame::TAZCommonStatistics::hideTAZCommonStatisticsModul() {
    hide();
}


void
GNETAZFrame::TAZCommonStatistics::updateStatistics() {
    if (myTAZFrameParent->myTAZCurrent->getTAZ()) {
        // declare ostringstream for statistics
        std::ostringstream information;
        information
                << "- Number of Edges: " << toString(myTAZFrameParent->myTAZCurrent->getTAZ()->getChildAdditionals().size() / 2) << "\n"
                << "- Min source: " << myTAZFrameParent->myTAZCurrent->getTAZ()->getAttribute(GNE_ATTR_MIN_SOURCE) << "\n"
                << "- Max source: " << myTAZFrameParent->myTAZCurrent->getTAZ()->getAttribute(GNE_ATTR_MAX_SOURCE) << "\n"
                << "- Average source: " << myTAZFrameParent->myTAZCurrent->getTAZ()->getAttribute(GNE_ATTR_AVERAGE_SOURCE) << "\n"
                << "\n"
                << "- Min sink: " << myTAZFrameParent->myTAZCurrent->getTAZ()->getAttribute(GNE_ATTR_MIN_SINK) << "\n"
                << "- Max sink: " << myTAZFrameParent->myTAZCurrent->getTAZ()->getAttribute(GNE_ATTR_MAX_SINK) << "\n"
                << "- Average sink: " << myTAZFrameParent->myTAZCurrent->getTAZ()->getAttribute(GNE_ATTR_AVERAGE_SINK);
        // set new label
        myStatisticsLabel->setText(information.str().c_str());
    } else {
        myStatisticsLabel->setText("No TAZ Selected");
    }
}

// ---------------------------------------------------------------------------
// GNETAZFrame::TAZSaveChanges - methods
// ---------------------------------------------------------------------------

GNETAZFrame::TAZSaveChanges::TAZSaveChanges(GNETAZFrame* TAZFrameParent) :
    FXGroupBox(TAZFrameParent->myContentFrame, "Modifications", GUIDesignGroupBoxFrame),
    myTAZFrameParent(TAZFrameParent) {
    // Create groupbox for save changes
    mySaveChangesButton = new FXButton(this, "Confirm changes", GUIIconSubSys::getIcon(GUIIcon::SAVE), this, MID_OK, GUIDesignButton);
    mySaveChangesButton->disable();
    // Create groupbox cancel changes
    myCancelChangesButton = new FXButton(this, "Cancel changes", GUIIconSubSys::getIcon(GUIIcon::CANCEL), this, MID_CANCEL, GUIDesignButton);
    myCancelChangesButton->disable();
}


GNETAZFrame::TAZSaveChanges::~TAZSaveChanges() {}


void
GNETAZFrame::TAZSaveChanges::showTAZSaveChangesModul() {
    show();
}


void
GNETAZFrame::TAZSaveChanges::hideTAZSaveChangesModul() {
    // cancel changes before hidding modul
    onCmdCancelChanges(0, 0, 0);
    hide();
}


void
GNETAZFrame::TAZSaveChanges::enableButtonsAndBeginUndoList() {
    // check that save changes is disabled
    if (!mySaveChangesButton->isEnabled()) {
        // enable mySaveChangesButton and myCancelChangesButton
        mySaveChangesButton->enable();
        myCancelChangesButton->enable();
        // start undo list set
        myTAZFrameParent->myViewNet->getUndoList()->p_begin("TAZ attributes");
    }
}


bool
GNETAZFrame::TAZSaveChanges::isChangesPending() const {
    // simply check if save Changes Button is enabled
    return mySaveChangesButton->isEnabled();
}


long
GNETAZFrame::TAZSaveChanges::onCmdSaveChanges(FXObject*, FXSelector, void*) {
    // check that save changes is enabled
    if (mySaveChangesButton->isEnabled()) {
        // disable mySaveChangesButton and myCancelChangesButtonand
        mySaveChangesButton->disable();
        myCancelChangesButton->disable();
        // finish undo list set
        myTAZFrameParent->myViewNet->getUndoList()->p_end();
    }
    return 1;
}


long
GNETAZFrame::TAZSaveChanges::onCmdCancelChanges(FXObject*, FXSelector, void*) {
    // check that save changes is enabled
    if (mySaveChangesButton->isEnabled()) {
        // disable buttons
        mySaveChangesButton->disable();
        myCancelChangesButton->disable();
        // abort undo list
        myTAZFrameParent->myViewNet->getUndoList()->p_abort();
        // always refresh TAZ Edges after removing TAZSources/Sinks
        myTAZFrameParent->myTAZCurrent->refreshTAZEdges();
        // update use edges button
        myTAZFrameParent->myTAZChildDefaultParameters->updateSelectEdgesButton();
    }
    return 1;
}

// ---------------------------------------------------------------------------
// GNETAZFrame::TAZChildDefaultParameters - methods
// ---------------------------------------------------------------------------

GNETAZFrame::TAZChildDefaultParameters::TAZChildDefaultParameters(GNETAZFrame* TAZFrameParent) :
    FXGroupBox(TAZFrameParent->myContentFrame, "TAZ Sources/Sinks", GUIDesignGroupBoxFrame),
    myTAZFrameParent(TAZFrameParent),
    myDefaultTAZSourceWeight(1),
    myDefaultTAZSinkWeight(1) {
    // create checkbox for toogle membership
    FXHorizontalFrame* toogleMembershipFrame = new FXHorizontalFrame(this, GUIDesignAuxiliarHorizontalFrame);
    new FXLabel(toogleMembershipFrame, "Membership", 0, GUIDesignLabelAttribute);
    myToggleMembership = new FXCheckButton(toogleMembershipFrame, "Toggle", this, MID_GNE_SET_ATTRIBUTE, GUIDesignCheckButton);
    // by default enabled
    myToggleMembership->setCheck(TRUE);
    // create default TAZ Source weight
    myDefaultTAZSourceFrame = new FXHorizontalFrame(this, GUIDesignAuxiliarHorizontalFrame);
    new FXLabel(myDefaultTAZSourceFrame, "New source", 0, GUIDesignLabelAttribute);
    myTextFieldDefaultValueTAZSources = new FXTextField(myDefaultTAZSourceFrame, GUIDesignTextFieldNCol, this, MID_GNE_SET_ATTRIBUTE, GUIDesignTextField);
    myTextFieldDefaultValueTAZSources->setText("1");
    // create default TAZ Sink weight
    myDefaultTAZSinkFrame = new FXHorizontalFrame(this, GUIDesignAuxiliarHorizontalFrame);
    new FXLabel(myDefaultTAZSinkFrame, "New sink", 0, GUIDesignLabelAttribute);
    myTextFieldDefaultValueTAZSinks = new FXTextField(myDefaultTAZSinkFrame, GUIDesignTextFieldNCol, this, MID_GNE_SET_ATTRIBUTE, GUIDesignTextField);
    myTextFieldDefaultValueTAZSinks->setText("1");
    // Create button for use selected edges
    myUseSelectedEdges = new FXButton(this, "Use selected edges", nullptr, this, MID_GNE_SELECT, GUIDesignButton);
    // Create information label
    std::ostringstream information;
    information
            << "- Toogle Membership:\n"
            << "  Create new Sources/Sinks\n"
            << "  with given weights.";
    myInformationLabel = new FXLabel(this, information.str().c_str(), 0, GUIDesignLabelFrameInformation);
}


GNETAZFrame::TAZChildDefaultParameters::~TAZChildDefaultParameters() {}


void
GNETAZFrame::TAZChildDefaultParameters::showTAZChildDefaultParametersModul() {
    // check if TAZ selection Statistics Modul has to be shown
    if (myToggleMembership->getCheck() == FALSE) {
        myTAZFrameParent->myTAZSelectionStatistics->showTAZSelectionStatisticsModul();
    } else {
        myTAZFrameParent->myTAZSelectionStatistics->hideTAZSelectionStatisticsModul();
    }
    // update selected button
    updateSelectEdgesButton();
    // show modul
    show();
}


void
GNETAZFrame::TAZChildDefaultParameters::hideTAZChildDefaultParametersModul() {
    // hide TAZ Selection Statistics Modul
    myTAZFrameParent->myTAZSelectionStatistics->hideTAZSelectionStatisticsModul();
    // hide modul
    hide();
}


void
GNETAZFrame::TAZChildDefaultParameters::updateSelectEdgesButton() {
    if (myToggleMembership->getCheck() == TRUE) {
        // check if use selected edges has to be enabled
        if (myTAZFrameParent->myTAZCurrent->getSelectedEdges().size() > 0) {
            myUseSelectedEdges->setText("Use selected edges");
            myUseSelectedEdges->enable();
        } else if (myTAZFrameParent->myTAZCurrent->getTAZEdges().size() > 0) {
            myUseSelectedEdges->setText("Remove all edges");
            myUseSelectedEdges->enable();
        } else {
            myUseSelectedEdges->setText("Use selected edges");
            myUseSelectedEdges->disable();
        }
    } else if (myTAZFrameParent->getTAZCurrentModul()->getTAZEdges().size() > 0) {
        // enable myUseSelectedEdges button
        myUseSelectedEdges->enable();
        // update mySelectEdgesOfSelection label
        if (myTAZFrameParent->myTAZSelectionStatistics->getEdgeAndTAZChildrenSelected().size() == 0) {
            // check if all edges of TAZChildren are selected
            bool allSelected = true;
            for (const auto& i : myTAZFrameParent->getTAZCurrentModul()->getTAZEdges()) {
                if (!i.edge->isAttributeCarrierSelected()) {
                    allSelected = false;
                }
            }
            if (allSelected) {
                myUseSelectedEdges->setText("Remove all edges from selection");
            } else {
                myUseSelectedEdges->setText("Add all edges to selection");
            }
        } else if (myTAZFrameParent->myTAZSelectionStatistics->getEdgeAndTAZChildrenSelected().size() == 1) {
            if (myTAZFrameParent->myTAZSelectionStatistics->getEdgeAndTAZChildrenSelected().front().edge->isAttributeCarrierSelected()) {
                myUseSelectedEdges->setText("Remove edge from selection");
            } else {
                myUseSelectedEdges->setText("Add edge to selection");
            }
        } else {
            // check if all edges of TAZChildren selected are selected
            bool allSelected = true;
            for (const auto& i : myTAZFrameParent->myTAZSelectionStatistics->getEdgeAndTAZChildrenSelected()) {
                if (!i.edge->isAttributeCarrierSelected()) {
                    allSelected = false;
                }
            }
            if (allSelected) {
                myUseSelectedEdges->setText(("Remove " + toString(myTAZFrameParent->myTAZSelectionStatistics->getEdgeAndTAZChildrenSelected().size()) + " from to selection").c_str());
            } else {
                myUseSelectedEdges->setText(("Add " + toString(myTAZFrameParent->myTAZSelectionStatistics->getEdgeAndTAZChildrenSelected().size()) + " edges to selection").c_str());
            }
        }
    } else {
        // TAZ doesn't have children, then disable button
        myUseSelectedEdges->disable();
    }
}


double
GNETAZFrame::TAZChildDefaultParameters::getDefaultTAZSourceWeight() const {
    return myDefaultTAZSourceWeight;
}


double
GNETAZFrame::TAZChildDefaultParameters::getDefaultTAZSinkWeight() const {
    return myDefaultTAZSinkWeight;
}


bool
GNETAZFrame::TAZChildDefaultParameters::getToggleMembership() const {
    return (myToggleMembership->getCheck() == TRUE);
}


long
GNETAZFrame::TAZChildDefaultParameters::onCmdSetDefaultValues(FXObject* obj, FXSelector, void*) {
    // find edited object
    if (obj == myToggleMembership) {
        // first clear selected edges
        myTAZFrameParent->myTAZSelectionStatistics->clearSelectedEdges();
        // set text of myToggleMembership
        if (myToggleMembership->getCheck() == TRUE) {
            myToggleMembership->setText("toogle");
            // show TAZSource/Sink Frames
            myDefaultTAZSourceFrame->show();
            myDefaultTAZSinkFrame->show();
            // update information label
            std::ostringstream information;
            information
                    << "- Toogle Membership:\n"
                    << "  Create new Sources/Sinks\n"
                    << "  with given weights.";
            myInformationLabel->setText(information.str().c_str());
            // hide TAZSelectionStatistics
            myTAZFrameParent->myTAZSelectionStatistics->hideTAZSelectionStatisticsModul();
            // check if use selected edges has to be enabled
            if (myTAZFrameParent->myTAZCurrent->getSelectedEdges().size() > 0) {
                myUseSelectedEdges->setText("Use selected edges");
            } else if (myTAZFrameParent->myTAZCurrent->getTAZEdges().size() > 0) {
                myUseSelectedEdges->setText("Remove all edges");
            } else {
                myUseSelectedEdges->setText("Use selected edges");
                myUseSelectedEdges->disable();
            }
        } else {
            myToggleMembership->setText("keep");
            // hide TAZSource/Sink Frames
            myDefaultTAZSourceFrame->hide();
            myDefaultTAZSinkFrame->hide();
            // update information label
            std::ostringstream information;
            information
                    << "- Keep Membership:\n"
                    << "  Select Sources/Sinks.\n"
                    << "- Press ESC to clear\n"
                    << "  current selection.";
            myInformationLabel->setText(information.str().c_str());
            // show TAZSelectionStatistics
            myTAZFrameParent->myTAZSelectionStatistics->showTAZSelectionStatisticsModul();
        }
        // update button
        updateSelectEdgesButton();
    } else if (obj == myTextFieldDefaultValueTAZSources) {
        // check if given value is valid
        if (GNEAttributeCarrier::canParse<double>(myTextFieldDefaultValueTAZSources->getText().text())) {
            myDefaultTAZSourceWeight = GNEAttributeCarrier::parse<double>(myTextFieldDefaultValueTAZSources->getText().text());
            // check if myDefaultTAZSourceWeight is greather than 0
            if (myDefaultTAZSourceWeight >= 0) {
                // set valid color
                myTextFieldDefaultValueTAZSources->setTextColor(FXRGB(0, 0, 0));
            } else {
                // set invalid color
                myTextFieldDefaultValueTAZSources->setTextColor(FXRGB(255, 0, 0));
                myDefaultTAZSourceWeight = 1;
            }
        } else {
            // set invalid color
            myTextFieldDefaultValueTAZSources->setTextColor(FXRGB(255, 0, 0));
            myDefaultTAZSourceWeight = 1;
        }
    } else if (obj == myTextFieldDefaultValueTAZSinks) {
        // check if given value is valid
        if (GNEAttributeCarrier::canParse<double>(myTextFieldDefaultValueTAZSinks->getText().text())) {
            myDefaultTAZSinkWeight = GNEAttributeCarrier::parse<double>(myTextFieldDefaultValueTAZSinks->getText().text());
            // check if myDefaultTAZSinkWeight is greather than 0
            if (myDefaultTAZSinkWeight >= 0) {
                // set valid color
                myTextFieldDefaultValueTAZSinks->setTextColor(FXRGB(0, 0, 0));
            } else {
                // set invalid color
                myTextFieldDefaultValueTAZSinks->setTextColor(FXRGB(255, 0, 0));
                myDefaultTAZSinkWeight = 1;
            }
        } else {
            // set invalid color
            myTextFieldDefaultValueTAZSinks->setTextColor(FXRGB(255, 0, 0));
            myDefaultTAZSinkWeight = 1;
        }
    }
    return 1;
}


long
GNETAZFrame::TAZChildDefaultParameters::onCmdUseSelectedEdges(FXObject*, FXSelector, void*) {
    // select edge or create new TAZ Source/Child, depending of myToggleMembership
    if (myToggleMembership->getCheck() == TRUE) {
        // first drop all edges
        myTAZFrameParent->dropTAZMembers();
        // iterate over selected edges and add it as TAZMember
        for (const auto& i : myTAZFrameParent->myTAZCurrent->getSelectedEdges()) {
            myTAZFrameParent->addOrRemoveTAZMember(i);
        }
        // update selected button
        updateSelectEdgesButton();
    } else {
        if (myTAZFrameParent->myTAZSelectionStatistics->getEdgeAndTAZChildrenSelected().size() == 0) {
            // first check if all TAZEdges are selected
            bool allSelected = true;
            for (const auto& i : myTAZFrameParent->getTAZCurrentModul()->getTAZEdges()) {
                if (!i.edge->isAttributeCarrierSelected()) {
                    allSelected = false;
                }
            }
            // select or unselect all depending of allSelected
            if (allSelected) {
                // remove form selection all TAZEdges
                for (const auto& i : myTAZFrameParent->getTAZCurrentModul()->getTAZEdges()) {
                    // enable save button
                    myTAZFrameParent->myTAZSaveChanges->enableButtonsAndBeginUndoList();
                    // change attribute selected
                    i.edge->setAttribute(GNE_ATTR_SELECTED, "false", myTAZFrameParent->myViewNet->getUndoList());
                }
            } else {
                // add to selection all TAZEdges
                for (const auto& i : myTAZFrameParent->getTAZCurrentModul()->getTAZEdges()) {
                    // enable save button
                    myTAZFrameParent->myTAZSaveChanges->enableButtonsAndBeginUndoList();
                    // change attribute selected
                    i.edge->setAttribute(GNE_ATTR_SELECTED, "true", myTAZFrameParent->myViewNet->getUndoList());
                }
            }
        } else {
            // first check if all TAZEdges are selected
            bool allSelected = true;
            for (const auto& i : myTAZFrameParent->myTAZSelectionStatistics->getEdgeAndTAZChildrenSelected()) {
                if (!i.edge->isAttributeCarrierSelected()) {
                    allSelected = false;
                }
            }
            // select or unselect all depending of allSelected
            if (allSelected) {
                // only remove from selection selected TAZEdges
                for (const auto& i : myTAZFrameParent->myTAZSelectionStatistics->getEdgeAndTAZChildrenSelected()) {
                    if (i.edge->isAttributeCarrierSelected()) {
                        // enable save button
                        myTAZFrameParent->myTAZSaveChanges->enableButtonsAndBeginUndoList();
                        // change attribute selected
                        i.edge->setAttribute(GNE_ATTR_SELECTED, "false", myTAZFrameParent->myViewNet->getUndoList());
                    }
                }
            } else {
                // only add to selection selected TAZEdges
                for (const auto& i : myTAZFrameParent->myTAZSelectionStatistics->getEdgeAndTAZChildrenSelected()) {
                    if (!i.edge->isAttributeCarrierSelected()) {
                        // enable save button
                        myTAZFrameParent->myTAZSaveChanges->enableButtonsAndBeginUndoList();
                        // change attribute selected
                        i.edge->setAttribute(GNE_ATTR_SELECTED, "true", myTAZFrameParent->myViewNet->getUndoList());
                    }
                }
            }
        }
    }
    // update selection button
    myTAZFrameParent->myTAZChildDefaultParameters->updateSelectEdgesButton();
    // update view net
    myTAZFrameParent->myViewNet->updateViewNet();
    return 1;
}

// ---------------------------------------------------------------------------
// GNETAZFrame::TAZSelectionStatistics - methods
// ---------------------------------------------------------------------------

GNETAZFrame::TAZSelectionStatistics::TAZSelectionStatistics(GNETAZFrame* TAZFrameParent) :
    FXGroupBox(TAZFrameParent->myContentFrame, "Selection Statistics", GUIDesignGroupBoxFrame),
    myTAZFrameParent(TAZFrameParent) {
    // create default TAZ Source weight
    myTAZSourceFrame = new FXHorizontalFrame(this, GUIDesignAuxiliarHorizontalFrame);
    new FXLabel(myTAZSourceFrame, "Source", 0, GUIDesignLabelAttribute);
    myTextFieldTAZSourceWeight = new FXTextField(myTAZSourceFrame, GUIDesignTextFieldNCol, this, MID_GNE_SET_ATTRIBUTE, GUIDesignTextField);
    myTAZSourceFrame->hide();
    // create default TAZ Sink weight
    myTAZSinkFrame = new FXHorizontalFrame(this, GUIDesignAuxiliarHorizontalFrame);
    new FXLabel(myTAZSinkFrame, "Sink", 0, GUIDesignLabelAttribute);
    myTextFieldTAZSinkWeight = new FXTextField(myTAZSinkFrame, GUIDesignTextFieldNCol, this, MID_GNE_SET_ATTRIBUTE, GUIDesignTextField);
    myTAZSinkFrame->hide();
    // create label for statistics
    myStatisticsLabel = new FXLabel(this, "Statistics", 0, GUIDesignLabelFrameInformation);
}


GNETAZFrame::TAZSelectionStatistics::~TAZSelectionStatistics() {}


void
GNETAZFrame::TAZSelectionStatistics::showTAZSelectionStatisticsModul() {
    // update Statistics before show
    updateStatistics();
    show();
}


void
GNETAZFrame::TAZSelectionStatistics::hideTAZSelectionStatisticsModul() {
    // clear children before hide
    clearSelectedEdges();
    hide();
}


bool
GNETAZFrame::TAZSelectionStatistics::selectEdge(const TAZCurrent::TAZEdge& TAZEdge) {
    // find TAZEdge using edge as criterium wasn't previously selected
    for (const auto& i : myEdgeAndTAZChildrenSelected) {
        if (i.edge == TAZEdge.edge) {
            throw ProcessError("TAZEdge already selected");
        }
    }
    // add edge and their TAZ Children into myTAZChildSelected
    myEdgeAndTAZChildrenSelected.push_back(TAZEdge);
    // always update statistics after insertion
    updateStatistics();
    // update edge colors
    myTAZFrameParent->myTAZEdgesGraphic->updateEdgeColors();
    // update selection button
    myTAZFrameParent->myTAZChildDefaultParameters->updateSelectEdgesButton();
    return true;
}


bool
GNETAZFrame::TAZSelectionStatistics::unselectEdge(GNEEdge* edge) {
    if (edge) {
        // find TAZEdge using edge as criterium
        for (auto i = myEdgeAndTAZChildrenSelected.begin(); i != myEdgeAndTAZChildrenSelected.end(); i++) {
            if (i->edge == edge) {
                myEdgeAndTAZChildrenSelected.erase(i);
                // always update statistics after insertion
                updateStatistics();
                // update edge colors
                myTAZFrameParent->myTAZEdgesGraphic->updateEdgeColors();
                // update selection button
                myTAZFrameParent->myTAZChildDefaultParameters->updateSelectEdgesButton();
                return true;
            }
        }
        // throw exception if edge wasn't found
        throw ProcessError("edge wasn't found");
    } else {
        throw ProcessError("Invalid edge");
    }
}


bool
GNETAZFrame::TAZSelectionStatistics::isEdgeSelected(GNEEdge* edge) {
    // find TAZEdge using edge as criterium
    for (const auto& i : myEdgeAndTAZChildrenSelected) {
        if (i.edge == edge) {
            return true;
        }
    }
    // edge wasn't found, then return false
    return false;
}


void
GNETAZFrame::TAZSelectionStatistics::clearSelectedEdges() {
    // clear all selected edges (and the TAZ Children)
    myEdgeAndTAZChildrenSelected.clear();
    // always update statistics after clear edges
    updateStatistics();
    // update edge colors
    myTAZFrameParent->myTAZEdgesGraphic->updateEdgeColors();
    // update selection button
    myTAZFrameParent->myTAZChildDefaultParameters->updateSelectEdgesButton();
}


const std::vector<GNETAZFrame::TAZCurrent::TAZEdge>&
GNETAZFrame::TAZSelectionStatistics::getEdgeAndTAZChildrenSelected() const {
    return myEdgeAndTAZChildrenSelected;
}


long
GNETAZFrame::TAZSelectionStatistics::onCmdSetNewValues(FXObject* obj, FXSelector, void*) {
    if (obj == myTextFieldTAZSourceWeight) {
        // check if given value is valid
        if (GNEAttributeCarrier::canParse<double>(myTextFieldTAZSourceWeight->getText().text())) {
            double newTAZSourceWeight = GNEAttributeCarrier::parse<double>(myTextFieldTAZSourceWeight->getText().text());
            // check if myDefaultTAZSourceWeight is greather than 0
            if (newTAZSourceWeight >= 0) {
                // set valid color in TextField
                myTextFieldTAZSourceWeight->setTextColor(FXRGB(0, 0, 0));
                // enable save button
                myTAZFrameParent->myTAZSaveChanges->enableButtonsAndBeginUndoList();
                // update weight of all TAZSources
                for (const auto&  i : myEdgeAndTAZChildrenSelected) {
                    i.TAZSource->setAttribute(SUMO_ATTR_WEIGHT, myTextFieldTAZSourceWeight->getText().text(), myTAZFrameParent->myViewNet->getUndoList());
                }
                // refresh TAZ Edges
                myTAZFrameParent->getTAZCurrentModul()->refreshTAZEdges();
            } else {
                // set invalid color
                myTextFieldTAZSourceWeight->setTextColor(FXRGB(255, 0, 0));
            }
        } else {
            // set invalid color
            myTextFieldTAZSourceWeight->setTextColor(FXRGB(255, 0, 0));
        }
    } else if (obj == myTextFieldTAZSinkWeight) {
        // check if given value is valid
        if (GNEAttributeCarrier::canParse<double>(myTextFieldTAZSinkWeight->getText().text())) {
            double newTAZSinkWeight = GNEAttributeCarrier::parse<double>(myTextFieldTAZSinkWeight->getText().text());
            // check if myDefaultTAZSinkWeight is greather than 0
            if (newTAZSinkWeight >= 0) {
                // set valid color in TextField
                myTextFieldTAZSinkWeight->setTextColor(FXRGB(0, 0, 0));
                // enable save button
                myTAZFrameParent->myTAZSaveChanges->enableButtonsAndBeginUndoList();
                // update weight of all TAZSources
                for (const auto&  i : myEdgeAndTAZChildrenSelected) {
                    i.TAZSink->setAttribute(SUMO_ATTR_WEIGHT, myTextFieldTAZSinkWeight->getText().text(), myTAZFrameParent->myViewNet->getUndoList());
                }
                // refresh TAZ Edges
                myTAZFrameParent->getTAZCurrentModul()->refreshTAZEdges();
            } else {
                // set invalid color
                myTextFieldTAZSinkWeight->setTextColor(FXRGB(255, 0, 0));
            }
        } else {
            // set invalid color
            myTextFieldTAZSinkWeight->setTextColor(FXRGB(255, 0, 0));
        }
    }
    return 1;
}


long
GNETAZFrame::TAZSelectionStatistics::onCmdSelectEdges(FXObject*, FXSelector, void*) {
    if (myEdgeAndTAZChildrenSelected.size() == 0) {
        // add to selection all TAZEdges
        for (const auto& i : myTAZFrameParent->getTAZCurrentModul()->getTAZEdges()) {
            // avoid empty undolists
            if (!i.edge->isAttributeCarrierSelected()) {
                // enable save button
                myTAZFrameParent->myTAZSaveChanges->enableButtonsAndBeginUndoList();
                // change attribute selected
                i.edge->setAttribute(GNE_ATTR_SELECTED, "true", myTAZFrameParent->myViewNet->getUndoList());
            }
        }
    } else {
        // only add to selection selected TAZEdges
        for (const auto& i : myEdgeAndTAZChildrenSelected) {
            // avoid empty undolists
            if (!i.edge->isAttributeCarrierSelected()) {
                // enable save button
                myTAZFrameParent->myTAZSaveChanges->enableButtonsAndBeginUndoList();
                // change attribute selected
                i.edge->setAttribute(GNE_ATTR_SELECTED, "true", myTAZFrameParent->myViewNet->getUndoList());
            }
        }
    }
    return 1;
}


void
GNETAZFrame::TAZSelectionStatistics::updateStatistics() {
    if (myEdgeAndTAZChildrenSelected.size() > 0) {
        // show TAZSources/Sinks frames
        myTAZSourceFrame->show();
        myTAZSinkFrame->show();
        // declare string sets for TextFields (to avoid duplicated values)
        std::set<std::string> weightSourceSet;
        std::set<std::string> weightSinkSet;
        // declare stadistic variables
        double weight = 0;
        double maxWeightSource = 0;
        double minWeightSource = -1;
        double averageWeightSource = 0;
        double maxWeightSink = 0;
        double minWeightSink = -1;
        double averageWeightSink = 0;
        // iterate over child additional
        for (const auto&  i : myEdgeAndTAZChildrenSelected) {
            //start with sources
            weight = i.TAZSource->getDepartWeight();
            // insert source weight in weightSinkTextField
            weightSourceSet.insert(toString(weight));
            // check max Weight
            if (maxWeightSource < weight) {
                maxWeightSource = weight;
            }
            // check min Weight
            if (minWeightSource == -1 || (maxWeightSource < weight)) {
                minWeightSource = weight;
            }
            // update Average
            averageWeightSource += weight;
            // continue with sinks
            weight = i.TAZSink->getDepartWeight();
            // save sink weight in weightSinkTextField
            weightSinkSet.insert(toString(weight));
            // check max Weight
            if (maxWeightSink < weight) {
                maxWeightSink = weight;
            }
            // check min Weight
            if (minWeightSink == -1 || (maxWeightSink < weight)) {
                minWeightSink = weight;
            }
            // update Average
            averageWeightSink += weight;
        }
        // calculate average
        averageWeightSource /= myEdgeAndTAZChildrenSelected.size();
        averageWeightSink /= myEdgeAndTAZChildrenSelected.size();
        // declare ostringstream for statistics
        std::ostringstream information;
        std::string edgeInformation;
        // first fill edgeInformation
        if (myEdgeAndTAZChildrenSelected.size() == 1) {
            edgeInformation = "- Edge ID: " + myEdgeAndTAZChildrenSelected.begin()->edge->getID();
        } else {
            edgeInformation = "- Number of edges: " + toString(myEdgeAndTAZChildrenSelected.size());
        }
        // fill rest of information
        information
                << edgeInformation << "\n"
                << "- Min source: " << toString(minWeightSource) << "\n"
                << "- Max source: " << toString(maxWeightSource) << "\n"
                << "- Average source: " << toString(averageWeightSource) << "\n"
                << "\n"
                << "- Min sink: " << toString(minWeightSink) << "\n"
                << "- Max sink: " << toString(maxWeightSink) << "\n"
                << "- Average sink: " << toString(averageWeightSink);
        // set new label
        myStatisticsLabel->setText(information.str().c_str());
        // set TextFields (Text and color)
        myTextFieldTAZSourceWeight->setText(joinToString(weightSourceSet, " ").c_str());
        myTextFieldTAZSourceWeight->setTextColor(FXRGB(0, 0, 0));
        myTextFieldTAZSinkWeight->setText(joinToString(weightSinkSet, " ").c_str());
        myTextFieldTAZSinkWeight->setTextColor(FXRGB(0, 0, 0));
    } else {
        // hide TAZSources/Sinks frames
        myTAZSourceFrame->hide();
        myTAZSinkFrame->hide();
        // hide myStatisticsLabel
        myStatisticsLabel->setText("No edges selected");
    }
}

// ---------------------------------------------------------------------------
// GNETAZFrame::TAZParameters- methods
// ---------------------------------------------------------------------------

GNETAZFrame::TAZParameters::TAZParameters(GNETAZFrame* TAZFrameParent) :
    FXGroupBox(TAZFrameParent->myContentFrame, "TAZ parameters", GUIDesignGroupBoxFrame),
    myTAZFrameParent(TAZFrameParent) {
    // create Button and string textField for color and set blue as default color
    FXHorizontalFrame* colorParameter = new FXHorizontalFrame(this, GUIDesignAuxiliarHorizontalFrame);
    myColorEditor = new FXButton(colorParameter, toString(SUMO_ATTR_COLOR).c_str(), 0, this, MID_GNE_SET_ATTRIBUTE_DIALOG, GUIDesignButtonAttribute);
    myTextFieldColor = new FXTextField(colorParameter, GUIDesignTextFieldNCol, this, MID_GNE_SET_ATTRIBUTE, GUIDesignTextField);
    myTextFieldColor->setText("blue");
    // create Label and CheckButton for use innen edges with true as default value
    FXHorizontalFrame* useInnenEdges = new FXHorizontalFrame(this, GUIDesignAuxiliarHorizontalFrame);
    new FXLabel(useInnenEdges, "Edges within", 0, GUIDesignLabelAttribute);
    myAddEdgesWithinCheckButton = new FXCheckButton(useInnenEdges, "use", this, MID_GNE_SET_ATTRIBUTE, GUIDesignCheckButton);
    myAddEdgesWithinCheckButton->setCheck(true);
    // Create help button
    myHelpTAZAttribute = new FXButton(this, "Help", 0, this, MID_HELP, GUIDesignButtonRectangular);
}


GNETAZFrame::TAZParameters::~TAZParameters() {}


void
GNETAZFrame::TAZParameters::showTAZParametersModul() {
    FXGroupBox::show();
}


void
GNETAZFrame::TAZParameters::hideTAZParametersModul() {
    FXGroupBox::hide();
}


bool
GNETAZFrame::TAZParameters::isCurrentParametersValid() const {
    return GNEAttributeCarrier::canParse<RGBColor>(myTextFieldColor->getText().text());
}


bool
GNETAZFrame::TAZParameters::isAddEdgesWithinEnabled() const {
    return (myAddEdgesWithinCheckButton->getCheck() == TRUE);
}


std::map<SumoXMLAttr, std::string>
GNETAZFrame::TAZParameters::getAttributesAndValues() const {
    std::map<SumoXMLAttr, std::string> parametersAndValues;
    // get color (currently the only editable attribute)
    parametersAndValues[SUMO_ATTR_COLOR] = myTextFieldColor->getText().text();
    return parametersAndValues;
}


long
GNETAZFrame::TAZParameters::onCmdSetColorAttribute(FXObject*, FXSelector, void*) {
    // create FXColorDialog
    FXColorDialog colordialog(this, tr("Color Dialog"));
    colordialog.setTarget(this);
    // If previous attribute wasn't correct, set black as default color
    if (GNEAttributeCarrier::canParse<RGBColor>(myTextFieldColor->getText().text())) {
        colordialog.setRGBA(MFXUtils::getFXColor(RGBColor::parseColor(myTextFieldColor->getText().text())));
    } else {
        colordialog.setRGBA(MFXUtils::getFXColor(RGBColor::parseColor("blue")));
    }
    // execute dialog to get a new color
    if (colordialog.execute()) {
        myTextFieldColor->setText(toString(MFXUtils::getRGBColor(colordialog.getRGBA())).c_str());
        onCmdSetAttribute(0, 0, 0);
    }
    return 0;
}


long
GNETAZFrame::TAZParameters::onCmdSetAttribute(FXObject*, FXSelector, void*) {
    // only COLOR text field has to be checked
    bool currentParametersValid = GNEAttributeCarrier::canParse<RGBColor>(myTextFieldColor->getText().text());
    // change color of textfield dependig of myCurrentParametersValid
    if (currentParametersValid) {
        myTextFieldColor->setTextColor(FXRGB(0, 0, 0));
        myTextFieldColor->killFocus();
    } else {
        myTextFieldColor->setTextColor(FXRGB(255, 0, 0));
        currentParametersValid = false;
    }
    // change useInnenEdgesCheckButton text
    if (myAddEdgesWithinCheckButton->getCheck() == TRUE) {
        myAddEdgesWithinCheckButton->setText("use");
    } else {
        myAddEdgesWithinCheckButton->setText("not use");
    }
    return 0;
}


long
GNETAZFrame::TAZParameters::onCmdHelp(FXObject*, FXSelector, void*) {
    myTAZFrameParent->openHelpAttributesDialog(GNEAttributeCarrier::getTagProperties(SUMO_TAG_TAZ));
    return 1;
}

// ---------------------------------------------------------------------------
// GNETAZFrame::TAZEdgesGraphic - methods
// ---------------------------------------------------------------------------

GNETAZFrame::TAZEdgesGraphic::TAZEdgesGraphic(GNETAZFrame* TAZFrameParent) :
    FXGroupBox(TAZFrameParent->myContentFrame, "Edges", GUIDesignGroupBoxFrame),
    myTAZFrameParent(TAZFrameParent),
    myEdgeDefaultColor(RGBColor::GREY),
    myEdgeSelectedColor(RGBColor::MAGENTA) {
    // create label for non taz edge color information
    FXLabel* NonTAZEdgeLabel = new FXLabel(this, "Non TAZ Edge", nullptr, GUIDesignLabelCenter);
    NonTAZEdgeLabel->setBackColor(MFXUtils::getFXColor(myEdgeDefaultColor));
    NonTAZEdgeLabel->setTextColor(MFXUtils::getFXColor(RGBColor::WHITE));
    // create label for selected TAZEdge color information
    FXLabel* selectedTAZEdgeLabel = new FXLabel(this, "Selected TAZ Edge", nullptr, GUIDesignLabelCenter);
    selectedTAZEdgeLabel->setBackColor(MFXUtils::getFXColor(myEdgeSelectedColor));
    // build rainbow
    GNEFrameModuls::buildRainbow(this);
    // create Radio button for show edges by source weight
    myColorBySourceWeight = new FXRadioButton(this, "Color by Source", this, MID_CHOOSEN_OPERATION, GUIDesignRadioButton);
    // create Radio button for show edges by sink weight
    myColorBySinkWeight = new FXRadioButton(this, "Color by Sink", this, MID_CHOOSEN_OPERATION, GUIDesignRadioButton);
    // create Radio button for show edges by source + sink weight
    myColorBySourcePlusSinkWeight = new FXRadioButton(this, "Color by Source + Sink", this, MID_CHOOSEN_OPERATION, GUIDesignRadioButton);
    // create Radio button for show edges by source - sink weight
    myColorBySourceMinusSinkWeight = new FXRadioButton(this, "Color by Source - Sink", this, MID_CHOOSEN_OPERATION, GUIDesignRadioButton);
    // show by source as default
    myColorBySourceWeight->setCheck(true);
}


GNETAZFrame::TAZEdgesGraphic::~TAZEdgesGraphic() {}


void
GNETAZFrame::TAZEdgesGraphic::showTAZEdgesGraphicModul() {
    // update edge colors
    updateEdgeColors();
    show();
}


void
GNETAZFrame::TAZEdgesGraphic::hideTAZEdgesGraphicModul() {
    // iterate over all edges and restore color
    for (const auto& i : myTAZFrameParent->myTAZCurrent->getNetEdges()) {
        for (const auto j : i->getLanes()) {
            j->setSpecialColor(nullptr);
        }
    }
    hide();
}


void
GNETAZFrame::TAZEdgesGraphic::updateEdgeColors() {
    const std::vector<RGBColor>& scaledColors = GNEViewNetHelper::getRainbowScaledColors();
    // start painting all edges in gray
    for (const auto& i : myTAZFrameParent->myTAZCurrent->getNetEdges()) {
        // set candidate color (in this case, gray)
        for (const auto j : i->getLanes()) {
            j->setSpecialColor(&myEdgeDefaultColor);
        }
    }
    // now paint Source/sinks colors
    for (const auto& i : myTAZFrameParent->myTAZCurrent->getTAZEdges()) {
        // set candidate color (in this case,
        for (const auto j : i.edge->getLanes()) {
            // check what will be painted (source, sink or both)
            if (myColorBySourceWeight->getCheck() == TRUE) {
                j->setSpecialColor(&scaledColors.at(i.sourceColor), i.TAZSource->getDepartWeight());
            } else if (myColorBySinkWeight->getCheck() == TRUE) {
                j->setSpecialColor(&scaledColors.at(i.sinkColor), i.TAZSink->getDepartWeight());
            } else if (myColorBySourcePlusSinkWeight->getCheck() == TRUE) {
                j->setSpecialColor(&scaledColors.at(i.sourcePlusSinkColor), i.TAZSource->getDepartWeight() + i.TAZSink->getDepartWeight());
            } else {
                j->setSpecialColor(&scaledColors.at(i.sourceMinusSinkColor), i.TAZSource->getDepartWeight() - i.TAZSink->getDepartWeight());
            }
        }
    }
    // as last step paint candidate colors
    for (const auto& i : myTAZFrameParent->myTAZSelectionStatistics->getEdgeAndTAZChildrenSelected()) {
        // set candidate selected color
        for (const auto& j : i.edge->getLanes()) {
            j->setSpecialColor(&myEdgeSelectedColor);
        }
    }
    // always update view after setting new colors
    myTAZFrameParent->myViewNet->updateViewNet();
}


long
GNETAZFrame::TAZEdgesGraphic::onCmdChoosenBy(FXObject* obj, FXSelector, void*) {
    // check what radio was pressed and disable the others
    if (obj == myColorBySourceWeight) {
        myColorBySinkWeight->setCheck(FALSE);
        myColorBySourcePlusSinkWeight->setCheck(FALSE);
        myColorBySourceMinusSinkWeight->setCheck(FALSE);
    } else if (obj == myColorBySinkWeight) {
        myColorBySourceWeight->setCheck(FALSE);
        myColorBySourcePlusSinkWeight->setCheck(FALSE);
        myColorBySourceMinusSinkWeight->setCheck(FALSE);
    } else if (obj == myColorBySourcePlusSinkWeight) {
        myColorBySourceWeight->setCheck(FALSE);
        myColorBySinkWeight->setCheck(FALSE);
        myColorBySourceMinusSinkWeight->setCheck(FALSE);
    } else if (obj == myColorBySourceMinusSinkWeight) {
        myColorBySourceWeight->setCheck(FALSE);
        myColorBySinkWeight->setCheck(FALSE);
        myColorBySourcePlusSinkWeight->setCheck(FALSE);
    }
    // update edge colors
    updateEdgeColors();
    return 1;
}

// ---------------------------------------------------------------------------
// GNETAZFrame - methods
// ---------------------------------------------------------------------------

GNETAZFrame::GNETAZFrame(FXHorizontalFrame* horizontalFrameParent, GNEViewNet* viewNet) :
    GNEFrame(horizontalFrameParent, viewNet, "TAZs") {

    // create current TAZ modul
    myTAZCurrent = new TAZCurrent(this);

    // Create TAZ Parameters modul
    myTAZParameters = new TAZParameters(this);

    /// @brief create  Netedit parameter
    myNeteditAttributes = new GNEFrameAttributesModuls::NeteditAttributes(this);

    // Create drawing controls modul
    myDrawingShape = new GNEFrameModuls::DrawingShape(this);

    // Create TAZ Edges Common Statistics modul
    myTAZCommonStatistics = new TAZCommonStatistics(this);

    // Create save TAZ Edges modul
    myTAZSaveChanges = new TAZSaveChanges(this);

    // Create TAZ Edges Common Parameters modul
    myTAZChildDefaultParameters = new TAZChildDefaultParameters(this);

    // Create TAZ Edges Selection Statistics modul
    myTAZSelectionStatistics = new TAZSelectionStatistics(this);

    // Create TAZ Edges Common Parameters modul
    myTAZEdgesGraphic = new TAZEdgesGraphic(this);

    // by default there isn't a TAZ
    myTAZCurrent->setTAZ(nullptr);
}


GNETAZFrame::~GNETAZFrame() {
}


void
GNETAZFrame::hide() {
    // hide frame
    GNEFrame::hide();
}


bool
GNETAZFrame::processClick(const Position& clickedPosition, const GNEViewNetHelper::ObjectsUnderCursor& objectsUnderCursor) {
    // Declare map to keep values
    std::map<SumoXMLAttr, std::string> valuesOfElement;
    if (myDrawingShape->isDrawing()) {
        // add or delete a new point depending of flag "delete last created point"
        if (myDrawingShape->getDeleteLastCreatedPoint()) {
            myDrawingShape->removeLastPoint();
        } else {
            myDrawingShape->addNewPoint(clickedPosition);
        }
        return true;
    } else if ((myTAZCurrent->getTAZ() == nullptr) || (objectsUnderCursor.getTAZElementFront() && myTAZCurrent->getTAZ() && !myTAZSaveChanges->isChangesPending())) {
        // if user click over an TAZ and there isn't changes pending, then select a new TAZ
        if (objectsUnderCursor.getTAZElementFront()) {
            // avoid reset of Frame if user doesn't click over an TAZ
            myTAZCurrent->setTAZ(dynamic_cast<GNETAZ*>(objectsUnderCursor.getTAZElementFront()));
            return true;
        } else {
            return false;
        }
    } else if (objectsUnderCursor.getEdgeFront()) {
        // if toogle Edge is enabled, select edge. In other case create two new TAZSource/Sinks
        if (myTAZChildDefaultParameters->getToggleMembership()) {
            // create new TAZSource/Sinks or delete it
            return addOrRemoveTAZMember(objectsUnderCursor.getEdgeFront());
        } else {
            // first check if clicked edge was previously selected
            if (myTAZSelectionStatistics->isEdgeSelected(objectsUnderCursor.getEdgeFront())) {
                // clear selected edges
                myTAZSelectionStatistics->clearSelectedEdges();
            } else {
                // iterate over TAZEdges saved in TAZCurrent (it contains the Edge and Source/sinks)
                for (const auto& i : myTAZCurrent->getTAZEdges()) {
                    if (i.edge == objectsUnderCursor.getEdgeFront()) {
                        // clear current selection (to avoid having two or more edges selected at the same time using mouse clicks)
                        myTAZSelectionStatistics->clearSelectedEdges();
                        // now select edge
                        myTAZSelectionStatistics->selectEdge(i);
                        // edge selected, then return true
                        return true;
                    }
                }
            }
            // edge wasn't selected, then return false
            return false;
        }
    } else {
        // nothing to do
        return false;
    }
}


void
GNETAZFrame::processEdgeSelection(const std::vector<GNEEdge*>& edges) {
    // first check that a TAZ is selected
    if (myTAZCurrent->getTAZ()) {
        // if "toogle Membership" is enabled, create new TAZSources/sinks. In other case simply select edges
        if (myTAZChildDefaultParameters->getToggleMembership()) {
            // iterate over edges
            for (auto i : edges) {
                // first check if edge owns a TAZEge
                if (myTAZCurrent->isTAZEdge(i) == false) {
                    // create new TAZ Sources/Sinks
                    addOrRemoveTAZMember(i);
                }
            }
        } else {
            // iterate over edges
            for (auto i : edges) {
                // first check that selected edge isn't already selected
                if (!myTAZSelectionStatistics->isEdgeSelected(i)) {
                    // iterate over TAZEdges saved in TAZCurrent (it contains the Edge and Source/sinks)
                    for (const auto& j : myTAZCurrent->getTAZEdges()) {
                        if (j.edge == i) {
                            myTAZSelectionStatistics->selectEdge(j);
                        }
                    }
                }
            }
        }
    }
}


GNEFrameModuls::DrawingShape*
GNETAZFrame::getDrawingShapeModul() const {
    return myDrawingShape;
}


GNETAZFrame::TAZCurrent*
GNETAZFrame::getTAZCurrentModul() const {
    return myTAZCurrent;
}


GNETAZFrame::TAZSelectionStatistics*
GNETAZFrame::getTAZSelectionStatisticsModul() const {
    return myTAZSelectionStatistics;
}


GNETAZFrame::TAZSaveChanges*
GNETAZFrame::getTAZSaveChangesModul() const {
    return myTAZSaveChanges;
}


bool
GNETAZFrame::shapeDrawed() {
    // show warning dialogbox and stop check if input parameters are valid
    if (!myTAZParameters->isCurrentParametersValid()) {
        return false;
    } else if (myDrawingShape->getTemporalShape().size() == 0) {
        WRITE_WARNING("TAZ shape cannot be empty");
        return false;
    } else {
        // Declare map to keep TAZ Parameters values
        std::map<SumoXMLAttr, std::string> valuesOfElement = myTAZParameters->getAttributesAndValues();

        // obtain Netedit attributes
        myNeteditAttributes->getNeteditAttributesAndValues(valuesOfElement, nullptr);

        // generate new ID
        valuesOfElement[SUMO_ATTR_ID] = myViewNet->getNet()->generateTAZElementID(SUMO_TAG_TAZ);

        // obtain shape and close it
        PositionVector shape = myDrawingShape->getTemporalShape();
        shape.closePolygon();
        valuesOfElement[SUMO_ATTR_SHAPE] = toString(shape);

        // check if TAZ has to be created with edges
        if (myTAZParameters->isAddEdgesWithinEnabled()) {
            std::vector<std::string> edgeIDs;
            auto ACsInBoundary = myViewNet->getAttributeCarriersInBoundary(shape.getBoxBoundary(), true);
            for (auto i : ACsInBoundary) {
                if (i.second->getTagProperty().getTag() == SUMO_TAG_EDGE) {
                    edgeIDs.push_back(i.first);
                }
            }
            valuesOfElement[SUMO_ATTR_EDGES] = toString(edgeIDs);
        } else {
            // TAZ is created without edges
            valuesOfElement[SUMO_ATTR_EDGES] = "";
        }
        // declare SUMOSAXAttributesImpl_Cached to convert valuesMap into SUMOSAXAttributes
        SUMOSAXAttributesImpl_Cached SUMOSAXAttrs(valuesOfElement, getPredefinedTagsMML(), toString(SUMO_TAG_TAZ));
        // return true if TAZ was successfully created
        return GNEAdditionalHandler::buildAdditional(myViewNet->getNet(), true, SUMO_TAG_TAZ, SUMOSAXAttrs, nullptr);
    }
}


bool
GNETAZFrame::addOrRemoveTAZMember(GNEEdge* edge) {
    // first check if edge exist;
    if (edge) {
        // first check if already exist (in this case, remove it)
        for (const auto& i : myTAZCurrent->getTAZEdges()) {
            if (i.edge == edge) {
                // enable save changes button
                myTAZSaveChanges->enableButtonsAndBeginUndoList();
                // remove Source and Sinks using GNEChange_Additional
                myViewNet->getUndoList()->add(new GNEChange_TAZElement(i.TAZSource, false), true);
                myViewNet->getUndoList()->add(new GNEChange_TAZElement(i.TAZSink, false), true);
                // always refresh TAZ Edges after removing TAZSources/Sinks
                myTAZCurrent->refreshTAZEdges();
                // update select edges button
                myTAZChildDefaultParameters->updateSelectEdgesButton();
                return true;
            }
        }
        // if wasn't found, then add it
        myTAZSaveChanges->enableButtonsAndBeginUndoList();
        // create TAZ Sink using GNEChange_Additional and value of TAZChild default parameters
        GNETAZSourceSink* TAZSource = new GNETAZSourceSink(SUMO_TAG_TAZSOURCE, myTAZCurrent->getTAZ(), edge, myTAZChildDefaultParameters->getDefaultTAZSourceWeight());
        myViewNet->getUndoList()->add(new GNEChange_TAZElement(TAZSource, true), true);
        // create TAZ Sink using GNEChange_Additional and value of TAZChild default parameters
        GNETAZSourceSink* TAZSink = new GNETAZSourceSink(SUMO_TAG_TAZSINK, myTAZCurrent->getTAZ(), edge, myTAZChildDefaultParameters->getDefaultTAZSinkWeight());
        myViewNet->getUndoList()->add(new GNEChange_TAZElement(TAZSink, true), true);
        // always refresh TAZ Edges after adding TAZSources/Sinks
        myTAZCurrent->refreshTAZEdges();
        // update selected button
        myTAZChildDefaultParameters->updateSelectEdgesButton();
        return true;
    } else {
        throw ProcessError("Edge cannot be null");
    }
}


void
GNETAZFrame::dropTAZMembers() {
    // iterate over all TAZEdges
    for (const auto& i : myTAZCurrent->getTAZEdges()) {
        // enable save changes button
        myTAZSaveChanges->enableButtonsAndBeginUndoList();
        // remove Source and Sinks using GNEChange_Additional
        myViewNet->getUndoList()->add(new GNEChange_TAZElement(i.TAZSource, false), true);
        myViewNet->getUndoList()->add(new GNEChange_TAZElement(i.TAZSink, false), true);
    }
    // always refresh TAZ Edges after removing TAZSources/Sinks
    myTAZCurrent->refreshTAZEdges();
}


/****************************************************************************/
