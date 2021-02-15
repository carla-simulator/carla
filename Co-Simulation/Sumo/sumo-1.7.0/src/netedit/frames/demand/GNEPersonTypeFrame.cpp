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
/// @file    GNEPersonTypeFrame.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Jun 2019
///
// The Widget for edit person type (pTypes with vclass='pedestrian) elements
/****************************************************************************/
#include <config.h>

#include <utils/gui/windows/GUIAppEnum.h>
#include <utils/gui/div/GUIDesigns.h>
#include <netedit/changes/GNEChange_DemandElement.h>
#include <netedit/elements/demand/GNEVehicleType.h>
#include <netedit/GNENet.h>
#include <netedit/GNEViewNet.h>
#include <netedit/GNEUndoList.h>

#include "GNEPersonTypeFrame.h"


// ===========================================================================
// FOX callback mapping
// ===========================================================================

FXDEFMAP(GNEPersonTypeFrame::PersonTypeSelector) personTypeSelectorMap[] = {
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_SET_TYPE,   GNEPersonTypeFrame::PersonTypeSelector::onCmdSelectItem)
};

FXDEFMAP(GNEPersonTypeFrame::PersonTypeEditor) personTypeEditorMap[] = {
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_CREATE,    GNEPersonTypeFrame::PersonTypeEditor::onCmdCreatePersonType),
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_DELETE,    GNEPersonTypeFrame::PersonTypeEditor::onCmdDeletePersonType),
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_RESET,     GNEPersonTypeFrame::PersonTypeEditor::onCmdResetPersonType),
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_COPY,      GNEPersonTypeFrame::PersonTypeEditor::onCmdCopyPersonType)
};

// Object implementation
FXIMPLEMENT(GNEPersonTypeFrame::PersonTypeSelector,   FXGroupBox,     personTypeSelectorMap,     ARRAYNUMBER(personTypeSelectorMap))
FXIMPLEMENT(GNEPersonTypeFrame::PersonTypeEditor,     FXGroupBox,     personTypeEditorMap,       ARRAYNUMBER(personTypeEditorMap))

// ===========================================================================
// method definitions
// ===========================================================================

// ---------------------------------------------------------------------------
// GNEPersonTypeFrame::PersonTypeSelector - methods
// ---------------------------------------------------------------------------

GNEPersonTypeFrame::PersonTypeSelector::PersonTypeSelector(GNEPersonTypeFrame* personTypeFrameParent) :
    FXGroupBox(personTypeFrameParent->myContentFrame, "Current Person Type", GUIDesignGroupBoxFrame),
    myPersonTypeFrameParent(personTypeFrameParent),
    myCurrentPersonType(nullptr) {
    // Create FXComboBox
    myTypeMatchBox = new FXComboBox(this, GUIDesignComboBoxNCol, this, MID_GNE_SET_TYPE, GUIDesignComboBox);
    // fill myTypeMatchBox with list of pTypes IDs
    for (const auto& i : myPersonTypeFrameParent->getViewNet()->getNet()->getAttributeCarriers()->getDemandElements().at(SUMO_TAG_PTYPE)) {
        myTypeMatchBox->appendItem(i.first.c_str());
    }
    // set DEFAULT_PEDTYPE_ID as default pType
    myCurrentPersonType = myPersonTypeFrameParent->getViewNet()->getNet()->retrieveDemandElement(SUMO_TAG_PTYPE, DEFAULT_PEDTYPE_ID);
    // Set visible items
    if (myTypeMatchBox->getNumItems() <= 20) {
        myTypeMatchBox->setNumVisible((int)myTypeMatchBox->getNumItems());
    } else {
        myTypeMatchBox->setNumVisible(20);
    }
    // PersonTypeSelector is always shown
    show();
}


GNEPersonTypeFrame::PersonTypeSelector::~PersonTypeSelector() {}


GNEDemandElement*
GNEPersonTypeFrame::PersonTypeSelector::getCurrentPersonType() const {
    return myCurrentPersonType;
}


void
GNEPersonTypeFrame::PersonTypeSelector::setCurrentPersonType(GNEDemandElement* vType) {
    myCurrentPersonType = vType;
    refreshPersonTypeSelector();
}


void
GNEPersonTypeFrame::PersonTypeSelector::refreshPersonTypeSelector() {
    bool valid = false;
    myTypeMatchBox->clearItems();
    // fill myTypeMatchBox with list of pTypes IDs
    for (const auto& i : myPersonTypeFrameParent->getViewNet()->getNet()->getAttributeCarriers()->getDemandElements().at(SUMO_TAG_PTYPE)) {
        myTypeMatchBox->appendItem(i.first.c_str());
    }
    // Set visible items
    if (myTypeMatchBox->getNumItems() <= 20) {
        myTypeMatchBox->setNumVisible((int)myTypeMatchBox->getNumItems());
    } else {
        myTypeMatchBox->setNumVisible(20);
    }
    // make sure that tag is in myTypeMatchBox
    for (int i = 0; i < (int)myTypeMatchBox->getNumItems(); i++) {
        if (myTypeMatchBox->getItem(i).text() == myCurrentPersonType->getID()) {
            myTypeMatchBox->setCurrentItem(i);
            valid = true;
        }
    }
    // Check that give vType type is valid
    if (!valid) {
        // set DEFAULT_VEHTYPE as default pType
        myCurrentPersonType = myPersonTypeFrameParent->getViewNet()->getNet()->retrieveDemandElement(SUMO_TAG_PTYPE, DEFAULT_VTYPE_ID);
        // refresh myTypeMatchBox again
        for (int i = 0; i < (int)myTypeMatchBox->getNumItems(); i++) {
            if (myTypeMatchBox->getItem(i).text() == myCurrentPersonType->getID()) {
                myTypeMatchBox->setCurrentItem(i);
            }
        }
    }
    // refresh person type editor modul
    myPersonTypeFrameParent->myPersonTypeEditor->refreshPersonTypeEditorModul();
    // show Attribute Editor modul if selected item is valid
    myPersonTypeFrameParent->myPersonTypeAttributesEditor->showAttributeEditorModul({myCurrentPersonType}, false, true);
}


long
GNEPersonTypeFrame::PersonTypeSelector::onCmdSelectItem(FXObject*, FXSelector, void*) {
    // Check if value of myTypeMatchBox correspond of an allowed additional tags
    for (const auto& i : myPersonTypeFrameParent->getViewNet()->getNet()->getAttributeCarriers()->getDemandElements().at(SUMO_TAG_PTYPE)) {
        if (i.first == myTypeMatchBox->getText().text()) {
            // set pointer
            myCurrentPersonType = i.second;
            // set color of myTypeMatchBox to black (valid)
            myTypeMatchBox->setTextColor(FXRGB(0, 0, 0));
            // refresh person type editor modul
            myPersonTypeFrameParent->myPersonTypeEditor->refreshPersonTypeEditorModul();
            // show moduls if selected item is valid
            myPersonTypeFrameParent->myPersonTypeAttributesEditor->showAttributeEditorModul({myCurrentPersonType}, false, true);
            // Write Warning in console if we're in testing mode
            WRITE_DEBUG(("Selected item '" + myTypeMatchBox->getText() + "' in PersonTypeSelector").text());
            return 1;
        }
    }
    myCurrentPersonType = nullptr;
    // refresh person type editor modul
    myPersonTypeFrameParent->myPersonTypeEditor->refreshPersonTypeEditorModul();
    // hide all moduls if selected item isn't valid
    myPersonTypeFrameParent->myPersonTypeAttributesEditor->hideAttributesEditorModul();
    // set color of myTypeMatchBox to red (invalid)
    myTypeMatchBox->setTextColor(FXRGB(255, 0, 0));
    // Write Warning in console if we're in testing mode
    WRITE_DEBUG("Selected invalid item in PersonTypeSelector");
    return 1;
}

// ---------------------------------------------------------------------------
// GNEPersonTypeFrame::PersonTypeEditor - methods
// ---------------------------------------------------------------------------

GNEPersonTypeFrame::PersonTypeEditor::PersonTypeEditor(GNEPersonTypeFrame* personTypeFrameParent) :
    FXGroupBox(personTypeFrameParent->myContentFrame, "Person Type Editor", GUIDesignGroupBoxFrame),
    myPersonTypeFrameParent(personTypeFrameParent) {
    // Create new person type
    myCreatePersonTypeButton = new FXButton(this, "Create Person Type", nullptr, this, MID_GNE_CREATE, GUIDesignButton);
    // Create delete person type
    myDeletePersonTypeButton = new FXButton(this, "Delete Person Type", nullptr, this, MID_GNE_DELETE, GUIDesignButton);
    // Create reset person type
    myResetDefaultPersonTypeButton = new FXButton(this, "Reset default Person Type", nullptr, this, MID_GNE_RESET, GUIDesignButton);
    // Create copy person type
    myCopyPersonTypeButton = new FXButton(this, "Copy Person Type", nullptr, this, MID_GNE_COPY, GUIDesignButton);
}


GNEPersonTypeFrame::PersonTypeEditor::~PersonTypeEditor() {}


void
GNEPersonTypeFrame::PersonTypeEditor::showPersonTypeEditorModul() {
    refreshPersonTypeEditorModul();
    show();
}


void
GNEPersonTypeFrame::PersonTypeEditor::hidePersonTypeEditorModul() {
    hide();
}


void
GNEPersonTypeFrame::PersonTypeEditor::refreshPersonTypeEditorModul() {
    // first check if selected pType is valid
    if (myPersonTypeFrameParent->myPersonTypeSelector->getCurrentPersonType() == nullptr) {
        // disable all buttons except create button
        myDeletePersonTypeButton->disable();
        myResetDefaultPersonTypeButton->disable();
        myCopyPersonTypeButton->disable();
    } else if (GNEAttributeCarrier::parse<bool>(myPersonTypeFrameParent->myPersonTypeSelector->getCurrentPersonType()->getAttribute(GNE_ATTR_DEFAULT_VTYPE))) {
        // enable copy button
        myCopyPersonTypeButton->enable();
        // hide delete person type buttond and show reset default person type button
        myDeletePersonTypeButton->hide();
        myResetDefaultPersonTypeButton->show();
        // check if reset default person type button has to be enabled or disabled
        if (GNEAttributeCarrier::parse<bool>(myPersonTypeFrameParent->myPersonTypeSelector->getCurrentPersonType()->getAttribute(GNE_ATTR_DEFAULT_VTYPE_MODIFIED))) {
            myResetDefaultPersonTypeButton->enable();
        } else {
            myResetDefaultPersonTypeButton->disable();
        }
    } else {
        // enable copy button
        myCopyPersonTypeButton->enable();
        // show delete person type button and hide reset default person type button
        myDeletePersonTypeButton->show();
        myDeletePersonTypeButton->enable();
        myResetDefaultPersonTypeButton->hide();
    }
    // update modul
    recalc();
}


long
GNEPersonTypeFrame::PersonTypeEditor::onCmdCreatePersonType(FXObject*, FXSelector, void*) {
    // obtain a new valid Vehicle Type ID
    std::string personTypeID = myPersonTypeFrameParent->myViewNet->getNet()->generateDemandElementID(SUMO_TAG_PTYPE);
    // create new person type
    GNEDemandElement* personType = new GNEVehicleType(myPersonTypeFrameParent->myViewNet->getNet(), personTypeID, SUMO_TAG_PTYPE);
    // add it using undoList (to allow undo-redo)
    myPersonTypeFrameParent->myViewNet->getUndoList()->p_begin("create person type");
    myPersonTypeFrameParent->myViewNet->getUndoList()->add(new GNEChange_DemandElement(personType, true), true);
    myPersonTypeFrameParent->myViewNet->getUndoList()->p_end();
    // set created person type in selector
    myPersonTypeFrameParent->myPersonTypeSelector->setCurrentPersonType(personType);
    // refresh PersonType Editor Modul
    myPersonTypeFrameParent->myPersonTypeEditor->refreshPersonTypeEditorModul();
    return 1;
}


long
GNEPersonTypeFrame::PersonTypeEditor::onCmdDeletePersonType(FXObject*, FXSelector, void*) {
    // show question dialog if vtype has already assigned persons
    if (myPersonTypeFrameParent->myPersonTypeSelector->getCurrentPersonType()->getChildDemandElements().size() > 0) {
        std::string plural = myPersonTypeFrameParent->myPersonTypeSelector->getCurrentPersonType()->getChildDemandElements().size() == 1 ? ("") : ("s");
        // show warning in gui testing debug mode
        WRITE_DEBUG("Opening FXMessageBox 'remove vType'");
        // Ask confirmation to user
        FXuint answer = FXMessageBox::question(getApp(), MBOX_YES_NO,
                                               ("Remove " + toString(SUMO_TAG_PTYPE) + "s").c_str(), "%s",
                                               ("Delete " + toString(SUMO_TAG_PTYPE) + " '" + myPersonTypeFrameParent->myPersonTypeSelector->getCurrentPersonType()->getID() +
                                                "' will remove " + toString(myPersonTypeFrameParent->myPersonTypeSelector->getCurrentPersonType()->getChildDemandElements().size()) +
                                                " person" + plural + ". Continue?").c_str());
        if (answer != 1) { // 1:yes, 2:no, 4:esc
            // write warning if netedit is running in testing mode
            if (answer == 2) {
                WRITE_DEBUG("Closed FXMessageBox 'remove vType' with 'No'");
            } else if (answer == 4) {
                WRITE_DEBUG("Closed FXMessageBox 'remove vType' with 'ESC'");
            }
        } else {
            // begin undo list operation
            myPersonTypeFrameParent->myViewNet->getUndoList()->p_begin("delete person type");
            // remove person type (and all of their children)
            myPersonTypeFrameParent->myViewNet->getNet()->deleteDemandElement(myPersonTypeFrameParent->myPersonTypeSelector->getCurrentPersonType(),
                    myPersonTypeFrameParent->myViewNet->getUndoList());
            // end undo list operation
            myPersonTypeFrameParent->myViewNet->getUndoList()->p_end();
        }
    } else {
        // begin undo list operation
        myPersonTypeFrameParent->myViewNet->getUndoList()->p_begin("delete person type");
        // remove person type (and all of their children)
        myPersonTypeFrameParent->myViewNet->getNet()->deleteDemandElement(myPersonTypeFrameParent->myPersonTypeSelector->getCurrentPersonType(),
                myPersonTypeFrameParent->myViewNet->getUndoList());
        // end undo list operation
        myPersonTypeFrameParent->myViewNet->getUndoList()->p_end();
    }
    return 1;
}


long
GNEPersonTypeFrame::PersonTypeEditor::onCmdResetPersonType(FXObject*, FXSelector, void*) {
    // begin reset default person type values
    myPersonTypeFrameParent->getViewNet()->getUndoList()->p_begin("reset default person type values");
    // reset all values of default person type
    for (const auto& i : GNEAttributeCarrier::getTagProperties(SUMO_TAG_PTYPE)) {
        // change all attributes with "" to reset it (except ID and vClass)
        if ((i.getAttr() != SUMO_ATTR_ID) && (i.getAttr() != SUMO_ATTR_VCLASS)) {
            myPersonTypeFrameParent->myPersonTypeSelector->getCurrentPersonType()->setAttribute(i.getAttr(), "", myPersonTypeFrameParent->myViewNet->getUndoList());
        }
    }
    // change manually VClass (because it depends of Default pType)
    if (myPersonTypeFrameParent->myPersonTypeSelector->getCurrentPersonType()->getAttribute(SUMO_ATTR_ID) == DEFAULT_PEDTYPE_ID) {
        myPersonTypeFrameParent->myPersonTypeSelector->getCurrentPersonType()->setAttribute(SUMO_ATTR_VCLASS, toString(SVC_PEDESTRIAN), myPersonTypeFrameParent->myViewNet->getUndoList());
    }
    // change special attribute GNE_ATTR_DEFAULT_VTYPE_MODIFIED
    myPersonTypeFrameParent->myPersonTypeSelector->getCurrentPersonType()->setAttribute(GNE_ATTR_DEFAULT_VTYPE_MODIFIED, "false", myPersonTypeFrameParent->myViewNet->getUndoList());
    // finish reset default person type values
    myPersonTypeFrameParent->getViewNet()->getUndoList()->p_end();
    // refresh PersonTypeSelector
    myPersonTypeFrameParent->myPersonTypeSelector->refreshPersonTypeSelector();
    return 1;
}


long
GNEPersonTypeFrame::PersonTypeEditor::onCmdCopyPersonType(FXObject*, FXSelector, void*) {
    // obtain a new valid person Type ID
    std::string personTypeID = myPersonTypeFrameParent->myViewNet->getNet()->generateDemandElementID(SUMO_TAG_PTYPE);
    // obtain person type in which new person Type will be based
    GNEVehicleType* vType = dynamic_cast<GNEVehicleType*>(myPersonTypeFrameParent->myPersonTypeSelector->getCurrentPersonType());
    // check that vType exist
    if (vType) {
        // create a new person Type based on the current selected person type
        GNEDemandElement* personTypeCopy = new GNEVehicleType(myPersonTypeFrameParent->myViewNet->getNet(), personTypeID, vType);
        // begin undo list operation
        myPersonTypeFrameParent->myViewNet->getUndoList()->p_begin("copy person type");
        // add it using undoList (to allow undo-redo)
        myPersonTypeFrameParent->myViewNet->getUndoList()->add(new GNEChange_DemandElement(personTypeCopy, true), true);
        // end undo list operation
        myPersonTypeFrameParent->myViewNet->getUndoList()->p_end();
        // refresh Vehicle Type Selector (to show the new pType)
        myPersonTypeFrameParent->myPersonTypeSelector->refreshPersonTypeSelector();
        // set created person type in selector
        myPersonTypeFrameParent->myPersonTypeSelector->setCurrentPersonType(personTypeCopy);
        // refresh PersonType Editor Modul
        myPersonTypeFrameParent->myPersonTypeEditor->refreshPersonTypeEditorModul();
    }
    return 1;
}

// ---------------------------------------------------------------------------
// GNEPersonTypeFrame - methods
// ---------------------------------------------------------------------------

GNEPersonTypeFrame::GNEPersonTypeFrame(FXHorizontalFrame* horizontalFrameParent, GNEViewNet* viewNet) :
    GNEFrame(horizontalFrameParent, viewNet, "Person Types") {

    // create modul for edit person types (Create, copy, etc.)
    myPersonTypeEditor = new PersonTypeEditor(this);

    // create person type selector
    myPersonTypeSelector = new PersonTypeSelector(this);

    // Create person type attributes editor
    myPersonTypeAttributesEditor = new GNEFrameAttributesModuls::AttributesEditor(this);

    // set "VTYPE_DEFAULT" as default person Type
    myPersonTypeSelector->setCurrentPersonType(myViewNet->getNet()->retrieveDemandElement(SUMO_TAG_PTYPE, DEFAULT_PEDTYPE_ID));
}


GNEPersonTypeFrame::~GNEPersonTypeFrame() {}


void
GNEPersonTypeFrame::show() {
    // refresh person type and Attribute Editor
    myPersonTypeSelector->refreshPersonTypeSelector();
    // show person type attributes editor (except extended attributes)
    myPersonTypeAttributesEditor->showAttributeEditorModul({myPersonTypeSelector->getCurrentPersonType()}, false, true);
    // show frame
    GNEFrame::show();
}


GNEPersonTypeFrame::PersonTypeSelector*
GNEPersonTypeFrame::getPersonTypeSelector() const {
    return myPersonTypeSelector;
}


void
GNEPersonTypeFrame::attributeUpdated() {
    myPersonTypeSelector->refreshPersonTypeSelector();
}


/****************************************************************************/
