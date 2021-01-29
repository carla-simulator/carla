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
/// @file    GNESelectorFrame.cpp
/// @author  Jakob Erdmann
/// @date    Mar 2011
///
// The Widget for modifying selections of network-elements
/****************************************************************************/
#include <config.h>

#include <netedit/GNENet.h>
#include <netedit/GNEUndoList.h>
#include <netedit/GNEViewNet.h>
#include <netedit/elements/additional/GNEAdditional.h>
#include <netedit/elements/additional/GNEPOI.h>
#include <netedit/elements/demand/GNERoute.h>
#include <netedit/elements/data/GNEDataSet.h>
#include <netedit/elements/data/GNEDataInterval.h>
#include <netedit/elements/data/GNEGenericData.h>
#include <netedit/elements/network/GNEConnection.h>
#include <netedit/elements/network/GNECrossing.h>
#include <netedit/elements/network/GNEEdge.h>
#include <netedit/elements/network/GNEJunction.h>
#include <netedit/elements/network/GNELane.h>
#include <utils/gui/div/GUIDesigns.h>
#include <utils/gui/globjects/GUIGlObjectStorage.h>
#include <utils/gui/windows/GUIAppEnum.h>

#include "GNESelectorFrame.h"


// ===========================================================================
// FOX callback mapping
// ===========================================================================
FXDEFMAP(GNESelectorFrame::LockGLObjectTypes::ObjectTypeEntry) ObjectTypeEntryMap[] = {
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_SET_ATTRIBUTE,  GNESelectorFrame::LockGLObjectTypes::ObjectTypeEntry::onCmdSetCheckBox)
};

FXDEFMAP(GNESelectorFrame::ModificationMode) ModificationModeMap[] = {
    FXMAPFUNC(SEL_COMMAND,  MID_CHOOSEN_OPERATION,  GNESelectorFrame::ModificationMode::onCmdSelectModificationMode)
};

FXDEFMAP(GNESelectorFrame::ElementSet) ElementSetMap[] = {
    FXMAPFUNC(SEL_COMMAND,  MID_CHOOSEN_ELEMENTS,   GNESelectorFrame::ElementSet::onCmdSelectElementSet)
};

FXDEFMAP(GNESelectorFrame::MatchAttribute) MatchAttributeMap[] = {
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_SELECTORFRAME_SELECTTAG,        GNESelectorFrame::MatchAttribute::onCmdSelMBTag),
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_SELECTORFRAME_SELECTATTRIBUTE,  GNESelectorFrame::MatchAttribute::onCmdSelMBAttribute),
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_SELECTORFRAME_PROCESSSTRING,    GNESelectorFrame::MatchAttribute::onCmdSelMBString),
    FXMAPFUNC(SEL_COMMAND,  MID_HELP,                               GNESelectorFrame::MatchAttribute::onCmdHelp)
};

FXDEFMAP(GNESelectorFrame::MatchGenericDataAttribute) MatchGenericDataAttributeMap[] = {
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_SELECTORFRAME_SETINTERVAL,      GNESelectorFrame::MatchGenericDataAttribute::onCmdSetInterval),
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_SELECTORFRAME_SETBEGIN,         GNESelectorFrame::MatchGenericDataAttribute::onCmdSetBegin),
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_SELECTORFRAME_SETEND,           GNESelectorFrame::MatchGenericDataAttribute::onCmdSetEnd),
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_SELECTORFRAME_SELECTTAG,        GNESelectorFrame::MatchGenericDataAttribute::onCmdSelectTag),
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_SELECTORFRAME_SELECTATTRIBUTE,  GNESelectorFrame::MatchGenericDataAttribute::onCmdSelectAttribute),
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_SELECTORFRAME_PROCESSSTRING,    GNESelectorFrame::MatchGenericDataAttribute::onCmdProcessString),
    FXMAPFUNC(SEL_COMMAND,  MID_HELP,                               GNESelectorFrame::MatchGenericDataAttribute::onCmdHelp)
};

FXDEFMAP(GNESelectorFrame::VisualScaling) VisualScalingMap[] = {
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_SELECTORFRAME_SELECTSCALE,      GNESelectorFrame::VisualScaling::onCmdScaleSelection)
};

FXDEFMAP(GNESelectorFrame::SelectionOperation) SelectionOperationMap[] = {
    FXMAPFUNC(SEL_COMMAND,  MID_CHOOSEN_LOAD,   GNESelectorFrame::SelectionOperation::onCmdLoad),
    FXMAPFUNC(SEL_COMMAND,  MID_CHOOSEN_SAVE,   GNESelectorFrame::SelectionOperation::onCmdSave),
    FXMAPFUNC(SEL_COMMAND,  MID_CHOOSEN_INVERT, GNESelectorFrame::SelectionOperation::onCmdInvert),
    FXMAPFUNC(SEL_COMMAND,  MID_CHOOSEN_CLEAR,  GNESelectorFrame::SelectionOperation::onCmdClear)
};

// Object implementation
FXIMPLEMENT(GNESelectorFrame::LockGLObjectTypes::ObjectTypeEntry,   FXObject,       ObjectTypeEntryMap,             ARRAYNUMBER(ObjectTypeEntryMap))
FXIMPLEMENT(GNESelectorFrame::ModificationMode,                     FXGroupBox,     ModificationModeMap,            ARRAYNUMBER(ModificationModeMap))
FXIMPLEMENT(GNESelectorFrame::ElementSet,                           FXGroupBox,     ElementSetMap,                  ARRAYNUMBER(ElementSetMap))
FXIMPLEMENT(GNESelectorFrame::MatchAttribute,                       FXGroupBox,     MatchAttributeMap,              ARRAYNUMBER(MatchAttributeMap))
FXIMPLEMENT(GNESelectorFrame::MatchGenericDataAttribute,            FXGroupBox,     MatchGenericDataAttributeMap,   ARRAYNUMBER(MatchGenericDataAttributeMap))
FXIMPLEMENT(GNESelectorFrame::VisualScaling,                        FXGroupBox,     VisualScalingMap,               ARRAYNUMBER(VisualScalingMap))
FXIMPLEMENT(GNESelectorFrame::SelectionOperation,                   FXGroupBox,     SelectionOperationMap,          ARRAYNUMBER(SelectionOperationMap))

// ===========================================================================
// method definitions
// ===========================================================================

// ---------------------------------------------------------------------------
// ModificationMode::LockGLObjectTypes - methods
// ---------------------------------------------------------------------------

GNESelectorFrame::LockGLObjectTypes::LockGLObjectTypes(GNESelectorFrame* selectorFrameParent) :
    FXGroupBox(selectorFrameParent->myContentFrame, "Locked selected items", GUIDesignGroupBoxFrame),
    mySelectorFrameParent(selectorFrameParent) {
    // create a matrix for TypeEntries
    FXMatrix* matrixLockGLObjectTypes = new FXMatrix(this, 3, GUIDesignMatrixLockGLTypes);
    // create typeEntries for Network elements
    myTypeEntries[GLO_JUNCTION] =           new ObjectTypeEntry(matrixLockGLObjectTypes, Supermode::NETWORK, "Junctions");
    myTypeEntries[GLO_EDGE] =               new ObjectTypeEntry(matrixLockGLObjectTypes, Supermode::NETWORK, "Edges");
    myTypeEntries[GLO_LANE] =               new ObjectTypeEntry(matrixLockGLObjectTypes, Supermode::NETWORK, "Lanes");
    myTypeEntries[GLO_CONNECTION] =         new ObjectTypeEntry(matrixLockGLObjectTypes, Supermode::NETWORK, "Connections");
    myTypeEntries[GLO_CROSSING] =           new ObjectTypeEntry(matrixLockGLObjectTypes, Supermode::NETWORK, "Crossings");
    myTypeEntries[GLO_ADDITIONALELEMENT] =  new ObjectTypeEntry(matrixLockGLObjectTypes, Supermode::NETWORK, "Additionals");
    myTypeEntries[GLO_TAZ] =                new ObjectTypeEntry(matrixLockGLObjectTypes, Supermode::NETWORK, "TAZs");
    myTypeEntries[GLO_POLYGON] =            new ObjectTypeEntry(matrixLockGLObjectTypes, Supermode::NETWORK, "Polygons");
    myTypeEntries[GLO_POI] =                new ObjectTypeEntry(matrixLockGLObjectTypes, Supermode::NETWORK, "POIs");
    // create typeEntries for Demand elements
    myTypeEntries[GLO_ROUTE] =              new ObjectTypeEntry(matrixLockGLObjectTypes, Supermode::DEMAND, "Routes");
    myTypeEntries[GLO_VEHICLE] =            new ObjectTypeEntry(matrixLockGLObjectTypes, Supermode::DEMAND, "Vehicles");
    myTypeEntries[GLO_ROUTEFLOW] =          new ObjectTypeEntry(matrixLockGLObjectTypes, Supermode::DEMAND, "Flows");
    myTypeEntries[GLO_TRIP] =               new ObjectTypeEntry(matrixLockGLObjectTypes, Supermode::DEMAND, "Trips");
    myTypeEntries[GLO_FLOW] =               new ObjectTypeEntry(matrixLockGLObjectTypes, Supermode::DEMAND, "Flow");
    myTypeEntries[GLO_STOP] =               new ObjectTypeEntry(matrixLockGLObjectTypes, Supermode::DEMAND, "Stops");
    myTypeEntries[GLO_PERSON] =             new ObjectTypeEntry(matrixLockGLObjectTypes, Supermode::DEMAND, "Persons");
    myTypeEntries[GLO_PERSONFLOW] =         new ObjectTypeEntry(matrixLockGLObjectTypes, Supermode::DEMAND, "PersonFlows");
    myTypeEntries[GLO_PERSONTRIP] =         new ObjectTypeEntry(matrixLockGLObjectTypes, Supermode::DEMAND, "PersonTrips");
    myTypeEntries[GLO_RIDE] =               new ObjectTypeEntry(matrixLockGLObjectTypes, Supermode::DEMAND, "Rides");
    myTypeEntries[GLO_WALK] =               new ObjectTypeEntry(matrixLockGLObjectTypes, Supermode::DEMAND, "Walks");
    myTypeEntries[GLO_PERSONSTOP] =         new ObjectTypeEntry(matrixLockGLObjectTypes, Supermode::DEMAND, "PersonStops");
    // create typeEntries for Data elements
    myTypeEntries[GLO_EDGEDATA] =           new ObjectTypeEntry(matrixLockGLObjectTypes, Supermode::DATA, "EdgeDatas");
    myTypeEntries[GLO_EDGERELDATA] =        new ObjectTypeEntry(matrixLockGLObjectTypes, Supermode::DATA, "EdgeRelDatas");
    myTypeEntries[GLO_TAZRELDATA] =        new ObjectTypeEntry(matrixLockGLObjectTypes, Supermode::DATA, "EdgeRelDatas");
}


GNESelectorFrame::LockGLObjectTypes::~LockGLObjectTypes() {
    // remove all type entries
    for (const auto& typeEntry : myTypeEntries) {
        delete typeEntry.second;
    }
}


void
GNESelectorFrame::LockGLObjectTypes::addedLockedObject(const GUIGlObjectType type) {
    if ((type >= 100) && (type < 199)) {
        myTypeEntries.at(GLO_ADDITIONALELEMENT)->counterUp();
    } else {
        myTypeEntries.at(type)->counterUp();
    }
}


void
GNESelectorFrame::LockGLObjectTypes::removeLockedObject(const GUIGlObjectType type) {
    if ((type >= 100) && (type < 199)) {
        myTypeEntries.at(GLO_ADDITIONALELEMENT)->counterDown();
    } else {
        myTypeEntries.at(type)->counterDown();
    }
}


bool
GNESelectorFrame::LockGLObjectTypes::IsObjectTypeLocked(const GUIGlObjectType type) const {
    if ((type >= 100) && (type < 199)) {
        return myTypeEntries.at(GLO_ADDITIONALELEMENT)->isGLTypeLocked();
    } else {
        return myTypeEntries.at(type)->isGLTypeLocked();
    }
}


void
GNESelectorFrame::LockGLObjectTypes::showTypeEntries() {
    for (const auto& typeEntry : myTypeEntries) {
        // show or hidde type entries depending of current supermode
        if (mySelectorFrameParent->myViewNet->getEditModes().isCurrentSupermodeNetwork() && (typeEntry.second->getSupermode() == Supermode::NETWORK)) {
            typeEntry.second->showObjectTypeEntry();
        } else if (mySelectorFrameParent->myViewNet->getEditModes().isCurrentSupermodeDemand() && (typeEntry.second->getSupermode() == Supermode::DEMAND)) {
            typeEntry.second->showObjectTypeEntry();
        } else if (mySelectorFrameParent->myViewNet->getEditModes().isCurrentSupermodeData() && (typeEntry.second->getSupermode() == Supermode::DATA)) {
            typeEntry.second->showObjectTypeEntry();
        } else {
            typeEntry.second->hideObjectTypeEntry();
        }
    }
    // recalc frame parent
    recalc();
}


GNESelectorFrame::LockGLObjectTypes::ObjectTypeEntry::ObjectTypeEntry(FXMatrix* matrixParent, const Supermode supermode, const std::string& label) :
    FXObject(),
    mySupermode(supermode),
    myLabelCounter(nullptr),
    myLabelTypeName(nullptr),
    myCheckBoxLocked(nullptr),
    myCounter(0) {
    // create elements
    myLabelCounter = new FXLabel(matrixParent, "0", nullptr, GUIDesignLabelLeft);
    myLabelTypeName = new FXLabel(matrixParent, (label + " ").c_str(), nullptr, GUIDesignLabelLeft);
    myCheckBoxLocked = new FXCheckButton(matrixParent, "unlocked", this, MID_GNE_SET_ATTRIBUTE, GUIDesignCheckButton);
}


Supermode
GNESelectorFrame::LockGLObjectTypes::ObjectTypeEntry::getSupermode() const {
    return mySupermode;
}


void
GNESelectorFrame::LockGLObjectTypes::ObjectTypeEntry::showObjectTypeEntry() {
    myLabelCounter->show();
    myLabelTypeName->show();
    myCheckBoxLocked->show();
}


void
GNESelectorFrame::LockGLObjectTypes::ObjectTypeEntry::hideObjectTypeEntry() {
    myLabelCounter->hide();
    myLabelTypeName->hide();
    myCheckBoxLocked->hide();
}


void
GNESelectorFrame::LockGLObjectTypes::ObjectTypeEntry::counterUp() {
    myCounter++;
    myLabelCounter->setText(toString(myCounter).c_str());
}


void
GNESelectorFrame::LockGLObjectTypes::ObjectTypeEntry::counterDown() {
    myCounter--;
    myLabelCounter->setText(toString(myCounter).c_str());
}


bool
GNESelectorFrame::LockGLObjectTypes::ObjectTypeEntry::isGLTypeLocked() const {
    return (myCheckBoxLocked->getCheck() == TRUE);
}


long
GNESelectorFrame::LockGLObjectTypes::ObjectTypeEntry::onCmdSetCheckBox(FXObject*, FXSelector, void*) {
    if (myCheckBoxLocked->getCheck() == TRUE) {
        myCheckBoxLocked->setText("locked");
    } else {
        myCheckBoxLocked->setText("unlocked");
    }
    return 1;
}


GNESelectorFrame::LockGLObjectTypes::ObjectTypeEntry::ObjectTypeEntry() :
    FXObject(),
    mySupermode(Supermode::NETWORK),
    myLabelCounter(nullptr),
    myLabelTypeName(nullptr),
    myCheckBoxLocked(nullptr),
    myCounter(0) {
}

// ---------------------------------------------------------------------------
// ModificationMode::ModificationMode - methods
// ---------------------------------------------------------------------------

GNESelectorFrame::ModificationMode::ModificationMode(GNESelectorFrame* selectorFrameParent) :
    FXGroupBox(selectorFrameParent->myContentFrame, "Modification Mode", GUIDesignGroupBoxFrame),
    myModificationModeType(Operation::ADD) {
    // Create all options buttons
    myAddRadioButton = new FXRadioButton(this, "add\t\tSelected objects are added to the previous selection",
                                         this, MID_CHOOSEN_OPERATION, GUIDesignRadioButton);
    myRemoveRadioButton = new FXRadioButton(this, "remove\t\tSelected objects are removed from the previous selection",
                                            this, MID_CHOOSEN_OPERATION, GUIDesignRadioButton);
    myKeepRadioButton = new FXRadioButton(this, "keep\t\tRestrict previous selection by the current selection",
                                          this, MID_CHOOSEN_OPERATION, GUIDesignRadioButton);
    myReplaceRadioButton = new FXRadioButton(this, "replace\t\tReplace previous selection by the current selection",
            this, MID_CHOOSEN_OPERATION, GUIDesignRadioButton);
    myAddRadioButton->setCheck(true);
}


GNESelectorFrame::ModificationMode::~ModificationMode() {}


GNESelectorFrame::ModificationMode::Operation
GNESelectorFrame::ModificationMode::getModificationMode() const {
    return myModificationModeType;
}


long
GNESelectorFrame::ModificationMode::onCmdSelectModificationMode(FXObject* obj, FXSelector, void*) {
    if (obj == myAddRadioButton) {
        myModificationModeType = Operation::ADD;
        myAddRadioButton->setCheck(true);
        myRemoveRadioButton->setCheck(false);
        myKeepRadioButton->setCheck(false);
        myReplaceRadioButton->setCheck(false);
        return 1;
    } else if (obj == myRemoveRadioButton) {
        myModificationModeType = Operation::SUB;
        myAddRadioButton->setCheck(false);
        myRemoveRadioButton->setCheck(true);
        myKeepRadioButton->setCheck(false);
        myReplaceRadioButton->setCheck(false);
        return 1;
    } else if (obj == myKeepRadioButton) {
        myModificationModeType = Operation::RESTRICT;
        myAddRadioButton->setCheck(false);
        myRemoveRadioButton->setCheck(false);
        myKeepRadioButton->setCheck(true);
        myReplaceRadioButton->setCheck(false);
        return 1;
    } else if (obj == myReplaceRadioButton) {
        myModificationModeType = Operation::REPLACE;
        myAddRadioButton->setCheck(false);
        myRemoveRadioButton->setCheck(false);
        myKeepRadioButton->setCheck(false);
        myReplaceRadioButton->setCheck(true);
        return 1;
    } else {
        return 0;
    }
}

// ---------------------------------------------------------------------------
// ModificationMode::ElementSet - methods
// ---------------------------------------------------------------------------

GNESelectorFrame::ElementSet::ElementSet(GNESelectorFrame* selectorFrameParent) :
    FXGroupBox(selectorFrameParent->myContentFrame, "Element Set", GUIDesignGroupBoxFrame),
    mySelectorFrameParent(selectorFrameParent),
    myCurrentElementSet(Type::NETWORKELEMENT) {
    // Create MatchTagBox for tags and fill it
    mySetComboBox = new FXComboBox(this, GUIDesignComboBoxNCol, this, MID_CHOOSEN_ELEMENTS, GUIDesignComboBox);
}


GNESelectorFrame::ElementSet::~ElementSet() {}


GNESelectorFrame::ElementSet::Type
GNESelectorFrame::ElementSet::getElementSet() const {
    return myCurrentElementSet;
}


void
GNESelectorFrame::ElementSet::refreshElementSet() {
    // first clear item
    mySetComboBox->clearItems();
    // now fill elements depending of supermode
    if (mySelectorFrameParent->myViewNet->getEditModes().isCurrentSupermodeNetwork()) {
        mySetComboBox->appendItem("network element");
        mySetComboBox->appendItem("Additional");
        mySetComboBox->appendItem("Shape");
        // show Modul
        show();
        // set num items
        mySetComboBox->setNumVisible(mySetComboBox->getNumItems());
    } else if (mySelectorFrameParent->myViewNet->getEditModes().isCurrentSupermodeDemand()) {
        mySetComboBox->appendItem("Demand Element");
        // hide Modul (because there is only an element)
        hide();
    } else if (mySelectorFrameParent->myViewNet->getEditModes().isCurrentSupermodeData()) {
        mySetComboBox->appendItem("Data Element");
        // hide Modul (because there is only an element)
        hide();
    }
    // update rest of elements
    onCmdSelectElementSet(0, 0, 0);
}


long
GNESelectorFrame::ElementSet::onCmdSelectElementSet(FXObject*, FXSelector, void*) {
    // check depending of current supermode
    if (mySelectorFrameParent->myViewNet->getEditModes().isCurrentSupermodeNetwork()) {
        // enable moduls
        mySelectorFrameParent->myMatchAttribute->showMatchAttribute();
        mySelectorFrameParent->myMatchGenericDataAttribute->hideMatchGenericDataAttribute();
        if (mySetComboBox->getText() == "network element") {
            myCurrentElementSet = Type::NETWORKELEMENT;
            mySetComboBox->setTextColor(FXRGB(0, 0, 0));
            // enable match attribute
            mySelectorFrameParent->myMatchAttribute->enableMatchAttribute();
        } else if (mySetComboBox->getText() == "Additional") {
            myCurrentElementSet = Type::ADDITIONALELEMENT;
            mySetComboBox->setTextColor(FXRGB(0, 0, 0));
            // enable match attribute
            mySelectorFrameParent->myMatchAttribute->enableMatchAttribute();
        } else if (mySetComboBox->getText() == "TAZ") {
            myCurrentElementSet = Type::TAZELEMENT;
            mySetComboBox->setTextColor(FXRGB(0, 0, 0));
            // enable match attribute
            mySelectorFrameParent->myMatchAttribute->enableMatchAttribute();
        } else if (mySetComboBox->getText() == "Shape") {
            myCurrentElementSet = Type::SHAPE;
            mySetComboBox->setTextColor(FXRGB(0, 0, 0));
            // enable match attribute
            mySelectorFrameParent->myMatchAttribute->enableMatchAttribute();
        } else {
            myCurrentElementSet = Type::INVALID;
            mySetComboBox->setTextColor(FXRGB(255, 0, 0));
            // disable match attribute
            mySelectorFrameParent->myMatchAttribute->disableMatchAttribute();
        }
    } else if (mySelectorFrameParent->myViewNet->getEditModes().isCurrentSupermodeDemand()) {
        // enable moduls
        mySelectorFrameParent->myMatchAttribute->showMatchAttribute();
        mySelectorFrameParent->myMatchGenericDataAttribute->hideMatchGenericDataAttribute();
        if (mySetComboBox->getText() == "Demand Element") {
            myCurrentElementSet = Type::DEMANDELEMENT;
            mySetComboBox->setTextColor(FXRGB(0, 0, 0));
            // enable match attribute
            mySelectorFrameParent->myMatchAttribute->enableMatchAttribute();
        } else {
            myCurrentElementSet = Type::INVALID;
            mySetComboBox->setTextColor(FXRGB(255, 0, 0));
            // disable match attribute
            mySelectorFrameParent->myMatchAttribute->disableMatchAttribute();
        }
    } else if (mySelectorFrameParent->myViewNet->getEditModes().isCurrentSupermodeData()) {
        // enable moduls
        mySelectorFrameParent->myMatchAttribute->hideMatchAttribute();
        mySelectorFrameParent->myMatchGenericDataAttribute->showMatchGenericDataAttribute();
        if (mySetComboBox->getText() == "Data Element") {
            myCurrentElementSet = Type::DATA;
            mySetComboBox->setTextColor(FXRGB(0, 0, 0));
            // enable match attribute
            mySelectorFrameParent->myMatchGenericDataAttribute->enableMatchGenericDataAttribute();
        } else {
            myCurrentElementSet = Type::INVALID;
            mySetComboBox->setTextColor(FXRGB(255, 0, 0));
            // disable match attribute
            mySelectorFrameParent->myMatchGenericDataAttribute->enableMatchGenericDataAttribute();
        }
    }
    return 1;
}

// ---------------------------------------------------------------------------
// ModificationMode::MatchAttribute - methods
// ---------------------------------------------------------------------------

GNESelectorFrame::MatchAttribute::MatchAttribute(GNESelectorFrame* selectorFrameParent) :
    FXGroupBox(selectorFrameParent->myContentFrame, "Match Attribute", GUIDesignGroupBoxFrame),
    mySelectorFrameParent(selectorFrameParent),
    myCurrentTag(SUMO_TAG_EDGE),
    myCurrentAttribute(SUMO_ATTR_ID) {
    // Create MatchTagBox for tags
    myMatchTagComboBox = new FXComboBox(this, GUIDesignComboBoxNCol, this, MID_GNE_SELECTORFRAME_SELECTTAG, GUIDesignComboBox);
    // Create listBox for Attributes
    myMatchAttrComboBox = new FXComboBox(this, GUIDesignComboBoxNCol, this, MID_GNE_SELECTORFRAME_SELECTATTRIBUTE, GUIDesignComboBox);
    // Create TextField for Match string
    myMatchString = new FXTextField(this, GUIDesignTextFieldNCol, this, MID_GNE_SELECTORFRAME_PROCESSSTRING, GUIDesignTextField);
    // Create help button
    new FXButton(this, "Help", nullptr, this, MID_HELP, GUIDesignButtonRectangular);
    // Fill list of sub-items (first element will be "edge")
    enableMatchAttribute();
    // Set speed of edge as default attribute
    myMatchAttrComboBox->setText("speed");
    myCurrentAttribute = SUMO_ATTR_SPEED;
    // Set default value for Match string
    myMatchString->setText(">10.0");
}


GNESelectorFrame::MatchAttribute::~MatchAttribute() {}


void
GNESelectorFrame::MatchAttribute::enableMatchAttribute() {
    // enable comboboxes and text field
    myMatchTagComboBox->enable();
    myMatchAttrComboBox->enable();
    myMatchString->enable();
    // Clear items of myMatchTagComboBox
    myMatchTagComboBox->clearItems();
    // Set items depending of current item set
    std::vector<std::pair<SumoXMLTag, const std::string> > ACTags;
    if (mySelectorFrameParent->myElementSet->getElementSet() == ElementSet::Type::NETWORKELEMENT) {
        ACTags = GNEAttributeCarrier::getAllowedTagsByCategory(GNETagProperties::TagType::NETWORKELEMENT, true);
    } else if (mySelectorFrameParent->myElementSet->getElementSet() == ElementSet::Type::ADDITIONALELEMENT) {
        ACTags = GNEAttributeCarrier::getAllowedTagsByCategory(GNETagProperties::TagType::ADDITIONALELEMENT, true);
    } else if (mySelectorFrameParent->myElementSet->getElementSet() == ElementSet::Type::SHAPE) {
        ACTags = GNEAttributeCarrier::getAllowedTagsByCategory(GNETagProperties::TagType::SHAPE, true);
    } else if (mySelectorFrameParent->myElementSet->getElementSet() == ElementSet::Type::TAZELEMENT) {
        ACTags = GNEAttributeCarrier::getAllowedTagsByCategory(GNETagProperties::TagType::TAZELEMENT, true);
    } else if (mySelectorFrameParent->myElementSet->getElementSet() == ElementSet::Type::DEMANDELEMENT) {
        ACTags = GNEAttributeCarrier::getAllowedTagsByCategory(GNETagProperties::TagType::DEMANDELEMENT | GNETagProperties::TagType::STOP, true);
    } else {
        throw ProcessError("Invalid element set");
    }
    // fill combo box
    for (const auto& ACTag : ACTags) {
        myMatchTagComboBox->appendItem(ACTag.second.c_str());
    }
    // set first item as current item
    myMatchTagComboBox->setCurrentItem(0);
    myMatchTagComboBox->setNumVisible(myMatchTagComboBox->getNumItems());
    // Fill attributes with the current element type
    onCmdSelMBTag(nullptr, 0, nullptr);
}


void
GNESelectorFrame::MatchAttribute::disableMatchAttribute() {
    // disable comboboxes and text field
    myMatchTagComboBox->disable();
    myMatchAttrComboBox->disable();
    myMatchString->disable();
    // change colors to black (even if there are invalid values)
    myMatchTagComboBox->setTextColor(FXRGB(0, 0, 0));
    myMatchAttrComboBox->setTextColor(FXRGB(0, 0, 0));
    myMatchString->setTextColor(FXRGB(0, 0, 0));
}


void
GNESelectorFrame::MatchAttribute::showMatchAttribute() {
    show();
}


void
GNESelectorFrame::MatchAttribute::hideMatchAttribute() {
    hide();
}


long
GNESelectorFrame::MatchAttribute::onCmdSelMBTag(FXObject*, FXSelector, void*) {
    // First check what type of elementes is being selected
    myCurrentTag = SUMO_TAG_NOTHING;
    // find current element tag
    std::vector<std::pair<SumoXMLTag, const std::string> > ACTags;
    if (mySelectorFrameParent->myElementSet->getElementSet() == ElementSet::Type::NETWORKELEMENT) {
        ACTags = GNEAttributeCarrier::getAllowedTagsByCategory(GNETagProperties::TagType::NETWORKELEMENT, true);
    } else if (mySelectorFrameParent->myElementSet->getElementSet() == ElementSet::Type::ADDITIONALELEMENT) {
        ACTags = GNEAttributeCarrier::getAllowedTagsByCategory(GNETagProperties::TagType::ADDITIONALELEMENT, true);
    } else if (mySelectorFrameParent->myElementSet->getElementSet() == ElementSet::Type::SHAPE) {
        ACTags = GNEAttributeCarrier::getAllowedTagsByCategory(GNETagProperties::TagType::SHAPE, true);
    } else if (mySelectorFrameParent->myElementSet->getElementSet() == ElementSet::Type::TAZELEMENT) {
        ACTags = GNEAttributeCarrier::getAllowedTagsByCategory(GNETagProperties::TagType::TAZELEMENT, true);
    } else if (mySelectorFrameParent->myElementSet->getElementSet() == ElementSet::Type::DEMANDELEMENT) {
        ACTags = GNEAttributeCarrier::getAllowedTagsByCategory(GNETagProperties::TagType::DEMANDELEMENT | GNETagProperties::TagType::STOP, true);
    } else {
        throw ProcessError("Unkown set");
    }
    // fill myMatchTagComboBox
    for (const auto& ACTag : ACTags) {
        if (ACTag.second == myMatchTagComboBox->getText().text()) {
            myCurrentTag = ACTag.first;
        }
    }
    // check that typed-by-user value is correct
    if (myCurrentTag != SUMO_TAG_NOTHING) {
        // obtain tag property (only for improve code legibility)
        const auto& tagValue = GNEAttributeCarrier::getTagProperties(myCurrentTag);
        // set color and enable items
        myMatchTagComboBox->setTextColor(FXRGB(0, 0, 0));
        myMatchAttrComboBox->enable();
        myMatchString->enable();
        myMatchAttrComboBox->clearItems();
        // fill attribute combo box
        for (const auto& attribute : tagValue) {
            myMatchAttrComboBox->appendItem(attribute.getAttrStr().c_str());
        }
        // Add extra attribute "Parameter"
        myMatchAttrComboBox->appendItem(toString(GNE_ATTR_PARAMETERS).c_str());
        // check if item can block movement
        if (tagValue.canBlockMovement()) {
            myMatchAttrComboBox->appendItem(toString(GNE_ATTR_BLOCK_MOVEMENT).c_str());
        }
        // check if item can block shape
        if (tagValue.canBlockShape()) {
            myMatchAttrComboBox->appendItem(toString(GNE_ATTR_BLOCK_SHAPE).c_str());
        }
        // check if item can close shape
        if (tagValue.canCloseShape()) {
            myMatchAttrComboBox->appendItem(toString(GNE_ATTR_CLOSE_SHAPE).c_str());
        }
        // check if item can have parent
        if (tagValue.isSlave()) {
            myMatchAttrComboBox->appendItem(toString(GNE_ATTR_PARENT).c_str());
        }
        // @ToDo: Here can be placed a button to set the default value
        myMatchAttrComboBox->setNumVisible(myMatchAttrComboBox->getNumItems());
        // check if we have to update attribute
        if (tagValue.hasAttribute(myCurrentAttribute)) {
            myMatchAttrComboBox->setText(toString(myCurrentAttribute).c_str());
        } else {
            onCmdSelMBAttribute(nullptr, 0, nullptr);
        }
    } else {
        // change color to red and disable items
        myMatchTagComboBox->setTextColor(FXRGB(255, 0, 0));
        myMatchAttrComboBox->disable();
        myMatchString->disable();
    }
    update();
    return 1;
}


long
GNESelectorFrame::MatchAttribute::onCmdSelMBAttribute(FXObject*, FXSelector, void*) {
    // first obtain a copy of item attributes vinculated with current tag
    auto tagPropertiesCopy = GNEAttributeCarrier::getTagProperties(myCurrentTag);
    // obtain tag property (only for improve code legibility)
    const auto& tagValue = GNEAttributeCarrier::getTagProperties(myCurrentTag);
    // add an extra AttributeValues to allow select ACs using as criterium "parameters"
    GNEAttributeProperties extraAttrProperty;
    extraAttrProperty = GNEAttributeProperties(GNE_ATTR_PARAMETERS,
                        GNEAttributeProperties::AttrProperty::STRING,
                        "Parameters");
    tagPropertiesCopy.addAttribute(extraAttrProperty);
    // add extra attribute if item can block movement
    if (tagValue.canBlockMovement()) {
        // add an extra AttributeValues to allow select ACs using as criterium "block movement"
        extraAttrProperty = GNEAttributeProperties(GNE_ATTR_BLOCK_MOVEMENT,
                            GNEAttributeProperties::AttrProperty::BOOL | GNEAttributeProperties::AttrProperty::DEFAULTVALUESTATIC,
                            "Block movement",
                            "false");
        tagPropertiesCopy.addAttribute(extraAttrProperty);
    }
    // add extra attribute if item can block shape
    if (tagValue.canBlockShape()) {
        // add an extra AttributeValues to allow select ACs using as criterium "block shape"
        extraAttrProperty = GNEAttributeProperties(GNE_ATTR_BLOCK_SHAPE,
                            GNEAttributeProperties::AttrProperty::BOOL | GNEAttributeProperties::AttrProperty::DEFAULTVALUESTATIC,
                            "Block shape",
                            "false");
        tagPropertiesCopy.addAttribute(extraAttrProperty);
    }
    // add extra attribute if item can close shape
    if (tagValue.canCloseShape()) {
        // add an extra AttributeValues to allow select ACs using as criterium "close shape"
        extraAttrProperty = GNEAttributeProperties(GNE_ATTR_CLOSE_SHAPE,
                            GNEAttributeProperties::AttrProperty::BOOL | GNEAttributeProperties::AttrProperty::DEFAULTVALUESTATIC,
                            "Close shape",
                            "true");
        tagPropertiesCopy.addAttribute(extraAttrProperty);
    }
    // add extra attribute if item can have parent
    if (tagValue.isSlave()) {
        // add an extra AttributeValues to allow select ACs using as criterium "parent"
        extraAttrProperty = GNEAttributeProperties(GNE_ATTR_PARENT,
                            GNEAttributeProperties::AttrProperty::STRING,
                            "Parent element");
        tagPropertiesCopy.addAttribute(extraAttrProperty);
    }
    // set current selected attribute
    myCurrentAttribute = SUMO_ATTR_NOTHING;
    for (const auto& attribute : tagPropertiesCopy) {
        if (attribute.getAttrStr() == myMatchAttrComboBox->getText().text()) {
            myCurrentAttribute = attribute.getAttr();
        }
    }
    // check if selected attribute is valid
    if (myCurrentAttribute != SUMO_ATTR_NOTHING) {
        myMatchAttrComboBox->setTextColor(FXRGB(0, 0, 0));
        myMatchString->enable();
    } else {
        myMatchAttrComboBox->setTextColor(FXRGB(255, 0, 0));
        myMatchString->disable();
    }
    return 1;
}


long
GNESelectorFrame::MatchAttribute::onCmdSelMBString(FXObject*, FXSelector, void*) {
    // obtain expresion
    std::string expr(myMatchString->getText().text());
    const auto& tagValue = GNEAttributeCarrier::getTagProperties(myCurrentTag);
    bool valid = true;
    if (expr == "") {
        // the empty expression matches all objects
        mySelectorFrameParent->handleIDs(mySelectorFrameParent->getMatches(myCurrentTag, myCurrentAttribute, '@', 0, expr));
    } else if (tagValue.hasAttribute(myCurrentAttribute) && tagValue.getAttributeProperties(myCurrentAttribute).isNumerical()) {
        // The expression must have the form
        //  <val matches if attr < val
        //  >val matches if attr > val
        //  =val matches if attr = val
        //  val matches if attr = val
        char compOp = expr[0];
        if (compOp == '<' || compOp == '>' || compOp == '=') {
            expr = expr.substr(1);
        } else {
            compOp = '=';
        }
        // check if value can be parsed to double
        if (GNEAttributeCarrier::canParse<double>(expr.c_str())) {
            mySelectorFrameParent->handleIDs(mySelectorFrameParent->getMatches(myCurrentTag, myCurrentAttribute, compOp, GNEAttributeCarrier::parse<double>(expr.c_str()), expr));
        } else {
            valid = false;
        }
    } else {
        // The expression must have the form
        //   =str: matches if <str> is an exact match
        //   !str: matches if <str> is not a substring
        //   ^str: matches if <str> is not an exact match
        //   str: matches if <str> is a substring (sends compOp '@')
        // Alternatively, if the expression is empty it matches all objects
        char compOp = expr[0];
        if (compOp == '=' || compOp == '!' || compOp == '^') {
            expr = expr.substr(1);
        } else {
            compOp = '@';
        }
        mySelectorFrameParent->handleIDs(mySelectorFrameParent->getMatches(myCurrentTag, myCurrentAttribute, compOp, 0, expr));
    }
    if (valid) {
        myMatchString->setTextColor(FXRGB(0, 0, 0));
        myMatchString->killFocus();
    } else {
        myMatchString->setTextColor(FXRGB(255, 0, 0));
    }
    return 1;
}


long
GNESelectorFrame::MatchAttribute::onCmdHelp(FXObject*, FXSelector, void*) {
    // Create dialog box
    FXDialogBox* additionalNeteditAttributesHelpDialog = new FXDialogBox(this, "Netedit Parameters Help", GUIDesignDialogBox);
    additionalNeteditAttributesHelpDialog->setIcon(GUIIconSubSys::getIcon(GUIIcon::MODEADDITIONAL));
    // set help text
    std::ostringstream help;
    help
            << "- The 'Match Attribute' controls allow to specify a set of objects which are then applied to the current selection\n"
            << "  according to the current 'Modification Mode'.\n"
            << "     1. Select an object type from the first input box\n"
            << "     2. Select an attribute from the second input box\n"
            << "     3. Enter a 'match expression' in the third input box and press <return>\n"
            << "\n"
            << "- The empty expression matches all objects\n"
            << "- For numerical attributes the match expression must consist of a comparison operator ('<', '>', '=') and a number.\n"
            << "- An object matches if the comparison between its attribute and the given number by the given operator evaluates to 'true'\n"
            << "\n"
            << "- For string attributes the match expression must consist of a comparison operator ('', '=', '!', '^') and a string.\n"
            << "     '' (no operator) matches if string is a substring of that object'ts attribute.\n"
            << "     '=' matches if string is an exact match.\n"
            << "     '!' matches if string is not a substring.\n"
            << "     '^' matches if string is not an exact match.\n"
            << "\n"
            << "- Examples:\n"
            << "     junction; id; 'foo' -> match all junctions that have 'foo' in their id\n"
            << "     junction; type; '=priority' -> match all junctions of type 'priority', but not of type 'priority_stop'\n"
            << "     edge; speed; '>10' -> match all edges with a speed above 10\n";
    // Create label with the help text
    new FXLabel(additionalNeteditAttributesHelpDialog, help.str().c_str(), nullptr, GUIDesignLabelFrameInformation);
    // Create horizontal separator
    new FXHorizontalSeparator(additionalNeteditAttributesHelpDialog, GUIDesignHorizontalSeparator);
    // Create frame for OK Button
    FXHorizontalFrame* myHorizontalFrameOKButton = new FXHorizontalFrame(additionalNeteditAttributesHelpDialog, GUIDesignAuxiliarHorizontalFrame);
    // Create Button Close (And two more horizontal frames to center it)
    new FXHorizontalFrame(myHorizontalFrameOKButton, GUIDesignAuxiliarHorizontalFrame);
    new FXButton(myHorizontalFrameOKButton, "OK\t\tclose", GUIIconSubSys::getIcon(GUIIcon::ACCEPT), additionalNeteditAttributesHelpDialog, FXDialogBox::ID_ACCEPT, GUIDesignButtonOK);
    new FXHorizontalFrame(myHorizontalFrameOKButton, GUIDesignAuxiliarHorizontalFrame);
    // Write Warning in console if we're in testing mode
    WRITE_DEBUG("Opening help dialog of selector frame");
    // create Dialog
    additionalNeteditAttributesHelpDialog->create();
    // show in the given position
    additionalNeteditAttributesHelpDialog->show(PLACEMENT_CURSOR);
    // refresh APP
    getApp()->refresh();
    // open as modal dialog (will block all windows until stop() or stopModal() is called)
    getApp()->runModalFor(additionalNeteditAttributesHelpDialog);
    // Write Warning in console if we're in testing mode
    WRITE_DEBUG("Close help dialog of selector frame");
    return 1;
}

// ---------------------------------------------------------------------------
// ModificationMode::MatchGenericDataAttribute - methods
// ---------------------------------------------------------------------------

GNESelectorFrame::MatchGenericDataAttribute::MatchGenericDataAttribute(GNESelectorFrame* selectorFrameParent) :
    FXGroupBox(selectorFrameParent->myContentFrame, "Match GenericData Attribute", GUIDesignGroupBoxFrame),
    mySelectorFrameParent(selectorFrameParent),
    myIntervalSelector(nullptr),
    myBegin(nullptr),
    myEnd(nullptr),
    myMatchGenericDataTagComboBox(nullptr),
    myMatchGenericDataAttrComboBox(nullptr),
    myCurrentTag(SUMO_TAG_EDGE),
    myMatchGenericDataString(nullptr) {
    // Create MatchGenericDataTagBox for tags
    new FXLabel(this, "Interval [begin, end]", nullptr, GUIDesignLabelThick);
    myIntervalSelector = new FXComboBox(this, GUIDesignComboBoxNCol, this, MID_GNE_SELECTORFRAME_SETINTERVAL, GUIDesignComboBoxStaticExtended);
    // Create textfield for begin and end
    FXHorizontalFrame* myHorizontalFrameBeginEnd = new FXHorizontalFrame(this, GUIDesignAuxiliarHorizontalFrame);
    myBegin = new FXTextField(myHorizontalFrameBeginEnd, GUIDesignTextFieldNCol, this, MID_GNE_SELECTORFRAME_SETBEGIN, GUIDesignTextField);
    myEnd = new FXTextField(myHorizontalFrameBeginEnd, GUIDesignTextFieldNCol, this, MID_GNE_SELECTORFRAME_SETEND, GUIDesignTextField);
    // Create MatchGenericDataTagBox myHorizontalFrameEnd tags
    myMatchGenericDataTagComboBox = new FXComboBox(this, GUIDesignComboBoxNCol, this, MID_GNE_SELECTORFRAME_SELECTTAG, GUIDesignComboBox);
    // Create listBox for Attributes
    myMatchGenericDataAttrComboBox = new FXComboBox(this, GUIDesignComboBoxNCol, this, MID_GNE_SELECTORFRAME_SELECTATTRIBUTE, GUIDesignComboBox);
    // Create TextField for MatchGenericData string
    myMatchGenericDataString = new FXTextField(this, GUIDesignTextFieldNCol, this, MID_GNE_SELECTORFRAME_PROCESSSTRING, GUIDesignTextField);
    // Create help button
    new FXButton(this, "Help", nullptr, this, MID_HELP, GUIDesignButtonRectangular);
    // Fill list of sub-items (first element will be "edge")
    enableMatchGenericDataAttribute();
    // Set speed of edge as default attribute
    myMatchGenericDataAttrComboBox->setText("speed");
    myCurrentAttribute = SUMO_ATTR_SPEED;
    // Set default value for MatchGenericData string
    myMatchGenericDataString->setText(">10.0");
}


GNESelectorFrame::MatchGenericDataAttribute::~MatchGenericDataAttribute() {}


void
GNESelectorFrame::MatchGenericDataAttribute::enableMatchGenericDataAttribute() {
    // first drop intervals
    myIntervals.clear();
    // iterate over all data sets
    for (const auto& dataSet : mySelectorFrameParent->getViewNet()->getNet()->retrieveDataSets()) {
        for (const auto& dataInterval : dataSet->getDataIntervalChildren()) {
            myIntervals[std::make_pair(dataInterval.second->getAttributeDouble(SUMO_ATTR_BEGIN), dataInterval.second->getAttributeDouble(SUMO_ATTR_END))] = -1;
        }
    }
    // disable modul if there isn't intervals
    if (myIntervals.size() == 0) {
        disableMatchGenericDataAttribute();
    } else {
        // enable comboboxes and text field
        myIntervalSelector->enable();
        myBegin->enable();
        myEnd->enable();
        myMatchGenericDataTagComboBox->enable();
        myMatchGenericDataAttrComboBox->enable();
        myMatchGenericDataString->enable();
        // clear combo box interval selector
        myIntervalSelector->clearItems();
        // fill combo Box
        for (auto& interval : myIntervals) {
            interval.second = myIntervalSelector->appendItem((" [" + toString(interval.first.first) + "," + toString(interval.first.second) + "]").c_str());
        }
        // set number of visible items
        if (myIntervalSelector->getNumItems() < 10) {
            myIntervalSelector->setNumVisible(myIntervalSelector->getNumItems());
        } else {
            myIntervalSelector->setNumVisible(10);
        }
        // Clear items of myMatchGenericDataTagComboBox
        myMatchGenericDataTagComboBox->clearItems();
        // update begin and end
        myBegin->setText(toString(myIntervals.begin()->first.first).c_str());
        myBegin->setTextColor(FXRGB(0, 0, 0));
        myEnd->setText(toString(myIntervals.begin()->first.second).c_str());
        myEnd->setTextColor(FXRGB(0, 0, 0));
        // get generic datas
        const auto genericDataTags = GNEAttributeCarrier::getAllowedTagsByCategory(GNETagProperties::TagType::GENERICDATA, true);
        // fill combo box
        for (const auto& genericDataTag : genericDataTags) {
            myMatchGenericDataTagComboBox->appendItem(genericDataTag.second.c_str());
        }
        // set first item as current item
        myMatchGenericDataTagComboBox->setCurrentItem(0);
        myMatchGenericDataTagComboBox->setNumVisible(myMatchGenericDataTagComboBox->getNumItems());
        // call select tag
        onCmdSelectTag(nullptr, 0, nullptr);
    }
}


void
GNESelectorFrame::MatchGenericDataAttribute::disableMatchGenericDataAttribute() {
    // disable comboboxes and text field
    myIntervalSelector->disable();
    myBegin->disable();
    myEnd->disable();
    myMatchGenericDataTagComboBox->disable();
    myMatchGenericDataAttrComboBox->disable();
    myMatchGenericDataString->disable();
    // change colors to black (even if there are invalid values)
    myMatchGenericDataTagComboBox->setTextColor(FXRGB(0, 0, 0));
    myMatchGenericDataAttrComboBox->setTextColor(FXRGB(0, 0, 0));
    myMatchGenericDataString->setTextColor(FXRGB(0, 0, 0));
}


void
GNESelectorFrame::MatchGenericDataAttribute::showMatchGenericDataAttribute() {
    // just show Modul
    show();
}


void
GNESelectorFrame::MatchGenericDataAttribute::hideMatchGenericDataAttribute() {
    // just hide modul
    hide();
}


long
GNESelectorFrame::MatchGenericDataAttribute::onCmdSetInterval(FXObject*, FXSelector, void*) {
    // iterate over interval and update begin and end
    for (auto& interval : myIntervals) {
        if (interval.second == myIntervalSelector->getCurrentItem()) {
            // update begin
            myBegin->setTextColor(FXRGB(0, 0, 0));
            myBegin->setText(toString(interval.first.first).c_str());
            // update end
            myEnd->setTextColor(FXRGB(0, 0, 0));
            myEnd->setText(toString(interval.first.second).c_str());
        }
    }
    // call onCmdSelectTag
    onCmdSelectTag(0, 0, 0);
    return 1;
}


long
GNESelectorFrame::MatchGenericDataAttribute::onCmdSetBegin(FXObject*, FXSelector, void*) {
    // check if can be parsed to double
    if (GNEAttributeCarrier::canParse<double>(myBegin->getText().text()) &&
            GNEAttributeCarrier::canParse<double>(myEnd->getText().text())) {
        // set valid color text and kill focus
        myBegin->setTextColor(FXRGB(0, 0, 0));
        myBegin->killFocus();
        // enable elements
        myMatchGenericDataTagComboBox->enable();
        myMatchGenericDataAttrComboBox->enable();
        myMatchGenericDataString->enable();
        // call onCmdSelectTag
        onCmdSelectTag(0, 0, 0);
    } else {
        // set invalid color text
        myBegin->setTextColor(FXRGB(255, 0, 0));
        // disable elements
        myMatchGenericDataTagComboBox->disable();
        myMatchGenericDataAttrComboBox->disable();
        myMatchGenericDataString->disable();
    }
    return 1;
}


long
GNESelectorFrame::MatchGenericDataAttribute::onCmdSetEnd(FXObject*, FXSelector, void*) {
    // check if can be parsed to double
    if (GNEAttributeCarrier::canParse<double>(myBegin->getText().text()) &&
            GNEAttributeCarrier::canParse<double>(myEnd->getText().text())) {
        // set valid color text and kill focus
        myEnd->setTextColor(FXRGB(0, 0, 0));
        myEnd->killFocus();
        // enable elements
        myMatchGenericDataTagComboBox->enable();
        myMatchGenericDataAttrComboBox->enable();
        myMatchGenericDataString->enable();
        // call onCmdSelectTag
        onCmdSelectTag(0, 0, 0);
    } else {
        // set invalid color text
        myEnd->setTextColor(FXRGB(255, 0, 0));
        // disable elements
        myMatchGenericDataTagComboBox->disable();
        myMatchGenericDataAttrComboBox->disable();
        myMatchGenericDataString->disable();
    }
    return 1;
}


long
GNESelectorFrame::MatchGenericDataAttribute::onCmdSelectTag(FXObject*, FXSelector, void*) {
    // First check what type of elementes is being selected
    myCurrentTag = SUMO_TAG_NOTHING;
    // get generic data tags
    const auto listOfTags = GNEAttributeCarrier::getAllowedTagsByCategory(GNETagProperties::TagType::GENERICDATA, true);
    // fill myMatchGenericDataTagComboBox
    for (const auto& genericDataTag : listOfTags) {
        if (genericDataTag.second == myMatchGenericDataTagComboBox->getText().text()) {
            myCurrentTag = genericDataTag.first;
        }
    }
    // check that typed-by-user value is correct
    if (myCurrentTag != SUMO_TAG_NOTHING) {
        // obtain begin and end
        const double begin = GNEAttributeCarrier::parse<double>(myBegin->getText().text());
        const double end = GNEAttributeCarrier::parse<double>(myEnd->getText().text());
        // obtain all Generic Data attributes for current generic tag
        auto attributes = mySelectorFrameParent->getViewNet()->getNet()->retrieveGenericDataParameters(toString(myCurrentTag), begin, end);
        // set color and enable items
        myMatchGenericDataTagComboBox->setTextColor(FXRGB(0, 0, 0));
        myMatchGenericDataAttrComboBox->enable();
        myMatchGenericDataString->enable();
        myMatchGenericDataAttrComboBox->clearItems();
        // add data set parent
        myMatchGenericDataAttrComboBox->appendItem(toString(GNE_ATTR_DATASET).c_str());
        // fill attribute combo box
        for (const auto& attribute : attributes) {
            myMatchGenericDataAttrComboBox->appendItem(attribute.c_str());
        }
        myMatchGenericDataAttrComboBox->setNumVisible(myMatchGenericDataAttrComboBox->getNumItems());
        onCmdSelectAttribute(nullptr, 0, nullptr);
    } else {
        // change color to red and disable items
        myMatchGenericDataTagComboBox->setTextColor(FXRGB(255, 0, 0));
        myMatchGenericDataAttrComboBox->disable();
        myMatchGenericDataString->disable();
    }
    update();
    return 1;
}


long
GNESelectorFrame::MatchGenericDataAttribute::onCmdSelectAttribute(FXObject*, FXSelector, void*) {
    // obtain all Generic Data attributes for current generic tag
    std::set<std::string> attributes = mySelectorFrameParent->getViewNet()->getNet()->retrieveGenericDataParameters(
                                           toString(myCurrentTag),
                                           GNEAttributeCarrier::parse<double>(myBegin->getText().text()),
                                           GNEAttributeCarrier::parse<double>(myEnd->getText().text()));
    // add extra data set parent attribute
    attributes.insert(toString(GNE_ATTR_DATASET));
    // clear current attribute
    myCurrentAttribute.clear();
    // set current selected attribute
    for (const auto& attribute : attributes) {
        if (attribute == myMatchGenericDataAttrComboBox->getText().text()) {
            myCurrentAttribute = attribute;
        }
    }
    // check if selected attribute is valid
    if (myCurrentAttribute.empty()) {
        myMatchGenericDataAttrComboBox->setTextColor(FXRGB(255, 0, 0));
        myMatchGenericDataString->disable();
    } else {
        myMatchGenericDataAttrComboBox->setTextColor(FXRGB(0, 0, 0));
        myMatchGenericDataString->enable();
    }
    return 1;
}


long
GNESelectorFrame::MatchGenericDataAttribute::onCmdProcessString(FXObject*, FXSelector, void*) {
    // obtain expression and tag value
    std::string expression = myMatchGenericDataString->getText().text();
    bool valid = true;
    // get all Generic datas
    const auto genericDatas = mySelectorFrameParent->getViewNet()->getNet()->getAttributeCarriers()->retrieveGenericDatas(myCurrentTag,
                              GNEAttributeCarrier::parse<double>(myBegin->getText().text()),
                              GNEAttributeCarrier::parse<double>(myEnd->getText().text()));
    if (expression == "") {
        // the empty expression matches all objects
        mySelectorFrameParent->handleIDs(mySelectorFrameParent->getGenericMatches(genericDatas, myCurrentAttribute, '@', 0, expression));
    } else if (myCurrentAttribute != toString(GNE_ATTR_DATASET)) {
        // The expression must have the form
        //  <val matches if attr < val
        //  >val matches if attr > val
        //  =val matches if attr = val
        //  val matches if attr = val
        char compOp = expression[0];
        if ((compOp == '<') || (compOp == '>') || (compOp == '=')) {
            expression = expression.substr(1);
        } else {
            compOp = '=';
        }
        // check if value can be parsed to double
        if (GNEAttributeCarrier::canParse<double>(expression.c_str())) {
            mySelectorFrameParent->handleIDs(mySelectorFrameParent->getGenericMatches(genericDatas, myCurrentAttribute, compOp, GNEAttributeCarrier::parse<double>(expression.c_str()), expression));
        } else {
            valid = false;
        }
    } else {
        // The expression must have the form
        //   =str: matches if <str> is an exact match
        //   !str: matches if <str> is not a substring
        //   ^str: matches if <str> is not an exact match
        //   str: matches if <str> is a substring (sends compOp '@')
        // Alternatively, if the expression is empty it matches all objects
        char compOp = expression[0];
        if ((compOp == '=') || (compOp == '!') || (compOp == '^')) {
            expression = expression.substr(1);
        } else {
            compOp = '@';
        }
        mySelectorFrameParent->handleIDs(mySelectorFrameParent->getGenericMatches(genericDatas, myCurrentAttribute, compOp, 0, expression));
    }
    // change color depending of flag "valid"
    if (valid) {
        myMatchGenericDataString->setTextColor(FXRGB(0, 0, 0));
        myMatchGenericDataString->killFocus();
    } else {
        myMatchGenericDataString->setTextColor(FXRGB(255, 0, 0));
    }
    return 1;
}


long
GNESelectorFrame::MatchGenericDataAttribute::onCmdHelp(FXObject*, FXSelector, void*) {
    // Create dialog box
    FXDialogBox* additionalNeteditAttributesHelpDialog = new FXDialogBox(this, "Netedit Parameters Help", GUIDesignDialogBox);
    additionalNeteditAttributesHelpDialog->setIcon(GUIIconSubSys::getIcon(GUIIcon::MODEADDITIONAL));
    // set help text
    std::ostringstream help;
    help
            << "- The 'MatchGenericData Attribute' controls allow to specify a set of objects which are then applied to the current selection\n"
            << "  according to the current 'Modification Mode'.\n"
            << "     1. Select an object type from the first input box\n"
            << "     2. Select an attribute from the second input box\n"
            << "     3. Enter a 'match expression' in the third input box and press <return>\n"
            << "\n"
            << "- The empty expression matches all objects\n"
            << "- For numerical attributes the match expression must consist of a comparison operator ('<', '>', '=') and a number.\n"
            << "- An object matches if the comparison between its attribute and the given number by the given operator evaluates to 'true'\n"
            << "\n"
            << "- For string attributes the match expression must consist of a comparison operator ('', '=', '!', '^') and a string.\n"
            << "     '' (no operator) matches if string is a substring of that object'ts attribute.\n"
            << "     '=' matches if string is an exact match.\n"
            << "     '!' matches if string is not a substring.\n"
            << "     '^' matches if string is not an exact match.\n"
            << "\n"
            << "- Examples:\n"
            << "     junction; id; 'foo' -> match all junctions that have 'foo' in their id\n"
            << "     junction; type; '=priority' -> match all junctions of type 'priority', but not of type 'priority_stop'\n"
            << "     edge; speed; '>10' -> match all edges with a speed above 10\n";
    // Create label with the help text
    new FXLabel(additionalNeteditAttributesHelpDialog, help.str().c_str(), nullptr, GUIDesignLabelFrameInformation);
    // Create horizontal separator
    new FXHorizontalSeparator(additionalNeteditAttributesHelpDialog, GUIDesignHorizontalSeparator);
    // Create frame for OK Button
    FXHorizontalFrame* myHorizontalFrameOKButton = new FXHorizontalFrame(additionalNeteditAttributesHelpDialog, GUIDesignAuxiliarHorizontalFrame);
    // Create Button Close (And two more horizontal frames to center it)
    new FXHorizontalFrame(myHorizontalFrameOKButton, GUIDesignAuxiliarHorizontalFrame);
    new FXButton(myHorizontalFrameOKButton, "OK\t\tclose", GUIIconSubSys::getIcon(GUIIcon::ACCEPT), additionalNeteditAttributesHelpDialog, FXDialogBox::ID_ACCEPT, GUIDesignButtonOK);
    new FXHorizontalFrame(myHorizontalFrameOKButton, GUIDesignAuxiliarHorizontalFrame);
    // Write Warning in console if we're in testing mode
    WRITE_DEBUG("Opening help dialog of selector frame");
    // create Dialog
    additionalNeteditAttributesHelpDialog->create();
    // show in the given position
    additionalNeteditAttributesHelpDialog->show(PLACEMENT_CURSOR);
    // refresh APP
    getApp()->refresh();
    // open as modal dialog (will block all windows until stop() or stopModal() is called)
    getApp()->runModalFor(additionalNeteditAttributesHelpDialog);
    // Write Warning in console if we're in testing mode
    WRITE_DEBUG("Close help dialog of selector frame");
    return 1;
}

// ---------------------------------------------------------------------------
// ModificationMode::VisualScaling - methods
// ---------------------------------------------------------------------------

GNESelectorFrame::VisualScaling::VisualScaling(GNESelectorFrame* selectorFrameParent) :
    FXGroupBox(selectorFrameParent->myContentFrame, "Visual Scaling", GUIDesignGroupBoxFrame),
    mySelectorFrameParent(selectorFrameParent) {
    // Create spin button and configure it
    mySelectionScaling = new FXRealSpinner(this, 7, this, MID_GNE_SELECTORFRAME_SELECTSCALE, GUIDesignSpinDial);
    //mySelectionScaling->setNumberFormat(1);
    //mySelectionScaling->setIncrements(0.1, .5, 1);
    mySelectionScaling->setIncrement(0.5);
    mySelectionScaling->setRange(1, 100000);
    mySelectionScaling->setValue(1);
    mySelectionScaling->setHelpText("Enlarge selected objects");
}


GNESelectorFrame::VisualScaling::~VisualScaling() {}


long
GNESelectorFrame::VisualScaling::onCmdScaleSelection(FXObject*, FXSelector, void*) {
    // set scale in viewnet
    mySelectorFrameParent->myViewNet->setSelectionScaling(mySelectionScaling->getValue());
    mySelectorFrameParent->myViewNet->updateViewNet();
    return 1;
}

// ---------------------------------------------------------------------------
// ModificationMode::SelectionOperation - methods
// ---------------------------------------------------------------------------

GNESelectorFrame::SelectionOperation::SelectionOperation(GNESelectorFrame* selectorFrameParent) :
    FXGroupBox(selectorFrameParent->myContentFrame, "Operations for selections", GUIDesignGroupBoxFrame),
    mySelectorFrameParent(selectorFrameParent) {
    // Create "Clear List" Button
    new FXButton(this, "Clear\t\t", nullptr, this, MID_CHOOSEN_CLEAR, GUIDesignButton);
    // Create "Invert" Button
    new FXButton(this, "Invert\t\t", nullptr, this, MID_CHOOSEN_INVERT, GUIDesignButton);
    // Create "Save" Button
    new FXButton(this, "Save\t\tSave ids of currently selected objects to a file.", nullptr, this, MID_CHOOSEN_SAVE, GUIDesignButton);
    // Create "Load" Button
    new FXButton(this, "Load\t\tLoad ids from a file according to the current modfication mode.", nullptr, this, MID_CHOOSEN_LOAD, GUIDesignButton);
}


GNESelectorFrame::SelectionOperation::~SelectionOperation() {}


long
GNESelectorFrame::SelectionOperation::onCmdLoad(FXObject*, FXSelector, void*) {
    // get the new file name
    FXFileDialog opendialog(this, "Open List of Selected Items");
    opendialog.setIcon(GUIIconSubSys::getIcon(GUIIcon::EMPTY));
    opendialog.setSelectMode(SELECTFILE_EXISTING);
    opendialog.setPatternList("Selection files (*.txt)\nAll files (*)");
    if (gCurrentFolder.length() != 0) {
        opendialog.setDirectory(gCurrentFolder);
    }
    if (opendialog.execute()) {
        std::vector<GNEAttributeCarrier*> loadedACs;
        gCurrentFolder = opendialog.getDirectory();
        std::string file = opendialog.getFilename().text();
        std::ostringstream msg;
        std::ifstream strm(file.c_str());
        // check if file can be opened
        if (!strm.good()) {
            WRITE_ERROR("Could not open '" + file + "'.");
            return 0;
        }
        while (strm.good()) {
            std::string line;
            strm >> line;
            // check if line isn't empty
            if (line.length() != 0) {
                // obtain GLObject
                GUIGlObject* object = GUIGlObjectStorage::gIDStorage.getObjectBlocking(line);
                // check if GUIGlObject exist and their  their GL type isn't blocked
                if ((object != nullptr) && !mySelectorFrameParent->myLockGLObjectTypes->IsObjectTypeLocked(object->getType())) {
                    // obtain GNEAttributeCarrier
                    GNEAttributeCarrier* AC = mySelectorFrameParent->myViewNet->getNet()->retrieveAttributeCarrier(object->getGlID(), false);
                    // check if AC exist and if is selectable
                    if (AC && AC->getTagProperty().isSelectable())
                        // now check if we're in the correct supermode to load this element
                        if (((mySelectorFrameParent->myViewNet->getEditModes().isCurrentSupermodeNetwork()) && !AC->getTagProperty().isDemandElement()) ||
                                ((mySelectorFrameParent->myViewNet->getEditModes().isCurrentSupermodeDemand()) && AC->getTagProperty().isDemandElement())) {
                            loadedACs.push_back(AC);
                        }
                }
            }
        }
        // change selected attribute in loaded ACs allowing undo/redo
        if (loadedACs.size() > 0) {
            mySelectorFrameParent->myViewNet->getUndoList()->p_begin("load selection");
            mySelectorFrameParent->handleIDs(loadedACs);
            mySelectorFrameParent->myViewNet->getUndoList()->p_end();
        }
    }
    mySelectorFrameParent->myViewNet->updateViewNet();
    return 1;
}


long
GNESelectorFrame::SelectionOperation::onCmdSave(FXObject*, FXSelector, void*) {
    FXString file = MFXUtils::getFilename2Write(this,
                    "Save List of selected Items", ".txt",
                    GUIIconSubSys::getIcon(GUIIcon::EMPTY), gCurrentFolder);
    if (file == "") {
        return 1;
    }
    try {
        OutputDevice& dev = OutputDevice::getDevice(file.text());
        // get selected attribute carriers
        const auto selectedACs = mySelectorFrameParent->myViewNet->getNet()->getSelectedAttributeCarriers(false);
        for (const auto& selectedAC : selectedACs) {
            GUIGlObject* object = dynamic_cast<GUIGlObject*>(selectedAC);
            if (object) {
                dev << GUIGlObject::TypeNames.getString(object->getType()) << ":" << selectedAC->getID() << "\n";
            }
        }
        dev.close();
    } catch (IOError& e) {
        // write warning if netedit is running in testing mode
        WRITE_DEBUG("Opening FXMessageBox 'error storing selection'");
        // open message box error
        FXMessageBox::error(this, MBOX_OK, "Storing Selection failed", "%s", e.what());
        // write warning if netedit is running in testing mode
        WRITE_DEBUG("Closed FXMessageBox 'error storing selection' with 'OK'");
    }
    return 1;
}


long
GNESelectorFrame::SelectionOperation::onCmdClear(FXObject*, FXSelector, void*) {
    // clear current selection
    mySelectorFrameParent->clearCurrentSelection();
    return 1;
}


long
GNESelectorFrame::SelectionOperation::onCmdInvert(FXObject*, FXSelector, void*) {
    // only continue if there is element for selecting
    if (mySelectorFrameParent->ACsToSelected()) {
        // obtan locks (only for improve code legibly)
        LockGLObjectTypes* locks = mySelectorFrameParent->getLockGLObjectTypes();
        // obtain undoList (only for improve code legibly)
        GNEUndoList* undoList = mySelectorFrameParent->myViewNet->getUndoList();
        // for invert selection, first clean current selection and next select elements of set "unselectedElements"
        undoList->p_begin("invert selection");
        // invert selection of elements depending of current supermode
        if (mySelectorFrameParent->myViewNet->getEditModes().isCurrentSupermodeNetwork()) {
            // iterate over junctions
            for (const auto& junction : mySelectorFrameParent->myViewNet->getNet()->getAttributeCarriers()->getJunctions()) {
                // check if junction selection is locked
                if (!locks->IsObjectTypeLocked(GLO_JUNCTION)) {
                    if (junction.second->isAttributeCarrierSelected()) {
                        junction.second->setAttribute(GNE_ATTR_SELECTED, "false", undoList);
                    } else {
                        junction.second->setAttribute(GNE_ATTR_SELECTED, "true", undoList);
                    }
                }
                // due we iterate over all junctions, only it's neccesary iterate over incoming edges
                for (const auto& incomingEdge : junction.second->getGNEIncomingEdges()) {
                    // only select edges if "select edges" flag is enabled. In other case, select only lanes
                    if (mySelectorFrameParent->myViewNet->getNetworkViewOptions().selectEdges()) {
                        // check if edge selection is locked
                        if (!locks->IsObjectTypeLocked(GLO_EDGE)) {
                            if (incomingEdge->isAttributeCarrierSelected()) {
                                incomingEdge->setAttribute(GNE_ATTR_SELECTED, "false", undoList);
                            } else {
                                incomingEdge->setAttribute(GNE_ATTR_SELECTED, "true", undoList);
                            }
                        }
                    } else {
                        // check if lane selection is locked
                        if (!locks->IsObjectTypeLocked(GLO_LANE)) {
                            for (const auto& lane : incomingEdge->getLanes()) {
                                if (lane->isAttributeCarrierSelected()) {
                                    lane->setAttribute(GNE_ATTR_SELECTED, "false", undoList);
                                } else {
                                    lane->setAttribute(GNE_ATTR_SELECTED, "true", undoList);
                                }
                            }
                        }
                    }
                    // check if connection selection is locked
                    if (!locks->IsObjectTypeLocked(GLO_CONNECTION)) {
                        for (const auto& connection : incomingEdge->getGNEConnections()) {
                            if (connection->isAttributeCarrierSelected()) {
                                connection->setAttribute(GNE_ATTR_SELECTED, "false", undoList);
                            } else {
                                connection->setAttribute(GNE_ATTR_SELECTED, "true", undoList);
                            }
                        }
                    }
                }
                // check if crossing selection is locked
                if (!locks->IsObjectTypeLocked(GLO_CROSSING)) {
                    for (const auto& crossing : junction.second->getGNECrossings()) {
                        if (crossing->isAttributeCarrierSelected()) {
                            crossing->setAttribute(GNE_ATTR_SELECTED, "false", undoList);
                        } else {
                            crossing->setAttribute(GNE_ATTR_SELECTED, "true", undoList);
                        }
                    }
                }
            }
            // check if additionals selection is locked
            if (!locks->IsObjectTypeLocked(GLO_ADDITIONALELEMENT)) {
                for (const auto& additionals : mySelectorFrameParent->myViewNet->getNet()->getAttributeCarriers()->getAdditionals()) {
                    // first check if additional is selectable
                    if (GNEAttributeCarrier::getTagProperties(additionals.first).isSelectable()) {
                        for (const auto& additional : additionals.second) {
                            if (additional.second->isAttributeCarrierSelected()) {
                                additional.second->setAttribute(GNE_ATTR_SELECTED, "false", undoList);
                            } else {
                                additional.second->setAttribute(GNE_ATTR_SELECTED, "true", undoList);
                            }
                            // now iterate over additional children
                            for (const auto& additionalChild : additional.second->getChildAdditionals()) {
                                // first check if additional child is selectable
                                if (additionalChild->getTagProperty().isSelectable()) {
                                    if (additionalChild->isAttributeCarrierSelected()) {
                                        additionalChild->setAttribute(GNE_ATTR_SELECTED, "false", undoList);
                                    } else {
                                        additionalChild->setAttribute(GNE_ATTR_SELECTED, "true", undoList);
                                    }
                                }
                            }
                        }
                    }
                }
            }
            // invert polygons
            if (!locks->IsObjectTypeLocked(GLO_POLYGON)) {
                for (const auto& polygon : mySelectorFrameParent->myViewNet->getNet()->getAttributeCarriers()->getShapes().at(SUMO_TAG_POLY)) {
                    if (polygon.second->isAttributeCarrierSelected()) {
                        polygon.second->setAttribute(GNE_ATTR_SELECTED, "false", undoList);
                    } else {
                        polygon.second->setAttribute(GNE_ATTR_SELECTED, "true", undoList);
                    }
                }
            }
            // invert TAZs
            if (!locks->IsObjectTypeLocked(GLO_TAZ)) {
                for (const auto& polygon : mySelectorFrameParent->myViewNet->getNet()->getAttributeCarriers()->getTAZElements().at(SUMO_TAG_TAZ)) {
                    if (polygon.second->isAttributeCarrierSelected()) {
                        polygon.second->setAttribute(GNE_ATTR_SELECTED, "false", undoList);
                    } else {
                        polygon.second->setAttribute(GNE_ATTR_SELECTED, "true", undoList);
                    }
                }
            }
            // invert POIs and POILanes
            if (!locks->IsObjectTypeLocked(GLO_POI)) {
                for (const auto& POI : mySelectorFrameParent->myViewNet->getNet()->getAttributeCarriers()->getShapes().at(SUMO_TAG_POI)) {
                    if (POI.second->isAttributeCarrierSelected()) {
                        POI.second->setAttribute(GNE_ATTR_SELECTED, "false", undoList);
                    } else {
                        POI.second->setAttribute(GNE_ATTR_SELECTED, "true", undoList);
                    }
                }
                for (const auto& POILane : mySelectorFrameParent->myViewNet->getNet()->getAttributeCarriers()->getShapes().at(SUMO_TAG_POILANE)) {
                    if (POILane.second->isAttributeCarrierSelected()) {
                        POILane.second->setAttribute(GNE_ATTR_SELECTED, "false", undoList);
                    } else {
                        POILane.second->setAttribute(GNE_ATTR_SELECTED, "true", undoList);
                    }
                }
            }
        } else if (mySelectorFrameParent->myViewNet->getEditModes().isCurrentSupermodeDemand()) {
            // invert routes
            if (!locks->IsObjectTypeLocked(GLO_ROUTE)) {
                for (const auto& route : mySelectorFrameParent->myViewNet->getNet()->getAttributeCarriers()->getDemandElements().at(SUMO_TAG_ROUTE)) {
                    if (route.second->isAttributeCarrierSelected()) {
                        route.second->setAttribute(GNE_ATTR_SELECTED, "false", undoList);
                    } else {
                        route.second->setAttribute(GNE_ATTR_SELECTED, "true", undoList);
                    }
                }
                for (const auto& embeddedRoute : mySelectorFrameParent->myViewNet->getNet()->getAttributeCarriers()->getDemandElements().at(GNE_TAG_ROUTE_EMBEDDED)) {
                    if (embeddedRoute.second->isAttributeCarrierSelected()) {
                        embeddedRoute.second->setAttribute(GNE_ATTR_SELECTED, "false", undoList);
                    } else {
                        embeddedRoute.second->setAttribute(GNE_ATTR_SELECTED, "true", undoList);
                    }
                }
            }
            // invert vehicles
            if (!locks->IsObjectTypeLocked(GLO_VEHICLE)) {
                for (const auto& vehicle : mySelectorFrameParent->myViewNet->getNet()->getAttributeCarriers()->getDemandElements().at(SUMO_TAG_VEHICLE)) {
                    if (vehicle.second->isAttributeCarrierSelected()) {
                        vehicle.second->setAttribute(GNE_ATTR_SELECTED, "false", undoList);
                    } else {
                        vehicle.second->setAttribute(GNE_ATTR_SELECTED, "true", undoList);
                    }
                }
                for (const auto& vehicle : mySelectorFrameParent->myViewNet->getNet()->getAttributeCarriers()->getDemandElements().at(GNE_TAG_VEHICLE_WITHROUTE)) {
                    if (vehicle.second->isAttributeCarrierSelected()) {
                        vehicle.second->setAttribute(GNE_ATTR_SELECTED, "false", undoList);
                    } else {
                        vehicle.second->setAttribute(GNE_ATTR_SELECTED, "true", undoList);
                    }
                }
            }
            // invert trips
            if (!locks->IsObjectTypeLocked(GLO_TRIP)) {
                for (const auto& trip : mySelectorFrameParent->myViewNet->getNet()->getAttributeCarriers()->getDemandElements().at(SUMO_TAG_TRIP)) {
                    if (trip.second->isAttributeCarrierSelected()) {
                        trip.second->setAttribute(GNE_ATTR_SELECTED, "false", undoList);
                    } else {
                        trip.second->setAttribute(GNE_ATTR_SELECTED, "true", undoList);
                    }
                }
            }
            // invert flows
            if (!locks->IsObjectTypeLocked(GLO_FLOW)) {
                for (const auto& flow : mySelectorFrameParent->myViewNet->getNet()->getAttributeCarriers()->getDemandElements().at(SUMO_TAG_FLOW)) {
                    if (flow.second->isAttributeCarrierSelected()) {
                        flow.second->setAttribute(GNE_ATTR_SELECTED, "false", undoList);
                    } else {
                        flow.second->setAttribute(GNE_ATTR_SELECTED, "true", undoList);
                    }
                }
            }
            // invert route flows
            if (!locks->IsObjectTypeLocked(GLO_ROUTEFLOW)) {
                for (const auto& routeFlow : mySelectorFrameParent->myViewNet->getNet()->getAttributeCarriers()->getDemandElements().at(GNE_TAG_FLOW_ROUTE)) {
                    if (routeFlow.second->isAttributeCarrierSelected()) {
                        routeFlow.second->setAttribute(GNE_ATTR_SELECTED, "false", undoList);
                    } else {
                        routeFlow.second->setAttribute(GNE_ATTR_SELECTED, "true", undoList);
                    }
                }
                for (const auto& routeFlow : mySelectorFrameParent->myViewNet->getNet()->getAttributeCarriers()->getDemandElements().at(GNE_TAG_FLOW_WITHROUTE)) {
                    if (routeFlow.second->isAttributeCarrierSelected()) {
                        routeFlow.second->setAttribute(GNE_ATTR_SELECTED, "false", undoList);
                    } else {
                        routeFlow.second->setAttribute(GNE_ATTR_SELECTED, "true", undoList);
                    }
                }
            }
            // invert stops
            if (!locks->IsObjectTypeLocked(GLO_STOP)) {
                for (const auto& demandElementTag : mySelectorFrameParent->myViewNet->getNet()->getAttributeCarriers()->getDemandElements()) {
                    for (const auto& demandElement : demandElementTag.second) {
                        for (const auto& stop : demandElement.second->getChildDemandElements()) {
                            if (stop->getTagProperty().isStop()) {
                                if (stop->isAttributeCarrierSelected()) {
                                    stop->setAttribute(GNE_ATTR_SELECTED, "false", undoList);
                                } else {
                                    stop->setAttribute(GNE_ATTR_SELECTED, "true", undoList);
                                }
                            }
                        }
                    }
                }
            }
            // invert person
            if (!locks->IsObjectTypeLocked(GLO_PERSON)) {
                for (const auto& person : mySelectorFrameParent->myViewNet->getNet()->getAttributeCarriers()->getDemandElements().at(SUMO_TAG_PERSON)) {
                    if (person.second->isAttributeCarrierSelected()) {
                        person.second->setAttribute(GNE_ATTR_SELECTED, "false", undoList);
                    } else {
                        person.second->setAttribute(GNE_ATTR_SELECTED, "true", undoList);
                    }
                }
            }
            // invert person flow
            if (!locks->IsObjectTypeLocked(GLO_PERSONFLOW)) {
                for (const auto& personFlow : mySelectorFrameParent->myViewNet->getNet()->getAttributeCarriers()->getDemandElements().at(SUMO_TAG_PERSONFLOW)) {
                    if (personFlow.second->isAttributeCarrierSelected()) {
                        personFlow.second->setAttribute(GNE_ATTR_SELECTED, "false", undoList);
                    } else {
                        personFlow.second->setAttribute(GNE_ATTR_SELECTED, "true", undoList);
                    }
                }
            }
            // invert person trip
            if (!locks->IsObjectTypeLocked(GLO_PERSONTRIP)) {
                for (const auto& personTripFromTo : mySelectorFrameParent->myViewNet->getNet()->getAttributeCarriers()->getDemandElements().at(GNE_TAG_PERSONTRIP_EDGE_EDGE)) {
                    if (personTripFromTo.second->isAttributeCarrierSelected()) {
                        personTripFromTo.second->setAttribute(GNE_ATTR_SELECTED, "false", undoList);
                    } else {
                        personTripFromTo.second->setAttribute(GNE_ATTR_SELECTED, "true", undoList);
                    }
                }
                for (const auto& personTripBusStop : mySelectorFrameParent->myViewNet->getNet()->getAttributeCarriers()->getDemandElements().at(GNE_TAG_PERSONTRIP_EDGE_BUSSTOP)) {
                    if (personTripBusStop.second->isAttributeCarrierSelected()) {
                        personTripBusStop.second->setAttribute(GNE_ATTR_SELECTED, "false", undoList);
                    } else {
                        personTripBusStop.second->setAttribute(GNE_ATTR_SELECTED, "true", undoList);
                    }
                }
            }
            // invert ride
            if (!locks->IsObjectTypeLocked(GLO_RIDE)) {
                for (const auto& rideFromTo : mySelectorFrameParent->myViewNet->getNet()->getAttributeCarriers()->getDemandElements().at(GNE_TAG_RIDE_EDGE_EDGE)) {
                    if (rideFromTo.second->isAttributeCarrierSelected()) {
                        rideFromTo.second->setAttribute(GNE_ATTR_SELECTED, "false", undoList);
                    } else {
                        rideFromTo.second->setAttribute(GNE_ATTR_SELECTED, "true", undoList);
                    }
                }
                for (const auto& rideBusStop : mySelectorFrameParent->myViewNet->getNet()->getAttributeCarriers()->getDemandElements().at(GNE_TAG_RIDE_EDGE_BUSSTOP)) {
                    if (rideBusStop.second->isAttributeCarrierSelected()) {
                        rideBusStop.second->setAttribute(GNE_ATTR_SELECTED, "false", undoList);
                    } else {
                        rideBusStop.second->setAttribute(GNE_ATTR_SELECTED, "true", undoList);
                    }
                }
            }
            // invert walks
            if (!locks->IsObjectTypeLocked(GLO_WALK)) {
                for (const auto& walkFromTo : mySelectorFrameParent->myViewNet->getNet()->getAttributeCarriers()->getDemandElements().at(GNE_TAG_WALK_EDGE_EDGE)) {
                    if (walkFromTo.second->isAttributeCarrierSelected()) {
                        walkFromTo.second->setAttribute(GNE_ATTR_SELECTED, "false", undoList);
                    } else {
                        walkFromTo.second->setAttribute(GNE_ATTR_SELECTED, "true", undoList);
                    }
                }
                for (const auto& walkBusStop : mySelectorFrameParent->myViewNet->getNet()->getAttributeCarriers()->getDemandElements().at(GNE_TAG_WALK_EDGE_BUSSTOP)) {
                    if (walkBusStop.second->isAttributeCarrierSelected()) {
                        walkBusStop.second->setAttribute(GNE_ATTR_SELECTED, "false", undoList);
                    } else {
                        walkBusStop.second->setAttribute(GNE_ATTR_SELECTED, "true", undoList);
                    }
                }
                for (const auto& walkRoute : mySelectorFrameParent->myViewNet->getNet()->getAttributeCarriers()->getDemandElements().at(GNE_TAG_WALK_ROUTE)) {
                    if (walkRoute.second->isAttributeCarrierSelected()) {
                        walkRoute.second->setAttribute(GNE_ATTR_SELECTED, "false", undoList);
                    } else {
                        walkRoute.second->setAttribute(GNE_ATTR_SELECTED, "true", undoList);
                    }
                }
            }
            // invert person stops
            if (!locks->IsObjectTypeLocked(GLO_PERSONSTOP)) {
                for (const auto& personStopLane : mySelectorFrameParent->myViewNet->getNet()->getAttributeCarriers()->getDemandElements().at(GNE_TAG_PERSONSTOP_EDGE)) {
                    if (personStopLane.second->isAttributeCarrierSelected()) {
                        personStopLane.second->setAttribute(GNE_ATTR_SELECTED, "false", undoList);
                    } else {
                        personStopLane.second->setAttribute(GNE_ATTR_SELECTED, "true", undoList);
                    }
                }
                for (const auto& personStopBusStop : mySelectorFrameParent->myViewNet->getNet()->getAttributeCarriers()->getDemandElements().at(GNE_TAG_PERSONSTOP_BUSSTOP)) {
                    if (personStopBusStop.second->isAttributeCarrierSelected()) {
                        personStopBusStop.second->setAttribute(GNE_ATTR_SELECTED, "false", undoList);
                    } else {
                        personStopBusStop.second->setAttribute(GNE_ATTR_SELECTED, "true", undoList);
                    }
                }
            }
        } else if (mySelectorFrameParent->myViewNet->getEditModes().isCurrentSupermodeData()) {
            // invert dataSets
            for (const auto& dataSet : mySelectorFrameParent->myViewNet->getNet()->getAttributeCarriers()->getDataSets()) {
                for (const auto& dataInterval : dataSet.second->getDataIntervalChildren()) {
                    for (const auto& genericData : dataInterval.second->getGenericDataChildren()) {
                        if ((!locks->IsObjectTypeLocked(GLO_EDGEDATA) && (genericData->getType() == GLO_EDGEDATA)) ||
                                (!locks->IsObjectTypeLocked(GLO_EDGERELDATA) && (genericData->getType() == GLO_EDGERELDATA))) {
                            if (genericData->isAttributeCarrierSelected()) {
                                genericData->setAttribute(GNE_ATTR_SELECTED, "false", undoList);
                            } else {
                                genericData->setAttribute(GNE_ATTR_SELECTED, "true", undoList);
                            }
                        }
                    }
                }
            }
        }
        // finish selection operation
        undoList->p_end();
    }
    return 1;
}

// ---------------------------------------------------------------------------
// GNESelectorFrame - methods
// ---------------------------------------------------------------------------

GNESelectorFrame::GNESelectorFrame(FXHorizontalFrame* horizontalFrameParent, GNEViewNet* viewNet) :
    GNEFrame(horizontalFrameParent, viewNet, "Selection") {
    // create selectedItems modul
    myLockGLObjectTypes = new LockGLObjectTypes(this);
    // create Modification Mode modul
    myModificationMode = new ModificationMode(this);
    // create ElementSet modul
    myElementSet = new ElementSet(this);
    // create MatchAttribute modul
    myMatchAttribute = new MatchAttribute(this);
    // create MatchGenericDataAttribute modul
    myMatchGenericDataAttribute = new MatchGenericDataAttribute(this);
    // create VisualScaling modul
    myVisualScaling = new VisualScaling(this);
    // create SelectionOperation modul
    mySelectionOperation = new SelectionOperation(this);
    // Create groupbox for information about selections
    FXGroupBox* selectionHintGroupBox = new FXGroupBox(myContentFrame, "Information", GUIDesignGroupBoxFrame);
    // Create Selection Hint
    new FXLabel(selectionHintGroupBox, " - Hold <SHIFT> for \n   rectangle selection.\n - Press <DEL> to\n   delete selected items.", nullptr, GUIDesignLabelFrameInformation);

}


GNESelectorFrame::~GNESelectorFrame() {}


void
GNESelectorFrame::show() {
    // show Type Entries depending of current supermode
    myLockGLObjectTypes->showTypeEntries();
    // refresh element set
    myElementSet->refreshElementSet();
    // Show frame
    GNEFrame::show();
}


void
GNESelectorFrame::hide() {
    // hide frame
    GNEFrame::hide();
}


void
GNESelectorFrame::clearCurrentSelection() const {
    // only continue if there is element for selecting
    if (ACsToSelected()) {
        // for invert selection, first clean current selection and next select elements of set "unselectedElements"
        myViewNet->getUndoList()->p_begin("invert selection");
        // invert selection of elements depending of current supermode
        if (myViewNet->getEditModes().isCurrentSupermodeNetwork()) {
            // iterate over junctions
            for (const auto& junction : myViewNet->getNet()->getAttributeCarriers()->getJunctions()) {
                // check if junction selection is locked
                if (!myLockGLObjectTypes->IsObjectTypeLocked(GLO_JUNCTION)) {
                    if (junction.second->isAttributeCarrierSelected()) {
                        junction.second->setAttribute(GNE_ATTR_SELECTED, "false", myViewNet->getUndoList());
                    }
                }
                // due we iterate over all junctions, only it's neccesary iterate over incoming edges
                for (const auto& edge : junction.second->getGNEIncomingEdges()) {
                    // check if edge selection is locked
                    if (!myLockGLObjectTypes->IsObjectTypeLocked(GLO_EDGE)) {
                        if (edge->isAttributeCarrierSelected()) {
                            edge->setAttribute(GNE_ATTR_SELECTED, "false", myViewNet->getUndoList());
                        }
                    }
                    // check if lane selection is locked
                    if (!myLockGLObjectTypes->IsObjectTypeLocked(GLO_LANE)) {
                        for (const auto& lane : edge->getLanes()) {
                            if (lane->isAttributeCarrierSelected()) {
                                lane->setAttribute(GNE_ATTR_SELECTED, "false", myViewNet->getUndoList());
                            }
                        }
                    }
                    // check if connection selection is locked
                    if (!myLockGLObjectTypes->IsObjectTypeLocked(GLO_CONNECTION)) {
                        for (const auto& connection : edge->getGNEConnections()) {
                            if (connection->isAttributeCarrierSelected()) {
                                connection->setAttribute(GNE_ATTR_SELECTED, "false", myViewNet->getUndoList());
                            }
                        }
                    }
                }
                // check if crossing selection is locked
                if (!myLockGLObjectTypes->IsObjectTypeLocked(GLO_CROSSING)) {
                    for (const auto& crossing : junction.second->getGNECrossings()) {
                        if (crossing->isAttributeCarrierSelected()) {
                            crossing->setAttribute(GNE_ATTR_SELECTED, "false", myViewNet->getUndoList());
                        }
                    }
                }
            }
            // check if additionals selection is locked
            if (!myLockGLObjectTypes->IsObjectTypeLocked(GLO_ADDITIONALELEMENT)) {
                for (const auto& additionals : myViewNet->getNet()->getAttributeCarriers()->getAdditionals()) {
                    // first check if additional is selectable
                    if (GNEAttributeCarrier::getTagProperties(additionals.first).isSelectable()) {
                        for (const auto& additional : additionals.second) {
                            if (additional.second->isAttributeCarrierSelected()) {
                                additional.second->setAttribute(GNE_ATTR_SELECTED, "false", myViewNet->getUndoList());
                            }
                            // now iterate over additional children
                            for (const auto& additionalChild : additional.second->getChildAdditionals()) {
                                // first check if additional child is selectable
                                if (additionalChild->getTagProperty().isSelectable() && additionalChild->isAttributeCarrierSelected()) {
                                    additionalChild->setAttribute(GNE_ATTR_SELECTED, "false", myViewNet->getUndoList());
                                }
                            }
                        }
                    }
                }
            }
            // unselect polygons
            if (!myLockGLObjectTypes->IsObjectTypeLocked(GLO_POLYGON)) {
                for (const auto& polygon : myViewNet->getNet()->getAttributeCarriers()->getShapes().at(SUMO_TAG_POLY)) {
                    if (polygon.second->isAttributeCarrierSelected()) {
                        polygon.second->setAttribute(GNE_ATTR_SELECTED, "false", myViewNet->getUndoList());
                    }
                }
            }
            // unselect TAZs
            if (!myLockGLObjectTypes->IsObjectTypeLocked(GLO_TAZ)) {
                for (const auto& polygon : myViewNet->getNet()->getAttributeCarriers()->getTAZElements().at(SUMO_TAG_TAZ)) {
                    if (polygon.second->isAttributeCarrierSelected()) {
                        polygon.second->setAttribute(GNE_ATTR_SELECTED, "false", myViewNet->getUndoList());
                    }
                }
            }
            // unselect POIs and POILanes
            if (!myLockGLObjectTypes->IsObjectTypeLocked(GLO_POI)) {
                for (const auto& POI : myViewNet->getNet()->getAttributeCarriers()->getShapes().at(SUMO_TAG_POI)) {
                    if (POI.second->isAttributeCarrierSelected()) {
                        POI.second->setAttribute(GNE_ATTR_SELECTED, "false", myViewNet->getUndoList());
                    }
                }
                for (const auto& POILane : myViewNet->getNet()->getAttributeCarriers()->getShapes().at(SUMO_TAG_POILANE)) {
                    if (POILane.second->isAttributeCarrierSelected()) {
                        POILane.second->setAttribute(GNE_ATTR_SELECTED, "false", myViewNet->getUndoList());
                    }
                }
            }
        } else if (myViewNet->getEditModes().isCurrentSupermodeDemand()) {
            // unselect routes
            if (!myLockGLObjectTypes->IsObjectTypeLocked(GLO_ROUTE)) {
                for (const auto& route : myViewNet->getNet()->getAttributeCarriers()->getDemandElements().at(SUMO_TAG_ROUTE)) {
                    if (route.second->isAttributeCarrierSelected()) {
                        route.second->setAttribute(GNE_ATTR_SELECTED, "false", myViewNet->getUndoList());
                    }
                }
            }
            // unselect vehicles
            if (!myLockGLObjectTypes->IsObjectTypeLocked(GLO_VEHICLE)) {
                for (const auto& vehicle : myViewNet->getNet()->getAttributeCarriers()->getDemandElements().at(SUMO_TAG_VEHICLE)) {
                    if (vehicle.second->isAttributeCarrierSelected()) {
                        vehicle.second->setAttribute(GNE_ATTR_SELECTED, "false", myViewNet->getUndoList());
                    }
                }
                for (const auto& vehicle : myViewNet->getNet()->getAttributeCarriers()->getDemandElements().at(GNE_TAG_VEHICLE_WITHROUTE)) {
                    if (vehicle.second->isAttributeCarrierSelected()) {
                        vehicle.second->setAttribute(GNE_ATTR_SELECTED, "false", myViewNet->getUndoList());
                    }
                    for (const auto& embeddedRoute : vehicle.second->getChildDemandElements()) {
                        if (embeddedRoute->isAttributeCarrierSelected()) {
                            embeddedRoute->setAttribute(GNE_ATTR_SELECTED, "false", myViewNet->getUndoList());
                        }
                    }
                }
            }
            // unselect trips
            if (!myLockGLObjectTypes->IsObjectTypeLocked(GLO_TRIP)) {
                for (const auto& trip : myViewNet->getNet()->getAttributeCarriers()->getDemandElements().at(SUMO_TAG_TRIP)) {
                    if (trip.second->isAttributeCarrierSelected()) {
                        trip.second->setAttribute(GNE_ATTR_SELECTED, "false", myViewNet->getUndoList());
                    }
                }
            }
            // unselect flows
            if (!myLockGLObjectTypes->IsObjectTypeLocked(GLO_FLOW)) {
                for (const auto& flow : myViewNet->getNet()->getAttributeCarriers()->getDemandElements().at(SUMO_TAG_FLOW)) {
                    if (flow.second->isAttributeCarrierSelected()) {
                        flow.second->setAttribute(GNE_ATTR_SELECTED, "false", myViewNet->getUndoList());
                    }
                }
            }
            // unselect route flows
            if (!myLockGLObjectTypes->IsObjectTypeLocked(GLO_ROUTEFLOW)) {
                for (const auto& routeFlow : myViewNet->getNet()->getAttributeCarriers()->getDemandElements().at(GNE_TAG_FLOW_ROUTE)) {
                    if (routeFlow.second->isAttributeCarrierSelected()) {
                        routeFlow.second->setAttribute(GNE_ATTR_SELECTED, "false", myViewNet->getUndoList());
                    }
                }
                for (const auto& routeFlow : myViewNet->getNet()->getAttributeCarriers()->getDemandElements().at(GNE_TAG_FLOW_WITHROUTE)) {
                    if (routeFlow.second->isAttributeCarrierSelected()) {
                        routeFlow.second->setAttribute(GNE_ATTR_SELECTED, "false", myViewNet->getUndoList());
                    }
                    for (const auto& embeddedRoute : routeFlow.second->getChildDemandElements()) {
                        if (embeddedRoute->isAttributeCarrierSelected()) {
                            embeddedRoute->setAttribute(GNE_ATTR_SELECTED, "false", myViewNet->getUndoList());
                        }
                    }
                }
            }
            // unselect stops
            if (!myLockGLObjectTypes->IsObjectTypeLocked(GLO_STOP)) {
                for (const auto& demandElementTag : myViewNet->getNet()->getAttributeCarriers()->getDemandElements()) {
                    for (const auto& demandElement : demandElementTag.second) {
                        for (const auto& stop : demandElement.second->getChildDemandElements()) {
                            if (stop->getTagProperty().isStop()) {
                                if (stop->isAttributeCarrierSelected()) {
                                    stop->setAttribute(GNE_ATTR_SELECTED, "false", myViewNet->getUndoList());
                                }
                            }
                        }
                    }
                }
            }
            // unselect person
            if (!myLockGLObjectTypes->IsObjectTypeLocked(GLO_PERSON)) {
                for (const auto& person : myViewNet->getNet()->getAttributeCarriers()->getDemandElements().at(SUMO_TAG_PERSON)) {
                    if (person.second->isAttributeCarrierSelected()) {
                        person.second->setAttribute(GNE_ATTR_SELECTED, "false", myViewNet->getUndoList());
                    }
                }
            }
            // unselect person flows
            if (!myLockGLObjectTypes->IsObjectTypeLocked(GLO_PERSONFLOW)) {
                for (const auto& personFlow : myViewNet->getNet()->getAttributeCarriers()->getDemandElements().at(SUMO_TAG_PERSONFLOW)) {
                    if (personFlow.second->isAttributeCarrierSelected()) {
                        personFlow.second->setAttribute(GNE_ATTR_SELECTED, "false", myViewNet->getUndoList());
                    }
                }
            }
            // unselect person trips
            if (!myLockGLObjectTypes->IsObjectTypeLocked(GLO_PERSONTRIP)) {
                for (const auto& personTripFromTo : myViewNet->getNet()->getAttributeCarriers()->getDemandElements().at(GNE_TAG_PERSONTRIP_EDGE_EDGE)) {
                    if (personTripFromTo.second->isAttributeCarrierSelected()) {
                        personTripFromTo.second->setAttribute(GNE_ATTR_SELECTED, "false", myViewNet->getUndoList());
                    }
                }
                for (const auto& personTripBusStop : myViewNet->getNet()->getAttributeCarriers()->getDemandElements().at(GNE_TAG_PERSONTRIP_EDGE_BUSSTOP)) {
                    if (personTripBusStop.second->isAttributeCarrierSelected()) {
                        personTripBusStop.second->setAttribute(GNE_ATTR_SELECTED, "false", myViewNet->getUndoList());
                    }
                }
            }
            // unselect ride
            if (!myLockGLObjectTypes->IsObjectTypeLocked(GLO_RIDE)) {
                for (const auto& rideFromTo : myViewNet->getNet()->getAttributeCarriers()->getDemandElements().at(GNE_TAG_RIDE_EDGE_EDGE)) {
                    if (rideFromTo.second->isAttributeCarrierSelected()) {
                        rideFromTo.second->setAttribute(GNE_ATTR_SELECTED, "false", myViewNet->getUndoList());
                    }
                }
                for (const auto& rideBusStop : myViewNet->getNet()->getAttributeCarriers()->getDemandElements().at(GNE_TAG_RIDE_EDGE_BUSSTOP)) {
                    if (rideBusStop.second->isAttributeCarrierSelected()) {
                        rideBusStop.second->setAttribute(GNE_ATTR_SELECTED, "false", myViewNet->getUndoList());
                    }
                }
            }
            // unselect walks
            if (!myLockGLObjectTypes->IsObjectTypeLocked(GLO_WALK)) {
                for (const auto& walkFromTo : myViewNet->getNet()->getAttributeCarriers()->getDemandElements().at(GNE_TAG_WALK_EDGE_EDGE)) {
                    if (walkFromTo.second->isAttributeCarrierSelected()) {
                        walkFromTo.second->setAttribute(GNE_ATTR_SELECTED, "false", myViewNet->getUndoList());
                    }
                }
                for (const auto& walkBusStop : myViewNet->getNet()->getAttributeCarriers()->getDemandElements().at(GNE_TAG_WALK_EDGE_BUSSTOP)) {
                    if (walkBusStop.second->isAttributeCarrierSelected()) {
                        walkBusStop.second->setAttribute(GNE_ATTR_SELECTED, "false", myViewNet->getUndoList());
                    }
                }
                for (const auto& walkRoute : myViewNet->getNet()->getAttributeCarriers()->getDemandElements().at(GNE_TAG_WALK_ROUTE)) {
                    if (walkRoute.second->isAttributeCarrierSelected()) {
                        walkRoute.second->setAttribute(GNE_ATTR_SELECTED, "false", myViewNet->getUndoList());
                    }
                }
            }
            // unselect person stops
            if (!myLockGLObjectTypes->IsObjectTypeLocked(GLO_PERSONSTOP)) {
                for (const auto& personStopLane : myViewNet->getNet()->getAttributeCarriers()->getDemandElements().at(GNE_TAG_PERSONSTOP_EDGE)) {
                    if (personStopLane.second->isAttributeCarrierSelected()) {
                        personStopLane.second->setAttribute(GNE_ATTR_SELECTED, "false", myViewNet->getUndoList());
                    }
                }
                for (const auto& personStopBusStop : myViewNet->getNet()->getAttributeCarriers()->getDemandElements().at(GNE_TAG_PERSONSTOP_BUSSTOP)) {
                    if (personStopBusStop.second->isAttributeCarrierSelected()) {
                        personStopBusStop.second->setAttribute(GNE_ATTR_SELECTED, "false", myViewNet->getUndoList());
                    }
                }
            }
        } else if (myViewNet->getEditModes().isCurrentSupermodeData()) {
            for (const auto& dataSet : myViewNet->getNet()->getAttributeCarriers()->getDataSets()) {
                for (const auto& dataInterval : dataSet.second->getDataIntervalChildren()) {
                    for (const auto& genericData : dataInterval.second->getGenericDataChildren()) {
                        if (genericData->isAttributeCarrierSelected()) {
                            genericData->setAttribute(GNE_ATTR_SELECTED, "false", myViewNet->getUndoList());
                        }
                    }
                }
            }
        }
        // finish selection operation
        myViewNet->getUndoList()->p_end();
    }
}


void
GNESelectorFrame::handleIDs(const std::vector<GNEAttributeCarrier*>& ACs, const ModificationMode::Operation setop) {
    // declare set operation
    const ModificationMode::Operation setOperation = ((setop == ModificationMode::Operation::DEFAULT) ? myModificationMode->getModificationMode() : setop);
    // declare two sets of attribute carriers, one for select and another for unselect
    std::set<std::pair<std::string, GNEAttributeCarrier*> > ACsToSelect, ACsToUnselect;
    // in restrict AND replace mode all current selected attribute carriers will be unselected
    if ((setOperation == ModificationMode::Operation::REPLACE) || (setOperation == ModificationMode::Operation::RESTRICT)) {
        // obtain selected ACs depending of current supermode
        std::vector<GNEAttributeCarrier*> selectedACs = myViewNet->getNet()->getSelectedAttributeCarriers(false);
        // add id into ACs to unselect
        for (const auto& selectedAC : selectedACs) {
            ACsToUnselect.insert(std::make_pair(selectedAC->getID(), selectedAC));
        }
    }
    // handle ids
    for (const auto& AC : ACs) {
        // iterate over AtributeCarriers an place it in ACsToSelect or ACsToUnselect
        switch (setOperation) {
            case GNESelectorFrame::ModificationMode::Operation::SUB:
                ACsToUnselect.insert(std::make_pair(AC->getID(), AC));
                break;
            case GNESelectorFrame::ModificationMode::Operation::RESTRICT:
                if (ACsToUnselect.find(std::make_pair(AC->getID(), AC)) != ACsToUnselect.end()) {
                    ACsToSelect.insert(std::make_pair(AC->getID(), AC));
                }
                break;
            default:
                ACsToSelect.insert(std::make_pair(AC->getID(), AC));
                break;
        }
    }
    // select junctions and their connections if Auto select junctions is enabled (note: only for "add mode")
    if (myViewNet->autoSelectNodes() && (setop == ModificationMode::Operation::ADD)) {
        std::vector<GNEEdge*> edgesToSelect;
        // iterate over ACsToSelect and extract edges
        for (const auto& AC : ACsToSelect) {
            if (AC.second->getTagProperty().getTag() == SUMO_TAG_EDGE) {
                edgesToSelect.push_back(myViewNet->getNet()->retrieveEdge(AC.second->getID()));
            }
        }
        // iterate over extracted edges
        for (const auto& edgeToSelect : edgesToSelect) {
            // select junction source and all connections and crossings
            ACsToSelect.insert(std::make_pair(edgeToSelect->getParentJunctions().front()->getID(), edgeToSelect->getParentJunctions().front()));
            for (const auto& connectionToSelect : edgeToSelect->getParentJunctions().front()->getGNEConnections()) {
                ACsToSelect.insert(std::make_pair(connectionToSelect->getID(), connectionToSelect));
            }
            for (const auto& crossingToSelect : edgeToSelect->getParentJunctions().front()->getGNECrossings()) {
                ACsToSelect.insert(std::make_pair(crossingToSelect->getID(), crossingToSelect));
            }
            // select junction destiny and all connections and crossings
            ACsToSelect.insert(std::make_pair(edgeToSelect->getParentJunctions().back()->getID(), edgeToSelect->getParentJunctions().back()));
            for (const auto& connectionToSelect : edgeToSelect->getParentJunctions().back()->getGNEConnections()) {
                ACsToSelect.insert(std::make_pair(connectionToSelect->getID(), connectionToSelect));
            }
            for (const auto& crossingToSelect : edgeToSelect->getParentJunctions().back()->getGNECrossings()) {
                ACsToSelect.insert(std::make_pair(crossingToSelect->getID(), crossingToSelect));
            }
        }
    }
    // only continue if there is ACs to select or unselect
    if ((ACsToSelect.size() + ACsToUnselect.size()) > 0) {
        // first unselect AC of ACsToUnselect and then selects AC of ACsToSelect
        myViewNet->getUndoList()->p_begin("selection");
        for (const auto& ACToUnselect : ACsToUnselect) {
            if (ACToUnselect.second->getTagProperty().isSelectable()) {
                ACToUnselect.second->setAttribute(GNE_ATTR_SELECTED, "false", myViewNet->getUndoList());
            }
        }
        for (const auto& ACToSelect : ACsToSelect) {
            if (ACToSelect.second->getTagProperty().isSelectable()) {
                ACToSelect.second->setAttribute(GNE_ATTR_SELECTED, "true", myViewNet->getUndoList());
            }
        }
        // finish operation
        myViewNet->getUndoList()->p_end();
    }
}


GNESelectorFrame::ModificationMode*
GNESelectorFrame::getModificationModeModul() const {
    return myModificationMode;
}


GNESelectorFrame::LockGLObjectTypes*
GNESelectorFrame::getLockGLObjectTypes() const {
    return myLockGLObjectTypes;
}


bool
GNESelectorFrame::ACsToSelected() const {
    if (myViewNet->getEditModes().isCurrentSupermodeNetwork()) {
        // check if exist junction and edges
        if (!myLockGLObjectTypes->IsObjectTypeLocked(GLO_JUNCTION) && (myViewNet->getNet()->getAttributeCarriers()->getJunctions().size() > 0)) {
            return true;
        }
        // check if exist connections
        if (!myLockGLObjectTypes->IsObjectTypeLocked(GLO_CONNECTION)) {
            for (const auto& junction : myViewNet->getNet()->getAttributeCarriers()->getJunctions()) {
                if (junction.second->getGNEConnections().size() > 0) {
                    return true;
                }
            }
        }
        // check if exist crossings
        if (!myLockGLObjectTypes->IsObjectTypeLocked(GLO_CROSSING)) {
            for (const auto& junction : myViewNet->getNet()->getAttributeCarriers()->getJunctions()) {
                if (junction.second->getGNECrossings().size() > 0) {
                    return true;
                }
            }
        }
        // check edges and lanes
        if ((!myLockGLObjectTypes->IsObjectTypeLocked(GLO_EDGE) || !myLockGLObjectTypes->IsObjectTypeLocked(GLO_LANE)) && (myViewNet->getNet()->getAttributeCarriers()->getEdges().size() > 0)) {
            return true;
        }
        // check if additionals selection is locked
        if (!myLockGLObjectTypes->IsObjectTypeLocked(GLO_ADDITIONALELEMENT)) {
            for (const auto& additionalTag : myViewNet->getNet()->getAttributeCarriers()->getAdditionals()) {
                // first check if additional is selectable
                if (GNEAttributeCarrier::getTagProperties(additionalTag.first).isSelectable() &&
                        (myViewNet->getNet()->getAttributeCarriers()->getAdditionals().at(additionalTag.first).size() > 0)) {
                    return true;
                }
            }
        }
        // check polygons
        if (!myLockGLObjectTypes->IsObjectTypeLocked(GLO_POLYGON) &&
                (myViewNet->getNet()->getAttributeCarriers()->getShapes().at(SUMO_TAG_POLY).size() > 0)) {
            return true;
        }
        // check POIs
        if (!myLockGLObjectTypes->IsObjectTypeLocked(GLO_POI) &&
                ((myViewNet->getNet()->getAttributeCarriers()->getShapes().at(SUMO_TAG_POI).size() > 0) ||
                 (myViewNet->getNet()->getAttributeCarriers()->getShapes().at(SUMO_TAG_POILANE).size() > 0))) {
            return true;
        }
    } else if (myViewNet->getEditModes().isCurrentSupermodeDemand()) {
        // get demand elements map
        const std::map<SumoXMLTag, std::map<std::string, GNEDemandElement*> >& demandElementsMap = myViewNet->getNet()->getAttributeCarriers()->getDemandElements();
        // check routes
        if (!myLockGLObjectTypes->IsObjectTypeLocked(GLO_ROUTE) && ((demandElementsMap.at(SUMO_TAG_ROUTE).size() + demandElementsMap.at(GNE_TAG_ROUTE_EMBEDDED).size()) > 0)) {
            return true;
        }
        // check vehicles
        if (!myLockGLObjectTypes->IsObjectTypeLocked(GLO_VEHICLE) && ((demandElementsMap.at(SUMO_TAG_VEHICLE).size() + demandElementsMap.at(GNE_TAG_VEHICLE_WITHROUTE).size()) > 0)) {
            return true;
        }
        // check trips
        if (!myLockGLObjectTypes->IsObjectTypeLocked(GLO_TRIP) && (demandElementsMap.at(SUMO_TAG_TRIP).size() > 0)) {
            return true;
        }
        // check flows
        if (!myLockGLObjectTypes->IsObjectTypeLocked(GLO_FLOW) && (demandElementsMap.at(SUMO_TAG_FLOW).size() > 0)) {
            return true;
        }
        // check route flows
        if (!myLockGLObjectTypes->IsObjectTypeLocked(GLO_ROUTEFLOW) && ((demandElementsMap.at(GNE_TAG_FLOW_ROUTE).size() + demandElementsMap.at(GNE_TAG_FLOW_WITHROUTE).size()) > 0)) {
            return true;
        }
        // check stops
        if (!myLockGLObjectTypes->IsObjectTypeLocked(GLO_STOP)) {
            for (const auto& demandElementTag : demandElementsMap) {
                for (const auto& demandElement : demandElementTag.second) {
                    for (const auto& stop : demandElement.second->getChildDemandElements()) {
                        if (stop->getTagProperty().isStop()) {
                            return true;
                        }
                    }
                }
            }
        }
        // check person
        if (!myLockGLObjectTypes->IsObjectTypeLocked(GLO_PERSON) && ((demandElementsMap.at(SUMO_TAG_PERSON).size()) > 0)) {
            return true;
        }
        // check person flows
        if (!myLockGLObjectTypes->IsObjectTypeLocked(GLO_PERSONFLOW) && (demandElementsMap.at(SUMO_TAG_PERSONFLOW).size() > 0)) {
            return true;
        }
        // check persontrips
        if (!myLockGLObjectTypes->IsObjectTypeLocked(GLO_PERSONTRIP)) {
            if ((demandElementsMap.at(GNE_TAG_PERSONTRIP_EDGE_EDGE).size() > 0) ||
                    (demandElementsMap.at(GNE_TAG_PERSONTRIP_EDGE_BUSSTOP).size() > 0)) {
                return true;
            }
        }
        // check ride
        if (!myLockGLObjectTypes->IsObjectTypeLocked(GLO_RIDE)) {
            if ((demandElementsMap.at(GNE_TAG_RIDE_EDGE_EDGE).size() > 0) ||
                    (demandElementsMap.at(GNE_TAG_RIDE_EDGE_BUSSTOP).size() > 0)) {
                return true;
            }
        }
        // check walks
        if (!myLockGLObjectTypes->IsObjectTypeLocked(GLO_WALK)) {
            if ((demandElementsMap.at(GNE_TAG_WALK_EDGE_EDGE).size() > 0) ||
                    (demandElementsMap.at(GNE_TAG_WALK_EDGE_BUSSTOP).size() > 0) ||
                    (demandElementsMap.at(GNE_TAG_WALK_ROUTE).size() > 0)) {
                return true;
            }
        }
        // check person stops
        if (!myLockGLObjectTypes->IsObjectTypeLocked(GLO_PERSONSTOP)) {
            if ((demandElementsMap.at(GNE_TAG_PERSONSTOP_EDGE).size() > 0) ||
                    (demandElementsMap.at(GNE_TAG_PERSONSTOP_BUSSTOP).size() > 0)) {
                return true;
            }
        }
    } else if (myViewNet->getEditModes().isCurrentSupermodeData()) {
        for (const auto& dataSet : myViewNet->getNet()->getAttributeCarriers()->getDataSets()) {
            for (const auto& dataInterval : dataSet.second->getDataIntervalChildren()) {
                for (const auto& genericData : dataInterval.second->getGenericDataChildren()) {
                    if ((!myLockGLObjectTypes->IsObjectTypeLocked(GLO_EDGEDATA) && (genericData->getType() == GLO_EDGEDATA)) ||
                            (!myLockGLObjectTypes->IsObjectTypeLocked(GLO_EDGERELDATA) && (genericData->getType() == GLO_EDGERELDATA))) {
                        return true;
                    }
                }
            }
        }
    }
    // nothing to select
    return false;
}


std::vector<GNEAttributeCarrier*>
GNESelectorFrame::getMatches(const SumoXMLTag ACTag, const SumoXMLAttr ACAttr, const char compOp, const double val, const std::string& expr) {
    std::vector<GNEAttributeCarrier*> result;
    // first retrieve all ACs using ACTag
    std::vector<GNEAttributeCarrier*> allACbyTag = myViewNet->getNet()->retrieveAttributeCarriers(ACTag);
    // get Tag value
    const auto& tagValue = GNEAttributeCarrier::getTagProperties(ACTag);
    // iterate over all ACs
    for (const auto& AC : allACbyTag) {
        if (expr == "") {
            result.push_back(AC);
        } else if (tagValue.hasAttribute(ACAttr) && tagValue.getAttributeProperties(ACAttr).isNumerical()) {
            double acVal;
            std::istringstream buf(AC->getAttribute(ACAttr));
            buf >> acVal;
            switch (compOp) {
                case '<':
                    if (acVal < val) {
                        result.push_back(AC);
                    }
                    break;
                case '>':
                    if (acVal > val) {
                        result.push_back(AC);
                    }
                    break;
                case '=':
                    if (acVal == val) {
                        result.push_back(AC);
                    }
                    break;
            }
        } else {
            // string match
            std::string acVal = AC->getAttributeForSelection(ACAttr);
            switch (compOp) {
                case '@':
                    if (acVal.find(expr) != std::string::npos) {
                        result.push_back(AC);
                    }
                    break;
                case '!':
                    if (acVal.find(expr) == std::string::npos) {
                        result.push_back(AC);
                    }
                    break;
                case '=':
                    if (acVal == expr) {
                        result.push_back(AC);
                    }
                    break;
                case '^':
                    if (acVal != expr) {
                        result.push_back(AC);
                    }
                    break;
            }
        }
    }
    return result;
}


std::vector<GNEAttributeCarrier*>
GNESelectorFrame::getGenericMatches(const std::vector<GNEGenericData*>& genericDatas, const std::string& attr, const char compOp, const double val, const std::string& expr) {
    std::vector<GNEAttributeCarrier*> result;
    // iterate over generic datas
    for (const auto& genericData : genericDatas) {
        if (expr == "") {
            result.push_back(genericData);
        } else if (attr != toString(GNE_ATTR_PARENT)) {
            double acVal;
            std::istringstream buf(genericData->getParameter(attr, "0"));
            buf >> acVal;
            switch (compOp) {
                case '<':
                    if (acVal < val) {
                        result.push_back(genericData);
                    }
                    break;
                case '>':
                    if (acVal > val) {
                        result.push_back(genericData);
                    }
                    break;
                case '=':
                    if (acVal == val) {
                        result.push_back(genericData);
                    }
                    break;
            }
        } else {
            // string match
            std::string acVal = genericData->getAttributeForSelection(GNE_ATTR_PARENT);
            switch (compOp) {
                case '@':
                    if (acVal.find(expr) != std::string::npos) {
                        result.push_back(genericData);
                    }
                    break;
                case '!':
                    if (acVal.find(expr) == std::string::npos) {
                        result.push_back(genericData);
                    }
                    break;
                case '=':
                    if (acVal == expr) {
                        result.push_back(genericData);
                    }
                    break;
                case '^':
                    if (acVal != expr) {
                        result.push_back(genericData);
                    }
                    break;
            }
        }
    }
    return result;
}


/****************************************************************************/
