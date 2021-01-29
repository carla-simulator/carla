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
/// @file    GNEFrameModuls.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Aug 2019
///
// Auxiliar class for GNEFrame Moduls
/****************************************************************************/
#include <config.h>

#include <netedit/GNEApplicationWindow.h>
#include <netedit/GNENet.h>
#include <netedit/GNEUndoList.h>
#include <netedit/GNEViewNet.h>
#include <netedit/GNEViewParent.h>
#include <netedit/changes/GNEChange_Children.h>
#include <netedit/elements/additional/GNEAdditional.h>
#include <netedit/elements/additional/GNEPOI.h>
#include <netedit/elements/additional/GNETAZ.h>
#include <netedit/elements/data/GNEDataInterval.h>
#include <netedit/elements/data/GNEGenericData.h>
#include <netedit/elements/data/GNEDataSet.h>
#include <netedit/elements/demand/GNEDemandElement.h>
#include <netedit/elements/network/GNEConnection.h>
#include <netedit/elements/network/GNECrossing.h>
#include <netedit/elements/network/GNEEdge.h>
#include <netedit/elements/network/GNEJunction.h>
#include <netedit/elements/network/GNELane.h>
#include <netedit/frames/common/GNEInspectorFrame.h>
#include <utils/foxtools/MFXMenuHeader.h>
#include <utils/gui/div/GLHelper.h>
#include <utils/gui/div/GUIDesigns.h>
#include <utils/gui/globjects/GLIncludes.h>
#include <utils/gui/windows/GUIAppEnum.h>

#include "GNEFrameModuls.h"


// ===========================================================================
// FOX callback mapping
// ===========================================================================

FXDEFMAP(GNEFrameModuls::TagSelector) TagSelectorMap[] = {
    FXMAPFUNC(SEL_COMMAND, MID_GNE_TAGTYPE_SELECTED,    GNEFrameModuls::TagSelector::onCmdSelectTagType),
    FXMAPFUNC(SEL_COMMAND, MID_GNE_TAG_SELECTED,        GNEFrameModuls::TagSelector::onCmdSelectTag)
};

FXDEFMAP(GNEFrameModuls::DemandElementSelector) DemandElementSelectorMap[] = {
    FXMAPFUNC(SEL_COMMAND, MID_GNE_SET_TYPE,    GNEFrameModuls::DemandElementSelector::onCmdSelectDemandElement),
};

FXDEFMAP(GNEFrameModuls::HierarchicalElementTree) HierarchicalElementTreeMap[] = {
    FXMAPFUNC(SEL_COMMAND,              MID_GNE_CENTER,                     GNEFrameModuls::HierarchicalElementTree::onCmdCenterItem),
    FXMAPFUNC(SEL_COMMAND,              MID_GNE_INSPECT,                    GNEFrameModuls::HierarchicalElementTree::onCmdInspectItem),
    FXMAPFUNC(SEL_COMMAND,              MID_GNE_DELETE,                     GNEFrameModuls::HierarchicalElementTree::onCmdDeleteItem),
    FXMAPFUNC(SEL_COMMAND,              MID_GNE_ACHIERARCHY_MOVEUP,         GNEFrameModuls::HierarchicalElementTree::onCmdMoveItemUp),
    FXMAPFUNC(SEL_COMMAND,              MID_GNE_ACHIERARCHY_MOVEDOWN,       GNEFrameModuls::HierarchicalElementTree::onCmdMoveItemDown),
    FXMAPFUNC(SEL_RIGHTBUTTONRELEASE,   MID_GNE_ACHIERARCHY_SHOWCHILDMENU,  GNEFrameModuls::HierarchicalElementTree::onCmdShowChildMenu)
};

FXDEFMAP(GNEFrameModuls::DrawingShape) DrawingShapeMap[] = {
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_STARTDRAWING,   GNEFrameModuls::DrawingShape::onCmdStartDrawing),
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_STOPDRAWING,    GNEFrameModuls::DrawingShape::onCmdStopDrawing),
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_ABORTDRAWING,   GNEFrameModuls::DrawingShape::onCmdAbortDrawing)
};

FXDEFMAP(GNEFrameModuls::OverlappedInspection) OverlappedInspectionMap[] = {
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_OVERLAPPED_NEXT,            GNEFrameModuls::OverlappedInspection::onCmdNextElement),
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_OVERLAPPED_PREVIOUS,        GNEFrameModuls::OverlappedInspection::onCmdPreviousElement),
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_OVERLAPPED_SHOWLIST,        GNEFrameModuls::OverlappedInspection::onCmdShowList),
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_OVERLAPPED_ITEMSELECTED,    GNEFrameModuls::OverlappedInspection::onCmdListItemSelected),
    FXMAPFUNC(SEL_COMMAND,  MID_HELP,                           GNEFrameModuls::OverlappedInspection::onCmdOverlappingHelp)
};

FXDEFMAP(GNEFrameModuls::PathCreator) PathCreatorMap[] = {
    FXMAPFUNC(SEL_COMMAND, MID_GNE_EDGEPATH_ABORT,          GNEFrameModuls::PathCreator::onCmdAbortPathCreation),
    FXMAPFUNC(SEL_COMMAND, MID_GNE_EDGEPATH_FINISH,         GNEFrameModuls::PathCreator::onCmdCreatePath),
    FXMAPFUNC(SEL_COMMAND, MID_GNE_EDGEPATH_REMOVELAST,     GNEFrameModuls::PathCreator::onCmdRemoveLastElement),
    FXMAPFUNC(SEL_COMMAND, MID_GNE_EDGEPATH_SHOWCANDIDATES, GNEFrameModuls::PathCreator::onCmdShowCandidateEdges)
};


// Object implementation
FXIMPLEMENT(GNEFrameModuls::TagSelector,                FXGroupBox,     TagSelectorMap,                 ARRAYNUMBER(TagSelectorMap))
FXIMPLEMENT(GNEFrameModuls::DemandElementSelector,      FXGroupBox,     DemandElementSelectorMap,       ARRAYNUMBER(DemandElementSelectorMap))
FXIMPLEMENT(GNEFrameModuls::HierarchicalElementTree,    FXGroupBox,     HierarchicalElementTreeMap,     ARRAYNUMBER(HierarchicalElementTreeMap))
FXIMPLEMENT(GNEFrameModuls::DrawingShape,               FXGroupBox,     DrawingShapeMap,                ARRAYNUMBER(DrawingShapeMap))
FXIMPLEMENT(GNEFrameModuls::OverlappedInspection,       FXGroupBox,     OverlappedInspectionMap,        ARRAYNUMBER(OverlappedInspectionMap))
FXIMPLEMENT(GNEFrameModuls::PathCreator,                FXGroupBox,     PathCreatorMap,                 ARRAYNUMBER(PathCreatorMap))


// ===========================================================================
// method definitions
// ===========================================================================

// ---------------------------------------------------------------------------
// GNEFrameModuls::TagSelector - methods
// ---------------------------------------------------------------------------

GNEFrameModuls::TagSelector::TagSelector(GNEFrame* frameParent, GNETagProperties::TagType type, bool onlyDrawables) :
    FXGroupBox(frameParent->myContentFrame, "Element", GUIDesignGroupBoxFrame),
    myFrameParent(frameParent) {
    // first check that property is valid
    switch (type) {
        case GNETagProperties::TagType::NETWORKELEMENT:
            setText("network elements");
            break;
        case GNETagProperties::TagType::ADDITIONALELEMENT:
            setText("Additional elements");
            break;
        case GNETagProperties::TagType::SHAPE:
            setText("Shape elements");
            break;
        case GNETagProperties::TagType::TAZELEMENT:
            setText("TAZ elements");
            break;
        case GNETagProperties::TagType::VEHICLE:
            setText("Vehicles");
            break;
        case GNETagProperties::TagType::STOP:
            setText("Stops");
            break;
        case GNETagProperties::TagType::PERSON:
            setText("Persons");
            break;
        case GNETagProperties::TagType::PERSONPLAN:
            setText("Person plans");
            // person plan type has four sub-groups
            myListOfTagTypes.push_back(std::make_pair("person trips", GNETagProperties::TagType::PERSONTRIP));
            myListOfTagTypes.push_back(std::make_pair("walks", GNETagProperties::TagType::WALK));
            myListOfTagTypes.push_back(std::make_pair("rides", GNETagProperties::TagType::RIDE));
            myListOfTagTypes.push_back(std::make_pair("stops", GNETagProperties::TagType::PERSONSTOP));
            break;
        case GNETagProperties::TagType::PERSONTRIP:
            setText("Person trips");
            break;
        case GNETagProperties::TagType::WALK:
            setText("Walks");
            break;
        case GNETagProperties::TagType::RIDE:
            setText("Rides");
            break;
        case GNETagProperties::TagType::PERSONSTOP:
            setText("Person stops");
            break;
        default:
            throw ProcessError("invalid tag property");
    }

    // Create FXComboBox
    myTagTypesMatchBox = new FXComboBox(this, GUIDesignComboBoxNCol, this, MID_GNE_TAGTYPE_SELECTED, GUIDesignComboBox);
    // Create FXComboBox
    myTagsMatchBox = new FXComboBox(this, GUIDesignComboBoxNCol, this, MID_GNE_TAG_SELECTED, GUIDesignComboBox);
    // Fill comboBox depending of TagTypes
    if (myListOfTagTypes.size() > 0) {
        // fill myTypeMatchBox with list of tags
        for (const auto& i : myListOfTagTypes) {
            myTagTypesMatchBox->appendItem(i.first.c_str());
        }
        // Set visible items
        myTagTypesMatchBox->setNumVisible((int)myTagTypesMatchBox->getNumItems());
        // fill myListOfTags with personTrips (the first Tag Type)
        myListOfTags = GNEAttributeCarrier::getAllowedTagsByCategory(GNETagProperties::TagType::PERSONTRIP, onlyDrawables);
    } else {
        myTagTypesMatchBox->hide();
        // fill myListOfTags
        myListOfTags = GNEAttributeCarrier::getAllowedTagsByCategory(type, onlyDrawables);
    }
    // fill myTypeMatchBox with list of tags
    for (const auto& tag : myListOfTags) {
        myTagsMatchBox->appendItem(tag.second.c_str());
    }
    // Set visible items
    myTagsMatchBox->setNumVisible((int)myTagsMatchBox->getNumItems());
    // TagSelector is always shown
    show();
}


GNEFrameModuls::TagSelector::~TagSelector() {}


void
GNEFrameModuls::TagSelector::showTagSelector() {
    show();
}


void
GNEFrameModuls::TagSelector::hideTagSelector() {
    hide();
}


const GNETagProperties&
GNEFrameModuls::TagSelector::getCurrentTagProperties() const {
    return myCurrentTagProperties;
}


void
GNEFrameModuls::TagSelector::setCurrentTagType(GNETagProperties::TagType tagType) {
    // set empty tag properties
    myCurrentTagProperties = GNETagProperties();
    // make sure that tag is in myTypeMatchBox
    for (int i = 0; i < (int)myTagsMatchBox->getNumItems(); i++) {
        if (myTagsMatchBox->getItem(i).text() == toString(tagType)) {
            myTagsMatchBox->setCurrentItem(i);
            // fill myListOfTags with personTrips (the first Tag Type)
            myListOfTags = GNEAttributeCarrier::getAllowedTagsByCategory(GNETagProperties::TagType::PERSONTRIP, true);
            // clear myTagsMatchBox
            myTagsMatchBox->clearItems();
            // fill myTypeMatchBox with list of tags
            for (const auto& tag : myListOfTags) {
                myTagsMatchBox->appendItem(tag.second.c_str());
            }
            // Set visible items
            myTagsMatchBox->setNumVisible((int)myTagsMatchBox->getNumItems());
        }
    }
    // call tag selected function
    myFrameParent->tagSelected();
}


void
GNEFrameModuls::TagSelector::setCurrentTag(SumoXMLTag newTag) {
    // set empty tag properties
    myCurrentTagProperties = GNETagProperties();
    // make sure that tag is in myTypeMatchBox
    for (int i = 0; i < (int)myTagsMatchBox->getNumItems(); i++) {
        if (myTagsMatchBox->getItem(i).text() == toString(newTag)) {
            myTagsMatchBox->setCurrentItem(i);
            // Set new current type
            myCurrentTagProperties = GNEAttributeCarrier::getTagProperties(newTag);
        }
    }
    // call tag selected function
    myFrameParent->tagSelected();
}


void
GNEFrameModuls::TagSelector::refreshTagProperties() {
    // simply call onCmdSelectItem (to avoid duplicated code)
    onCmdSelectTag(0, 0, 0);
}


long GNEFrameModuls::TagSelector::onCmdSelectTagType(FXObject*, FXSelector, void*) {
    // Check if value of myTypeMatchBox correspond of an allowed additional tags
    for (const auto& i : myListOfTagTypes) {
        if (i.first == myTagTypesMatchBox->getText().text()) {
            // set color of myTagTypesMatchBox to black (valid)
            myTagTypesMatchBox->setTextColor(FXRGB(0, 0, 0));
            // fill myListOfTags with personTrips (the first Tag Type)
            myListOfTags = GNEAttributeCarrier::getAllowedTagsByCategory(i.second, true);
            // show and clear myTagsMatchBox
            myTagsMatchBox->show();
            myTagsMatchBox->clearItems();
            // fill myTypeMatchBox with list of tags
            for (const auto& tag : myListOfTags) {
                myTagsMatchBox->appendItem(tag.second.c_str());
            }
            // Set visible items
            myTagsMatchBox->setNumVisible((int)myTagsMatchBox->getNumItems());
            // Write Warning in console if we're in testing mode
            WRITE_DEBUG(("Selected item '" + myTagsMatchBox->getText() + "' in TagTypeSelector").text());
            // call onCmdSelectTag
            return onCmdSelectTag(nullptr, 0, nullptr);
        }
    }
    // if TagType isn't valid, hide myTagsMatchBox
    myTagsMatchBox->hide();
    // if additional name isn't correct, set SUMO_TAG_NOTHING as current type
    myCurrentTagProperties = myInvalidTagProperty;
    // call tag selected function
    myFrameParent->tagSelected();
    // set color of myTagTypesMatchBox to red (invalid)
    myTagTypesMatchBox->setTextColor(FXRGB(255, 0, 0));
    // Write Warning in console if we're in testing mode
    WRITE_DEBUG("Selected invalid item in TagTypeSelector");
    return 1;
}


long
GNEFrameModuls::TagSelector::onCmdSelectTag(FXObject*, FXSelector, void*) {
    // Check if value of myTypeMatchBox correspond of an allowed additional tags
    for (const auto& tag : myListOfTags) {
        if (tag.second == myTagsMatchBox->getText().text()) {
            // set color of myTypeMatchBox to black (valid)
            myTagsMatchBox->setTextColor(FXRGB(0, 0, 0));
            // Set new current type
            myCurrentTagProperties = GNEAttributeCarrier::getTagProperties(tag.first);
            // call tag selected function
            myFrameParent->tagSelected();
            // Write Warning in console if we're in testing mode
            WRITE_DEBUG(("Selected item '" + myTagsMatchBox->getText() + "' in TagSelector").text());
            return 1;
        }
    }
    // if additional name isn't correct, set SUMO_TAG_NOTHING as current type
    myCurrentTagProperties = myInvalidTagProperty;
    // call tag selected function
    myFrameParent->tagSelected();
    // set color of myTypeMatchBox to red (invalid)
    myTagsMatchBox->setTextColor(FXRGB(255, 0, 0));
    // Write Warning in console if we're in testing mode
    WRITE_DEBUG("Selected invalid item in TagSelector");
    return 1;
}

// ---------------------------------------------------------------------------
// GNEFrameModuls::DemandElementSelector - methods
// ---------------------------------------------------------------------------

GNEFrameModuls::DemandElementSelector::DemandElementSelector(GNEFrame* frameParent, SumoXMLTag demandElementTag) :
    FXGroupBox(frameParent->myContentFrame, ("Parent " + toString(demandElementTag)).c_str(), GUIDesignGroupBoxFrame),
    myFrameParent(frameParent),
    myCurrentDemandElement(nullptr),
    myDemandElementTags({demandElementTag}) {
    // Create FXComboBox
    myDemandElementsMatchBox = new FXComboBox(this, GUIDesignComboBoxNCol, this, MID_GNE_SET_TYPE, GUIDesignComboBox);
    // refresh demand element MatchBox
    refreshDemandElementSelector();
    // shown after creation
    show();
}


GNEFrameModuls::DemandElementSelector::DemandElementSelector(GNEFrame* frameParent, const std::vector<GNETagProperties::TagType>& tagTypes) :
    FXGroupBox(frameParent->myContentFrame, "Parent element", GUIDesignGroupBoxFrame),
    myFrameParent(frameParent),
    myCurrentDemandElement(nullptr) {
    // fill myDemandElementTags
    for (const auto& tagType : tagTypes) {
        const auto tagsByCategory = GNEAttributeCarrier::getAllowedTagsByCategory(tagType, false);
        for (const auto& tagByCategory : tagsByCategory) {
            myDemandElementTags.push_back(tagByCategory.first);
        }
    }
    // Create FXComboBox
    myDemandElementsMatchBox = new FXComboBox(this, GUIDesignComboBoxNCol, this, MID_GNE_SET_TYPE, GUIDesignComboBox);
    // refresh demand element MatchBox
    refreshDemandElementSelector();
    // shown after creation
    show();
}


GNEFrameModuls::DemandElementSelector::~DemandElementSelector() {}


GNEDemandElement*
GNEFrameModuls::DemandElementSelector::getCurrentDemandElement() const {
    return myCurrentDemandElement;
}


const std::vector<SumoXMLTag>&
GNEFrameModuls::DemandElementSelector::getAllowedTags() const {
    return myDemandElementTags;
}


void
GNEFrameModuls::DemandElementSelector::setDemandElement(GNEDemandElement* demandElement) {
    // first check that demandElement tag correspond to a tag of myDemandElementTags
    if (std::find(myDemandElementTags.begin(), myDemandElementTags.end(), demandElement->getTagProperty().getTag()) != myDemandElementTags.end()) {
        // update text of myDemandElementsMatchBox
        myDemandElementsMatchBox->setText(demandElement->getID().c_str());
        // Set new current demand element
        myCurrentDemandElement = demandElement;
        // call demandElementSelected function
        myFrameParent->demandElementSelected();
    }
}


void
GNEFrameModuls::DemandElementSelector::showDemandElementSelector() {
    // first refresh modul
    refreshDemandElementSelector();
    // if current selected item isn't valid, set DEFAULT_VTYPE_ID or DEFAULT_PEDTYPE_ID
    if (myCurrentDemandElement) {
        myDemandElementsMatchBox->setText(myCurrentDemandElement->getID().c_str());
    } else if (myDemandElementTags.size() == 1) {
        if (myDemandElementTags.at(0) == SUMO_TAG_VTYPE) {
            myDemandElementsMatchBox->setText(DEFAULT_VTYPE_ID.c_str());
        } else if (myDemandElementTags.at(0) == SUMO_TAG_PTYPE) {
            myDemandElementsMatchBox->setText(DEFAULT_PEDTYPE_ID.c_str());
        }
    }
    onCmdSelectDemandElement(nullptr, 0, nullptr);
    show();
}


void
GNEFrameModuls::DemandElementSelector::hideDemandElementSelector() {
    hide();
}


bool
GNEFrameModuls::DemandElementSelector::isDemandElementSelectorShown() const {
    return shown();
}


void
GNEFrameModuls::DemandElementSelector::refreshDemandElementSelector() {
    // clear demand elements comboBox
    myDemandElementsMatchBox->clearItems();
    // fill myTypeMatchBox with list of demand elements
    for (const auto& i : myDemandElementTags) {
        // special case for VTypes and PTypes
        if (i == SUMO_TAG_VTYPE) {
            // add default Vehicle an Bike types in the first and second positions
            myDemandElementsMatchBox->appendItem(DEFAULT_VTYPE_ID.c_str());
            myDemandElementsMatchBox->appendItem(DEFAULT_BIKETYPE_ID.c_str());
            // add rest of vTypes
            for (const auto& j : myFrameParent->getViewNet()->getNet()->getAttributeCarriers()->getDemandElements().at(i)) {
                // avoid insert duplicated default vType
                if ((j.first != DEFAULT_VTYPE_ID) && (j.first != DEFAULT_BIKETYPE_ID)) {
                    myDemandElementsMatchBox->appendItem(j.first.c_str());
                }
            }
        } else if (i == SUMO_TAG_PTYPE) {
            // add default Person type in the firs
            myDemandElementsMatchBox->appendItem(DEFAULT_PEDTYPE_ID.c_str());
            // add rest of pTypes
            for (const auto& j : myFrameParent->getViewNet()->getNet()->getAttributeCarriers()->getDemandElements().at(i)) {
                // avoid insert duplicated default pType
                if (j.first != DEFAULT_PEDTYPE_ID) {
                    myDemandElementsMatchBox->appendItem(j.first.c_str());
                }
            }
        } else {
            // insert all Ids
            for (const auto& j : myFrameParent->getViewNet()->getNet()->getAttributeCarriers()->getDemandElements().at(i)) {
                myDemandElementsMatchBox->appendItem(j.first.c_str());
            }
        }
    }
    // Set number of  items (maximum 10)
    if (myDemandElementsMatchBox->getNumItems() < 10) {
        myDemandElementsMatchBox->setNumVisible((int)myDemandElementsMatchBox->getNumItems());
    } else {
        myDemandElementsMatchBox->setNumVisible(10);
    }
    // update myCurrentDemandElement
    if (myDemandElementsMatchBox->getNumItems() == 0) {
        myCurrentDemandElement = nullptr;
    } else if (myCurrentDemandElement) {
        for (int i = 0; i < myDemandElementsMatchBox->getNumItems(); i++) {
            if (myDemandElementsMatchBox->getItem(i).text() == myCurrentDemandElement->getID()) {
                myDemandElementsMatchBox->setCurrentItem(i, FALSE);
            }
        }
    } else {
        // set first element in the list as myCurrentDemandElement (Special case for default person and vehicle type)
        if (myDemandElementsMatchBox->getItem(0).text() == DEFAULT_VTYPE_ID) {
            myCurrentDemandElement = myFrameParent->getViewNet()->getNet()->getAttributeCarriers()->getDemandElements().at(SUMO_TAG_VTYPE).at(DEFAULT_VTYPE_ID);
        } else if (myDemandElementsMatchBox->getItem(0).text() == DEFAULT_PEDTYPE_ID) {
            myCurrentDemandElement = myFrameParent->getViewNet()->getNet()->getAttributeCarriers()->getDemandElements().at(SUMO_TAG_PTYPE).at(DEFAULT_PEDTYPE_ID);
        } else {
            // disable myCurrentDemandElement
            myCurrentDemandElement = nullptr;
            // update myCurrentDemandElement with the first allowed element
            for (auto i = myDemandElementTags.begin(); (i != myDemandElementTags.end()) && (myCurrentDemandElement == nullptr); i++) {
                if (myFrameParent->getViewNet()->getNet()->getAttributeCarriers()->getDemandElements().at(*i).size() > 0) {
                    myCurrentDemandElement = myFrameParent->getViewNet()->getNet()->getAttributeCarriers()->getDemandElements().at(*i).begin()->second;
                }
            }
        }
    }
}


GNEEdge*
GNEFrameModuls::DemandElementSelector::getPersonPlanPreviousEdge() const {
    if (myCurrentDemandElement == nullptr) {
        return nullptr;
    }
    if (!myCurrentDemandElement->getTagProperty().isPerson()) {
        return nullptr;
    }
    if (myCurrentDemandElement->getChildDemandElements().empty()) {
        return nullptr;
    }
    // get last person plan
    const GNEDemandElement* lastPersonPlan = myCurrentDemandElement->getChildDemandElements().back();
    // check tag
    switch (lastPersonPlan->getTagProperty().getTag()) {
        case GNE_TAG_PERSONTRIP_EDGE_EDGE:
        case GNE_TAG_WALK_EDGES:
        case GNE_TAG_WALK_EDGE_EDGE:
        case GNE_TAG_RIDE_EDGE_EDGE:
        case GNE_TAG_PERSONSTOP_EDGE:
            return lastPersonPlan->getParentEdges().back();
        case GNE_TAG_PERSONTRIP_EDGE_BUSSTOP:
        case GNE_TAG_WALK_EDGE_BUSSTOP:
        case GNE_TAG_RIDE_EDGE_BUSSTOP:
        case GNE_TAG_PERSONSTOP_BUSSTOP:
            return lastPersonPlan->getParentAdditionals().back()->getParentLanes().front()->getParentEdge();
        case GNE_TAG_WALK_ROUTE:
            return lastPersonPlan->getParentDemandElements().back()->getParentEdges().back();
        default:
            return nullptr;
    }
}


long
GNEFrameModuls::DemandElementSelector::onCmdSelectDemandElement(FXObject*, FXSelector, void*) {
    // Check if value of myTypeMatchBox correspond to a demand element
    for (const auto& i : myDemandElementTags) {
        for (const auto& j : myFrameParent->getViewNet()->getNet()->getAttributeCarriers()->getDemandElements().at(i)) {
            if (j.first == myDemandElementsMatchBox->getText().text()) {
                // set color of myTypeMatchBox to black (valid)
                myDemandElementsMatchBox->setTextColor(FXRGB(0, 0, 0));
                // Set new current demand element
                myCurrentDemandElement = j.second;
                // call demandElementSelected function
                myFrameParent->demandElementSelected();
                // Write Warning in console if we're in testing mode
                WRITE_DEBUG(("Selected item '" + myDemandElementsMatchBox->getText() + "' in DemandElementSelector").text());
                return 1;
            }
        }
    }
    // if demand element selected is invalid, set demand element as null
    myCurrentDemandElement = nullptr;
    // call demandElementSelected function
    myFrameParent->demandElementSelected();
    // change color of myDemandElementsMatchBox to red (invalid)
    myDemandElementsMatchBox->setTextColor(FXRGB(255, 0, 0));
    // Write Warning in console if we're in testing mode
    WRITE_DEBUG("Selected invalid item in DemandElementSelector");
    return 1;
}

// ---------------------------------------------------------------------------
// GNEFrameModuls::HierarchicalElementTree - methods
// ---------------------------------------------------------------------------

GNEFrameModuls::HierarchicalElementTree::HierarchicalElementTree(GNEFrame* frameParent) :
    FXGroupBox(frameParent->myContentFrame, "Hierarchy", GUIDesignGroupBoxFrame),
    myFrameParent(frameParent),
    myHE(nullptr),
    myClickedAC(nullptr),
    myClickedJunction(nullptr),
    myClickedEdge(nullptr),
    myClickedLane(nullptr),
    myClickedCrossing(nullptr),
    myClickedConnection(nullptr),
    myClickedShape(nullptr),
    myClickedTAZElement(nullptr),
    myClickedAdditional(nullptr),
    myClickedDemandElement(nullptr),
    myClickedDataSet(nullptr),
    myClickedDataInterval(nullptr),
    myClickedGenericData(nullptr) {
    // Create three list
    myTreelist = new FXTreeList(this, this, MID_GNE_ACHIERARCHY_SHOWCHILDMENU, GUIDesignTreeListFrame);
    hide();
}


GNEFrameModuls::HierarchicalElementTree::~HierarchicalElementTree() {}


void
GNEFrameModuls::HierarchicalElementTree::showHierarchicalElementTree(GNEAttributeCarrier* AC) {
    myHE = dynamic_cast<GNEHierarchicalElement*>(AC);
    // show HierarchicalElementTree and refresh HierarchicalElementTree
    if (myHE) {
        show();
        refreshHierarchicalElementTree();
    }
}


void
GNEFrameModuls::HierarchicalElementTree::hideHierarchicalElementTree() {
    // set all pointers null
    myHE = nullptr;
    myClickedAC = nullptr;
    myClickedJunction = nullptr;
    myClickedEdge = nullptr;
    myClickedLane = nullptr;
    myClickedCrossing = nullptr;
    myClickedConnection = nullptr;
    myClickedShape = nullptr;
    myClickedTAZElement = nullptr;
    myClickedAdditional = nullptr;
    myClickedDemandElement = nullptr;
    myClickedDataSet = nullptr;
    myClickedDataInterval = nullptr;
    myClickedGenericData = nullptr;
    // hide modul
    hide();
}


void
GNEFrameModuls::HierarchicalElementTree::refreshHierarchicalElementTree() {
    // clear items
    myTreelist->clearItems();
    myTreeItemToACMap.clear();
    myTreeItemsConnections.clear();
    // show children of myHE
    if (myHE) {
        showHierarchicalElementChildren(myHE, showAttributeCarrierParents());
    }
}


void
GNEFrameModuls::HierarchicalElementTree::removeCurrentEditedAttributeCarrier(const GNEAttributeCarrier* AC) {
    // simply check if AC is the same of myHE
    if (AC == myHE) {
        myHE = nullptr;
    }
}


long
GNEFrameModuls::HierarchicalElementTree::onCmdShowChildMenu(FXObject*, FXSelector, void* eventData) {
    // Obtain event
    FXEvent* e = (FXEvent*)eventData;
    // obtain FXTreeItem in the given position
    FXTreeItem* item = myTreelist->getItemAt(e->win_x, e->win_y);
    // open Pop-up if FXTreeItem has a Attribute Carrier vinculated
    if (item && (myTreeItemsConnections.find(item) == myTreeItemsConnections.end())) {
        createPopUpMenu(e->root_x, e->root_y, myTreeItemToACMap[item]);
    }
    return 1;
}


long
GNEFrameModuls::HierarchicalElementTree::onCmdCenterItem(FXObject*, FXSelector, void*) {
    // Center item
    if (myClickedJunction) {
        myFrameParent->myViewNet->centerTo(myClickedJunction->getGlID(), true, -1);
    } else if (myClickedEdge) {
        myFrameParent->myViewNet->centerTo(myClickedEdge->getGlID(), true, -1);
    } else if (myClickedLane) {
        myFrameParent->myViewNet->centerTo(myClickedLane->getGlID(), true, -1);
    } else if (myClickedCrossing) {
        myFrameParent->myViewNet->centerTo(myClickedCrossing->getGlID(), true, -1);
    } else if (myClickedConnection) {
        myFrameParent->myViewNet->centerTo(myClickedConnection->getGlID(), true, -1);
    } else if (myClickedAdditional) {
        myFrameParent->myViewNet->centerTo(myClickedAdditional->getGlID(), true, -1);
    } else if (myClickedShape) {
        myFrameParent->myViewNet->centerTo(myClickedShape->getGlID(), true, -1);
    } else if (myClickedTAZElement) {
        myFrameParent->myViewNet->centerTo(myClickedTAZElement->getGlID(), true, -1);
    } else if (myClickedDemandElement) {
        myFrameParent->myViewNet->centerTo(myClickedDemandElement->getGlID(), true, -1);
    } else if (myClickedGenericData) {
        myFrameParent->myViewNet->centerTo(myClickedGenericData->getGlID(), true, -1);
    }
    // update view after centering
    myFrameParent->myViewNet->updateViewNet();
    return 1;
}


long
GNEFrameModuls::HierarchicalElementTree::onCmdInspectItem(FXObject*, FXSelector, void*) {
    if ((myHE != nullptr) && (myClickedAC != nullptr)) {
        myFrameParent->myViewNet->getViewParent()->getInspectorFrame()->inspectChild(myClickedAC, myHE);
    }
    return 1;
}


long
GNEFrameModuls::HierarchicalElementTree::onCmdDeleteItem(FXObject*, FXSelector, void*) {
    // check if Inspector frame was opened before removing
    const std::vector<GNEAttributeCarrier*>& currentInspectedACs = myFrameParent->myViewNet->getViewParent()->getInspectorFrame()->getAttributesEditor()->getEditedACs();
    // Remove Attribute Carrier
    if (myClickedJunction) {
        myFrameParent->myViewNet->getNet()->deleteJunction(myClickedJunction, myFrameParent->myViewNet->getUndoList());
    } else if (myClickedEdge) {
        myFrameParent->myViewNet->getNet()->deleteEdge(myClickedEdge, myFrameParent->myViewNet->getUndoList(), false);
    } else if (myClickedLane) {
        myFrameParent->myViewNet->getNet()->deleteLane(myClickedLane, myFrameParent->myViewNet->getUndoList(), false);
    } else if (myClickedCrossing) {
        myFrameParent->myViewNet->getNet()->deleteCrossing(myClickedCrossing, myFrameParent->myViewNet->getUndoList());
    } else if (myClickedConnection) {
        myFrameParent->myViewNet->getNet()->deleteConnection(myClickedConnection, myFrameParent->myViewNet->getUndoList());
    } else if (myClickedAdditional) {
        myFrameParent->myViewNet->getNet()->deleteAdditional(myClickedAdditional, myFrameParent->myViewNet->getUndoList());
    } else if (myClickedShape) {
        myFrameParent->myViewNet->getNet()->deleteShape(myClickedShape, myFrameParent->myViewNet->getUndoList());
    } else if (myClickedTAZElement) {
        myFrameParent->myViewNet->getNet()->deleteTAZElement(myClickedTAZElement, myFrameParent->myViewNet->getUndoList());
    } else if (myClickedDemandElement) {
        // check that default VTypes aren't removed
        if ((myClickedDemandElement->getTagProperty().getTag() == SUMO_TAG_VTYPE) && (GNEAttributeCarrier::parse<bool>(myClickedDemandElement->getAttribute(GNE_ATTR_DEFAULT_VTYPE)))) {
            WRITE_WARNING("Default Vehicle Type '" + myClickedDemandElement->getAttribute(SUMO_ATTR_ID) + "' cannot be removed");
            return 1;
        } else if (myClickedDemandElement->getTagProperty().isPersonPlan() && (myClickedDemandElement->getParentDemandElements().front()->getChildDemandElements().size() == 1)) {
            // we need to check if we're removing the last person plan of a person.
            myFrameParent->myViewNet->getNet()->deleteDemandElement(myClickedDemandElement->getParentDemandElements().front(), myFrameParent->myViewNet->getUndoList());
        } else {
            myFrameParent->myViewNet->getNet()->deleteDemandElement(myClickedDemandElement, myFrameParent->myViewNet->getUndoList());
        }
    } else if (myClickedDataSet) {
        myFrameParent->myViewNet->getNet()->deleteDataSet(myClickedDataSet, myFrameParent->myViewNet->getUndoList());
    } else if (myClickedDataInterval) {
        // check if we have to remove data Set
        if (myClickedDataInterval->getDataSetParent()->getDataIntervalChildren().size() == 1) {
            myFrameParent->myViewNet->getNet()->deleteDataSet(myClickedDataInterval->getDataSetParent(), myFrameParent->myViewNet->getUndoList());
        } else {
            myFrameParent->myViewNet->getNet()->deleteDataInterval(myClickedDataInterval, myFrameParent->myViewNet->getUndoList());
        }
    } else if (myClickedGenericData) {
        // check if we have to remove interval
        if (myClickedGenericData->getDataIntervalParent()->getGenericDataChildren().size() == 1) {
            // check if we have to remove data Set
            if (myClickedGenericData->getDataIntervalParent()->getDataSetParent()->getDataIntervalChildren().size() == 1) {
                myFrameParent->myViewNet->getNet()->deleteDataSet(myClickedGenericData->getDataIntervalParent()->getDataSetParent(), myFrameParent->myViewNet->getUndoList());
            } else {
                myFrameParent->myViewNet->getNet()->deleteDataInterval(myClickedGenericData->getDataIntervalParent(), myFrameParent->myViewNet->getUndoList());
            }
        } else {
            myFrameParent->myViewNet->getNet()->deleteGenericData(myClickedGenericData, myFrameParent->myViewNet->getUndoList());
        }
    }
    // update net
    myFrameParent->myViewNet->updateViewNet();
    // refresh AC Hierarchy
    refreshHierarchicalElementTree();
    // check if inspector frame has to be shown again
    if (currentInspectedACs.size() == 1) {
        if (currentInspectedACs.front() != myClickedAC) {
            myFrameParent->myViewNet->getViewParent()->getInspectorFrame()->inspectSingleElement(currentInspectedACs.front());
        } else {
            // inspect a nullprt element to reset inspector frame
            myFrameParent->myViewNet->getViewParent()->getInspectorFrame()->inspectSingleElement(nullptr);
        }
    }
    return 1;
}


long
GNEFrameModuls::HierarchicalElementTree::onCmdMoveItemUp(FXObject*, FXSelector, void*) {
    // currently only children of demand elements can be moved
    if (myClickedDemandElement) {
        myFrameParent->myViewNet->getUndoList()->p_begin(("moving up " + myClickedDemandElement->getTagStr()).c_str());
        // move element one position back
        myFrameParent->myViewNet->getUndoList()->add(new GNEChange_Children(myClickedDemandElement->getParentDemandElements().at(0), myClickedDemandElement,
                GNEChange_Children::Operation::MOVE_BACK), true);
        myFrameParent->myViewNet->getUndoList()->p_end();
    }
    // refresh after moving child
    refreshHierarchicalElementTree();
    return 1;
}


long
GNEFrameModuls::HierarchicalElementTree::onCmdMoveItemDown(FXObject*, FXSelector, void*) {
    // currently only children of demand elements can be moved
    if (myClickedDemandElement) {
        myFrameParent->myViewNet->getUndoList()->p_begin(("moving down " + myClickedDemandElement->getTagStr()).c_str());
        // move element one position front
        myFrameParent->myViewNet->getUndoList()->add(new GNEChange_Children(myClickedDemandElement->getParentDemandElements().at(0), myClickedDemandElement,
                GNEChange_Children::Operation::MOVE_FRONT), true);
        myFrameParent->myViewNet->getUndoList()->p_end();
    }
    // refresh after moving child
    refreshHierarchicalElementTree();
    return 1;
}


void
GNEFrameModuls::HierarchicalElementTree::createPopUpMenu(int X, int Y, GNEAttributeCarrier* clickedAC) {
    // first check that AC exist
    if (clickedAC) {
        // set current clicked AC
        myClickedAC = clickedAC;
        // cast all elements
        myClickedJunction = dynamic_cast<GNEJunction*>(clickedAC);
        myClickedEdge = dynamic_cast<GNEEdge*>(clickedAC);
        myClickedLane = dynamic_cast<GNELane*>(clickedAC);
        myClickedCrossing = dynamic_cast<GNECrossing*>(clickedAC);
        myClickedConnection = dynamic_cast<GNEConnection*>(clickedAC);
        myClickedShape = dynamic_cast<GNEShape*>(clickedAC);
        myClickedTAZElement = dynamic_cast<GNETAZElement*>(clickedAC);
        myClickedAdditional = dynamic_cast<GNEAdditional*>(clickedAC);
        myClickedDemandElement = dynamic_cast<GNEDemandElement*>(clickedAC);
        myClickedDataSet = dynamic_cast<GNEDataSet*>(clickedAC);
        myClickedDataInterval = dynamic_cast<GNEDataInterval*>(clickedAC);
        myClickedGenericData = dynamic_cast<GNEGenericData*>(clickedAC);
        // create FXMenuPane
        FXMenuPane* pane = new FXMenuPane(myTreelist);
        // set item name and icon
        new MFXMenuHeader(pane, myFrameParent->myViewNet->getViewParent()->getGUIMainWindow()->getBoldFont(), myClickedAC->getPopUpID().c_str(), myClickedAC->getIcon());
        // insert separator
        new FXMenuSeparator(pane);
        // create center menu command
        FXMenuCommand* centerMenuCommand = new FXMenuCommand(pane, "Center", GUIIconSubSys::getIcon(GUIIcon::RECENTERVIEW), this, MID_GNE_CENTER);
        // disable Centering for Vehicle Types, data sets and data intervals
        if (myClickedAC->getTagProperty().isVehicleType() || (myClickedAC->getTagProperty().getTag() == SUMO_TAG_DATASET) ||
                (myClickedAC->getTagProperty().getTag() == SUMO_TAG_DATAINTERVAL)) {
            centerMenuCommand->disable();
        }
        // create inspect and delete menu commands
        FXMenuCommand* inspectMenuCommand = new FXMenuCommand(pane, "Inspect", GUIIconSubSys::getIcon(GUIIcon::MODEINSPECT), this, MID_GNE_INSPECT);
        FXMenuCommand* deleteMenuCommand = new FXMenuCommand(pane, "Delete", GUIIconSubSys::getIcon(GUIIcon::MODEDELETE), this, MID_GNE_DELETE);
        // check if inspect and delete menu commands has to be disabled
        if (GNEFrameAttributesModuls::isSupermodeValid(myFrameParent->myViewNet, myClickedAC) == false) {
            inspectMenuCommand->disable();
            deleteMenuCommand->disable();
        }
        // now chec if given AC support manually moving of their item up and down (Currently only for certain demand elements)
        /* if (myClickedDemandElement && myClickedAC->getTagProperty().canBeSortedManually()) {
            // insert separator
            new FXMenuSeparator(pane);
            // create both moving menu commands
            FXMenuCommand* moveUpMenuCommand = new FXMenuCommand(pane, "Move up", GUIIconSubSys::getIcon(GUIIcon::ARROW_UP), this, MID_GNE_ACHIERARCHY_MOVEUP);
            FXMenuCommand* moveDownMenuCommand = new FXMenuCommand(pane, "Move down", GUIIconSubSys::getIcon(GUIIcon::ARROW_DOWN), this, MID_GNE_ACHIERARCHY_MOVEDOWN);
            // check if both commands has to be disabled
            if (myClickedDemandElement->getTagProperty().isPersonStop()) {
                moveUpMenuCommand->setText("Move up (Stops cannot be moved)");
                moveDownMenuCommand->setText("Move down (Stops cannot be moved)");
                moveUpMenuCommand->disable();
                moveDownMenuCommand->disable();
            } else {
                // check if moveUpMenuCommand has to be disabled
                if (myClickedDemandElement->getParentDemandElements().front()->getChildDemandElements().front() == myClickedDemandElement) {
                    moveUpMenuCommand->setText("Move up (It's already the first element)");
                    moveUpMenuCommand->disable();
                } else if (myClickedDemandElement->getParentDemandElements().front()->getPreviousChildDemandElement(myClickedDemandElement)->getTagProperty().isPersonStop()) {
                    moveUpMenuCommand->setText("Move up (Previous element is a Stop)");
                    moveUpMenuCommand->disable();
                }
                // check if moveDownMenuCommand has to be disabled
                if (myClickedDemandElement->getParentDemandElements().front()->getChildDemandElements().back() == myClickedDemandElement) {
                    moveDownMenuCommand->setText("Move down (It's already the last element)");
                    moveDownMenuCommand->disable();
                } else if (myClickedDemandElement->getParentDemandElements().front()->getNextChildDemandElement(myClickedDemandElement)->getTagProperty().isPersonStop()) {
                    moveDownMenuCommand->setText("Move down (Next element is a Stop)");
                    moveDownMenuCommand->disable();
                }
            }
        } */
        // Center in the mouse position and create pane
        pane->setX(X);
        pane->setY(Y);
        pane->create();
        pane->show();
    } else {
        // set all clicked elements to null
        myClickedAC = nullptr;
        myClickedJunction = nullptr;
        myClickedEdge = nullptr;
        myClickedLane = nullptr;
        myClickedCrossing = nullptr;
        myClickedConnection = nullptr;
        myClickedShape = nullptr;
        myClickedTAZElement = nullptr;
        myClickedAdditional = nullptr;
        myClickedDemandElement = nullptr;
        myClickedDataSet = nullptr;
        myClickedDataInterval = nullptr;
        myClickedGenericData = nullptr;
    }
}


FXTreeItem*
GNEFrameModuls::HierarchicalElementTree::showAttributeCarrierParents() {
    if (myHE->getTagProperty().isNetworkElement()) {
        // check demand element type
        switch (myHE->getTagProperty().getTag()) {
            case SUMO_TAG_EDGE: {
                // obtain Edge
                GNEEdge* edge = myFrameParent->myViewNet->getNet()->retrieveEdge(myHE->getID(), false);
                if (edge) {
                    // insert Junctions of edge in tree (Pararell because a edge has always two Junctions)
                    FXTreeItem* junctionSourceItem = myTreelist->insertItem(nullptr, nullptr, (edge->getParentJunctions().front()->getHierarchyName() + " origin").c_str(), edge->getParentJunctions().front()->getIcon(), edge->getParentJunctions().front()->getIcon());
                    FXTreeItem* junctionDestinyItem = myTreelist->insertItem(nullptr, nullptr, (edge->getParentJunctions().front()->getHierarchyName() + " destiny").c_str(), edge->getParentJunctions().front()->getIcon(), edge->getParentJunctions().front()->getIcon());
                    junctionDestinyItem->setExpanded(true);
                    // Save items in myTreeItemToACMap
                    myTreeItemToACMap[junctionSourceItem] = edge->getParentJunctions().front();
                    myTreeItemToACMap[junctionDestinyItem] = edge->getParentJunctions().back();
                    // return junction destiny Item
                    return junctionDestinyItem;
                } else {
                    return nullptr;
                }
            }
            case SUMO_TAG_LANE: {
                // obtain lane
                GNELane* lane = myFrameParent->myViewNet->getNet()->retrieveLane(myHE->getID(), false);
                if (lane) {
                    // obtain parent edge
                    GNEEdge* edge = myFrameParent->myViewNet->getNet()->retrieveEdge(lane->getParentEdge()->getID());
                    //inser Junctions of lane of edge in tree (Pararell because a edge has always two Junctions)
                    FXTreeItem* junctionSourceItem = myTreelist->insertItem(nullptr, nullptr, (edge->getParentJunctions().front()->getHierarchyName() + " origin").c_str(), edge->getParentJunctions().front()->getIcon(), edge->getParentJunctions().front()->getIcon());
                    FXTreeItem* junctionDestinyItem = myTreelist->insertItem(nullptr, nullptr, (edge->getParentJunctions().front()->getHierarchyName() + " destiny").c_str(), edge->getParentJunctions().front()->getIcon(), edge->getParentJunctions().front()->getIcon());
                    junctionDestinyItem->setExpanded(true);
                    // Create edge item
                    FXTreeItem* edgeItem = myTreelist->insertItem(nullptr, junctionDestinyItem, edge->getHierarchyName().c_str(), edge->getIcon(), edge->getIcon());
                    edgeItem->setExpanded(true);
                    // Save items in myTreeItemToACMap
                    myTreeItemToACMap[junctionSourceItem] = edge->getParentJunctions().front();
                    myTreeItemToACMap[junctionDestinyItem] = edge->getParentJunctions().back();
                    myTreeItemToACMap[edgeItem] = edge;
                    // return edge item
                    return edgeItem;
                } else {
                    return nullptr;
                }
            }
            case SUMO_TAG_CROSSING: {
                // obtain Crossing
                GNECrossing* crossing = myFrameParent->myViewNet->getNet()->retrieveCrossing(myHE->getID(), false);
                if (crossing) {
                    // obtain junction
                    GNEJunction* junction = crossing->getParentJunction();
                    // create junction item
                    FXTreeItem* junctionItem = myTreelist->insertItem(nullptr, nullptr, junction->getHierarchyName().c_str(), junction->getIcon(), junction->getIcon());
                    junctionItem->setExpanded(true);
                    // Save items in myTreeItemToACMap
                    myTreeItemToACMap[junctionItem] = junction;
                    // return junction Item
                    return junctionItem;
                } else {
                    return nullptr;
                }
            }
            case SUMO_TAG_CONNECTION: {
                // obtain Connection
                GNEConnection* connection = myFrameParent->myViewNet->getNet()->retrieveConnection(myHE->getID(), false);
                if (connection) {
                    // create edge from item
                    FXTreeItem* edgeFromItem = myTreelist->insertItem(nullptr, nullptr, connection->getEdgeFrom()->getHierarchyName().c_str(), connection->getEdgeFrom()->getIcon(), connection->getEdgeFrom()->getIcon());
                    edgeFromItem->setExpanded(true);
                    // create edge to item
                    FXTreeItem* edgeToItem = myTreelist->insertItem(nullptr, nullptr, connection->getEdgeTo()->getHierarchyName().c_str(), connection->getEdgeTo()->getIcon(), connection->getEdgeTo()->getIcon());
                    edgeToItem->setExpanded(true);
                    // create connection item
                    FXTreeItem* connectionItem = myTreelist->insertItem(nullptr, edgeToItem, connection->getHierarchyName().c_str(), connection->getIcon(), connection->getIcon());
                    connectionItem->setExpanded(true);
                    // Save items in myTreeItemToACMap
                    myTreeItemToACMap[edgeFromItem] = connection->getEdgeFrom();
                    myTreeItemToACMap[edgeToItem] = connection->getEdgeTo();
                    myTreeItemToACMap[connectionItem] = connection;
                    // return connection item
                    return connectionItem;
                } else {
                    return nullptr;
                }
            }
            default:
                break;
        }
    } else if (myHE->getTagProperty().getTag() == SUMO_TAG_POILANE) {
        // Obtain POILane
        GNEShape* POILane = myFrameParent->myViewNet->getNet()->retrieveShape(SUMO_TAG_POILANE, myHE->getID(), false);
        if (POILane) {
            // obtain parent lane
            GNELane* lane = myFrameParent->myViewNet->getNet()->retrieveLane(POILane->getParentLanes().at(0)->getID());
            // obtain parent edge
            GNEEdge* edge = myFrameParent->myViewNet->getNet()->retrieveEdge(lane->getParentEdge()->getID());
            //inser Junctions of lane of edge in tree (Pararell because a edge has always two Junctions)
            FXTreeItem* junctionSourceItem = myTreelist->insertItem(nullptr, nullptr, (edge->getParentJunctions().front()->getHierarchyName() + " origin").c_str(), edge->getParentJunctions().front()->getIcon(), edge->getParentJunctions().front()->getIcon());
            FXTreeItem* junctionDestinyItem = myTreelist->insertItem(nullptr, nullptr, (edge->getParentJunctions().front()->getHierarchyName() + " destiny").c_str(), edge->getParentJunctions().front()->getIcon(), edge->getParentJunctions().front()->getIcon());
            junctionDestinyItem->setExpanded(true);
            // Create edge item
            FXTreeItem* edgeItem = myTreelist->insertItem(nullptr, junctionDestinyItem, edge->getHierarchyName().c_str(), edge->getIcon(), edge->getIcon());
            edgeItem->setExpanded(true);
            // Create lane item
            FXTreeItem* laneItem = myTreelist->insertItem(nullptr, edgeItem, lane->getHierarchyName().c_str(), lane->getIcon(), lane->getIcon());
            laneItem->setExpanded(true);
            // Save items in myTreeItemToACMap
            myTreeItemToACMap[junctionSourceItem] = edge->getParentJunctions().front();
            myTreeItemToACMap[junctionDestinyItem] = edge->getParentJunctions().back();
            myTreeItemToACMap[edgeItem] = edge;
            myTreeItemToACMap[laneItem] = lane;
            // return Lane item
            return laneItem;
        } else {
            return nullptr;
        }
    } else if (myHE->getTagProperty().isAdditionalElement()) {
        // Obtain Additional
        GNEAdditional* additional = myFrameParent->myViewNet->getNet()->retrieveAdditional(myHE->getTagProperty().getTag(), myHE->getID(), false);
        if (additional) {
            // declare auxiliar FXTreeItem, due a demand element can have multiple "roots"
            FXTreeItem* root = nullptr;
            // check if there is demand elements parents
            if (additional->getParentAdditionals().size() > 0) {
                // check if we have more than one edge
                if (additional->getParentAdditionals().size() > 1) {
                    // insert first item
                    addListItem(additional->getParentAdditionals().front());
                    // insert "spacer"
                    if (additional->getParentAdditionals().size() > 2) {
                        addListItem(nullptr, ("..." + toString((int)additional->getParentAdditionals().size() - 2) + " additionals...").c_str(), 0, false);
                    }
                }
                // return last inserted item
                root = addListItem(additional->getParentAdditionals().back());
            }
            // check if there is parent demand elements
            if (additional->getParentDemandElements().size() > 0) {
                // check if we have more than one demand element
                if (additional->getParentDemandElements().size() > 1) {
                    // insert first item
                    addListItem(additional->getParentDemandElements().front());
                    // insert "spacer"
                    if (additional->getParentDemandElements().size() > 2) {
                        addListItem(nullptr, ("..." + toString((int)additional->getParentDemandElements().size() - 2) + " demand elements...").c_str(), 0, false);
                    }
                }
                // return last inserted item
                root = addListItem(additional->getParentDemandElements().back());
            }
            // check if there is parent edges
            if (additional->getParentEdges().size() > 0) {
                // check if we have more than one edge
                if (additional->getParentEdges().size() > 1) {
                    // insert first item
                    addListItem(additional->getParentEdges().front());
                    // insert "spacer"
                    if (additional->getParentEdges().size() > 2) {
                        addListItem(nullptr, ("..." + toString((int)additional->getParentEdges().size() - 2) + " edges...").c_str(), 0, false);
                    }
                }
                // return last inserted item
                root = addListItem(additional->getParentEdges().back());
            }
            // check if there is parent lanes
            if (additional->getParentLanes().size() > 0) {
                // check if we have more than one parent lane
                if (additional->getParentLanes().size() > 1) {
                    // insert first item
                    addListItem(additional->getParentLanes().front());
                    // insert "spacer"
                    if (additional->getParentLanes().size() > 2) {
                        addListItem(nullptr, ("..." + toString((int)additional->getParentLanes().size() - 2) + " lanes...").c_str(), 0, false);
                    }
                }
                // return last inserted item
                root = addListItem(additional->getParentLanes().back());
            }
            // return last inserted list item
            return root;
        }
    } else if (myHE->getTagProperty().isTAZElement()) {
        // Obtain TAZElement
        GNETAZElement* TAZElement = myFrameParent->myViewNet->getNet()->retrieveTAZElement(myHE->getTagProperty().getTag(), myHE->getID(), false);
        if (TAZElement) {
            // declare auxiliar FXTreeItem, due a demand element can have multiple "roots"
            FXTreeItem* root = nullptr;
            // check if there is demand elements parents
            if (TAZElement->getParentTAZElements().size() > 0) {
                // check if we have more than one edge
                if (TAZElement->getParentTAZElements().size() > 1) {
                    // insert first item
                    addListItem(TAZElement->getParentTAZElements().front());
                    // insert "spacer"
                    if (TAZElement->getParentTAZElements().size() > 2) {
                        addListItem(nullptr, ("..." + toString((int)TAZElement->getParentTAZElements().size() - 2) + " TAZElements...").c_str(), 0, false);
                    }
                }
                // return last inserted item
                root = addListItem(TAZElement->getParentTAZElements().back());
            }
            // check if there is parent demand elements
            if (TAZElement->getParentDemandElements().size() > 0) {
                // check if we have more than one demand element
                if (TAZElement->getParentDemandElements().size() > 1) {
                    // insert first item
                    addListItem(TAZElement->getParentDemandElements().front());
                    // insert "spacer"
                    if (TAZElement->getParentDemandElements().size() > 2) {
                        addListItem(nullptr, ("..." + toString((int)TAZElement->getParentDemandElements().size() - 2) + " demand elements...").c_str(), 0, false);
                    }
                }
                // return last inserted item
                root = addListItem(TAZElement->getParentDemandElements().back());
            }
            // check if there is parent edges
            if (TAZElement->getParentEdges().size() > 0) {
                // check if we have more than one edge
                if (TAZElement->getParentEdges().size() > 1) {
                    // insert first item
                    addListItem(TAZElement->getParentEdges().front());
                    // insert "spacer"
                    if (TAZElement->getParentEdges().size() > 2) {
                        addListItem(nullptr, ("..." + toString((int)TAZElement->getParentEdges().size() - 2) + " edges...").c_str(), 0, false);
                    }
                }
                // return last inserted item
                root = addListItem(TAZElement->getParentEdges().back());
            }
            // check if there is parent lanes
            if (TAZElement->getParentLanes().size() > 0) {
                // check if we have more than one parent lane
                if (TAZElement->getParentLanes().size() > 1) {
                    // insert first item
                    addListItem(TAZElement->getParentLanes().front());
                    // insert "spacer"
                    if (TAZElement->getParentLanes().size() > 2) {
                        addListItem(nullptr, ("..." + toString((int)TAZElement->getParentLanes().size() - 2) + " lanes...").c_str(), 0, false);
                    }
                }
                // return last inserted item
                root = addListItem(TAZElement->getParentLanes().back());
            }
            // return last inserted list item
            return root;
        }
    } else if (myHE->getTagProperty().isDemandElement()) {
        // Obtain DemandElement
        GNEDemandElement* demandElement = myFrameParent->myViewNet->getNet()->retrieveDemandElement(myHE->getTagProperty().getTag(), myHE->getID(), false);
        if (demandElement) {
            // declare auxiliar FXTreeItem, due a demand element can have multiple "roots"
            FXTreeItem* root = nullptr;
            // check if there is demand elements parents
            if (demandElement->getParentAdditionals().size() > 0) {
                // check if we have more than one edge
                if (demandElement->getParentAdditionals().size() > 1) {
                    // insert first item
                    addListItem(demandElement->getParentAdditionals().front());
                    // insert "spacer"
                    if (demandElement->getParentAdditionals().size() > 2) {
                        addListItem(nullptr, ("..." + toString((int)demandElement->getParentAdditionals().size() - 2) + " additionals...").c_str(), 0, false);
                    }
                }
                // return last inserted item
                root = addListItem(demandElement->getParentAdditionals().back());
            }
            // check if there is parent demand elements
            if (demandElement->getParentDemandElements().size() > 0) {
                // check if we have more than one demand element
                if (demandElement->getParentDemandElements().size() > 1) {
                    // insert first item
                    addListItem(demandElement->getParentDemandElements().front());
                    // insert "spacer"
                    if (demandElement->getParentDemandElements().size() > 2) {
                        addListItem(nullptr, ("..." + toString((int)demandElement->getParentDemandElements().size() - 2) + " demand elements...").c_str(), 0, false);
                    }
                }
                // return last inserted item
                root = addListItem(demandElement->getParentDemandElements().back());
            }
            // check if there is parent edges
            if (demandElement->getParentEdges().size() > 0) {
                // check if we have more than one edge
                if (demandElement->getParentEdges().size() > 1) {
                    // insert first item
                    addListItem(demandElement->getParentEdges().front());
                    // insert "spacer"
                    if (demandElement->getParentEdges().size() > 2) {
                        addListItem(nullptr, ("..." + toString((int)demandElement->getParentEdges().size() - 2) + " edges...").c_str(), 0, false);
                    }
                }
                // return last inserted item
                root = addListItem(demandElement->getParentEdges().back());
            }
            // check if there is parent lanes
            if (demandElement->getParentLanes().size() > 0) {
                // check if we have more than one parent lane
                if (demandElement->getParentLanes().size() > 1) {
                    // insert first item
                    addListItem(demandElement->getParentLanes().front());
                    // insert "spacer"
                    if (demandElement->getParentLanes().size() > 2) {
                        addListItem(nullptr, ("..." + toString((int)demandElement->getParentLanes().size() - 2) + " lanes...").c_str(), 0, false);
                    }
                }
                // return last inserted item
                root = addListItem(demandElement->getParentLanes().back());
            }
            // return last inserted list item
            return root;
        }

    } else if (myHE->getTagProperty().isDataElement()) {
        // check if is a GNEDataInterval or a GNEGenericData
        if (myHE->getTagProperty().getTag() == SUMO_TAG_DATASET) {
            return nullptr;
        } else if (myHE->getTagProperty().getTag() == SUMO_TAG_DATAINTERVAL) {
            return addListItem(myFrameParent->myViewNet->getNet()->retrieveDataSet(myHE->getID()));
        } else {
            // Obtain DataElement
            GNEGenericData* dataElement = dynamic_cast<GNEGenericData*>(myHE);
            if (dataElement) {
                // declare auxiliar FXTreeItem, due a data element can have multiple "roots"
                FXTreeItem* root = nullptr;
                // set dataset
                addListItem(dataElement->getDataIntervalParent()->getDataSetParent());
                // set data interval
                addListItem(dataElement->getDataIntervalParent());
                // check if there is data elements parents
                if (dataElement->getParentAdditionals().size() > 0) {
                    // check if we have more than one edge
                    if (dataElement->getParentAdditionals().size() > 1) {
                        // insert first item
                        addListItem(dataElement->getParentAdditionals().front());
                        // insert "spacer"
                        if (dataElement->getParentAdditionals().size() > 2) {
                            addListItem(nullptr, ("..." + toString((int)dataElement->getParentAdditionals().size() - 2) + " additionals...").c_str(), 0, false);
                        }
                    }
                    // return last inserted item
                    root = addListItem(dataElement->getParentAdditionals().back());
                }
                // check if there is parent demand elements
                if (dataElement->getParentDemandElements().size() > 0) {
                    // check if we have more than one demand element
                    if (dataElement->getParentDemandElements().size() > 1) {
                        // insert first item
                        addListItem(dataElement->getParentDemandElements().front());
                        // insert "spacer"
                        if (dataElement->getParentDemandElements().size() > 2) {
                            addListItem(nullptr, ("..." + toString((int)dataElement->getParentDemandElements().size() - 2) + " demand elements...").c_str(), 0, false);
                        }
                    }
                    // return last inserted item
                    root = addListItem(dataElement->getParentDemandElements().back());
                }
                // check if there is parent edges
                if (dataElement->getParentEdges().size() > 0) {
                    // check if we have more than one edge
                    if (dataElement->getParentEdges().size() > 1) {
                        // insert first ege
                        if (dataElement->getTagProperty().getTag() == SUMO_TAG_EDGEREL) {
                            addListItem(dataElement->getParentEdges().front(), nullptr, "from ");
                        } else {
                            addListItem(dataElement->getParentEdges().front());
                        }
                        // insert "spacer"
                        if (dataElement->getParentEdges().size() > 2) {
                            addListItem(nullptr, ("..." + toString((int)dataElement->getParentEdges().size() - 2) + " edges...").c_str(), 0, false);
                        }
                    }
                    // insert last ege
                    if (dataElement->getTagProperty().getTag() == SUMO_TAG_EDGEREL) {
                        addListItem(dataElement->getParentEdges().back(), nullptr, "to ");
                    } else {
                        addListItem(dataElement->getParentEdges().back());
                    }
                }
                // check if there is parent lanes
                if (dataElement->getParentLanes().size() > 0) {
                    // check if we have more than one parent lane
                    if (dataElement->getParentLanes().size() > 1) {
                        // insert first item
                        addListItem(dataElement->getParentLanes().front());
                        // insert "spacer"
                        if (dataElement->getParentLanes().size() > 2) {
                            addListItem(nullptr, ("..." + toString((int)dataElement->getParentLanes().size() - 2) + " lanes...").c_str(), 0, false);
                        }
                    }
                    // return last inserted item
                    root = addListItem(dataElement->getParentLanes().back());
                }
                // return last inserted list item
                return root;
            }
        }
    }
    // there aren't parents
    return nullptr;
}


void
GNEFrameModuls::HierarchicalElementTree::showHierarchicalElementChildren(GNEHierarchicalElement* HE, FXTreeItem* itemParent) {
    if (HE->getTagProperty().isNetworkElement()) {
        // Switch gl type of ac
        switch (HE->getTagProperty().getTag()) {
            case SUMO_TAG_JUNCTION: {
                // retrieve junction
                GNEJunction* junction = myFrameParent->myViewNet->getNet()->retrieveJunction(HE->getID(), false);
                if (junction) {
                    // insert junction item
                    FXTreeItem* junctionItem = addListItem(HE, itemParent);
                    // insert edges
                    for (auto i : junction->getChildEdges()) {
                        showHierarchicalElementChildren(i, junctionItem);
                    }
                    // insert crossings
                    for (auto i : junction->getGNECrossings()) {
                        showHierarchicalElementChildren(i, junctionItem);
                    }
                }
                break;
            }
            case SUMO_TAG_EDGE: {
                // retrieve edge
                GNEEdge* edge = myFrameParent->myViewNet->getNet()->retrieveEdge(HE->getID(), false);
                if (edge) {
                    // insert edge item
                    FXTreeItem* edgeItem = addListItem(HE, itemParent);
                    // insert lanes
                    for (const auto& i : edge->getLanes()) {
                        showHierarchicalElementChildren(i, edgeItem);
                    }
                    // insert child additional
                    for (const auto& i : edge->getChildAdditionals()) {
                        showHierarchicalElementChildren(i, edgeItem);
                    }
                    // insert child shapes
                    for (const auto& i : edge->getChildShapes()) {
                        showHierarchicalElementChildren(i, edgeItem);
                    }
                    // insert child TAZElements
                    for (const auto& i : edge->getChildTAZElements()) {
                        showHierarchicalElementChildren(i, edgeItem);
                    }
                    // insert child demand elements
                    for (const auto& i : edge->getChildDemandElements()) {
                        showHierarchicalElementChildren(i, edgeItem);
                    }
                    /*

                    CHECK THIS

                    // insert demand elements children (note: use getChildDemandElementsSortedByType to avoid duplicated elements)
                    for (const auto& i : edge->getChildDemandElementsByType(SUMO_TAG_ROUTE)) {
                        showHierarchicalElementChildren(i, edgeItem);
                    }
                    for (const auto& i : edge->getChildDemandElementsByType(SUMO_TAG_TRIP)) {
                        showHierarchicalElementChildren(i, edgeItem);
                    }
                    for (const auto& i : edge->getChildDemandElementsByType(SUMO_TAG_FLOW)) {
                        showHierarchicalElementChildren(i, edgeItem);
                    }
                    */
                    // show data elements
                    for (const auto& i : edge->getChildGenericDatas()) {
                        showHierarchicalElementChildren(i, edgeItem);
                    }
                }
                break;
            }
            case SUMO_TAG_LANE: {
                // retrieve lane
                GNELane* lane = myFrameParent->myViewNet->getNet()->retrieveLane(HE->getID(), false);
                if (lane) {
                    // insert lane item
                    FXTreeItem* laneItem = addListItem(HE, itemParent);
                    // insert child additional
                    for (const auto& i : lane->getChildAdditionals()) {
                        showHierarchicalElementChildren(i, laneItem);
                    }
                    // insert child shapes
                    for (const auto& i : lane->getChildShapes()) {
                        showHierarchicalElementChildren(i, laneItem);
                    }
                    // insert child TAZElements
                    for (const auto& i : lane->getChildTAZElements()) {
                        showHierarchicalElementChildren(i, laneItem);
                    }
                    // insert demand elements children
                    for (const auto& i : lane->getChildDemandElements()) {
                        showHierarchicalElementChildren(i, laneItem);
                    }
                    // insert incoming connections of lanes (by default isn't expanded)
                    if (lane->getGNEIncomingConnections().size() > 0) {
                        std::vector<GNEConnection*> incomingLaneConnections = lane->getGNEIncomingConnections();
                        // insert intermediate list item
                        FXTreeItem* incomingConnections = addListItem(laneItem, "Incomings", incomingLaneConnections.front()->getIcon(), false);
                        // insert incoming connections
                        for (auto i : incomingLaneConnections) {
                            showHierarchicalElementChildren(i, incomingConnections);
                        }
                    }
                    // insert outcoming connections of lanes (by default isn't expanded)
                    if (lane->getGNEOutcomingConnections().size() > 0) {
                        std::vector<GNEConnection*> outcomingLaneConnections = lane->getGNEOutcomingConnections();
                        // insert intermediate list item
                        FXTreeItem* outgoingConnections = addListItem(laneItem, "Outgoing", outcomingLaneConnections.front()->getIcon(), false);
                        // insert outcoming connections
                        for (auto i : outcomingLaneConnections) {
                            showHierarchicalElementChildren(i, outgoingConnections);
                        }
                    }
                }
                break;
            }
            case SUMO_TAG_CROSSING:
            case SUMO_TAG_CONNECTION: {
                // insert connection item
                addListItem(HE, itemParent);
                break;
            }
            default:
                break;
        }
    } else if (HE->getTagProperty().isAdditionalElement() || HE->getTagProperty().isShape() || HE->getTagProperty().isTAZElement() || HE->getTagProperty().isDemandElement()) {
        // insert additional item
        FXTreeItem* treeItem = addListItem(HE, itemParent);
        // insert child edges
        for (const auto& i : HE->getChildEdges()) {
            showHierarchicalElementChildren(i, treeItem);
        }
        // insert child lanes
        for (const auto& i : HE->getChildLanes()) {
            showHierarchicalElementChildren(i, treeItem);
        }
        // insert additional children
        for (const auto& i : HE->getChildAdditionals()) {
            showHierarchicalElementChildren(i, treeItem);
        }
        // insert child shapes
        for (const auto& i : HE->getChildShapes()) {
            showHierarchicalElementChildren(i, treeItem);
        }
        // insert TAZElements children
        for (const auto& i : HE->getChildTAZElements()) {
            showHierarchicalElementChildren(i, treeItem);
        }
        // insert child demand elements
        for (const auto& i : HE->getChildDemandElements()) {
            showHierarchicalElementChildren(i, treeItem);
        }
    } else if (HE->getTagProperty().isDataElement()) {
        // insert data item
        FXTreeItem* dataElementItem = addListItem(HE, itemParent);
        // insert intervals
        if (HE->getTagProperty().getTag() == SUMO_TAG_DATASET) {
            GNEDataSet* dataSet = myFrameParent->myViewNet->getNet()->retrieveDataSet(HE->getID());
            // iterate over intevals
            for (const auto& interval : dataSet->getDataIntervalChildren()) {
                showHierarchicalElementChildren(interval.second, dataElementItem);
            }
        } else if (HE->getTagProperty().getTag() == SUMO_TAG_DATAINTERVAL) {
            GNEDataInterval* dataInterval = dynamic_cast<GNEDataInterval*>(HE);
            // iterate over generic datas
            for (const auto& genericData : dataInterval->getGenericDataChildren()) {
                showHierarchicalElementChildren(genericData, dataElementItem);
            }
        }
    }
}


FXTreeItem*
GNEFrameModuls::HierarchicalElementTree::addListItem(GNEAttributeCarrier* AC, FXTreeItem* itemParent, std::string prefix, std::string sufix) {
    // insert item in Tree list
    FXTreeItem* item = myTreelist->insertItem(nullptr, itemParent, (prefix + AC->getHierarchyName() + sufix).c_str(), AC->getIcon(), AC->getIcon());
    // insert item in map
    myTreeItemToACMap[item] = AC;
    // by default item is expanded
    item->setExpanded(true);
    // return created FXTreeItem
    return item;
}


FXTreeItem*
GNEFrameModuls::HierarchicalElementTree::addListItem(FXTreeItem* itemParent, const std::string& text, FXIcon* icon, bool expanded) {
    // insert item in Tree list
    FXTreeItem* item = myTreelist->insertItem(nullptr, itemParent, text.c_str(), icon, icon);
    // expand item depending of flag expanded
    item->setExpanded(expanded);
    // return created FXTreeItem
    return item;
}

// ---------------------------------------------------------------------------
// GNEFrameModuls::DrawingShape - methods
// ---------------------------------------------------------------------------

GNEFrameModuls::DrawingShape::DrawingShape(GNEFrame* frameParent) :
    FXGroupBox(frameParent->myContentFrame, "Drawing", GUIDesignGroupBoxFrame),
    myFrameParent(frameParent),
    myDeleteLastCreatedPoint(false) {
    // create start and stop buttons
    myStartDrawingButton = new FXButton(this, "Start drawing", 0, this, MID_GNE_STARTDRAWING, GUIDesignButton);
    myStopDrawingButton = new FXButton(this, "Stop drawing", 0, this, MID_GNE_STOPDRAWING, GUIDesignButton);
    myAbortDrawingButton = new FXButton(this, "Abort drawing", 0, this, MID_GNE_ABORTDRAWING, GUIDesignButton);
    // create information label
    std::ostringstream information;
    information
            << "- 'Start drawing' or ENTER\n"
            << "  to create shape.\n"
            << "- 'Stop drawing' or ESC to\n"
            << "  abort shape creation.\n"
            << "- 'Shift + Click' to remove\n"
            << "  last inserted point.";
    myInformationLabel = new FXLabel(this, information.str().c_str(), 0, GUIDesignLabelFrameInformation);
    // disable stop and abort functions as init
    myStopDrawingButton->disable();
    myAbortDrawingButton->disable();
}


GNEFrameModuls::DrawingShape::~DrawingShape() {}


void GNEFrameModuls::DrawingShape::showDrawingShape() {
    // abort current drawing before show
    abortDrawing();
    // show FXGroupBox
    FXGroupBox::show();
}


void GNEFrameModuls::DrawingShape::hideDrawingShape() {
    // abort current drawing before hide
    abortDrawing();
    // show FXGroupBox
    FXGroupBox::hide();
}


void
GNEFrameModuls::DrawingShape::startDrawing() {
    // Only start drawing if DrawingShape modul is shown
    if (shown()) {
        // change buttons
        myStartDrawingButton->disable();
        myStopDrawingButton->enable();
        myAbortDrawingButton->enable();
    }
}


void
GNEFrameModuls::DrawingShape::stopDrawing() {
    // try to build shape
    if (myFrameParent->shapeDrawed()) {
        // clear created points
        myTemporalShape.clear();
        // change buttons
        myStartDrawingButton->enable();
        myStopDrawingButton->disable();
        myAbortDrawingButton->disable();
    } else {
        // abort drawing if shape cannot be created
        abortDrawing();
    }
}


void
GNEFrameModuls::DrawingShape::abortDrawing() {
    // clear created points
    myTemporalShape.clear();
    // change buttons
    myStartDrawingButton->enable();
    myStopDrawingButton->disable();
    myAbortDrawingButton->disable();
}


void
GNEFrameModuls::DrawingShape::addNewPoint(const Position& P) {
    if (myStopDrawingButton->isEnabled()) {
        myTemporalShape.push_back(P);
    } else {
        throw ProcessError("A new point cannot be added if drawing wasn't started");
    }
}


void
GNEFrameModuls::DrawingShape::removeLastPoint() {
    if (myTemporalShape.size() > 1) {
        myTemporalShape.pop_back();
    }
}


const PositionVector&
GNEFrameModuls::DrawingShape::getTemporalShape() const {
    return myTemporalShape;
}


bool
GNEFrameModuls::DrawingShape::isDrawing() const {
    return myStopDrawingButton->isEnabled();
}


void
GNEFrameModuls::DrawingShape::setDeleteLastCreatedPoint(bool value) {
    myDeleteLastCreatedPoint = value;
}


bool
GNEFrameModuls::DrawingShape::getDeleteLastCreatedPoint() {
    return myDeleteLastCreatedPoint;
}


long
GNEFrameModuls::DrawingShape::onCmdStartDrawing(FXObject*, FXSelector, void*) {
    startDrawing();
    return 0;
}


long
GNEFrameModuls::DrawingShape::onCmdStopDrawing(FXObject*, FXSelector, void*) {
    stopDrawing();
    return 0;
}


long
GNEFrameModuls::DrawingShape::onCmdAbortDrawing(FXObject*, FXSelector, void*) {
    abortDrawing();
    return 0;
}

// ---------------------------------------------------------------------------
// GNEFrameModuls::SelectorParent - methods
// ---------------------------------------------------------------------------

GNEFrameModuls::SelectorParent::SelectorParent(GNEFrame* frameParent) :
    FXGroupBox(frameParent->myContentFrame, "Parent selector", GUIDesignGroupBoxFrame),
    myFrameParent(frameParent),
    myParentTag(SUMO_TAG_NOTHING) {
    // Create label with the type of SelectorParent
    myParentsLabel = new FXLabel(this, "No additional selected", nullptr, GUIDesignLabelLeftThick);
    // Create list
    myParentsList = new FXList(this, this, MID_GNE_SET_TYPE, GUIDesignListSingleElementFixedHeight);
    // Hide List
    hideSelectorParentModul();
}


GNEFrameModuls::SelectorParent::~SelectorParent() {}


std::string
GNEFrameModuls::SelectorParent::getIdSelected() const {
    for (int i = 0; i < myParentsList->getNumItems(); i++) {
        if (myParentsList->isItemSelected(i)) {
            return myParentsList->getItem(i)->getText().text();
        }
    }
    return "";
}


void
GNEFrameModuls::SelectorParent::setIDSelected(const std::string& id) {
    // first unselect all
    for (int i = 0; i < myParentsList->getNumItems(); i++) {
        myParentsList->getItem(i)->setSelected(false);
    }
    // select element if correspond to given ID
    for (int i = 0; i < myParentsList->getNumItems(); i++) {
        if (myParentsList->getItem(i)->getText().text() == id) {
            myParentsList->getItem(i)->setSelected(true);
        }
    }
    // recalc myFirstParentsList
    myParentsList->recalc();
}


bool
GNEFrameModuls::SelectorParent::showSelectorParentModul(SumoXMLTag additionalType) {
    // make sure that we're editing an additional tag
    const auto listOfTags = GNEAttributeCarrier::getAllowedTagsByCategory(GNETagProperties::TagType::ADDITIONALELEMENT, false);
    for (const auto tag : listOfTags) {
        if (tag.first == additionalType) {
            myParentTag = additionalType;
            myParentsLabel->setText(("Parent type: " + tag.second).c_str());
            refreshSelectorParentModul();
            show();
            return true;
        }
    }
    return false;
}


void
GNEFrameModuls::SelectorParent::hideSelectorParentModul() {
    myParentTag = SUMO_TAG_NOTHING;
    hide();
}


void
GNEFrameModuls::SelectorParent::refreshSelectorParentModul() {
    myParentsList->clearItems();
    if (myParentTag != SUMO_TAG_NOTHING) {
        // fill list with IDs of additionals
        for (const auto& i : myFrameParent->getViewNet()->getNet()->getAttributeCarriers()->getAdditionals().at(myParentTag)) {
            myParentsList->appendItem(i.first.c_str());
        }
    }
}

// ---------------------------------------------------------------------------
// GNEFrameModuls::OverlappedInspection - methods
// ---------------------------------------------------------------------------

GNEFrameModuls::OverlappedInspection::OverlappedInspection(GNEFrame* frameParent) :
    FXGroupBox(frameParent->myContentFrame, "Overlapped elements", GUIDesignGroupBoxFrame),
    myFrameParent(frameParent),
    myFilteredTag(SUMO_TAG_NOTHING),
    myItemIndex(0) {
    // build elements
    buildFXElements();
}


GNEFrameModuls::OverlappedInspection::OverlappedInspection(GNEFrame* frameParent, const SumoXMLTag filteredTag) :
    FXGroupBox(frameParent->myContentFrame, ("Overlapped " + toString(filteredTag) + "s").c_str(), GUIDesignGroupBoxFrame),
    myFrameParent(frameParent),
    myFilteredTag(filteredTag),
    myItemIndex(0) {
    // build elements
    buildFXElements();
}


GNEFrameModuls::OverlappedInspection::~OverlappedInspection() {}


void
GNEFrameModuls::OverlappedInspection::showOverlappedInspection(const GNEViewNetHelper::ObjectsUnderCursor& objectsUnderCursor, const Position& clickedPosition) {
    // first clear myOverlappedACs
    myOverlappedACs.clear();
    // reserve
    myOverlappedACs.reserve(objectsUnderCursor.getClickedAttributeCarriers().size());
    // iterate over objects under cursor
    for (const auto& AC : objectsUnderCursor.getClickedAttributeCarriers()) {
        bool insert = true;
        // special case for supermode data
        if (myFrameParent->getViewNet()->getEditModes().isCurrentSupermodeData() &&
                !AC->getTagProperty().isGenericData()) {
            insert = false;
        }
        // check filter
        if ((myFilteredTag != SUMO_TAG_NOTHING) && (AC->getTagProperty().getTag() != myFilteredTag)) {
            insert = false;
        }
        if (insert) {
            myOverlappedACs.push_back(AC);
        }
    }
    mySavedClickedPosition = clickedPosition;
    // by default we inspect first element
    myItemIndex = 0;
    // update text of current index button
    myCurrentIndexButton->setText(("1 / " + toString(myOverlappedACs.size())).c_str());
    // clear and fill list again
    myOverlappedElementList->clearItems();
    for (int i = 0; i < (int)myOverlappedACs.size(); i++) {
        myOverlappedElementList->insertItem(i, myOverlappedACs.at(i)->getID().c_str(), myOverlappedACs.at(i)->getIcon());
    }
    // set first element as selected element
    myOverlappedElementList->getItem(0)->setSelected(TRUE);
    // by default list hidden
    myOverlappedElementList->hide();
    // show template editor
    show();
}


void
GNEFrameModuls::OverlappedInspection::hideOverlappedInspection() {
    // hide modul
    hide();
}


bool
GNEFrameModuls::OverlappedInspection::overlappedInspectionShown() const {
    return shown();
}


int
GNEFrameModuls::OverlappedInspection::getNumberOfOverlappedACs() const {
    return (int)myOverlappedACs.size();
}


bool
GNEFrameModuls::OverlappedInspection::checkSavedPosition(const Position& clickedPosition) const {
    return (mySavedClickedPosition.distanceSquaredTo2D(clickedPosition) < 0.25);
}


bool
GNEFrameModuls::OverlappedInspection::nextElement(const Position& clickedPosition) {
    // first check if OverlappedInspection is shown
    if (shown()) {
        // check if given position is near saved position
        if (checkSavedPosition(clickedPosition)) {
            // inspect next element
            onCmdNextElement(0, 0, 0);
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}


bool
GNEFrameModuls::OverlappedInspection::previousElement(const Position& clickedPosition) {
    // first check if OverlappedInspection is shown
    if (shown()) {
        // check if given position is near saved position
        if (checkSavedPosition(clickedPosition)) {
            // inspect previousElement
            onCmdPreviousElement(0, 0, 0);
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}


long
GNEFrameModuls::OverlappedInspection::onCmdPreviousElement(FXObject*, FXSelector, void*) {
    // check if there is items
    if (myOverlappedElementList->getNumItems() > 0) {
        // unselect current list element
        myOverlappedElementList->getItem((int)myItemIndex)->setSelected(FALSE);
        // set index (it works as a ring)
        if (myItemIndex > 0) {
            myItemIndex--;
        } else {
            myItemIndex = (myOverlappedACs.size() - 1);
        }
        // selected current list element
        myOverlappedElementList->getItem((int)myItemIndex)->setSelected(TRUE);
        myOverlappedElementList->update();
        // update current index button
        myCurrentIndexButton->setText((toString(myItemIndex + 1) + " / " + toString(myOverlappedACs.size())).c_str());
        // inspect overlapped attribute carrier
        myFrameParent->selectedOverlappedElement(myOverlappedACs.at(myItemIndex));
        // show OverlappedInspection again (because it's hidden in inspectSingleElement)
        show();
    }
    return 1;
}


long
GNEFrameModuls::OverlappedInspection::onCmdNextElement(FXObject*, FXSelector, void*) {
    // check if there is items
    if (myOverlappedElementList->getNumItems() > 0) {
        // unselect current list element
        myOverlappedElementList->getItem((int)myItemIndex)->setSelected(FALSE);
        // set index (it works as a ring)
        myItemIndex = (myItemIndex + 1) % myOverlappedACs.size();
        // selected current list element
        myOverlappedElementList->getItem((int)myItemIndex)->setSelected(TRUE);
        myOverlappedElementList->update();
        // update current index button
        myCurrentIndexButton->setText((toString(myItemIndex + 1) + " / " + toString(myOverlappedACs.size())).c_str());
        // inspect overlapped attribute carrier
        myFrameParent->selectedOverlappedElement(myOverlappedACs.at(myItemIndex));
        // show OverlappedInspection again (because it's hidden in inspectSingleElement)
        show();
    }
    return 1;
}


long
GNEFrameModuls::OverlappedInspection::onCmdShowList(FXObject*, FXSelector, void*) {
    // show or hidde element list
    if (myOverlappedElementList->shown()) {
        myOverlappedElementList->hide();
    } else {
        myOverlappedElementList->show();
    }
    if (myOverlappedElementList->getNumItems() <= 10) {
        myOverlappedElementList->setHeight(23 * myOverlappedElementList->getNumItems());
    } else {
        myOverlappedElementList->setHeight(230);
    }
    myOverlappedElementList->recalc();
    // recalc and update frame
    recalc();
    return 1;
}

long
GNEFrameModuls::OverlappedInspection::onCmdListItemSelected(FXObject*, FXSelector, void*) {
    for (int i = 0; i < myOverlappedElementList->getNumItems(); i++) {
        if (myOverlappedElementList->getItem(i)->isSelected()) {
            myItemIndex = i;
            // update current index button
            myCurrentIndexButton->setText((toString(myItemIndex + 1) + " / " + toString(myOverlappedACs.size())).c_str());
            // inspect overlapped attribute carrier
            myFrameParent->selectedOverlappedElement(myOverlappedACs.at(myItemIndex));
            // show OverlappedInspection again (because it's hidden in inspectSingleElement)
            show();
            return 1;
        }
    }
    return 0;
}


long
GNEFrameModuls::OverlappedInspection::onCmdOverlappingHelp(FXObject*, FXSelector, void*) {
    FXDialogBox* helpDialog = new FXDialogBox(this, "GEO attributes Help", GUIDesignDialogBox);
    std::ostringstream help;
    help
            << " - Click in the same position\n"
            << "   for inspect next element\n"
            << " - Shift + Click in the same\n"
            << "   position for inspect\n"
            << "   previous element";
    new FXLabel(helpDialog, help.str().c_str(), nullptr, GUIDesignLabelFrameInformation);
    // "OK"
    new FXButton(helpDialog, "OK\t\tclose", GUIIconSubSys::getIcon(GUIIcon::ACCEPT), helpDialog, FXDialogBox::ID_ACCEPT, GUIDesignButtonOK);
    helpDialog->create();
    helpDialog->show();
    return 1;
}


GNEFrameModuls::OverlappedInspection::OverlappedInspection() :
    myFrameParent(nullptr),
    myPreviousElement(nullptr),
    myCurrentIndexButton(nullptr),
    myNextElement(nullptr),
    myOverlappedElementList(nullptr),
    myHelpButton(nullptr),
    myFilteredTag(SUMO_TAG_NOTHING),
    myItemIndex(0) {
}


void
GNEFrameModuls::OverlappedInspection::buildFXElements() {
    FXHorizontalFrame* frameButtons = new FXHorizontalFrame(this, GUIDesignAuxiliarHorizontalFrame);
    // Create previous Item Button
    myPreviousElement = new FXButton(frameButtons, "", GUIIconSubSys::getIcon(GUIIcon::BIGARROWLEFT), this, MID_GNE_OVERLAPPED_PREVIOUS, GUIDesignButtonIconRectangular);
    // create current index button
    myCurrentIndexButton = new FXButton(frameButtons, "", nullptr, this, MID_GNE_OVERLAPPED_SHOWLIST, GUIDesignButton);
    // Create next Item Button
    myNextElement = new FXButton(frameButtons, "", GUIIconSubSys::getIcon(GUIIcon::BIGARROWRIGHT), this, MID_GNE_OVERLAPPED_NEXT, GUIDesignButtonIconRectangular);
    // Create list of overlapped elements (by default hidden)
    myOverlappedElementList = new FXList(this, this, MID_GNE_OVERLAPPED_ITEMSELECTED, GUIDesignListFixedHeight);
    // by default list of overlapped elements is hidden)
    myOverlappedElementList->hide();
    // Create help button
    myHelpButton = new FXButton(this, "Help", nullptr, this, MID_HELP, GUIDesignButtonRectangular);
}

// ---------------------------------------------------------------------------
// GNEFrameModuls::PathCreator - methods
// ---------------------------------------------------------------------------

GNEFrameModuls::PathCreator::Path::Path(const SUMOVehicleClass vClass, GNEEdge* edge) :
    mySubPath({edge}),
          myFromBusStop(nullptr),
          myToBusStop(nullptr),
          myConflictVClass(false),
myConflictDisconnected(false) {
    // check if we have to change vClass flag
    if (edge->getNBEdge()->getNumLanesThatAllow(vClass) == 0) {
        myConflictVClass = true;
    }
}


GNEFrameModuls::PathCreator::Path::Path(GNEViewNet* viewNet, const SUMOVehicleClass vClass, GNEEdge* edgeFrom, GNEEdge* edgeTo) :
    myFromBusStop(nullptr),
    myToBusStop(nullptr),
    myConflictVClass(false),
    myConflictDisconnected(false) {
    // calculate subpath
    mySubPath = viewNet->getNet()->getPathCalculator()->calculatePath(vClass, {edgeFrom, edgeTo});
    // if subPath is empty, try it with pedestrian (i.e. ignoring vCass)
    if (mySubPath.empty()) {
        mySubPath = viewNet->getNet()->getPathCalculator()->calculatePath(SVC_PEDESTRIAN, {edgeFrom, edgeTo});
        if (mySubPath.empty()) {
            mySubPath = { edgeFrom, edgeTo };
            myConflictDisconnected = true;
        } else {
            myConflictVClass = true;
        }
    }
}


const std::vector<GNEEdge*>&
GNEFrameModuls::PathCreator::Path::getSubPath() const {
    return mySubPath;
}


GNEAdditional* GNEFrameModuls::PathCreator::Path::getFromBusStop() const {
    return myFromBusStop;
}


GNEAdditional* GNEFrameModuls::PathCreator::Path::getToBusStop() const {
    return myToBusStop;
}


bool
GNEFrameModuls::PathCreator::Path::isConflictVClass() const {
    return myConflictVClass;
}


bool
GNEFrameModuls::PathCreator::Path::isConflictDisconnected() const {
    return myConflictDisconnected;
}


GNEFrameModuls::PathCreator::Path::Path() :
    myFromBusStop(nullptr),
    myToBusStop(nullptr),
    myConflictVClass(false),
    myConflictDisconnected(false) {
}


GNEFrameModuls::PathCreator::PathCreator(GNEFrame* frameParent) :
    FXGroupBox(frameParent->myContentFrame, "Route creator", GUIDesignGroupBoxFrame),
    myFrameParent(frameParent),
    myVClass(SVC_PASSENGER),
    myCreationMode(0),
    myFromStoppingPlace(nullptr),
    myToStoppingPlace(nullptr),
    myRoute(nullptr) {
    // create label for route info
    myInfoRouteLabel = new FXLabel(this, "No edges selected", 0, GUIDesignLabelFrameThicked);
    // create button for finish route creation
    myFinishCreationButton = new FXButton(this, "Finish route creation", nullptr, this, MID_GNE_EDGEPATH_FINISH, GUIDesignButton);
    myFinishCreationButton->disable();
    // create button for abort route creation
    myAbortCreationButton = new FXButton(this, "Abort route creation", nullptr, this, MID_GNE_EDGEPATH_ABORT, GUIDesignButton);
    myAbortCreationButton->disable();
    // create button for remove last inserted edge
    myRemoveLastInsertedElement = new FXButton(this, "Remove last inserted edge", nullptr, this, MID_GNE_EDGEPATH_REMOVELAST, GUIDesignButton);
    myRemoveLastInsertedElement->disable();
    // create check button
    myShowCandidateEdges = new FXCheckButton(this, "Show candidate edges", this, MID_GNE_EDGEPATH_SHOWCANDIDATES, GUIDesignCheckButton);
    myShowCandidateEdges->setCheck(TRUE);
    // create shift label
    myShiftLabel = new FXLabel(this,
                               "SHIFT-click: ignore vClass",
                               0, GUIDesignLabelFrameInformation);
    // create control label
    myControlLabel = new FXLabel(this,
                                 "CTRL-click: add disconnected",
                                 0, GUIDesignLabelFrameInformation);
    // create backspace label (always shown)
    new FXLabel(this,
                "BACKSPACE: undo click",
                0, GUIDesignLabelFrameInformation);
}


GNEFrameModuls::PathCreator::~PathCreator() {}


void
GNEFrameModuls::PathCreator::showPathCreatorModul(SumoXMLTag tag, const bool firstElement, const bool consecutives) {
    // declare flag
    bool showPathCreator = true;
    // first abort creation
    abortPathCreation();
    // disable buttons
    myFinishCreationButton->disable();
    myAbortCreationButton->disable();
    myRemoveLastInsertedElement->disable();
    // reset creation mode
    myCreationMode = 0;
    // set first element
    if (firstElement) {
        myCreationMode |= REQUIERE_FIRSTELEMENT;
    }
    // set consecutive or non consecuives
    if (consecutives) {
        myCreationMode |= CONSECUTIVE_EDGES;
    } else {
        myCreationMode |= NONCONSECUTIVE_EDGES;
    }
    // set specific mode depending of tag
    switch (tag) {
        // routes
        case SUMO_TAG_ROUTE:
        case GNE_TAG_ROUTE_EMBEDDED:
            myCreationMode |= SHOW_CANDIDATE_EDGES;
            myCreationMode |= START_EDGE;
            myCreationMode |= END_EDGE;
            break;
        // vehicles
        case SUMO_TAG_VEHICLE:
        case GNE_TAG_FLOW_ROUTE:
        case GNE_TAG_WALK_ROUTE:
            myCreationMode |= SINGLE_ELEMENT;
            myCreationMode |= ROUTE;
            break;
        case SUMO_TAG_TRIP:
        case SUMO_TAG_FLOW:
        case GNE_TAG_VEHICLE_WITHROUTE:
        case GNE_TAG_FLOW_WITHROUTE:
            myCreationMode |= SHOW_CANDIDATE_EDGES;
            myCreationMode |= START_EDGE;
            myCreationMode |= END_EDGE;
            break;
        // edges
        case GNE_TAG_WALK_EDGES:
            myCreationMode |= SHOW_CANDIDATE_EDGES;
            myCreationMode |= START_EDGE;
            myCreationMode |= END_EDGE;
            break;
        // edge->edge
        case GNE_TAG_PERSONTRIP_EDGE_EDGE:
        case GNE_TAG_WALK_EDGE_EDGE:
        case GNE_TAG_RIDE_EDGE_EDGE:
            myCreationMode |= SHOW_CANDIDATE_EDGES;
            myCreationMode |= ONLY_FROMTO;
            myCreationMode |= START_EDGE;
            myCreationMode |= END_EDGE;
            break;
        // edge->busStop
        case GNE_TAG_PERSONTRIP_EDGE_BUSSTOP:
        case GNE_TAG_WALK_EDGE_BUSSTOP:
        case GNE_TAG_RIDE_EDGE_BUSSTOP:
            myCreationMode |= SHOW_CANDIDATE_EDGES;
            myCreationMode |= ONLY_FROMTO;
            myCreationMode |= START_BUSSTOP;
            myCreationMode |= END_BUSSTOP;
            break;
        // busStop->edge
        case GNE_TAG_PERSONTRIP_BUSSTOP_EDGE:
        case GNE_TAG_WALK_BUSSTOP_EDGE:
        case GNE_TAG_RIDE_BUSSTOP_EDGE:
            myCreationMode |= SHOW_CANDIDATE_EDGES;
            myCreationMode |= ONLY_FROMTO;
            myCreationMode |= START_BUSSTOP;
            myCreationMode |= END_EDGE;
            break;
        // busStop->busStop
        case GNE_TAG_PERSONTRIP_BUSSTOP_BUSSTOP:
        case GNE_TAG_WALK_BUSSTOP_BUSSTOP:
        case GNE_TAG_RIDE_BUSSTOP_BUSSTOP:
            myCreationMode |= ONLY_FROMTO;
            myCreationMode |= START_BUSSTOP;
            myCreationMode |= END_BUSSTOP;
            break;
        // stops
        case GNE_TAG_PERSONSTOP_BUSSTOP:
            myCreationMode |= SINGLE_ELEMENT;
            myCreationMode |= START_BUSSTOP;
            break;
        case GNE_TAG_PERSONSTOP_EDGE:
            myCreationMode |= SINGLE_ELEMENT;
            myCreationMode |= START_EDGE;
            break;
        // generic datas
        case SUMO_TAG_EDGEREL:
            myCreationMode |= ONLY_FROMTO;
            myCreationMode |= START_EDGE;
            myCreationMode |= END_EDGE;
            break;
        default:
            showPathCreator = false;
            break;
    }
    // check if show path creator
    if (showPathCreator) {
        // update edge colors
        updateEdgeColors();
        // recalc before show (to avoid graphic problems)
        recalc();
        // show modul
        show();
    } else {
        // hide modul
        hide();
    }
}


void
GNEFrameModuls::PathCreator::hidePathCreatorModul() {
    // clear path
    clearPath();
    // hide modul
    hide();
}


SUMOVehicleClass
GNEFrameModuls::PathCreator::getVClass() const {
    return myVClass;
}


void
GNEFrameModuls::PathCreator::setVClass(SUMOVehicleClass vClass) {
    myVClass = vClass;
    // update edge colors
    updateEdgeColors();
}


bool
GNEFrameModuls::PathCreator::addEdge(GNEEdge* edge, const bool shiftKeyPressed, const bool controlKeyPressed) {
    // check if edges are allowed
    if (((myCreationMode & CONSECUTIVE_EDGES) + (myCreationMode & NONCONSECUTIVE_EDGES) +
            (myCreationMode & START_EDGE) + (myCreationMode & END_EDGE)) == 0) {
        return false;
    }
    // check if only an edge is allowed
    if ((myCreationMode & SINGLE_ELEMENT) && (mySelectedEdges.size() == 1)) {
        return false;
    }
    // continue depending of number of selected eges
    if (mySelectedEdges.size() > 0) {
        // check double edges
        if (mySelectedEdges.back() == edge) {
            // Write warning
            WRITE_WARNING("Double edges aren't allowed");
            // abort add edge
            return false;
        }
        // check consecutive edges
        if (myCreationMode & Mode::CONSECUTIVE_EDGES) {
            // check that new edge is consecutive
            const auto& outgoingEdges = mySelectedEdges.back()->getParentJunctions().back()->getGNEOutgoingEdges();
            if (std::find(outgoingEdges.begin(), outgoingEdges.end(), edge) == outgoingEdges.end()) {
                // Write warning
                WRITE_WARNING("Only consecutives edges are allowed");
                // abort add edge
                return false;
            }
        }
    }
    // check number of edges
    if (mySelectedEdges.size() == 2 && (myCreationMode & Mode::ONLY_FROMTO)) {
        // Write warning
        WRITE_WARNING("Only two edges are allowed");
        // abort add edge
        return false;
    }
    // check candidate edge
    if ((myShowCandidateEdges->getCheck() == TRUE) && !edge->isPossibleCandidate()) {
        if (edge->isSpecialCandidate()) {
            if (!shiftKeyPressed) {
                // Write warning
                WRITE_WARNING("Invalid edge (SHIFT + click to add an invalid vClass edge)");
                // abort add edge
                return false;
            }
        } else if (edge->isConflictedCandidate()) {
            if (!controlKeyPressed) {
                // Write warning
                WRITE_WARNING("Invalid edge (CONTROL + click to add a disconnected edge)");
                // abort add edge
                return false;
            }
        }
    }
    // All checks ok, then add it in selected elements
    mySelectedEdges.push_back(edge);
    // enable abort route button
    myAbortCreationButton->enable();
    // enable finish button
    myFinishCreationButton->enable();
    // disable undo/redo
    myFrameParent->myViewNet->getViewParent()->getGNEAppWindows()->disableUndoRedo("route creation");
    // enable or disable remove last edge button
    if (mySelectedEdges.size() > 1) {
        myRemoveLastInsertedElement->enable();
    } else {
        myRemoveLastInsertedElement->disable();
    }
    // recalculate path
    recalculatePath();
    // update info route label
    updateInfoRouteLabel();
    // update edge colors
    updateEdgeColors();
    return true;
}


std::vector<GNEEdge*>
GNEFrameModuls::PathCreator::getSelectedEdges() const {
    return mySelectedEdges;
}


bool
GNEFrameModuls::PathCreator::addStoppingPlace(GNEAdditional* stoppingPlace, const bool /*shiftKeyPressed*/, const bool /*controlKeyPressed*/) {
    // check if stoppingPlaces aren allowed
    if (((myCreationMode & START_BUSSTOP) + (myCreationMode & END_BUSSTOP)) == 0) {
        return false;
    }
    // check if only a busStop is allowed
    if ((myCreationMode & SINGLE_ELEMENT) && myFromStoppingPlace) {
        return false;
    }
    // first add startBusStop
    if (myCreationMode & START_BUSSTOP) {
        // check if previously stopping place from was set
        if (myFromStoppingPlace) {
            // check if previously stopping place to was set
            if ((myCreationMode & END_BUSSTOP) && myToStoppingPlace) {
                return false;
            } else {
                myToStoppingPlace = stoppingPlace;
            }
        } else {
            myFromStoppingPlace = stoppingPlace;
        }
    } else if (myCreationMode & END_BUSSTOP) {
        // check if previously stopping place from was set
        if (myToStoppingPlace) {
            return false;
        } else {
            myToStoppingPlace = stoppingPlace;
        }
    }
    // enable abort route button
    myAbortCreationButton->enable();
    // enable finish button
    myFinishCreationButton->enable();
    // disable undo/redo
    myFrameParent->myViewNet->getViewParent()->getGNEAppWindows()->disableUndoRedo("route creation");
    // enable or disable remove last stoppingPlace button
    if (myFromStoppingPlace || myToStoppingPlace) {
        myRemoveLastInsertedElement->enable();
    } else {
        myRemoveLastInsertedElement->disable();
    }
    // recalculate path
    recalculatePath();
    // update info route label
    updateInfoRouteLabel();
    // update stoppingPlace colors
    updateEdgeColors();
    return true;
}


GNEAdditional*
GNEFrameModuls::PathCreator::getFromStoppingPlace(SumoXMLTag expectedTag) const {
    if (myFromStoppingPlace && (myFromStoppingPlace->getTagProperty().getTag() == expectedTag)) {
        return myFromStoppingPlace;
    } else {
        return nullptr;
    }
}


GNEAdditional*
GNEFrameModuls::PathCreator::getToStoppingPlace(SumoXMLTag expectedTag) const {
    if (myToStoppingPlace && (myToStoppingPlace->getTagProperty().getTag() == expectedTag)) {
        return myToStoppingPlace;
    } else {
        return nullptr;
    }
}


bool
GNEFrameModuls::PathCreator::addRoute(GNEDemandElement* route, const bool /*shiftKeyPressed*/, const bool /*controlKeyPressed*/) {
    // check if routes aren allowed
    if ((myCreationMode & ROUTE) == 0) {
        return false;
    }
    // check if previously a route was added
    if (myRoute) {
        return false;
    }
    // set route
    myRoute = route;
    // recalculate path
    recalculatePath();
    updateInfoRouteLabel();
    updateEdgeColors();
    return true;
}


GNEDemandElement*
GNEFrameModuls::PathCreator::getRoute() const {
    return myRoute;
}


const std::vector<GNEFrameModuls::PathCreator::Path>&
GNEFrameModuls::PathCreator::getPath() const {
    return myPath;
}


bool
GNEFrameModuls::PathCreator::drawCandidateEdgesWithSpecialColor() const {
    return (myShowCandidateEdges->getCheck() == TRUE);
}


void
GNEFrameModuls::PathCreator::updateEdgeColors() {
    // reset all flags
    for (const auto& edge : myFrameParent->myViewNet->getNet()->getAttributeCarriers()->getEdges()) {
        edge.second->resetCandidateFlags();
    }
    // set reachability
    if (mySelectedEdges.size() > 0) {
        // only coloring edges if checkbox "show candidate edges" is enabled
        if ((myShowCandidateEdges->getCheck() == TRUE) && (myCreationMode & SHOW_CANDIDATE_EDGES)) {
            // mark all edges as conflicted (to mark special candidates)
            for (const auto& edge : myFrameParent->myViewNet->getNet()->getAttributeCarriers()->getEdges()) {
                edge.second->setConflictedCandidate(true);
            }
            // set special candidates (Edges that are connected but aren't compatibles with current vClass
            setSpecialCandidates(mySelectedEdges.back());
            // mark again all edges as conflicted (to mark possible candidates)
            for (const auto& edge : myFrameParent->myViewNet->getNet()->getAttributeCarriers()->getEdges()) {
                edge.second->setConflictedCandidate(true);
            }
            // set possible candidates (Edges that are connected AND are compatibles with current vClass
            setPossibleCandidates(mySelectedEdges.back(), myVClass);
        }
        // now mark selected eges
        for (const auto& edge : mySelectedEdges) {
            edge->resetCandidateFlags();
            edge->setSourceCandidate(true);
        }
        // finally mark last selected element as target
        mySelectedEdges.back()->resetCandidateFlags();
        mySelectedEdges.back()->setTargetCandidate(true);
    } else if (myShowCandidateEdges->getCheck() == TRUE && (myCreationMode & SHOW_CANDIDATE_EDGES)) {
        // mark all edges that have at least one lane that allow given vClass
        for (const auto& edge : myFrameParent->myViewNet->getNet()->getAttributeCarriers()->getEdges()) {
            if (edge.second->getNBEdge()->getNumLanesThatAllow(myVClass) > 0) {
                edge.second->setPossibleCandidate(true);
            } else {
                edge.second->setSpecialCandidate(true);
            }
        }
    }
    // update view net
    myFrameParent->myViewNet->updateViewNet();
}


void
GNEFrameModuls::PathCreator::drawTemporalRoute(const GUIVisualizationSettings* s) const {
    if (myPath.size() > 0) {
        const double lineWidth = 0.35;
        const double lineWidthin = 0.25;
        // Add a draw matrix
        glPushMatrix();
        // Start with the drawing of the area traslating matrix to origin
        glTranslated(0, 0, GLO_MAX - 0.1);
        // set first color
        GLHelper::setColor(RGBColor::GREY);
        // iterate over path
        for (int i = 0; i < (int)myPath.size(); i++) {
            // get path
            const GNEFrameModuls::PathCreator::Path& path = myPath.at(i);
            // draw line over
            for (int j = 0; j < (int)path.getSubPath().size(); j++) {
                const GNELane* lane = path.getSubPath().at(j)->getLanes().back();
                if (((i == 0) && (j == 0)) || (j > 0)) {
                    GLHelper::drawBoxLines(lane->getLaneShape(), lineWidth);
                }
                // draw connection between lanes
                if ((j + 1) < (int)path.getSubPath().size()) {
                    const GNELane* nextLane = path.getSubPath().at(j + 1)->getLanes().back();
                    if (lane->getLane2laneConnections().exist(nextLane)) {
                        GLHelper::drawBoxLines(lane->getLane2laneConnections().getLane2laneGeometry(nextLane).getShape(), lineWidth);
                    } else {
                        GLHelper::drawBoxLines({lane->getLaneShape().back(), nextLane->getLaneShape().front()}, lineWidth);
                    }
                }
            }
        }
        glTranslated(0, 0, 0.1);
        // iterate over path again
        for (int i = 0; i < (int)myPath.size(); i++) {
            // get path
            const GNEFrameModuls::PathCreator::Path& path = myPath.at(i);
            // set path color color
            if ((myCreationMode & SHOW_CANDIDATE_EDGES) == 0) {
                GLHelper::setColor(RGBColor::ORANGE);
            } else if (path.isConflictDisconnected()) {
                GLHelper::setColor(s->candidateColorSettings.conflict);
            } else if (path.isConflictVClass()) {
                GLHelper::setColor(s->candidateColorSettings.special);
            } else {
                GLHelper::setColor(RGBColor::ORANGE);
            }
            // draw line over
            for (int j = 0; j < (int)path.getSubPath().size(); j++) {
                const GNELane* lane = path.getSubPath().at(j)->getLanes().back();
                if (((i == 0) && (j == 0)) || (j > 0)) {
                    GLHelper::drawBoxLines(lane->getLaneShape(), lineWidthin);
                }
                // draw connection between lanes
                if ((j + 1) < (int)path.getSubPath().size()) {
                    const GNELane* nextLane = path.getSubPath().at(j + 1)->getLanes().back();
                    if (lane->getLane2laneConnections().exist(nextLane)) {
                        GLHelper::drawBoxLines(lane->getLane2laneConnections().getLane2laneGeometry(nextLane).getShape(), lineWidthin);
                    } else {
                        GLHelper::drawBoxLines({ lane->getLaneShape().back(), nextLane->getLaneShape().front() }, lineWidthin);
                    }
                }
            }
        }
        // Pop last matrix
        glPopMatrix();
    }
}


void
GNEFrameModuls::PathCreator::createPath() {
    // call create path implemented in frame parent
    myFrameParent->createPath();
}


void
GNEFrameModuls::PathCreator::abortPathCreation() {
    // first check that there is elements
    if ((mySelectedEdges.size() > 0) || myFromStoppingPlace || myToStoppingPlace || myRoute) {
        // unblock undo/redo
        myFrameParent->myViewNet->getViewParent()->getGNEAppWindows()->enableUndoRedo();
        // clear edges
        clearPath();
        // disable buttons
        myFinishCreationButton->disable();
        myAbortCreationButton->disable();
        myRemoveLastInsertedElement->disable();
        // update info route label
        updateInfoRouteLabel();
        // update reachability
        updateEdgeColors();
        // update view (to see the new route)
        myFrameParent->getViewNet()->updateViewNet();
    }
}


void
GNEFrameModuls::PathCreator::removeLastElement() {
    if (mySelectedEdges.size() > 1) {
        // remove special color of last selected edge
        mySelectedEdges.back()->resetCandidateFlags();
        // remove last edge
        mySelectedEdges.pop_back();
        // change last edge flag
        if ((mySelectedEdges.size() > 0) && mySelectedEdges.back()->isSourceCandidate()) {
            mySelectedEdges.back()->setSourceCandidate(false);
            mySelectedEdges.back()->setTargetCandidate(true);
        }
        // enable or disable remove last edge button
        if (mySelectedEdges.size() > 1) {
            myRemoveLastInsertedElement->enable();
        } else {
            myRemoveLastInsertedElement->disable();
        }
        // recalculate path
        recalculatePath();
        // update info route label
        updateInfoRouteLabel();
        // update reachability
        updateEdgeColors();
        // update view
        myFrameParent->myViewNet->updateViewNet();
    }
}


long
GNEFrameModuls::PathCreator::onCmdCreatePath(FXObject*, FXSelector, void*) {
    // just call create path
    createPath();
    return 1;
}


long
GNEFrameModuls::PathCreator::onCmdAbortPathCreation(FXObject*, FXSelector, void*) {
    // just call abort path creation
    abortPathCreation();
    return 1;
}


long
GNEFrameModuls::PathCreator::onCmdRemoveLastElement(FXObject*, FXSelector, void*) {
    // just call remove last element
    removeLastElement();
    return 1;
}


long
GNEFrameModuls::PathCreator::onCmdShowCandidateEdges(FXObject*, FXSelector, void*) {
    // update labels
    if (myShowCandidateEdges->getCheck() == TRUE) {
        myShiftLabel->show();
        myControlLabel->show();
    } else {
        myShiftLabel->hide();
        myControlLabel->hide();
    }
    // recalc frame
    recalc();
    // update edge colors (view will be updated within function)
    updateEdgeColors();
    return 1;
}


void
GNEFrameModuls::PathCreator::updateInfoRouteLabel() {
    if (myPath.size() > 0) {
        // declare variables for route info
        double length = 0;
        double speed = 0;
        int pathSize = 0;
        for (const auto& path : myPath) {
            for (const auto& edge : path.getSubPath()) {
                length += edge->getNBEdge()->getLength();
                speed += edge->getNBEdge()->getSpeed();
            }
            pathSize += (int)path.getSubPath().size();
        }
        // declare ostringstream for label and fill it
        std::ostringstream information;
        information
                << "- Selected edges: " << toString(mySelectedEdges.size()) << "\n"
                << "- Path edges: " << toString(pathSize) << "\n"
                << "- Length: " << toString(length) << "\n"
                << "- Average speed: " << toString(speed / pathSize);
        // set new label
        myInfoRouteLabel->setText(information.str().c_str());
    } else {
        myInfoRouteLabel->setText("No edges selected");
    }
}


void
GNEFrameModuls::PathCreator::clearPath() {
    // reset all flags
    for (const auto& edge : myFrameParent->myViewNet->getNet()->getAttributeCarriers()->getEdges()) {
        edge.second->resetCandidateFlags();
    }
    // clear edges, additionals and route
    mySelectedEdges.clear();
    myFromStoppingPlace = nullptr;
    myToStoppingPlace = nullptr;
    myRoute = nullptr;
    // clear path
    myPath.clear();
    // update info route label
    updateInfoRouteLabel();
}


void
GNEFrameModuls::PathCreator::recalculatePath() {
    // first clear path
    myPath.clear();
    // set edges
    std::vector<GNEEdge*> edges;
    // add route edges
    if (myRoute) {
        edges = myRoute->getParentEdges();
    } else {
        // add from stopping place edge
        if (myFromStoppingPlace) {
            edges.push_back(myFromStoppingPlace->getParentLanes().front()->getParentEdge());
        }
        // add selected edges
        for (const auto& edge : mySelectedEdges) {
            edges.push_back(edge);
        }
        // add to stopping place edge
        if (myToStoppingPlace) {
            edges.push_back(myToStoppingPlace->getParentLanes().front()->getParentEdge());
        }
    }
    // fill paths
    if (edges.size() == 1) {
        myPath.push_back(Path(myVClass, edges.front()));
    } else {
        // add every segment
        for (int i = 1; i < (int)edges.size(); i++) {
            myPath.push_back(Path(myFrameParent->getViewNet(), myVClass, edges.at(i - 1), edges.at(i)));
        }
    }
}


void
GNEFrameModuls::PathCreator::setSpecialCandidates(GNEEdge* originEdge) {
    // first calculate reachability for pedestrians (we use it, because pedestran can walk in almost all edges)
    myFrameParent->getViewNet()->getNet()->getPathCalculator()->calculateReachability(SVC_PEDESTRIAN, originEdge);
    // change flags
    for (const auto& edge : myFrameParent->getViewNet()->getNet()->getAttributeCarriers()->getEdges()) {
        for (const auto& lane : edge.second->getLanes()) {
            if (lane->getReachability() > 0) {
                lane->getParentEdge()->resetCandidateFlags();
                lane->getParentEdge()->setSpecialCandidate(true);
            }
        }
    }
}

void
GNEFrameModuls::PathCreator::setPossibleCandidates(GNEEdge* originEdge, const SUMOVehicleClass vClass) {
    // first calculate reachability for pedestrians
    myFrameParent->getViewNet()->getNet()->getPathCalculator()->calculateReachability(vClass, originEdge);
    // change flags
    for (const auto& edge : myFrameParent->getViewNet()->getNet()->getAttributeCarriers()->getEdges()) {
        for (const auto& lane : edge.second->getLanes()) {
            if (lane->getReachability() > 0) {
                lane->getParentEdge()->resetCandidateFlags();
                lane->getParentEdge()->setPossibleCandidate(true);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// GNERouteFrame::Legend - methods
// ---------------------------------------------------------------------------

GNEFrameModuls::PathLegend::PathLegend(GNEFrame* frameParent) :
    FXGroupBox(frameParent->myContentFrame, "Legend", GUIDesignGroupBoxFrame) {
    // declare label
    FXLabel* legendLabel = nullptr;
    // edge candidate
    legendLabel = new FXLabel(this, " edge candidate", 0, GUIDesignLabelLeft);
    legendLabel->setBackColor(MFXUtils::getFXColor(frameParent->getViewNet()->getVisualisationSettings().candidateColorSettings.possible));
    legendLabel->setTextColor(MFXUtils::getFXColor(RGBColor::WHITE));
    // last edge selected
    legendLabel = new FXLabel(this, " last edge selected", 0, GUIDesignLabelLeft);
    legendLabel->setBackColor(MFXUtils::getFXColor(frameParent->getViewNet()->getVisualisationSettings().candidateColorSettings.target));
    // edge selected
    legendLabel = new FXLabel(this, " edge selected", 0, GUIDesignLabelLeft);
    legendLabel->setBackColor(MFXUtils::getFXColor(frameParent->getViewNet()->getVisualisationSettings().candidateColorSettings.source));
    // edge conflict (vClass)
    legendLabel = new FXLabel(this, " edge conflic (vClass)", 0, GUIDesignLabelLeft);
    legendLabel->setBackColor(MFXUtils::getFXColor(frameParent->getViewNet()->getVisualisationSettings().candidateColorSettings.special));
    // edge disconnected
    legendLabel = new FXLabel(this, " edge disconnected", 0, GUIDesignLabelLeft);
    legendLabel->setBackColor(MFXUtils::getFXColor(frameParent->getViewNet()->getVisualisationSettings().candidateColorSettings.conflict));
}


GNEFrameModuls::PathLegend::~PathLegend() {}


void
GNEFrameModuls::PathLegend::showPathLegendModul() {
    show();
}

void
GNEFrameModuls::PathLegend::hidePathLegendModul() {
    hide();
}

// ---------------------------------------------------------------------------
// GNEFrameModuls - methods
// ---------------------------------------------------------------------------

FXLabel*
GNEFrameModuls::buildRainbow(FXComposite* parent) {
    // create label for color information
    FXLabel* label = new FXLabel(parent, "Scale: Min -> Max", nullptr, GUIDesignLabelCenterThick);
    // create frame for color scale
    FXHorizontalFrame* horizontalFrameColors = new FXHorizontalFrame(parent, GUIDesignAuxiliarHorizontalFrame);
    for (const auto& color : GNEViewNetHelper::getRainbowScaledColors()) {
        FXLabel* colorLabel = new FXLabel(horizontalFrameColors, "", nullptr, GUIDesignLabelLeft);
        colorLabel->setBackColor(MFXUtils::getFXColor(color));
    }
    // return label
    return label;
}

/****************************************************************************/
