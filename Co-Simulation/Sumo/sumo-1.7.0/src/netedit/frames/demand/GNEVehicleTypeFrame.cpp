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
/// @file    GNEVehicleTypeFrame.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Feb 2018
///
// The Widget for edit Vehicle Type elements
/****************************************************************************/
#include <config.h>

#include <netedit/GNENet.h>
#include <netedit/GNEUndoList.h>
#include <netedit/GNEViewNet.h>
#include <netedit/changes/GNEChange_DemandElement.h>
#include <netedit/elements/demand/GNEVehicleType.h>
#include <netedit/dialogs/GNEVehicleTypeDialog.h>
#include <utils/gui/div/GUIDesigns.h>
#include <utils/gui/windows/GUIAppEnum.h>

#include "GNEVehicleTypeFrame.h"


// ===========================================================================
// FOX callback mapping
// ===========================================================================

FXDEFMAP(GNEVehicleTypeFrame::VehicleTypeSelector) vehicleTypeSelectorMap[] = {
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_SET_TYPE,   GNEVehicleTypeFrame::VehicleTypeSelector::onCmdSelectItem)
};

FXDEFMAP(GNEVehicleTypeFrame::VehicleTypeEditor) vehicleTypeEditorMap[] = {
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_CREATE,    GNEVehicleTypeFrame::VehicleTypeEditor::onCmdCreateVehicleType),
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_DELETE,    GNEVehicleTypeFrame::VehicleTypeEditor::onCmdDeleteVehicleType),
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_RESET,     GNEVehicleTypeFrame::VehicleTypeEditor::onCmdResetVehicleType),
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_COPY,      GNEVehicleTypeFrame::VehicleTypeEditor::onCmdCopyVehicleType)
};

// Object implementation
FXIMPLEMENT(GNEVehicleTypeFrame::VehicleTypeSelector,   FXGroupBox,     vehicleTypeSelectorMap,     ARRAYNUMBER(vehicleTypeSelectorMap))
FXIMPLEMENT(GNEVehicleTypeFrame::VehicleTypeEditor,     FXGroupBox,     vehicleTypeEditorMap,       ARRAYNUMBER(vehicleTypeEditorMap))

// ===========================================================================
// method definitions
// ===========================================================================

// ---------------------------------------------------------------------------
// GNEVehicleTypeFrame::VehicleTypeSelector - methods
// ---------------------------------------------------------------------------

GNEVehicleTypeFrame::VehicleTypeSelector::VehicleTypeSelector(GNEVehicleTypeFrame* vehicleTypeFrameParent) :
    FXGroupBox(vehicleTypeFrameParent->myContentFrame, "Current Vehicle Type", GUIDesignGroupBoxFrame),
    myVehicleTypeFrameParent(vehicleTypeFrameParent),
    myCurrentVehicleType(nullptr) {
    // Create FXComboBox
    myTypeMatchBox = new FXComboBox(this, GUIDesignComboBoxNCol, this, MID_GNE_SET_TYPE, GUIDesignComboBox);
    // add default Vehicle an Bike types in the first and second positions
    myTypeMatchBox->appendItem(DEFAULT_VTYPE_ID.c_str());
    myTypeMatchBox->appendItem(DEFAULT_BIKETYPE_ID.c_str());
    // fill myTypeMatchBox with list of VTypes IDs
    for (const auto& i : myVehicleTypeFrameParent->getViewNet()->getNet()->getAttributeCarriers()->getDemandElements().at(SUMO_TAG_VTYPE)) {
        if ((i.first != DEFAULT_VTYPE_ID) && (i.first != DEFAULT_BIKETYPE_ID)) {
            myTypeMatchBox->appendItem(i.first.c_str());
        }
    }
    // set DEFAULT_VEHTYPE as default VType
    myCurrentVehicleType = myVehicleTypeFrameParent->getViewNet()->getNet()->retrieveDemandElement(SUMO_TAG_VTYPE, DEFAULT_VTYPE_ID);
    // Set visible items
    if (myTypeMatchBox->getNumItems() <= 20) {
        myTypeMatchBox->setNumVisible((int)myTypeMatchBox->getNumItems());
    } else {
        myTypeMatchBox->setNumVisible(20);
    }
    // VehicleTypeSelector is always shown
    show();
}


GNEVehicleTypeFrame::VehicleTypeSelector::~VehicleTypeSelector() {}


GNEDemandElement*
GNEVehicleTypeFrame::VehicleTypeSelector::getCurrentVehicleType() const {
    return myCurrentVehicleType;
}


void
GNEVehicleTypeFrame::VehicleTypeSelector::setCurrentVehicleType(GNEDemandElement* vType) {
    myCurrentVehicleType = vType;
    refreshVehicleTypeSelector();
}


void
GNEVehicleTypeFrame::VehicleTypeSelector::refreshVehicleTypeSelector() {
    bool valid = false;
    // clear items
    myTypeMatchBox->clearItems();
    // add default Vehicle an Bike types in the first and second positions
    myTypeMatchBox->appendItem(DEFAULT_VTYPE_ID.c_str());
    myTypeMatchBox->appendItem(DEFAULT_BIKETYPE_ID.c_str());
    // fill myTypeMatchBox with list of VTypes IDs
    for (const auto& i : myVehicleTypeFrameParent->getViewNet()->getNet()->getAttributeCarriers()->getDemandElements().at(SUMO_TAG_VTYPE)) {
        if ((i.first != DEFAULT_VTYPE_ID) && (i.first != DEFAULT_BIKETYPE_ID)) {
            myTypeMatchBox->appendItem(i.first.c_str());
        }
    }
    // Set visible items
    if (myTypeMatchBox->getNumItems() <= 20) {
        myTypeMatchBox->setNumVisible((int)myTypeMatchBox->getNumItems());
    } else {
        myTypeMatchBox->setNumVisible(20);
    }
    // make sure that tag is in myTypeMatchBox
    for (int i = 0; i < (int)myTypeMatchBox->getNumItems(); i++) {
        if (myTypeMatchBox->getItem(i).text() == myCurrentVehicleType->getID()) {
            myTypeMatchBox->setCurrentItem(i);
            valid = true;
        }
    }
    // Check that give vType type is valid
    if (!valid) {
        // set DEFAULT_VEHTYPE as default VType
        myCurrentVehicleType = myVehicleTypeFrameParent->getViewNet()->getNet()->retrieveDemandElement(SUMO_TAG_VTYPE, DEFAULT_VTYPE_ID);
        // refresh myTypeMatchBox again
        for (int i = 0; i < (int)myTypeMatchBox->getNumItems(); i++) {
            if (myTypeMatchBox->getItem(i).text() == myCurrentVehicleType->getID()) {
                myTypeMatchBox->setCurrentItem(i);
            }
        }
    }
    // refresh vehicle type editor modul
    myVehicleTypeFrameParent->myVehicleTypeEditor->refreshVehicleTypeEditorModul();
    // show Attribute Editor modul if selected item is valid
    myVehicleTypeFrameParent->myVehicleTypeAttributesEditor->showAttributeEditorModul({myCurrentVehicleType}, false, true);
}


void
GNEVehicleTypeFrame::VehicleTypeSelector::refreshVehicleTypeSelectorIDs() {
    if (myCurrentVehicleType) {
        myTypeMatchBox->setItem(myTypeMatchBox->getCurrentItem(), myCurrentVehicleType->getID().c_str());
    }
}


long
GNEVehicleTypeFrame::VehicleTypeSelector::onCmdSelectItem(FXObject*, FXSelector, void*) {
    // Check if value of myTypeMatchBox correspond of an allowed additional tags
    for (const auto& i : myVehicleTypeFrameParent->getViewNet()->getNet()->getAttributeCarriers()->getDemandElements().at(SUMO_TAG_VTYPE)) {
        if (i.first == myTypeMatchBox->getText().text()) {
            // set pointer
            myCurrentVehicleType = i.second;
            // set color of myTypeMatchBox to black (valid)
            myTypeMatchBox->setTextColor(FXRGB(0, 0, 0));
            // refresh vehicle type editor modul
            myVehicleTypeFrameParent->myVehicleTypeEditor->refreshVehicleTypeEditorModul();
            // show moduls if selected item is valid
            myVehicleTypeFrameParent->myVehicleTypeAttributesEditor->showAttributeEditorModul({myCurrentVehicleType}, false, true);
            // Write Warning in console if we're in testing mode
            WRITE_DEBUG(("Selected item '" + myTypeMatchBox->getText() + "' in VehicleTypeSelector").text());
            return 1;
        }
    }
    myCurrentVehicleType = nullptr;
    // refresh vehicle type editor modul
    myVehicleTypeFrameParent->myVehicleTypeEditor->refreshVehicleTypeEditorModul();
    // hide all moduls if selected item isn't valid
    myVehicleTypeFrameParent->myVehicleTypeAttributesEditor->hideAttributesEditorModul();
    // set color of myTypeMatchBox to red (invalid)
    myTypeMatchBox->setTextColor(FXRGB(255, 0, 0));
    // Write Warning in console if we're in testing mode
    WRITE_DEBUG("Selected invalid item in VehicleTypeSelector");
    return 1;
}

// ---------------------------------------------------------------------------
// GNEVehicleTypeFrame::VehicleTypeEditor - methods
// ---------------------------------------------------------------------------

GNEVehicleTypeFrame::VehicleTypeEditor::VehicleTypeEditor(GNEVehicleTypeFrame* vehicleTypeFrameParent) :
    FXGroupBox(vehicleTypeFrameParent->myContentFrame, "Vehicle Type Editor", GUIDesignGroupBoxFrame),
    myVehicleTypeFrameParent(vehicleTypeFrameParent) {
    // Create new vehicle type
    myCreateVehicleTypeButton = new FXButton(this, "Create Vehicle Type", nullptr, this, MID_GNE_CREATE, GUIDesignButton);
    // Create delete vehicle type
    myDeleteVehicleTypeButton = new FXButton(this, "Delete Vehicle Type", nullptr, this, MID_GNE_DELETE, GUIDesignButton);
    // Create reset vehicle type
    myResetDefaultVehicleTypeButton = new FXButton(this, "Reset default Vehicle Type", nullptr, this, MID_GNE_RESET, GUIDesignButton);
    // Create copy vehicle type
    myCopyVehicleTypeButton = new FXButton(this, "Copy Vehicle Type", nullptr, this, MID_GNE_COPY, GUIDesignButton);
}


GNEVehicleTypeFrame::VehicleTypeEditor::~VehicleTypeEditor() {}


void
GNEVehicleTypeFrame::VehicleTypeEditor::showVehicleTypeEditorModul() {
    refreshVehicleTypeEditorModul();
    show();
}


void
GNEVehicleTypeFrame::VehicleTypeEditor::hideVehicleTypeEditorModul() {
    hide();
}


void
GNEVehicleTypeFrame::VehicleTypeEditor::refreshVehicleTypeEditorModul() {
    // first check if selected VType is valid
    if (myVehicleTypeFrameParent->myVehicleTypeSelector->getCurrentVehicleType() == nullptr) {
        // disable all buttons except create button
        myDeleteVehicleTypeButton->disable();
        myResetDefaultVehicleTypeButton->disable();
        myCopyVehicleTypeButton->disable();
    } else if (GNEAttributeCarrier::parse<bool>(myVehicleTypeFrameParent->myVehicleTypeSelector->getCurrentVehicleType()->getAttribute(GNE_ATTR_DEFAULT_VTYPE))) {
        // enable copy button
        myCopyVehicleTypeButton->enable();
        // hide delete vehicle type buttond and show reset default vehicle type button
        myDeleteVehicleTypeButton->hide();
        myResetDefaultVehicleTypeButton->show();
        // check if reset default vehicle type button has to be enabled or disabled
        if (GNEAttributeCarrier::parse<bool>(myVehicleTypeFrameParent->myVehicleTypeSelector->getCurrentVehicleType()->getAttribute(GNE_ATTR_DEFAULT_VTYPE_MODIFIED))) {
            myResetDefaultVehicleTypeButton->enable();
        } else {
            myResetDefaultVehicleTypeButton->disable();
        }
    } else {
        // enable copy button
        myCopyVehicleTypeButton->enable();
        // show delete vehicle type button and hide reset default vehicle type button
        myDeleteVehicleTypeButton->show();
        myDeleteVehicleTypeButton->enable();
        myResetDefaultVehicleTypeButton->hide();
    }
    // update modul
    recalc();
}


long
GNEVehicleTypeFrame::VehicleTypeEditor::onCmdCreateVehicleType(FXObject*, FXSelector, void*) {
    // obtain a new valid Vehicle Type ID
    std::string vehicleTypeID = myVehicleTypeFrameParent->myViewNet->getNet()->generateDemandElementID(SUMO_TAG_VTYPE);
    // create new vehicle type
    GNEDemandElement* vehicleType = new GNEVehicleType(myVehicleTypeFrameParent->myViewNet->getNet(), vehicleTypeID, SUMO_TAG_VTYPE);
    // add it using undoList (to allow undo-redo)
    myVehicleTypeFrameParent->myViewNet->getUndoList()->p_begin("create vehicle type");
    myVehicleTypeFrameParent->myViewNet->getUndoList()->add(new GNEChange_DemandElement(vehicleType, true), true);
    myVehicleTypeFrameParent->myViewNet->getUndoList()->p_end();
    // set created vehicle type in selector
    myVehicleTypeFrameParent->myVehicleTypeSelector->setCurrentVehicleType(vehicleType);
    // refresh VehicleType Editor Modul
    myVehicleTypeFrameParent->myVehicleTypeEditor->refreshVehicleTypeEditorModul();
    return 1;
}


long
GNEVehicleTypeFrame::VehicleTypeEditor::onCmdDeleteVehicleType(FXObject*, FXSelector, void*) {
    // show question dialog if vtype has already assigned vehicles
    if (myVehicleTypeFrameParent->myVehicleTypeSelector->getCurrentVehicleType()->getChildDemandElements().size() > 0) {
        std::string plural = myVehicleTypeFrameParent->myVehicleTypeSelector->getCurrentVehicleType()->getChildDemandElements().size() == 1 ? ("") : ("s");
        // show warning in gui testing debug mode
        WRITE_DEBUG("Opening FXMessageBox 'remove vType'");
        // Ask confirmation to user
        FXuint answer = FXMessageBox::question(getApp(), MBOX_YES_NO,
                                               ("Remove " + toString(SUMO_TAG_VTYPE) + "s").c_str(), "%s",
                                               ("Delete " + toString(SUMO_TAG_VTYPE) + " '" + myVehicleTypeFrameParent->myVehicleTypeSelector->getCurrentVehicleType()->getID() +
                                                "' will remove " + toString(myVehicleTypeFrameParent->myVehicleTypeSelector->getCurrentVehicleType()->getChildDemandElements().size()) +
                                                " vehicle" + plural + ". Continue?").c_str());
        if (answer != 1) { // 1:yes, 2:no, 4:esc
            // write warning if netedit is running in testing mode
            if (answer == 2) {
                WRITE_DEBUG("Closed FXMessageBox 'remove vType' with 'No'");
            } else if (answer == 4) {
                WRITE_DEBUG("Closed FXMessageBox 'remove vType' with 'ESC'");
            }
        } else {
            // begin undo list operation
            myVehicleTypeFrameParent->myViewNet->getUndoList()->p_begin("delete vehicle type");
            // remove vehicle type (and all of their children)
            myVehicleTypeFrameParent->myViewNet->getNet()->deleteDemandElement(myVehicleTypeFrameParent->myVehicleTypeSelector->getCurrentVehicleType(),
                    myVehicleTypeFrameParent->myViewNet->getUndoList());
            // end undo list operation
            myVehicleTypeFrameParent->myViewNet->getUndoList()->p_end();
        }
    } else {
        // begin undo list operation
        myVehicleTypeFrameParent->myViewNet->getUndoList()->p_begin("delete vehicle type");
        // remove vehicle type (and all of their children)
        myVehicleTypeFrameParent->myViewNet->getNet()->deleteDemandElement(myVehicleTypeFrameParent->myVehicleTypeSelector->getCurrentVehicleType(),
                myVehicleTypeFrameParent->myViewNet->getUndoList());
        // end undo list operation
        myVehicleTypeFrameParent->myViewNet->getUndoList()->p_end();
    }
    return 1;
}


long
GNEVehicleTypeFrame::VehicleTypeEditor::onCmdResetVehicleType(FXObject*, FXSelector, void*) {
    // begin reset default vehicle type values
    myVehicleTypeFrameParent->getViewNet()->getUndoList()->p_begin("reset default vehicle type values");
    // reset all values of default vehicle type
    for (const auto& i : GNEAttributeCarrier::getTagProperties(SUMO_TAG_VTYPE)) {
        // change all attributes with "" to reset it (except ID and vClass)
        if ((i.getAttr() != SUMO_ATTR_ID) && (i.getAttr() != SUMO_ATTR_VCLASS)) {
            myVehicleTypeFrameParent->myVehicleTypeSelector->getCurrentVehicleType()->setAttribute(i.getAttr(), "", myVehicleTypeFrameParent->myViewNet->getUndoList());
        }
    }
    // change manually VClass (because it depends of Default VType)
    if (myVehicleTypeFrameParent->myVehicleTypeSelector->getCurrentVehicleType()->getAttribute(SUMO_ATTR_ID) == DEFAULT_VTYPE_ID) {
        myVehicleTypeFrameParent->myVehicleTypeSelector->getCurrentVehicleType()->setAttribute(SUMO_ATTR_VCLASS, toString(SVC_PASSENGER), myVehicleTypeFrameParent->myViewNet->getUndoList());
    } else if (myVehicleTypeFrameParent->myVehicleTypeSelector->getCurrentVehicleType()->getAttribute(SUMO_ATTR_ID) == DEFAULT_BIKETYPE_ID) {
        myVehicleTypeFrameParent->myVehicleTypeSelector->getCurrentVehicleType()->setAttribute(SUMO_ATTR_VCLASS, toString(SVC_BICYCLE), myVehicleTypeFrameParent->myViewNet->getUndoList());
    }
    // change special attribute GNE_ATTR_DEFAULT_VTYPE_MODIFIED
    myVehicleTypeFrameParent->myVehicleTypeSelector->getCurrentVehicleType()->setAttribute(GNE_ATTR_DEFAULT_VTYPE_MODIFIED, "false", myVehicleTypeFrameParent->myViewNet->getUndoList());
    // finish reset default vehicle type values
    myVehicleTypeFrameParent->getViewNet()->getUndoList()->p_end();
    // refresh VehicleTypeSelector
    myVehicleTypeFrameParent->myVehicleTypeSelector->refreshVehicleTypeSelector();
    return 1;
}


long
GNEVehicleTypeFrame::VehicleTypeEditor::onCmdCopyVehicleType(FXObject*, FXSelector, void*) {
    // obtain a new valid Vehicle Type ID
    std::string vehicleTypeID = myVehicleTypeFrameParent->myViewNet->getNet()->generateDemandElementID(SUMO_TAG_VTYPE);
    // obtain vehicle type in which new Vehicle Type will be based
    GNEVehicleType* vType = dynamic_cast<GNEVehicleType*>(myVehicleTypeFrameParent->myVehicleTypeSelector->getCurrentVehicleType());
    // check that vType exist
    if (vType) {
        // create a new Vehicle Type based on the current selected vehicle type
        GNEDemandElement* vehicleTypeCopy = new GNEVehicleType(myVehicleTypeFrameParent->myViewNet->getNet(), vehicleTypeID, vType);
        // begin undo list operation
        myVehicleTypeFrameParent->myViewNet->getUndoList()->p_begin("copy vehicle type");
        // add it using undoList (to allow undo-redo)
        myVehicleTypeFrameParent->myViewNet->getUndoList()->add(new GNEChange_DemandElement(vehicleTypeCopy, true), true);
        // end undo list operation
        myVehicleTypeFrameParent->myViewNet->getUndoList()->p_end();
        // refresh Vehicle Type Selector (to show the new VType)
        myVehicleTypeFrameParent->myVehicleTypeSelector->refreshVehicleTypeSelector();
        // set created vehicle type in selector
        myVehicleTypeFrameParent->myVehicleTypeSelector->setCurrentVehicleType(vehicleTypeCopy);
        // refresh VehicleType Editor Modul
        myVehicleTypeFrameParent->myVehicleTypeEditor->refreshVehicleTypeEditorModul();
    }
    return 1;
}

// ---------------------------------------------------------------------------
// GNEVehicleTypeFrame - methods
// ---------------------------------------------------------------------------

GNEVehicleTypeFrame::GNEVehicleTypeFrame(FXHorizontalFrame* horizontalFrameParent, GNEViewNet* viewNet) :
    GNEFrame(horizontalFrameParent, viewNet, "Vehicle Types") {

    // create modul for edit vehicle types (Create, copy, etc.)
    myVehicleTypeEditor = new VehicleTypeEditor(this);

    // create vehicle type selector
    myVehicleTypeSelector = new VehicleTypeSelector(this);

    // Create vehicle type attributes editor
    myVehicleTypeAttributesEditor = new GNEFrameAttributesModuls::AttributesEditor(this);

    // create modul for open extended attributes dialog
    myAttributesEditorExtended = new GNEFrameAttributesModuls::AttributesEditorExtended(this);

    // set "VTYPE_DEFAULT" as default vehicle Type
    myVehicleTypeSelector->setCurrentVehicleType(myViewNet->getNet()->retrieveDemandElement(SUMO_TAG_VTYPE, DEFAULT_VTYPE_ID));
}


GNEVehicleTypeFrame::~GNEVehicleTypeFrame() {}


void
GNEVehicleTypeFrame::show() {
    // refresh vehicle type and Attribute Editor
    myVehicleTypeSelector->refreshVehicleTypeSelector();
    // show vehicle type attributes editor (except extended attributes)
    myVehicleTypeAttributesEditor->showAttributeEditorModul({myVehicleTypeSelector->getCurrentVehicleType()}, false, true);
    // show frame
    GNEFrame::show();
}


GNEVehicleTypeFrame::VehicleTypeSelector*
GNEVehicleTypeFrame::getVehicleTypeSelector() const {
    return myVehicleTypeSelector;
}


void
GNEVehicleTypeFrame::attributeUpdated() {
    // after changing an attribute myVehicleTypeSelector, we need to update the list of vehicleTypeSelector, because ID could be changed
    myVehicleTypeSelector->refreshVehicleTypeSelectorIDs();
}


void
GNEVehicleTypeFrame::attributesEditorExtendedDialogOpened() {
    // open vehicle type dialog
    if (myVehicleTypeSelector->getCurrentVehicleType()) {
        GNEVehicleTypeDialog(myVehicleTypeSelector->getCurrentVehicleType(), true);
        // call "showAttributeEditorModul" to refresh attribute list
        myVehicleTypeAttributesEditor->showAttributeEditorModul({myVehicleTypeSelector->getCurrentVehicleType()}, false, true);
    }
}


/****************************************************************************/
