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
/// @file    GNEFrameAttributesModuls.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Aug 2019
///
// Auxiliar class for GNEFrame Moduls (only for attributes edition)
/****************************************************************************/
#include <config.h>

#include <netedit/GNENet.h>
#include <netedit/GNEUndoList.h>
#include <netedit/GNEViewNet.h>
#include <netedit/elements/demand/GNERouteHandler.h>
#include <netedit/dialogs/GNEAllowDisallow.h>
#include <netedit/dialogs/GNEParametersDialog.h>
#include <netedit/elements/network/GNELane.h>
#include <utils/common/StringTokenizer.h>
#include <utils/gui/div/GUIDesigns.h>
#include <utils/gui/images/GUITexturesHelper.h>
#include <utils/gui/windows/GUIAppEnum.h>
#include <utils/options/OptionsCont.h>
#include <utils/vehicle/SUMOVehicleParameter.h>

#include "GNEFrame.h"
#include "GNEFrameAttributesModuls.h"


// ===========================================================================
// FOX callback mapping
// ===========================================================================

FXDEFMAP(GNEFrameAttributesModuls::AttributesCreatorRow) RowCreatorMap[] = {
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_SET_ATTRIBUTE,          GNEFrameAttributesModuls::AttributesCreatorRow::onCmdSetAttribute),
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_SET_ATTRIBUTE_BOOL,     GNEFrameAttributesModuls::AttributesCreatorRow::onCmdSelectCheckButton),
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_SET_ATTRIBUTE_DIALOG,   GNEFrameAttributesModuls::AttributesCreatorRow::onCmdSelectColorButton),
};

FXDEFMAP(GNEFrameAttributesModuls::AttributesCreator) AttributesCreatorMap[] = {
    FXMAPFUNC(SEL_COMMAND,  MID_HELP,   GNEFrameAttributesModuls::AttributesCreator::onCmdHelp)
};

FXDEFMAP(GNEFrameAttributesModuls::AttributesCreatorFlow) AttributesCreatorFlowMap[] = {
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_SET_ATTRIBUTE,          GNEFrameAttributesModuls::AttributesCreatorFlow::onCmdSetFlowAttribute),
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_SET_ATTRIBUTE_BUTTON,   GNEFrameAttributesModuls::AttributesCreatorFlow::onCmdSelectFlowRadioButton),
};

FXDEFMAP(GNEFrameAttributesModuls::AttributesEditorRow) AttributesEditorRowMap[] = {
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_SET_ATTRIBUTE,          GNEFrameAttributesModuls::AttributesEditorRow::onCmdSetAttribute),
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_SET_ATTRIBUTE_BOOL,     GNEFrameAttributesModuls::AttributesEditorRow::onCmdSelectCheckButton),
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_SET_ATTRIBUTE_DIALOG,   GNEFrameAttributesModuls::AttributesEditorRow::onCmdOpenAttributeDialog),
};

FXDEFMAP(GNEFrameAttributesModuls::AttributesEditor) AttributesEditorMap[] = {
    FXMAPFUNC(SEL_COMMAND,  MID_HELP,   GNEFrameAttributesModuls::AttributesEditor::onCmdAttributesEditorHelp)
};

FXDEFMAP(GNEFrameAttributesModuls::AttributesEditorFlow) AttributesEditorFlowMap[] = {
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_SET_ATTRIBUTE,          GNEFrameAttributesModuls::AttributesEditorFlow::onCmdSetFlowAttribute),
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_SET_ATTRIBUTE_BUTTON,   GNEFrameAttributesModuls::AttributesEditorFlow::onCmdSelectFlowRadioButton),
};

FXDEFMAP(GNEFrameAttributesModuls::AttributesEditorExtended) AttributesEditorExtendedMap[] = {
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_SET_ATTRIBUTE_DIALOG,   GNEFrameAttributesModuls::AttributesEditorExtended::onCmdOpenDialog)
};

FXDEFMAP(GNEFrameAttributesModuls::ParametersEditor) ParametersEditorMap[] = {
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_SET_ATTRIBUTE_DIALOG,   GNEFrameAttributesModuls::ParametersEditor::onCmdEditParameters),
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_SET_ATTRIBUTE,          GNEFrameAttributesModuls::ParametersEditor::onCmdSetParameters)
};

FXDEFMAP(GNEFrameAttributesModuls::DrawingShape) DrawingShapeMap[] = {
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_STARTDRAWING,   GNEFrameAttributesModuls::DrawingShape::onCmdStartDrawing),
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_STOPDRAWING,    GNEFrameAttributesModuls::DrawingShape::onCmdStopDrawing),
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_ABORTDRAWING,   GNEFrameAttributesModuls::DrawingShape::onCmdAbortDrawing)
};

FXDEFMAP(GNEFrameAttributesModuls::NeteditAttributes) NeteditAttributesMap[] = {
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_SET_ATTRIBUTE,  GNEFrameAttributesModuls::NeteditAttributes::onCmdSetNeteditAttribute),
    FXMAPFUNC(SEL_COMMAND,  MID_HELP,               GNEFrameAttributesModuls::NeteditAttributes::onCmdHelp)
};

// Object implementation
FXIMPLEMENT(GNEFrameAttributesModuls::AttributesCreatorRow,         FXHorizontalFrame,  RowCreatorMap,                  ARRAYNUMBER(RowCreatorMap))
FXIMPLEMENT(GNEFrameAttributesModuls::AttributesCreator,            FXGroupBox,         AttributesCreatorMap,           ARRAYNUMBER(AttributesCreatorMap))
FXIMPLEMENT(GNEFrameAttributesModuls::AttributesCreatorFlow,        FXGroupBox,         AttributesCreatorFlowMap,       ARRAYNUMBER(AttributesCreatorFlowMap))
FXIMPLEMENT(GNEFrameAttributesModuls::AttributesEditorRow,          FXHorizontalFrame,  AttributesEditorRowMap,         ARRAYNUMBER(AttributesEditorRowMap))
FXIMPLEMENT(GNEFrameAttributesModuls::AttributesEditor,             FXGroupBox,         AttributesEditorMap,            ARRAYNUMBER(AttributesEditorMap))
FXIMPLEMENT(GNEFrameAttributesModuls::AttributesEditorFlow,         FXGroupBox,         AttributesEditorFlowMap,        ARRAYNUMBER(AttributesEditorFlowMap))
FXIMPLEMENT(GNEFrameAttributesModuls::AttributesEditorExtended,     FXGroupBox,         AttributesEditorExtendedMap,    ARRAYNUMBER(AttributesEditorExtendedMap))
FXIMPLEMENT(GNEFrameAttributesModuls::ParametersEditor,             FXGroupBox,         ParametersEditorMap,            ARRAYNUMBER(ParametersEditorMap))
FXIMPLEMENT(GNEFrameAttributesModuls::DrawingShape,                 FXGroupBox,         DrawingShapeMap,                ARRAYNUMBER(DrawingShapeMap))
FXIMPLEMENT(GNEFrameAttributesModuls::NeteditAttributes,            FXGroupBox,         NeteditAttributesMap,           ARRAYNUMBER(NeteditAttributesMap))


// ===========================================================================
// method definitions
// ===========================================================================

// ---------------------------------------------------------------------------
// GNEFrameAttributesModuls::AttributesCreatorRow - methods
// ---------------------------------------------------------------------------

GNEFrameAttributesModuls::AttributesCreatorRow::AttributesCreatorRow(AttributesCreator* AttributesCreatorParent, const GNEAttributeProperties& attrProperties) :
    FXHorizontalFrame(AttributesCreatorParent, GUIDesignAuxiliarHorizontalFrame),
    myAttributesCreatorParent(AttributesCreatorParent),
    myAttrProperties(attrProperties) {
    // Create left visual elements
    myAttributeLabel = new FXLabel(this, "name", nullptr, GUIDesignLabelAttribute);
    myAttributeLabel->hide();
    myAttributeCheckButton = new FXCheckButton(this, "name", this, MID_GNE_SET_ATTRIBUTE_BOOL, GUIDesignCheckButtonAttribute);
    myAttributeCheckButton->hide();
    myAttributeColorButton = new FXButton(this, "ColorButton", nullptr, this, MID_GNE_SET_ATTRIBUTE_DIALOG, GUIDesignButtonAttribute);
    myAttributeColorButton->hide();
    // Create right visual elements
    myValueTextField = new FXTextField(this, GUIDesignTextFieldNCol, this, MID_GNE_SET_ATTRIBUTE, GUIDesignTextField);
    myValueTextField->hide();
    myValueCheckButton = new FXCheckButton(this, "Disabled", this, MID_GNE_SET_ATTRIBUTE, GUIDesignCheckButton);
    myValueCheckButton->hide();
    // by default attribute check button is true (except for until)
    if ((attrProperties.getTagPropertyParent().isStop() || attrProperties.getTagPropertyParent().isPersonStop()) && (attrProperties.getAttr() == SUMO_ATTR_UNTIL)) {
        myAttributeCheckButton->setCheck(FALSE);
    } else {
        myAttributeCheckButton->setCheck(TRUE);
    }
    // only create if parent was created
    if (getParent()->id()) {
        // create AttributesCreatorRow
        FXHorizontalFrame::create();
        // reset invalid value
        myInvalidValue = "";
        // special case for attribute ID
        if (attrProperties.getAttr() == SUMO_ATTR_ID) {
            // show check button and disable it
            myAttributeCheckButton->setText(myAttrProperties.getAttrStr().c_str());
            myAttributeCheckButton->setCheck(false);
            myAttributeCheckButton->show();
            // show text field and disable it
            myValueTextField->setTextColor(FXRGB(0, 0, 0));
            myValueTextField->disable();
            // generate ID
            myValueTextField->setText(generateID().c_str());
            // show textField
            myValueTextField->show();
        } else {
            // show label, button for edit colors or radio button
            if (myAttrProperties.isColor()) {
                myAttributeColorButton->setTextColor(FXRGB(0, 0, 0));
                myAttributeColorButton->setText(myAttrProperties.getAttrStr().c_str());
                myAttributeColorButton->show();
            } else if (myAttrProperties.isActivatable()) {
                myAttributeCheckButton->setText(myAttrProperties.getAttrStr().c_str());
                myAttributeCheckButton->show();
                // special case for attributes "Parking", "until" and "duration" (by default disabled)
                if ((myAttrProperties.getTagPropertyParent().isStop() || myAttrProperties.getTagPropertyParent().isPersonStop()) &&
                        (myAttrProperties.getAttr() == SUMO_ATTR_UNTIL || myAttrProperties.getAttr() == SUMO_ATTR_EXTENSION ||
                         myAttrProperties.getAttr() == SUMO_ATTR_PARKING)) {
                    myAttributeCheckButton->setCheck(FALSE);
                } else {
                    myAttributeCheckButton->setCheck(TRUE);
                }
            } else {
                myAttributeLabel->setText(myAttrProperties.getAttrStr().c_str());
                myAttributeLabel->show();
            }
            if (myAttrProperties.isBool()) {
                if (GNEAttributeCarrier::parse<bool>(attrProperties.getDefaultValue())) {
                    myValueCheckButton->setCheck(true);
                    myValueCheckButton->setText("true");
                } else {
                    myValueCheckButton->setCheck(false);
                    myValueCheckButton->setText("false");
                }
                myValueCheckButton->show();
                // if it's associated to a label button and is disabled, then disable myValueCheckButton
                if (myAttributeCheckButton->shown() && (myAttributeCheckButton->getCheck() == FALSE)) {
                    myValueCheckButton->disable();
                }
            } else {
                myValueTextField->setTextColor(FXRGB(0, 0, 0));
                myValueTextField->setText(attrProperties.getDefaultValue().c_str());
                myValueTextField->show();
                // if it's associated to a label button and is disabled, then disable myValueTextField
                if (myAttributeCheckButton->shown() && (myAttributeCheckButton->getCheck() == FALSE)) {
                    myValueTextField->disable();
                }
            }
        }
        // show AttributesCreatorRow
        show();
    }
}


void
GNEFrameAttributesModuls::AttributesCreatorRow::destroy() {
    // only destroy if parent was created
    if (getParent()->id()) {
        FXHorizontalFrame::destroy();
    }
}


const GNEAttributeProperties&
GNEFrameAttributesModuls::AttributesCreatorRow::getAttrProperties() const {
    return myAttrProperties;
}


std::string
GNEFrameAttributesModuls::AttributesCreatorRow::getValue() const {
    if (myAttrProperties.isBool()) {
        return (myValueCheckButton->getCheck() == 1) ? "1" : "0";
    } else {
        return myValueTextField->getText().text();
    }
}


bool
GNEFrameAttributesModuls::AttributesCreatorRow::getAttributeCheckButtonCheck() const {
    if (shown()) {
        return myAttributeCheckButton->getCheck() == TRUE;
    } else {
        return false;
    }
}


void
GNEFrameAttributesModuls::AttributesCreatorRow::setAttributeCheckButtonCheck(bool value) {
    if (shown()) {
        // set radio button
        myAttributeCheckButton->setCheck(value);
        // enable or disable input fields
        if (value) {
            if (myAttrProperties.isBool()) {
                myValueCheckButton->enable();
            } else {
                myValueTextField->enable();
            }
        } else {
            if (myAttrProperties.isBool()) {
                myValueCheckButton->disable();
            } else {
                myValueTextField->disable();
            }
        }
    }
}


void
GNEFrameAttributesModuls::AttributesCreatorRow::enableAttributesCreatorRow() {
    if (myAttrProperties.isBool()) {
        return myValueCheckButton->enable();
    } else {
        return myValueTextField->enable();
    }
}


void
GNEFrameAttributesModuls::AttributesCreatorRow::disableAttributesCreatorRow() {
    if (myAttrProperties.isBool()) {
        return myValueCheckButton->disable();
    } else {
        return myValueTextField->disable();
    }
}


bool
GNEFrameAttributesModuls::AttributesCreatorRow::isAttributesCreatorRowEnabled() const {
    if (!shown()) {
        return false;
    } else if (myAttrProperties.isBool()) {
        return myValueCheckButton->isEnabled();
    } else {
        return myValueTextField->isEnabled();
    }
}


void
GNEFrameAttributesModuls::AttributesCreatorRow::refreshRow() const {
    // currently only row with ID attribute must be updated
    if (myAttrProperties.getAttr() == SUMO_ATTR_ID) {
        myValueTextField->setText(generateID().c_str());
    }
}


const std::string&
GNEFrameAttributesModuls::AttributesCreatorRow::isAttributeValid() const {
    return myInvalidValue;
}


GNEFrameAttributesModuls::AttributesCreator*
GNEFrameAttributesModuls::AttributesCreatorRow::getAttributesCreatorParent() const {
    return myAttributesCreatorParent;
}


long
GNEFrameAttributesModuls::AttributesCreatorRow::onCmdSetAttribute(FXObject* obj, FXSelector, void*) {
    // We assume that current value is valid
    myInvalidValue = "";
    // Check if format of current value of myTextField is correct
    if (obj == myValueCheckButton) {
        if (myValueCheckButton->getCheck()) {
            myValueCheckButton->setText("true");
        } else {
            myValueCheckButton->setText("false");
        }
    } else if (myAttrProperties.isComplex()) {
        // check complex attribute
        myInvalidValue = checkComplexAttribute(myValueTextField->getText().text());
    } else if (myAttrProperties.isInt()) {
        // first filter int attributes
        if (GNEAttributeCarrier::canParse<double>(myValueTextField->getText().text())) {
            // obtain parsed attribute
            double parsedDouble = GNEAttributeCarrier::parse<double>(myValueTextField->getText().text());
            // check if parsed double doesn't have decimal part (or is 0)
            if ((parsedDouble - (int)parsedDouble) == 0) {
                myValueTextField->setText(toString((int)parsedDouble).c_str(), FALSE);
                // Check if int value must be positive
                if (myAttrProperties.isPositive() && (parsedDouble < 0)) {
                    myInvalidValue = "'" + myAttrProperties.getAttrStr() + "' cannot be negative";
                }
            } else {
                myInvalidValue = "'" + myAttrProperties.getAttrStr() + "' doesn't have a valid 'int' format";
            }
        } else {
            myInvalidValue = "'" + myAttrProperties.getAttrStr() + "' doesn't have a valid 'int' format";
        }

    } else if (myAttrProperties.getAttr() == SUMO_ATTR_ANGLE) {
        if (GNEAttributeCarrier::canParse<double>(myValueTextField->getText().text())) {
            // filter angle
            double angle = GNEAttributeCarrier::parse<double>(myValueTextField->getText().text());
            // filter if angle isn't between [0,360]
            if ((angle < 0) || (angle > 360)) {
                // apply modul
                angle = fmod(angle, 360);
            }
            // update Textfield
            myValueTextField->setText(toString(angle).c_str(), FALSE);
        } else {
            myInvalidValue = "'" + myAttrProperties.getAttrStr() + "' doesn't have a valid 'float' format between [0, 360]";
        }
    } else if (myAttrProperties.isSUMOTime()) {
        // time attributes work as positive doubles
        if (!GNEAttributeCarrier::canParse<SUMOTime>(myValueTextField->getText().text())) {
            myInvalidValue = "'" + myAttrProperties.getAttrStr() + "' doesn't have a valid SUMOTime format";
        }
    } else if (myAttrProperties.isFloat()) {
        if (GNEAttributeCarrier::canParse<double>(myValueTextField->getText().text())) {
            // convert string to double
            double doubleValue = GNEAttributeCarrier::parse<double>(myValueTextField->getText().text());
            // Check if double value must be positive
            if (myAttrProperties.isPositive() && (doubleValue < 0)) {
                myInvalidValue = "'" + myAttrProperties.getAttrStr() + "' cannot be negative";
                // check if double value is a probability
            } else if (myAttrProperties.isProbability() && ((doubleValue < 0) || doubleValue > 1)) {
                myInvalidValue = "'" + myAttrProperties.getAttrStr() + "' takes only values between 0 and 1";
            } else if (myAttrProperties.hasAttrRange() && ((doubleValue < myAttrProperties.getMinimumRange()) || doubleValue > myAttrProperties.getMaximumRange())) {
                myInvalidValue = "'" + myAttrProperties.getAttrStr() + "' takes only values between " + toString(myAttrProperties.getMinimumRange()) + " and " + toString(myAttrProperties.getMaximumRange());
            } else if ((myAttributesCreatorParent->getCurrentTagProperties().getTag() == SUMO_TAG_E2DETECTOR) && (myAttrProperties.getAttr() == SUMO_ATTR_LENGTH) && (doubleValue == 0)) {
                myInvalidValue = "E2 length cannot be 0";
            }
        } else {
            myInvalidValue = "'" + myAttrProperties.getAttrStr() + "' doesn't have a valid 'float' format";
        }
    } else if (myAttrProperties.isColor()) {
        // check if filename format is valid
        if (GNEAttributeCarrier::canParse<RGBColor>(myValueTextField->getText().text()) == false) {
            myInvalidValue = "'" + myAttrProperties.getAttrStr() + "' doesn't have a valid 'RBGColor' format";
        }
    } else if (myAttrProperties.isFilename()) {
        const std::string file = myValueTextField->getText().text();
        // check if filename format is valid
        if (SUMOXMLDefinitions::isValidFilename(file) == false) {
            myInvalidValue = "input contains invalid characters for a filename";
        } else if (myAttrProperties.getAttr() == SUMO_ATTR_IMGFILE) {
            if (!file.empty()) {
                // only load value if file exist and can be loaded
                if (GUITexturesHelper::getTextureID(file) == -1) {
                    myInvalidValue = "doesn't exist image '" + file + "'";
                }
            }
        }
    } else if (myAttrProperties.getAttr() == SUMO_ATTR_NAME) {
        const std::string name = myValueTextField->getText().text();
        // check if name format is valid
        if (SUMOXMLDefinitions::isValidAttribute(name) == false) {
            myInvalidValue = "input contains invalid characters";
        }
    } else if (myAttrProperties.getAttr() == SUMO_ATTR_VTYPES) {
        const std::string types = myValueTextField->getText().text();
        // if list of VTypes isn't empty, check that all characters are valid
        if (!types.empty() && !SUMOXMLDefinitions::isValidListOfTypeID(types)) {
            myInvalidValue = "list of IDs contains invalid characters";
        }
    } else if (myAttrProperties.getAttr() == SUMO_ATTR_INDEX) {
        // special case for stop indx
        const std::string index = myValueTextField->getText().text();
        if ((index != "fit") && (index != "end") && !GNEAttributeCarrier::canParse<int>(index)) {
            myInvalidValue = "index isn't either 'fit' or 'end' or a valid positive int";
        } else if (GNEAttributeCarrier::canParse<int>(index) && (GNEAttributeCarrier::parse<int>(index) < 0)) {
            myInvalidValue = "index cannot be negative";
        }
    } else if ((myAttrProperties.getAttr() == SUMO_ATTR_EXPECTED) || (myAttrProperties.getAttr() == SUMO_ATTR_EXPECTED_CONTAINERS)) {
        // check if attribute can be parsed in a list of Ids
        std::vector<std::string> vehicleIDs = GNEAttributeCarrier::parse<std::vector<std::string> >(myValueTextField->getText().text());
        // check every ID
        for (const auto& i : vehicleIDs) {
            if (!SUMOXMLDefinitions::isValidVehicleID(i)) {
                myInvalidValue = "invalid id used in " + myAttrProperties.getAttrStr();
            }
        }
    } else if (myAttrProperties.getAttr() == SUMO_ATTR_TRIP_ID) {
        if (!SUMOXMLDefinitions::isValidVehicleID(myValueTextField->getText().text())) {
            myInvalidValue = "invalid id used in " + myAttrProperties.getAttrStr();
        }
    } else if (myAttrProperties.getAttr() == SUMO_ATTR_ID) {
        // check ID depending of tag
        if (myAttrProperties.getTagPropertyParent().isNetworkElement() && !SUMOXMLDefinitions::isValidNetID(myValueTextField->getText().text())) {
            myInvalidValue = "invalid id used in " + myAttrProperties.getAttrStr();
        } else if (myAttrProperties.getTagPropertyParent().isDetector() && !SUMOXMLDefinitions::isValidDetectorID(myValueTextField->getText().text())) {
            myInvalidValue = "invalid id used in " + myAttrProperties.getAttrStr();
        } else if (myAttrProperties.getTagPropertyParent().isAdditionalElement() &&  !SUMOXMLDefinitions::isValidNetID(myValueTextField->getText().text())) {
            myInvalidValue = "invalid id used in " + myAttrProperties.getAttrStr();
        } else if (myAttrProperties.getTagPropertyParent().isShape() &&  !SUMOXMLDefinitions::isValidTypeID(myValueTextField->getText().text())) {
            myInvalidValue = "invalid id used in " + myAttrProperties.getAttrStr();
        } else if (myAttrProperties.getTagPropertyParent().isDemandElement() && !SUMOXMLDefinitions::isValidVehicleID(myValueTextField->getText().text())) {
            myInvalidValue = "invalid id used in " + myAttrProperties.getAttrStr();
        }
    } else if ((myAttrProperties.getAttr() == SUMO_ATTR_FREQUENCY) && myAttrProperties.getTagPropertyParent().isDetector()) {
        if (!myValueTextField->getText().empty()) {
            if (!GNEAttributeCarrier::canParse<double>(myValueTextField->getText().text())) {
                myInvalidValue = "'" + myAttrProperties.getAttrStr() + "' doesn't have a valid 'float' or empty format";
            } else if (GNEAttributeCarrier::parse<double>(myValueTextField->getText().text()) < 0) {
                myInvalidValue = "'" + myAttrProperties.getAttrStr() + "' cannot be negative";
            }
        }
    }
    // change color of text field depending of myCurrentValueValid
    if (myInvalidValue.size() == 0) {
        myValueTextField->setTextColor(FXRGB(0, 0, 0));
        myValueTextField->killFocus();
    } else {
        // IF value of TextField isn't valid, change their color to Red
        myValueTextField->setTextColor(FXRGB(255, 0, 0));
    }
    // Update aditional frame
    update();
    return 1;
}


long
GNEFrameAttributesModuls::AttributesCreatorRow::onCmdSelectCheckButton(FXObject*, FXSelector, void*) {
    if (myAttributeCheckButton->getCheck()) {
        // enable input values
        myValueCheckButton->enable();
        myValueTextField->enable();
        // refresh row
        refreshRow();
    } else {
        // disable input values
        myValueCheckButton->disable();
        myValueTextField->disable();
    }
    return 0;
}


long
GNEFrameAttributesModuls::AttributesCreatorRow::onCmdSelectColorButton(FXObject*, FXSelector, void*) {
    // create FXColorDialog
    FXColorDialog colordialog(this, tr("Color Dialog"));
    colordialog.setTarget(this);
    // If previous attribute wasn't correct, set black as default color
    if (GNEAttributeCarrier::canParse<RGBColor>(myValueTextField->getText().text())) {
        colordialog.setRGBA(MFXUtils::getFXColor(RGBColor::parseColor(myValueTextField->getText().text())));
    } else {
        colordialog.setRGBA(MFXUtils::getFXColor(RGBColor::parseColor(myAttrProperties.getDefaultValue())));
    }
    // execute dialog to get a new color
    if (colordialog.execute()) {
        myValueTextField->setText(toString(MFXUtils::getRGBColor(colordialog.getRGBA())).c_str());
        onCmdSetAttribute(nullptr, 0, nullptr);
    }
    return 0;
}


std::string
GNEFrameAttributesModuls::AttributesCreatorRow::checkComplexAttribute(const std::string& value) {
    // declare values needed to check if given complex parameters are valid
    std::string errorMessage;
    DepartDefinition dd;
    DepartLaneDefinition dld;
    DepartPosDefinition dpd;
    DepartPosLatDefinition dpld;
    ArrivalLaneDefinition ald;
    DepartSpeedDefinition dsd;
    ArrivalPosDefinition apd;
    ArrivalPosLatDefinition apld;
    ArrivalSpeedDefinition asd;
    SVCPermissions mode;
    int valueInt;
    double valueDouble;
    SUMOTime valueSUMOTime;
    // check complex attribute
    switch (myAttrProperties.getAttr()) {
        case SUMO_ATTR_DEPART:
        case SUMO_ATTR_BEGIN:
            SUMOVehicleParameter::parseDepart(value, myAttrProperties.getAttrStr(), "", valueSUMOTime, dd, errorMessage);
            break;
        case SUMO_ATTR_DEPARTLANE:
            SUMOVehicleParameter::parseDepartLane(value, myAttrProperties.getAttrStr(), "", valueInt, dld, errorMessage);
            break;
        case SUMO_ATTR_DEPARTPOS:
            SUMOVehicleParameter::parseDepartPos(value, myAttrProperties.getAttrStr(), "", valueDouble, dpd, errorMessage);
            break;
        case SUMO_ATTR_DEPARTSPEED:
            SUMOVehicleParameter::parseDepartSpeed(value, myAttrProperties.getAttrStr(), "", valueDouble, dsd, errorMessage);
            break;
        case SUMO_ATTR_ARRIVALLANE:
            SUMOVehicleParameter::parseArrivalLane(value, myAttrProperties.getAttrStr(), "", valueInt, ald, errorMessage);
            break;
        case SUMO_ATTR_ARRIVALPOS:
            SUMOVehicleParameter::parseArrivalPos(value, myAttrProperties.getAttrStr(), "", valueDouble, apd, errorMessage);
            break;
        case SUMO_ATTR_ARRIVALSPEED:
            SUMOVehicleParameter::parseArrivalSpeed(value, myAttrProperties.getAttrStr(), "", valueDouble, asd, errorMessage);
            break;
        case SUMO_ATTR_DEPARTPOS_LAT:
            SUMOVehicleParameter::parseDepartPosLat(value, myAttrProperties.getAttrStr(), "", valueDouble, dpld, errorMessage);
            break;
        case SUMO_ATTR_ARRIVALPOS_LAT:
            SUMOVehicleParameter::parseArrivalPosLat(value, myAttrProperties.getAttrStr(), "", valueDouble, apld, errorMessage);
            break;
        case SUMO_ATTR_MODES:
            SUMOVehicleParameter::parsePersonModes(value, myAttrProperties.getAttrStr(), "", mode, errorMessage);
            break;
        default:
            throw ProcessError("Invalid complex attribute");
    }
    // return error message (Will be empty if value is valid)
    return errorMessage;
}


std::string
GNEFrameAttributesModuls::AttributesCreatorRow::generateID() const {
    if (myAttrProperties.getTagPropertyParent().isShape()) {
        return myAttributesCreatorParent->getFrameParent()->getViewNet()->getNet()->generateShapeID(myAttrProperties.getTagPropertyParent().getTag());
    } else if (myAttrProperties.getTagPropertyParent().isAdditionalElement()) {
        return myAttributesCreatorParent->getFrameParent()->getViewNet()->getNet()->generateAdditionalID(myAttrProperties.getTagPropertyParent().getTag());
    } else if (myAttrProperties.getTagPropertyParent().isDemandElement()) {
        return myAttributesCreatorParent->getFrameParent()->getViewNet()->getNet()->generateDemandElementID(myAttrProperties.getTagPropertyParent().getTag());
    } else {
        return "";
    }
}


bool
GNEFrameAttributesModuls::AttributesCreatorRow::isValidID() const {
    return (myAttributesCreatorParent->getFrameParent()->getViewNet()->getNet()->retrieveAdditional(
                myAttrProperties.getTagPropertyParent().getTag(),
                myValueTextField->getText().text(), false) == nullptr);
}

// ---------------------------------------------------------------------------
// GNEFrameAttributesModuls::AttributesCreator - methods
// ---------------------------------------------------------------------------

GNEFrameAttributesModuls::AttributesCreator::AttributesCreator(GNEFrame* frameParent) :
    FXGroupBox(frameParent->myContentFrame, "Internal attributes", GUIDesignGroupBoxFrame),
    myFrameParent(frameParent) {
    // resize myAttributesCreatorRows
    myAttributesCreatorRows.resize(GNEAttributeCarrier::MAXNUMBEROFATTRIBUTES, nullptr);
    // create myAttributesCreatorFlow
    myAttributesCreatorFlow = new AttributesCreatorFlow(this);
    // create help button
    myHelpButton = new FXButton(this, "Help", nullptr, this, MID_HELP, GUIDesignButtonRectangular);
}


GNEFrameAttributesModuls::AttributesCreator::~AttributesCreator() {}


void
GNEFrameAttributesModuls::AttributesCreator::showAttributesCreatorModul(const GNETagProperties& tagProperties, const std::vector<SumoXMLAttr>& hiddenAttributes) {
    // set current tag Properties
    myTagProperties = tagProperties;
    // first destroy all rows
    for (int i = 0; i < (int)myAttributesCreatorRows.size(); i++) {
        // destroy and delete all rows
        if (myAttributesCreatorRows.at(i) != nullptr) {
            myAttributesCreatorRows.at(i)->destroy();
            delete myAttributesCreatorRows.at(i);
            myAttributesCreatorRows.at(i) = nullptr;
        }
    }
    // now declare a flag to show Flow editor
    bool showFlowEditor = false;
    // iterate over tag attributes and create AttributesCreatorRows for every attribute
    for (const auto& i : myTagProperties) {
        // declare falg to check conditions for show attribute
        bool showAttribute = true;
        // check that only non-unique attributes (except ID) are created (And depending of includeExtendedAttributes)
        if (i.isUnique() && (i.getAttr() != SUMO_ATTR_ID)) {
            showAttribute = false;
        }
        // check if attribute must stay hidden
        if (std::find(hiddenAttributes.begin(), hiddenAttributes.end(), i.getAttr()) != hiddenAttributes.end()) {
            showAttribute = false;
        }
        // check if attribute is a flow definitionattribute
        if (i.isFlowDefinition()) {
            showAttribute = false;
            showFlowEditor = true;
        }
        // check special case for vaporizer IDs
        if ((i.getAttr() == SUMO_ATTR_ID) && (i.getTagPropertyParent().getTag() == SUMO_TAG_VAPORIZER)) {
            showAttribute = false;
        }
        // check special case for VType IDs in vehicle Frame
        if ((i.getAttr() == SUMO_ATTR_TYPE) && (myFrameParent->getViewNet()->getEditModes().isCurrentSupermodeDemand()) &&
                (myFrameParent->getViewNet()->getEditModes().demandEditMode == DemandEditMode::DEMAND_VEHICLE)) {
            showAttribute = false;
        }
        // show attribute depending of showAttribute flag
        if (showAttribute) {
            myAttributesCreatorRows.at(i.getPositionListed()) = new AttributesCreatorRow(this, i);
        }
    }
    // reparent help button (to place it at bottom)
    myHelpButton->reparent(this);
    // recalc
    recalc();
    // check if flow editor has to be shown
    if (showFlowEditor) {
        myAttributesCreatorFlow->showAttributesCreatorFlowModul(tagProperties.hasAttribute(SUMO_ATTR_PERSONSPERHOUR));
    } else {
        myAttributesCreatorFlow->hideAttributesCreatorFlowModul();
    }
    // show
    show();
}


void
GNEFrameAttributesModuls::AttributesCreator::hideAttributesCreatorModul() {
    // hide attributes creator flow
    myAttributesCreatorFlow->hideAttributesCreatorFlowModul();
    // hide modul
    hide();
}


GNEFrame*
GNEFrameAttributesModuls::AttributesCreator::getFrameParent() const {
    return myFrameParent;
}


std::map<SumoXMLAttr, std::string>
GNEFrameAttributesModuls::AttributesCreator::getAttributesAndValues(bool includeAll) const {
    std::map<SumoXMLAttr, std::string> values;
    // get standard parameters
    for (int i = 0; i < (int)myAttributesCreatorRows.size(); i++) {
        if (myAttributesCreatorRows.at(i) && myAttributesCreatorRows.at(i)->getAttrProperties().getAttr() != SUMO_ATTR_NOTHING) {
            // flag for row enabled
            bool rowEnabled = myAttributesCreatorRows.at(i)->isAttributesCreatorRowEnabled();
            // flag for default attributes
            bool hasDefaultStaticValue = !myAttributesCreatorRows.at(i)->getAttrProperties().hasStaticDefaultValue() || (myAttributesCreatorRows.at(i)->getAttrProperties().getDefaultValue() != myAttributesCreatorRows.at(i)->getValue());
            // flag for enablitables attributes
            bool isFlowDefinitionAttribute = myAttributesCreatorRows.at(i)->getAttrProperties().isFlowDefinition();
            // flag for optional attributes
            bool isActivatableAttribute = myAttributesCreatorRows.at(i)->getAttrProperties().isActivatable() && myAttributesCreatorRows.at(i)->getAttributeCheckButtonCheck();
            // check if flags configuration allow to include values
            if (rowEnabled && (includeAll || hasDefaultStaticValue || isFlowDefinitionAttribute || isActivatableAttribute)) {
                values[myAttributesCreatorRows.at(i)->getAttrProperties().getAttr()] = myAttributesCreatorRows.at(i)->getValue();
            }
        }
    }
    // add extra flow attributes (only will updated if myAttributesCreatorFlow is shown)
    myAttributesCreatorFlow->setFlowParameters(values);
    // return values
    return values;
}


GNETagProperties
GNEFrameAttributesModuls::AttributesCreator::getCurrentTagProperties() const {
    return myTagProperties;
}


void
GNEFrameAttributesModuls::AttributesCreator::showWarningMessage(std::string extra) const {
    std::string errorMessage;
    // iterate over standar parameters
    for (const auto& i : myTagProperties) {
        if (errorMessage.empty() && myAttributesCreatorRows.at(i.getPositionListed())) {
            // Return string with the error if at least one of the parameter isn't valid
            std::string attributeValue = myAttributesCreatorRows.at(i.getPositionListed())->isAttributeValid();
            if (attributeValue.size() != 0) {
                errorMessage = attributeValue;
            }
        }
    }
    // show warning box if input parameters aren't invalid
    if (extra.size() == 0) {
        errorMessage = "Invalid input parameter of " + myTagProperties.getTagStr() + ": " + errorMessage;
    } else {
        errorMessage = "Invalid input parameter of " + myTagProperties.getTagStr() + ": " + extra;
    }

    // set message in status bar
    myFrameParent->myViewNet->setStatusBarText(errorMessage);
    // Write Warning in console if we're in testing mode
    WRITE_DEBUG(errorMessage);
}


bool
GNEFrameAttributesModuls::AttributesCreator::areValuesValid() const {
    // iterate over standar parameters
    for (auto i : myTagProperties) {
        // Return false if error message of attriuve isn't empty
        if (myAttributesCreatorRows.at(i.getPositionListed()) && myAttributesCreatorRows.at(i.getPositionListed())->isAttributeValid().size() != 0) {
            return false;
        }
    }
    return true;
}


void
GNEFrameAttributesModuls::AttributesCreator::refreshRows() {
    // currently only row with attribute ID must be refresh
    if (myTagProperties.hasAttribute(SUMO_ATTR_ID) && (myTagProperties.getTag() != SUMO_TAG_VAPORIZER)) {
        myAttributesCreatorRows[myTagProperties.getAttributeProperties(SUMO_ATTR_ID).getPositionListed()]->refreshRow();
    }
}

long
GNEFrameAttributesModuls::AttributesCreator::onCmdHelp(FXObject*, FXSelector, void*) {
    // open Help attributes dialog
    myFrameParent->openHelpAttributesDialog(myTagProperties);
    return 1;
}

// ---------------------------------------------------------------------------
// GNEFrameAttributesModuls::AttributesCreatorFlow - methods
// ---------------------------------------------------------------------------

GNEFrameAttributesModuls::AttributesCreatorFlow::AttributesCreatorFlow(AttributesCreator* attributesCreatorParent) :
    FXGroupBox(attributesCreatorParent->getFrameParent()->myContentFrame, "Flow attributes", GUIDesignGroupBoxFrame),
    myAttributesCreatorParent(attributesCreatorParent),
    myFlowParameters(VEHPARS_END_SET | VEHPARS_NUMBER_SET) {
    // declare auxiliar horizontal frame
    FXHorizontalFrame* auxiliarHorizontalFrame = nullptr;
    // create elements for end attribute
    auxiliarHorizontalFrame = new FXHorizontalFrame(this, GUIDesignAuxiliarHorizontalFrame);
    myAttributeEndRadioButton = new FXRadioButton(auxiliarHorizontalFrame, toString(SUMO_ATTR_END).c_str(), this, MID_GNE_SET_ATTRIBUTE_BUTTON, GUIDesignRadioButtonAttribute);
    myValueEndTextField = new FXTextField(auxiliarHorizontalFrame, GUIDesignTextFieldNCol, this, MID_GNE_SET_ATTRIBUTE, GUIDesignTextField);
    // create elements for number attribute
    auxiliarHorizontalFrame = new FXHorizontalFrame(this, GUIDesignAuxiliarHorizontalFrame);
    myAttributeNumberRadioButton = new FXRadioButton(auxiliarHorizontalFrame, toString(SUMO_ATTR_NUMBER).c_str(), this, MID_GNE_SET_ATTRIBUTE_BUTTON, GUIDesignRadioButtonAttribute);
    myValueNumberTextField = new FXTextField(auxiliarHorizontalFrame, GUIDesignTextFieldNCol, this, MID_GNE_SET_ATTRIBUTE, GUIDesignTextField);
    // create elements for vehsPerHour attribute
    auxiliarHorizontalFrame = new FXHorizontalFrame(this, GUIDesignAuxiliarHorizontalFrame);
    myAttributeVehsPerHourRadioButton = new FXRadioButton(auxiliarHorizontalFrame, toString(SUMO_ATTR_VEHSPERHOUR).c_str(), this, MID_GNE_SET_ATTRIBUTE_BUTTON, GUIDesignRadioButtonAttribute);
    myValueVehsPerHourTextField = new FXTextField(auxiliarHorizontalFrame, GUIDesignTextFieldNCol, this, MID_GNE_SET_ATTRIBUTE, GUIDesignTextField);
    // create elements for period attribute
    auxiliarHorizontalFrame = new FXHorizontalFrame(this, GUIDesignAuxiliarHorizontalFrame);
    myAttributePeriodRadioButton = new FXRadioButton(auxiliarHorizontalFrame, toString(SUMO_ATTR_PERIOD).c_str(), this, MID_GNE_SET_ATTRIBUTE_BUTTON, GUIDesignRadioButtonAttribute);
    myValuePeriodTextField = new FXTextField(auxiliarHorizontalFrame, GUIDesignTextFieldNCol, this, MID_GNE_SET_ATTRIBUTE, GUIDesignTextField);
    // create elements for Probability attribute
    auxiliarHorizontalFrame = new FXHorizontalFrame(this, GUIDesignAuxiliarHorizontalFrame);
    myAttributeProbabilityRadioButton = new FXRadioButton(auxiliarHorizontalFrame, toString(SUMO_ATTR_PROB).c_str(), this, MID_GNE_SET_ATTRIBUTE_BUTTON, GUIDesignRadioButtonAttribute);
    myValueProbabilityTextField = new FXTextField(auxiliarHorizontalFrame, GUIDesignTextFieldNCol, this, MID_GNE_SET_ATTRIBUTE, GUIDesignTextField);
    // set default values
    myValueEndTextField->setText("3600");
    myValueNumberTextField->setText("1800");
    myValueVehsPerHourTextField->setText("1800");
    myValuePeriodTextField->setText("2");
    myValueProbabilityTextField->setText("0.5");
    // refresh attributes
    refreshAttributesCreatorFlow();
}


GNEFrameAttributesModuls::AttributesCreatorFlow::~AttributesCreatorFlow() {}


void
GNEFrameAttributesModuls::AttributesCreatorFlow::showAttributesCreatorFlowModul(const bool persons) {
    if (persons) {
        myAttributeVehsPerHourRadioButton->setText(toString(SUMO_ATTR_PERSONSPERHOUR).c_str());
    } else {
        myAttributeVehsPerHourRadioButton->setText(toString(SUMO_ATTR_VEHSPERHOUR).c_str());
    }
    // show
    show();
}


void
GNEFrameAttributesModuls::AttributesCreatorFlow::hideAttributesCreatorFlowModul() {
    hide();
}


void
GNEFrameAttributesModuls::AttributesCreatorFlow::refreshAttributesCreatorFlow() {
    if (myFlowParameters & VEHPARS_END_SET) {
        myAttributeEndRadioButton->setCheck(TRUE);
        myValueEndTextField->enable();
    } else {
        myAttributeEndRadioButton->setCheck(FALSE);
        myValueEndTextField->disable();
    }
    if (myFlowParameters & VEHPARS_NUMBER_SET) {
        myAttributeNumberRadioButton->setCheck(TRUE);
        myValueNumberTextField->enable();
    } else {
        myAttributeNumberRadioButton->setCheck(FALSE);
        myValueNumberTextField->disable();
    }
    if (myFlowParameters & VEHPARS_VPH_SET) {
        myAttributeVehsPerHourRadioButton->setCheck(TRUE);
        myValueVehsPerHourTextField->enable();
    } else {
        myAttributeVehsPerHourRadioButton->setCheck(FALSE);
        myValueVehsPerHourTextField->disable();
    }
    if (myFlowParameters & VEHPARS_PERIOD_SET) {
        myAttributePeriodRadioButton->setCheck(TRUE);
        myValuePeriodTextField->enable();
    } else {
        myAttributePeriodRadioButton->setCheck(FALSE);
        myValuePeriodTextField->disable();
    }
    if (myFlowParameters & VEHPARS_PROB_SET) {
        myAttributeProbabilityRadioButton->setCheck(TRUE);
        myValueProbabilityTextField->enable();
    } else {
        myAttributeProbabilityRadioButton->setCheck(FALSE);
        myValueProbabilityTextField->disable();
    }
}


void
GNEFrameAttributesModuls::AttributesCreatorFlow::setFlowParameters(std::map<SumoXMLAttr, std::string>& parameters) {
    if (myFlowParameters & VEHPARS_END_SET) {
        parameters[SUMO_ATTR_END] = myValueEndTextField->getText().text();
    }
    if (myFlowParameters & VEHPARS_NUMBER_SET) {
        parameters[SUMO_ATTR_NUMBER] = myValueNumberTextField->getText().text();
    }
    if (myFlowParameters & VEHPARS_VPH_SET) {
        if (myAttributeVehsPerHourRadioButton->getText().text() == toString(SUMO_ATTR_VEHSPERHOUR)) {
            parameters[SUMO_ATTR_VEHSPERHOUR] = myValueVehsPerHourTextField->getText().text();
        } else {
            parameters[SUMO_ATTR_PERSONSPERHOUR] = myValueVehsPerHourTextField->getText().text();
        }
    }
    if (myFlowParameters & VEHPARS_PERIOD_SET) {
        parameters[SUMO_ATTR_PERIOD] = myValuePeriodTextField->getText().text();
    }
    if (myFlowParameters & VEHPARS_PROB_SET) {
        parameters[SUMO_ATTR_PROB] = myValueProbabilityTextField->getText().text();
    }
}


void
GNEFrameAttributesModuls::AttributesCreatorFlow::showWarningMessage(std::string /* extra */) const {
    std::string errorMessage;
    /*
    // iterate over standar parameters
    for (const auto& i : myTagProperties) {
        if (errorMessage.empty() && myAttributesCreatorRows.at(i.getPositionListed())) {
            // Return string with the error if at least one of the parameter isn't valid
            std::string attributeValue = myAttributesCreatorRows.at(i.getPositionListed())->isAttributeValid();
            if (attributeValue.size() != 0) {
                errorMessage = attributeValue;
            }
        }
    }
    // show warning box if input parameters aren't invalid
    if (extra.size() == 0) {
        errorMessage = "Invalid input parameter of " + myTagProperties.getTagStr() + ": " + errorMessage;
    } else {
        errorMessage = "Invalid input parameter of " + myTagProperties.getTagStr() + ": " + extra;
    }
    */
    // set message in status bar
    myAttributesCreatorParent->getFrameParent()->myViewNet->setStatusBarText(errorMessage);
    // Write Warning in console if we're in testing mode
    WRITE_DEBUG(errorMessage);
}


bool
GNEFrameAttributesModuls::AttributesCreatorFlow::areValuesValid() const {
    // check every flow attribute
    if (myFlowParameters & VEHPARS_END_SET) {
        if (GNEAttributeCarrier::canParse<double>(myValueEndTextField->getText().text())) {
            if (GNEAttributeCarrier::parse<double>(myValueEndTextField->getText().text()) < 0) {
                return false;
            }
        } else {
            return false;
        }
    }
    if (myFlowParameters & VEHPARS_NUMBER_SET) {
        if (GNEAttributeCarrier::canParse<double>(myValueNumberTextField->getText().text())) {
            if (GNEAttributeCarrier::parse<double>(myValueNumberTextField->getText().text()) < 0) {
                return false;
            }
        } else {
            return false;
        }
    }
    if (myFlowParameters & VEHPARS_VPH_SET) {
        if (GNEAttributeCarrier::canParse<double>(myValueVehsPerHourTextField->getText().text())) {
            if (GNEAttributeCarrier::parse<double>(myValueVehsPerHourTextField->getText().text()) < 0) {
                return false;
            }
        } else {
            return false;
        }
    }
    if (myFlowParameters & VEHPARS_PERIOD_SET) {
        if (GNEAttributeCarrier::canParse<double>(myValuePeriodTextField->getText().text())) {
            if (GNEAttributeCarrier::parse<double>(myValuePeriodTextField->getText().text()) < 0) {
                return false;
            }
        } else {
            return false;
        }
    }
    if (myFlowParameters & VEHPARS_PROB_SET) {
        if (GNEAttributeCarrier::canParse<double>(myValueProbabilityTextField->getText().text())) {
            if (GNEAttributeCarrier::parse<double>(myValueProbabilityTextField->getText().text()) < 0) {
                return false;
            }
        } else {
            return false;
        }
    }
    return true;
}


long
GNEFrameAttributesModuls::AttributesCreatorFlow::onCmdSetFlowAttribute(FXObject* obj, FXSelector, void*) {
    // obtain clicked textfield
    FXTextField* textField = nullptr;
    // check what text field was pressed
    if (obj == myValueEndTextField) {
        textField = myValueEndTextField;
    } else if (obj == myValueNumberTextField) {
        textField = myValueNumberTextField;
    } else if (obj == myValueVehsPerHourTextField) {
        textField = myValueVehsPerHourTextField;
    } else if (obj == myValuePeriodTextField) {
        textField = myValuePeriodTextField;
    } else if (obj == myValueProbabilityTextField) {
        textField = myValueProbabilityTextField;
    } else {
        throw ProcessError("Invalid text field");
    }
    // check if value is valid
    if (GNEAttributeCarrier::canParse<double>(textField->getText().text()) && (GNEAttributeCarrier::parse<double>(textField->getText().text()) >= 0)) {
        textField->setTextColor(FXRGB(0, 0, 0));
    } else {
        textField->setTextColor(FXRGB(255, 0, 0));
    }
    textField->killFocus();
    return 1;
}


long
GNEFrameAttributesModuls::AttributesCreatorFlow::onCmdSelectFlowRadioButton(FXObject* obj, FXSelector, void*) {
    // check what check button was pressed
    if (obj == myAttributeEndRadioButton) {
        GNERouteHandler::setFlowParameters(SUMO_ATTR_END, myFlowParameters);
    } else if (obj == myAttributeNumberRadioButton) {
        GNERouteHandler::setFlowParameters(SUMO_ATTR_NUMBER, myFlowParameters);
    } else if (obj == myAttributeVehsPerHourRadioButton) {
        if (myAttributeVehsPerHourRadioButton->getText().text() == toString(SUMO_ATTR_VEHSPERHOUR)) {
            GNERouteHandler::setFlowParameters(SUMO_ATTR_VEHSPERHOUR, myFlowParameters);
        } else {
            GNERouteHandler::setFlowParameters(SUMO_ATTR_PERSONSPERHOUR, myFlowParameters);
        }
    } else if (obj == myAttributePeriodRadioButton) {
        GNERouteHandler::setFlowParameters(SUMO_ATTR_PERIOD, myFlowParameters);
    } else if (obj == myAttributeProbabilityRadioButton) {
        GNERouteHandler::setFlowParameters(SUMO_ATTR_PROB, myFlowParameters);
    } else {
        throw ProcessError("Invalid Radio Button");
    }
    // refresh attributes
    refreshAttributesCreatorFlow();
    return 1;
}

// ---------------------------------------------------------------------------
// GNEFrameAttributesModuls::AttributesEditorRow - methods
// ---------------------------------------------------------------------------

GNEFrameAttributesModuls::AttributesEditorRow::AttributesEditorRow(GNEFrameAttributesModuls::AttributesEditor* attributeEditorParent, const GNEAttributeProperties& ACAttr, const std::string& value, bool attributeEnabled) :
    FXHorizontalFrame(attributeEditorParent, GUIDesignAuxiliarHorizontalFrame),
    myAttributesEditorParent(attributeEditorParent),
    myACAttr(ACAttr),
    myMultiple(GNEAttributeCarrier::parse<std::vector<std::string>>(value).size() > 1) {
    // Create and hide label
    myAttributeLabel = new FXLabel(this, "attributeLabel", nullptr, GUIDesignLabelAttribute);
    myAttributeLabel->hide();
    // Create and hide check button
    myAttributeCheckButton = new FXCheckButton(this, "attributeCheckButton", this, MID_GNE_SET_ATTRIBUTE_BOOL, GUIDesignCheckButtonAttribute);
    myAttributeCheckButton->hide();
    // Create and hide ButtonCombinableChoices
    myAttributeButtonCombinableChoices = new FXButton(this, "attributeButtonCombinableChoices", nullptr, this, MID_GNE_SET_ATTRIBUTE_DIALOG, GUIDesignButtonAttribute);
    myAttributeButtonCombinableChoices->hide();
    // create and hidde color editor
    myAttributeColorButton = new FXButton(this, "attributeColorButton", nullptr, this, MID_GNE_SET_ATTRIBUTE_DIALOG, GUIDesignButtonAttribute);
    myAttributeColorButton->hide();
    // Create and hide textField for string attributes
    myValueTextField = new FXTextField(this, GUIDesignTextFieldNCol, this, MID_GNE_SET_ATTRIBUTE, GUIDesignTextField);
    myValueTextField->hide();
    // Create and hide ComboBox
    myValueComboBoxChoices = new FXComboBox(this, GUIDesignComboBoxNCol, this, MID_GNE_SET_ATTRIBUTE, GUIDesignComboBoxAttribute);
    myValueComboBoxChoices->hide();
    // Create and hide checkButton
    myValueCheckButton = new FXCheckButton(this, "", this, MID_GNE_SET_ATTRIBUTE, GUIDesignCheckButton);
    myValueCheckButton->hide();
    // only create if parent was created
    if (getParent()->id()) {
        // create AttributesEditorRow
        FXHorizontalFrame::create();
        // start enabling all elements, depending if attribute is enabled
        if (attributeEnabled == false) {
            myValueTextField->disable();
            myValueComboBoxChoices->disable();
            myValueCheckButton->disable();
        } else {
            myValueTextField->enable();
            myValueComboBoxChoices->enable();
            myValueCheckButton->enable();
        }
        // if Tag correspond to an network element but we're in demand mode (or vice versa), disable all elements
        if (myACAttr.getAttr() != SUMO_ATTR_NOTHING) {
            if (isSupermodeValid(myAttributesEditorParent->getFrameParent()->myViewNet, myACAttr)) {
                myAttributeButtonCombinableChoices->enable();
                myAttributeColorButton->enable();
                myAttributeCheckButton->enable();
            } else {
                myAttributeColorButton->disable();
                myAttributeCheckButton->disable();
                myValueTextField->disable();
                myValueComboBoxChoices->disable();
                myValueCheckButton->disable();
                myAttributeButtonCombinableChoices->disable();
            }
        }
        // set left column
        if (myACAttr.isColor()) {
            // show color button
            myAttributeColorButton->setTextColor(FXRGB(0, 0, 0));
            myAttributeColorButton->setText(myACAttr.getAttrStr().c_str());
            myAttributeColorButton->show();
        } else if (myACAttr.isActivatable()) {
            // show checkbox button
            myAttributeCheckButton->setTextColor(FXRGB(0, 0, 0));
            myAttributeCheckButton->setText(myACAttr.getAttrStr().c_str());
            myAttributeCheckButton->show();
            // check or uncheck depending of attributeEnabled
            if (attributeEnabled) {
                myAttributeCheckButton->setCheck(TRUE);
            } else {
                myAttributeCheckButton->setCheck(FALSE);
            }
        } else {
            // Show attribute Label
            myAttributeLabel->setText(myACAttr.getAttrStr().c_str());
            myAttributeLabel->show();
        }
        // Set field depending of the type of value
        if (myACAttr.isBool()) {
            // first we need to check if all boolean values are equal
            bool allBooleanValuesEqual = true;
            // declare  boolean vector
            std::vector<bool> booleanVector;
            // check if value can be parsed to a boolean vector
            if (GNEAttributeCarrier::canParse<std::vector<bool> >(value)) {
                booleanVector = GNEAttributeCarrier::parse<std::vector<bool> >(value);
            }
            // iterate over pased booleans comparing all element with the first
            for (const auto& i : booleanVector) {
                if (i != booleanVector.front()) {
                    allBooleanValuesEqual = false;
                }
            }
            // use checkbox or textfield depending if all booleans are equal
            if (allBooleanValuesEqual) {
                // set check button
                if ((booleanVector.size() > 0) && booleanVector.front()) {
                    myValueCheckButton->setCheck(true);
                    myValueCheckButton->setText("true");
                } else {
                    myValueCheckButton->setCheck(false);
                    myValueCheckButton->setText("false");
                }
                // show check button
                myValueCheckButton->show();
            } else {
                // show list of bools (0 1)
                myValueTextField->setText(value.c_str());
                myValueTextField->setTextColor(FXRGB(0, 0, 0));
                myValueTextField->show();
            }
        } else if (myACAttr.isDiscrete()) {
            // Check if are VClasses
            if ((myACAttr.getDiscreteValues().size() > 0) && myACAttr.isVClasses()) {
                // hide label
                myAttributeLabel->hide();
                // Show button combinable choices
                myAttributeButtonCombinableChoices->setText(myACAttr.getAttrStr().c_str());
                myAttributeButtonCombinableChoices->show();
                // Show string with the values
                myValueTextField->setText(value.c_str());
                myValueTextField->setTextColor(FXRGB(0, 0, 0));
                myValueTextField->show();
            } else if (!myMultiple) {
                // fill comboBox
                myValueComboBoxChoices->clearItems();
                for (const auto& it : myACAttr.getDiscreteValues()) {
                    myValueComboBoxChoices->appendItem(it.c_str());
                }
                // show combo box with values
                myValueComboBoxChoices->setNumVisible((int)myACAttr.getDiscreteValues().size());
                myValueComboBoxChoices->setCurrentItem(myValueComboBoxChoices->findItem(value.c_str()));
                myValueComboBoxChoices->setTextColor(FXRGB(0, 0, 0));
                myValueComboBoxChoices->show();
            } else {
                // represent combinable choices in multiple selections always with a textfield instead with a comboBox
                myValueTextField->setText(value.c_str());
                myValueTextField->setTextColor(FXRGB(0, 0, 0));
                myValueTextField->show();
            }
        } else {
            // In any other case (String, list, etc.), show value as String
            myValueTextField->setText(value.c_str());
            myValueTextField->setTextColor(FXRGB(0, 0, 0));
            myValueTextField->show();
        }
        // Show AttributesEditorRow
        show();
    }
}


void
GNEFrameAttributesModuls::AttributesEditorRow::destroy() {
    // only destroy if parent was created
    if (getParent()->id()) {
        FXHorizontalFrame::destroy();
    }
}


void
GNEFrameAttributesModuls::AttributesEditorRow::refreshAttributesEditorRow(const std::string& value, bool forceRefresh, bool attributeEnabled) {
    // start enabling all elements, depending if attribute is enabled
    if (attributeEnabled == false) {
        myValueTextField->disable();
        myValueComboBoxChoices->disable();
        myValueCheckButton->disable();
    } else {
        myValueTextField->enable();
        myValueComboBoxChoices->enable();
        myValueCheckButton->enable();
    }
    // if Tag correspond to an network element but we're in demand mode (or vice versa), disable all elements
    if (myACAttr.getAttr() != SUMO_ATTR_NOTHING) {
        if (isSupermodeValid(myAttributesEditorParent->getFrameParent()->myViewNet, myACAttr)) {
            myAttributeButtonCombinableChoices->enable();
            myAttributeColorButton->enable();
            myAttributeCheckButton->enable();
        } else {
            myAttributeColorButton->disable();
            myAttributeCheckButton->disable();
            myValueTextField->disable();
            myValueComboBoxChoices->disable();
            myValueCheckButton->disable();
            myAttributeButtonCombinableChoices->disable();
        }
    }
    // set check buton
    if (myAttributeCheckButton->shown()) {
        myAttributeCheckButton->setCheck(attributeEnabled);
    }
    if (myValueTextField->shown()) {
        // set last valid value and restore color if onlyValid is disabled
        if (myValueTextField->getTextColor() == FXRGB(0, 0, 0) || forceRefresh) {
            myValueTextField->setText(value.c_str());
            myValueTextField->setTextColor(FXRGB(0, 0, 0));
        }
    } else if (myValueComboBoxChoices->shown()) {
        // fill comboBox again
        myValueComboBoxChoices->clearItems();
        for (const auto& it : myACAttr.getDiscreteValues()) {
            myValueComboBoxChoices->appendItem(it.c_str());
        }
        // show combo box with values
        myValueComboBoxChoices->setNumVisible((int)myACAttr.getDiscreteValues().size());
        myValueComboBoxChoices->setCurrentItem(myValueComboBoxChoices->findItem(value.c_str()));
        myValueComboBoxChoices->setTextColor(FXRGB(0, 0, 0));
        myValueComboBoxChoices->show();
    } else if (myValueCheckButton->shown()) {
        if (GNEAttributeCarrier::canParse<bool>(value)) {
            myValueCheckButton->setCheck(GNEAttributeCarrier::parse<bool>(value));
        } else {
            myValueCheckButton->setCheck(false);
        }
    }
}


bool
GNEFrameAttributesModuls::AttributesEditorRow::isAttributesEditorRowValid() const {
    return ((myValueTextField->getTextColor() == FXRGB(0, 0, 0)) && (myValueComboBoxChoices->getTextColor() == FXRGB(0, 0, 0)));
}


long
GNEFrameAttributesModuls::AttributesEditorRow::onCmdOpenAttributeDialog(FXObject* obj, FXSelector, void*) {
    if (obj == myAttributeColorButton) {
        // create FXColorDialog
        FXColorDialog colordialog(this, tr("Color Dialog"));
        colordialog.setTarget(this);
        // If previous attribute wasn't correct, set black as default color
        if (GNEAttributeCarrier::canParse<RGBColor>(myValueTextField->getText().text())) {
            colordialog.setRGBA(MFXUtils::getFXColor(RGBColor::parseColor(myValueTextField->getText().text())));
        } else if (!myACAttr.getDefaultValue().empty()) {
            colordialog.setRGBA(MFXUtils::getFXColor(RGBColor::parseColor(myACAttr.getDefaultValue())));
        } else {
            colordialog.setRGBA(MFXUtils::getFXColor(RGBColor::BLACK));
        }
        // execute dialog to get a new color
        if (colordialog.execute()) {
            std::string newValue = toString(MFXUtils::getRGBColor(colordialog.getRGBA()));
            myValueTextField->setText(newValue.c_str());
            if (myAttributesEditorParent->getEditedACs().front()->isValid(myACAttr.getAttr(), newValue)) {
                // if its valid for the first AC than its valid for all (of the same type)
                if (myAttributesEditorParent->getEditedACs().size() > 1) {
                    myAttributesEditorParent->getFrameParent()->myViewNet->getUndoList()->p_begin("Change multiple attributes");
                }
                // Set new value of attribute in all selected ACs
                for (const auto& it_ac : myAttributesEditorParent->getEditedACs()) {
                    it_ac->setAttribute(myACAttr.getAttr(), newValue, myAttributesEditorParent->getFrameParent()->myViewNet->getUndoList());
                }
                // If previously value was incorrect, change font color to black
                myValueTextField->setTextColor(FXRGB(0, 0, 0));
                myValueTextField->killFocus();
            }
        }
        return 0;
    } else if (obj == myAttributeButtonCombinableChoices) {
        // if its valid for the first AC than its valid for all (of the same type)
        if (myAttributesEditorParent->getEditedACs().size() > 1) {
            myAttributesEditorParent->getFrameParent()->myViewNet->getUndoList()->p_begin("Change multiple attributes");
        }
        // open GNEAllowDisallow
        GNEAllowDisallow(myAttributesEditorParent->getFrameParent()->myViewNet, myAttributesEditorParent->getEditedACs().front()).execute();
        std::string allowed = myAttributesEditorParent->getEditedACs().front()->getAttribute(SUMO_ATTR_ALLOW);
        // Set new value of attribute in all selected ACs
        for (const auto& it_ac : myAttributesEditorParent->getEditedACs()) {
            it_ac->setAttribute(SUMO_ATTR_ALLOW, allowed, myAttributesEditorParent->getFrameParent()->myViewNet->getUndoList());
        }
        // finish change multiple attributes
        if (myAttributesEditorParent->getEditedACs().size() > 1) {
            myAttributesEditorParent->getFrameParent()->myViewNet->getUndoList()->p_end();
        }
        // update frame parent after attribute sucesfully set
        myAttributesEditorParent->getFrameParent()->attributeUpdated();
        return 1;
    } else {
        throw ProcessError("Invalid call to onCmdOpenAttributeDialog");
    }
}


long
GNEFrameAttributesModuls::AttributesEditorRow::onCmdSetAttribute(FXObject*, FXSelector, void*) {
    // Declare changed value
    std::string newVal;
    // First, obtain the string value of the new attribute depending of their type
    if (myACAttr.isBool()) {
        // first check if we're editing boolean as a list of string or as a checkbox
        if (myValueCheckButton->shown()) {
            // Set true o false depending of the checkBox
            if (myValueCheckButton->getCheck()) {
                myValueCheckButton->setText("true");
                newVal = "true";
            } else {
                myValueCheckButton->setText("false");
                newVal = "false";
            }
        } else {
            // obtain boolean value of myValueTextField (because we're inspecting multiple attribute carriers with different values)
            newVal = myValueTextField->getText().text();
        }
    } else if (myACAttr.isDiscrete()) {
        // Check if are VClasses
        if ((myACAttr.getDiscreteValues().size() > 0) && myACAttr.isVClasses()) {
            // Get value obtained using AttributesEditor
            newVal = myValueTextField->getText().text();
        } else if (!myMultiple) {
            // Get value of ComboBox
            newVal = myValueComboBoxChoices->getText().text();
        } else {
            // due this is a multiple selection, obtain value of myValueTextField instead of comboBox
            newVal = myValueTextField->getText().text();
        }
    } else {
        // Check if default value of attribute must be set
        if (myValueTextField->getText().empty() && myACAttr.hasStaticDefaultValue()) {
            newVal = myACAttr.getDefaultValue();
            myValueTextField->setText(newVal.c_str());
        } else if (myACAttr.isInt() && GNEAttributeCarrier::canParse<double>(myValueTextField->getText().text())) {
            // filter int attributes
            double doubleValue = GNEAttributeCarrier::parse<double>(myValueTextField->getText().text());
            // check if myValueTextField has to be updated
            if ((doubleValue - (int)doubleValue) == 0) {
                newVal = toString((int)doubleValue);
                myValueTextField->setText(newVal.c_str(), FALSE);
            }
        } else if ((myACAttr.getAttr() == SUMO_ATTR_ANGLE) && GNEAttributeCarrier::canParse<double>(myValueTextField->getText().text())) {
            // filter angle
            double angle = GNEAttributeCarrier::parse<double>(myValueTextField->getText().text());
            // filter if angle isn't between [0,360]
            if ((angle < 0) || (angle > 360)) {
                // apply modul
                angle = fmod(angle, 360);
            }
            // set newVal
            newVal = toString(angle);
            // update Textfield
            myValueTextField->setText(newVal.c_str(), FALSE);
        } else {
            // obtain value of myValueTextField
            newVal = myValueTextField->getText().text();
        }
    }
    // we need a extra check for Position and Shape Values, due #2658
    if ((myACAttr.getAttr() == SUMO_ATTR_POSITION) || (myACAttr.getAttr() == SUMO_ATTR_SHAPE)) {
        newVal = stripWhitespaceAfterComma(newVal);
    }
    // Check if attribute must be changed
    if ((myAttributesEditorParent->getEditedACs().size() > 0) && myAttributesEditorParent->getEditedACs().front()->isValid(myACAttr.getAttr(), newVal)) {
        // if its valid for the first AC than its valid for all (of the same type)
        if (myAttributesEditorParent->getEditedACs().size() > 1) {
            myAttributesEditorParent->getFrameParent()->myViewNet->getUndoList()->p_begin("Change multiple attributes");
        } else if (myACAttr.getAttr() == SUMO_ATTR_ID) {
            // IDs attribute has to be encapsulated
            myAttributesEditorParent->getFrameParent()->myViewNet->getUndoList()->p_begin("change " + myACAttr.getTagPropertyParent().getTagStr() + " attribute");
        }
        // Set new value of attribute in all selected ACs
        for (const auto& it_ac : myAttributesEditorParent->getEditedACs()) {
            it_ac->setAttribute(myACAttr.getAttr(), newVal, myAttributesEditorParent->getFrameParent()->myViewNet->getUndoList());
        }
        // finish change multiple attributes or ID Attributes
        if (myAttributesEditorParent->getEditedACs().size() > 1) {
            myAttributesEditorParent->getFrameParent()->myViewNet->getUndoList()->p_end();
        } else if (myACAttr.getAttr() == SUMO_ATTR_ID) {
            myAttributesEditorParent->getFrameParent()->myViewNet->getUndoList()->p_end();
        }
        // If previously value was incorrect, change font color to black
        if (myACAttr.isVClasses()) {
            myValueTextField->setTextColor(FXRGB(0, 0, 0));
            myValueTextField->killFocus();
            // in this case, we need to refresh the other values (For example, allow/Disallow objects)
            myAttributesEditorParent->refreshAttributeEditor(false, false);
        } else if (myACAttr.isDiscrete()) {
            myValueComboBoxChoices->setTextColor(FXRGB(0, 0, 0));
            myValueComboBoxChoices->killFocus();
        } else if (myValueTextField != nullptr) {
            myValueTextField->setTextColor(FXRGB(0, 0, 0));
            myValueTextField->killFocus();
        }
        // update frame parent after attribute sucesfully set
        myAttributesEditorParent->getFrameParent()->attributeUpdated();
    } else {
        // If value of TextField isn't valid, change color to Red depending of type
        if (myACAttr.isVClasses()) {
            myValueTextField->setTextColor(FXRGB(255, 0, 0));
            myValueTextField->killFocus();
        } else if (myACAttr.isDiscrete()) {
            myValueComboBoxChoices->setTextColor(FXRGB(255, 0, 0));
            myValueComboBoxChoices->killFocus();
        } else if (myValueTextField != nullptr) {
            myValueTextField->setTextColor(FXRGB(255, 0, 0));
        }
        // Write Warning in console if we're in testing mode
        WRITE_DEBUG("Value '" + newVal + "' for attribute " + myACAttr.getAttrStr() + " of " + myACAttr.getTagPropertyParent().getTagStr() + " isn't valid");
    }
    return 1;
}


long
GNEFrameAttributesModuls::AttributesEditorRow::onCmdSelectCheckButton(FXObject*, FXSelector, void*) {
    if (myAttributeCheckButton->getCheck()) {
        // enable input values
        myValueCheckButton->enable();
        myValueTextField->enable();
    } else {
        // disable input values
        myValueCheckButton->disable();
        myValueTextField->disable();
    }
    return 0;
}


GNEFrameAttributesModuls::AttributesEditorRow::AttributesEditorRow() :
    myMultiple(false) {
}


std::string
GNEFrameAttributesModuls::AttributesEditorRow::stripWhitespaceAfterComma(const std::string& stringValue) {
    std::string result(stringValue);
    while (result.find(", ") != std::string::npos) {
        result = StringUtils::replace(result, ", ", ",");
    }
    return result;
}

// ---------------------------------------------------------------------------
// GNEFrameAttributesModuls::AttributesEditor - methods
// ---------------------------------------------------------------------------

GNEFrameAttributesModuls::AttributesEditor::AttributesEditor(GNEFrame* FrameParent) :
    FXGroupBox(FrameParent->myContentFrame, "Internal attributes", GUIDesignGroupBoxFrame),
    myFrameParent(FrameParent),
    myIncludeExtended(true) {
    // resize myAttributesEditorRows
    myAttributesEditorRows.resize(GNEAttributeCarrier::MAXNUMBEROFATTRIBUTES, nullptr);
    // create myAttributesFlowEditor
    myAttributesEditorFlow = new AttributesEditorFlow(this);
    // leave it hidden
    myAttributesEditorFlow->hideAttributesEditorFlowModul();
    // Create help button
    myHelpButton = new FXButton(this, "Help", nullptr, this, MID_HELP, GUIDesignButtonRectangular);
}


void
GNEFrameAttributesModuls::AttributesEditor::showAttributeEditorModul(const std::vector<GNEAttributeCarrier*>& ACs, bool includeExtended, bool forceAttributeEnabled) {
    myEditedACs = ACs;
    myIncludeExtended = includeExtended;
    // first remove all rows
    for (int i = 0; i < (int)myAttributesEditorRows.size(); i++) {
        // destroy and delete all rows
        if (myAttributesEditorRows.at(i) != nullptr) {
            myAttributesEditorRows.at(i)->destroy();
            delete myAttributesEditorRows.at(i);
            myAttributesEditorRows.at(i) = nullptr;
        }
    }
    // declare flag to check if flow editor has to be shown
    bool showFlowEditor = false;
    if (myEditedACs.size() > 0) {
        // Iterate over attributes
        for (const auto& tagProperty : myEditedACs.front()->getTagProperty()) {
            // declare flag to show/hidde atribute
            bool editAttribute = true;
            // disable editing for unique attributes in case of multi-selection
            if ((myEditedACs.size() > 1) && tagProperty.isUnique()) {
                editAttribute = false;
            }
            // disable editing of extended attributes if includeExtended isn't enabled
            if (tagProperty.isExtended() && !includeExtended) {
                editAttribute = false;
            }
            // disable editing of flow definition attributes, but enable flow editor
            if (tagProperty.isFlowDefinition()) {
                editAttribute = false;
                showFlowEditor = true;
            }
            // continue if attribute is editable
            if (editAttribute) {
                // Declare a set of occuring values and insert attribute's values of item (note: We use a set to avoid repeated values)
                std::set<std::string> occuringValues;
                // iterate over edited attributes
                for (const auto& it_ac : myEditedACs) {
                    occuringValues.insert(it_ac->getAttribute(tagProperty.getAttr()));
                }
                // get current value
                std::ostringstream oss;
                for (auto values = occuringValues.begin(); values != occuringValues.end(); values++) {
                    if (values != occuringValues.begin()) {
                        oss << " ";
                    }
                    oss << *values;
                }
                // obtain value to be shown in row
                std::string value = oss.str();
                // declare a flag for enabled attributes
                bool attributeEnabled = myEditedACs.front()->isAttributeEnabled(tagProperty.getAttr());
                // overwritte value if attribute is disabled (used by LinkIndex)
                if (attributeEnabled == false) {
                    value = myEditedACs.front()->getAlternativeValueForDisabledAttributes(tagProperty.getAttr());
                }
                // extra check for Triggered and container Triggered
                if (myEditedACs.front()->getTagProperty().isStop() || myEditedACs.front()->getTagProperty().isPersonStop()) {
                    if ((tagProperty.getAttr() == SUMO_ATTR_EXPECTED) && (myEditedACs.front()->isAttributeEnabled(SUMO_ATTR_TRIGGERED) == false)) {
                        attributeEnabled = false;
                    } else if ((tagProperty.getAttr() == SUMO_ATTR_EXPECTED_CONTAINERS) && (myEditedACs.front()->isAttributeEnabled(SUMO_ATTR_CONTAINER_TRIGGERED) == false)) {
                        attributeEnabled = false;
                    }
                }
                // if forceEnablellAttribute is enable, force attributeEnabled (except for ID)
                if (forceAttributeEnabled && (tagProperty.getAttr() != SUMO_ATTR_ID)) {
                    attributeEnabled = true;
                }
                // create attribute editor row
                myAttributesEditorRows[tagProperty.getPositionListed()] = new AttributesEditorRow(this, tagProperty, value, attributeEnabled);
            }
        }
        // check if Flow editor has to be shown
        if (showFlowEditor) {
            myAttributesEditorFlow->showAttributeEditorFlowModul();
        } else {
            myAttributesEditorFlow->hideAttributesEditorFlowModul();
        }
        // show AttributesEditor
        show();
    } else {
        myAttributesEditorFlow->hideAttributesEditorFlowModul();
    }
    // reparent help button (to place it at bottom)
    myHelpButton->reparent(this);
}


void
GNEFrameAttributesModuls::AttributesEditor::hideAttributesEditorModul() {
    // clear myEditedACs
    myEditedACs.clear();
    // hide AttributesEditorFlowModul
    myAttributesEditorFlow->hideAttributesEditorFlowModul();
    // hide also AttributesEditor
    hide();
}


void
GNEFrameAttributesModuls::AttributesEditor::refreshAttributeEditor(bool forceRefreshShape, bool forceRefreshPosition) {
    if (myEditedACs.size() > 0) {
        // Iterate over attributes
        for (const auto& tagProperty : myEditedACs.front()->getTagProperty()) {
            // declare flag to show/hidde atribute
            bool editAttribute = true;
            // disable editing for unique attributes in case of multi-selection
            if ((myEditedACs.size() > 1) && tagProperty.isUnique()) {
                editAttribute = false;
            }
            // disable editing of extended attributes if includeExtended isn't enabled
            if (tagProperty.isExtended() && !myIncludeExtended) {
                editAttribute = false;
            }
            // disable editing of flow definition attributes, but enable flow editor
            if (tagProperty.isFlowDefinition()) {
                editAttribute = false;
            }
            // continue if attribute is editable
            if (editAttribute) {
                // Declare a set of occuring values and insert attribute's values of item (note: We use a set to avoid repeated values)
                std::set<std::string> occuringValues;
                // iterate over edited attributes
                for (const auto& it_ac : myEditedACs) {
                    occuringValues.insert(it_ac->getAttribute(tagProperty.getAttr()));
                }
                // get current value
                std::ostringstream oss;
                for (auto values = occuringValues.begin(); values != occuringValues.end(); values++) {
                    if (values != occuringValues.begin()) {
                        oss << " ";
                    }
                    oss << *values;
                }
                // obtain value to be shown in row
                std::string value = oss.str();
                // declare a flag for enabled attributes
                bool attributeEnabled = myEditedACs.front()->isAttributeEnabled(tagProperty.getAttr());
                // overwritte value if attribute is disabled (used by LinkIndex)
                if (attributeEnabled == false) {
                    value = myEditedACs.front()->getAlternativeValueForDisabledAttributes(tagProperty.getAttr());
                }
                // extra check for Triggered and container Triggered
                if (myEditedACs.front()->getTagProperty().isStop() || myEditedACs.front()->getTagProperty().isPersonStop()) {
                    if ((tagProperty.getAttr() == SUMO_ATTR_EXPECTED) && (myEditedACs.front()->isAttributeEnabled(SUMO_ATTR_TRIGGERED) == false)) {
                        attributeEnabled = false;
                    } else if ((tagProperty.getAttr() == SUMO_ATTR_EXPECTED_CONTAINERS) && (myEditedACs.front()->isAttributeEnabled(SUMO_ATTR_CONTAINER_TRIGGERED) == false)) {
                        attributeEnabled = false;
                    }
                }
                /*
                // if forceEnablellAttribute is enable, force attributeEnabled (except for ID)
                if (myForceAttributeEnabled && (tagProperty.getAttr() != SUMO_ATTR_ID)) {
                    attributeEnabled = true;
                }
                */
                // Check if Position or Shape refresh has to be forced
                if ((tagProperty.getAttr() == SUMO_ATTR_SHAPE) && forceRefreshShape) {
                    myAttributesEditorRows[tagProperty.getPositionListed()]->refreshAttributesEditorRow(value, true, attributeEnabled);
                } else if ((tagProperty.getAttr()  == SUMO_ATTR_POSITION) && forceRefreshPosition) {
                    // Refresh attributes maintain invalid values
                    myAttributesEditorRows[tagProperty.getPositionListed()]->refreshAttributesEditorRow(value, true, attributeEnabled);
                } else {
                    // Refresh attributes maintain invalid values
                    myAttributesEditorRows[tagProperty.getPositionListed()]->refreshAttributesEditorRow(value, false, attributeEnabled);
                }
            }
        }
        // check if flow editor has to be update
        if (myAttributesEditorFlow->isAttributesEditorFlowModulShown()) {
            myAttributesEditorFlow->refreshAttributeEditorFlow();
        }
    }
}


GNEFrame*
GNEFrameAttributesModuls::AttributesEditor::getFrameParent() const {
    return myFrameParent;
}


const std::vector<GNEAttributeCarrier*>&
GNEFrameAttributesModuls::AttributesEditor::getEditedACs() const {
    return myEditedACs;
}


void
GNEFrameAttributesModuls::AttributesEditor::removeEditedAC(GNEAttributeCarrier* AC) {
    // Only remove if there is inspected ACs
    if (myEditedACs.size() > 0) {
        // Try to find AC in myACs
        auto i = std::find(myEditedACs.begin(), myEditedACs.end(), AC);
        // if was found
        if (i != myEditedACs.end()) {
            // erase AC from inspected ACs
            myEditedACs.erase(i);
            // Write Warning in console if we're in testing mode
            WRITE_DEBUG("Removed inspected element from Inspected ACs. " + toString(myEditedACs.size()) + " ACs remains.");
            // Inspect multi selection again (To refresh Modul)
            showAttributeEditorModul(myEditedACs, myIncludeExtended, false);
        }
    }
}


long
GNEFrameAttributesModuls::AttributesEditor::onCmdAttributesEditorHelp(FXObject*, FXSelector, void*) {
    // open Help attributes dialog if there is inspected ACs
    if (myEditedACs.size() > 0) {
        // open Help attributes dialog
        myFrameParent->openHelpAttributesDialog(myEditedACs.front()->getTagProperty());
    }
    return 1;
}

// ---------------------------------------------------------------------------
// GNEFrameAttributesModuls::AttributesEditorFlow - methods
// ---------------------------------------------------------------------------

GNEFrameAttributesModuls::AttributesEditorFlow::AttributesEditorFlow(AttributesEditor* attributesEditorParent) :
    FXGroupBox(attributesEditorParent->getFrameParent()->myContentFrame, "Flow attributes", GUIDesignGroupBoxFrame),
    myAttributesEditorParent(attributesEditorParent) {
    // declare auxiliar horizontal frame
    FXHorizontalFrame* auxiliarHorizontalFrame = nullptr;
    // create elements for end attribute
    auxiliarHorizontalFrame = new FXHorizontalFrame(this, GUIDesignAuxiliarHorizontalFrame);
    myAttributeEndRadioButton = new FXRadioButton(auxiliarHorizontalFrame, toString(SUMO_ATTR_END).c_str(), this, MID_GNE_SET_ATTRIBUTE_BUTTON, GUIDesignRadioButtonAttribute);
    myValueEndTextField = new FXTextField(auxiliarHorizontalFrame, GUIDesignTextFieldNCol, this, MID_GNE_SET_ATTRIBUTE, GUIDesignTextField);
    // create elements for number attribute
    auxiliarHorizontalFrame = new FXHorizontalFrame(this, GUIDesignAuxiliarHorizontalFrame);
    myAttributeNumberRadioButton = new FXRadioButton(auxiliarHorizontalFrame, toString(SUMO_ATTR_NUMBER).c_str(), this, MID_GNE_SET_ATTRIBUTE_BUTTON, GUIDesignRadioButtonAttribute);
    myValueNumberTextField = new FXTextField(auxiliarHorizontalFrame, GUIDesignTextFieldNCol, this, MID_GNE_SET_ATTRIBUTE, GUIDesignTextField);
    // create elements for vehsPerHour attribute
    auxiliarHorizontalFrame = new FXHorizontalFrame(this, GUIDesignAuxiliarHorizontalFrame);
    myAttributeVehsPerHourRadioButton = new FXRadioButton(auxiliarHorizontalFrame, toString(SUMO_ATTR_VEHSPERHOUR).c_str(), this, MID_GNE_SET_ATTRIBUTE_BUTTON, GUIDesignRadioButtonAttribute);
    myValueVehsPerHourTextField = new FXTextField(auxiliarHorizontalFrame, GUIDesignTextFieldNCol, this, MID_GNE_SET_ATTRIBUTE, GUIDesignTextField);
    // create elements for period attribute
    auxiliarHorizontalFrame = new FXHorizontalFrame(this, GUIDesignAuxiliarHorizontalFrame);
    myAttributePeriodRadioButton = new FXRadioButton(auxiliarHorizontalFrame, toString(SUMO_ATTR_PERIOD).c_str(), this, MID_GNE_SET_ATTRIBUTE_BUTTON, GUIDesignRadioButtonAttribute);
    myValuePeriodTextField = new FXTextField(auxiliarHorizontalFrame, GUIDesignTextFieldNCol, this, MID_GNE_SET_ATTRIBUTE, GUIDesignTextField);
    // create elements for Probability attribute
    auxiliarHorizontalFrame = new FXHorizontalFrame(this, GUIDesignAuxiliarHorizontalFrame);
    myAttributeProbabilityRadioButton = new FXRadioButton(auxiliarHorizontalFrame, toString(SUMO_ATTR_PROB).c_str(), this, MID_GNE_SET_ATTRIBUTE_BUTTON, GUIDesignRadioButtonAttribute);
    myValueProbabilityTextField = new FXTextField(auxiliarHorizontalFrame, GUIDesignTextFieldNCol, this, MID_GNE_SET_ATTRIBUTE, GUIDesignTextField);
}


void
GNEFrameAttributesModuls::AttributesEditorFlow::showAttributeEditorFlowModul() {
    if (myAttributesEditorParent->getEditedACs().size() > 0) {
        // refresh attributeEditorFlowModul
        refreshAttributeEditorFlow();
        // show flow
        show();
    } else {
        hide();
    }
}


void
GNEFrameAttributesModuls::AttributesEditorFlow::hideAttributesEditorFlowModul() {
    // simply hide modul
    hide();
}


bool
GNEFrameAttributesModuls::AttributesEditorFlow::isAttributesEditorFlowModulShown() const {
    return shown();
}


void
GNEFrameAttributesModuls::AttributesEditorFlow::refreshAttributeEditorFlow() {
    if (myAttributesEditorParent->getEditedACs().size() > 0) {
        // simply refresh every flow attribute
        refreshEnd();
        refreshNumber();
        refreshVehsPerHour();
        refreshPeriod();
        refreshProbability();
    }
}


long
GNEFrameAttributesModuls::AttributesEditorFlow::onCmdSetFlowAttribute(FXObject* obj, FXSelector, void*) {
    // obtain undoList (To improve code legibly)
    GNEUndoList* undoList = myAttributesEditorParent->getFrameParent()->myViewNet->getUndoList();
    SumoXMLAttr attr = SUMO_ATTR_NOTHING;
    std::string value;
    // check what check button was pressed
    if (obj == myValueEndTextField) {
        attr = SUMO_ATTR_END;
        value = myValueEndTextField->getText().text();
    } else if (obj == myValueNumberTextField) {
        attr = SUMO_ATTR_NUMBER;
        value = myValueNumberTextField->getText().text();
    } else if (obj == myValueVehsPerHourTextField) {
        // check attribute
        if (myAttributesEditorParent->getEditedACs().front()->getTagProperty().hasAttribute(SUMO_ATTR_VEHSPERHOUR)) {
            attr = SUMO_ATTR_VEHSPERHOUR;
        } else {
            attr = SUMO_ATTR_PERSONSPERHOUR;
        }
        value = myValueVehsPerHourTextField->getText().text();
    } else if (obj == myValuePeriodTextField) {
        attr = SUMO_ATTR_PERIOD;
        value = myValuePeriodTextField->getText().text();
    } else if (obj == myValueProbabilityTextField) {
        attr = SUMO_ATTR_PROB;
        value = myValueProbabilityTextField->getText().text();
    } else {
        throw ProcessError("Invalid text field");
    }
    // write debug (for Netedit tests)
    WRITE_DEBUG("Selected checkBox for attribute '" + toString(attr) + "'");
    // check if we're editing multiple attributes
    if (myAttributesEditorParent->getEditedACs().size() > 1) {
        undoList->p_begin("Change multiple " + toString(attr) + " attributes");
    }
    // enable attribute with undo/redo
    for (const auto& i : myAttributesEditorParent->getEditedACs()) {
        i->setAttribute(attr, value, undoList);
    }
    // check if we're editing multiple attributes
    if (myAttributesEditorParent->getEditedACs().size() > 1) {
        undoList->p_end();
    }
    // refresh Attributes edito parent
    refreshAttributeEditorFlow();
    return 1;
}


long
GNEFrameAttributesModuls::AttributesEditorFlow::onCmdSelectFlowRadioButton(FXObject* obj, FXSelector, void*) {
    // obtain undoList (To improve code legibly)
    GNEUndoList* undoList = myAttributesEditorParent->getFrameParent()->myViewNet->getUndoList();
    SumoXMLAttr attr = SUMO_ATTR_NOTHING;
    // check what check button was pressed
    if (obj == myAttributeEndRadioButton) {
        attr = SUMO_ATTR_END;
    } else if (obj == myAttributeNumberRadioButton) {
        attr = SUMO_ATTR_NUMBER;
    } else if (obj == myAttributeVehsPerHourRadioButton) {
        attr = SUMO_ATTR_VEHSPERHOUR;
    } else if (obj == myAttributePeriodRadioButton) {
        attr = SUMO_ATTR_PERIOD;
    } else if (obj == myAttributeProbabilityRadioButton) {
        attr = SUMO_ATTR_PROB;
    } else {
        throw ProcessError("Invalid Radio Button");
    }
    // write debug (for Netedit tests)
    WRITE_DEBUG("Selected checkBox for attribute '" + toString(attr) + "'");
    // begin undo list
    if (myAttributesEditorParent->getEditedACs().size() > 1) {
        undoList->p_begin("enable multiple " + toString(attr) + " attributes");
    } else {
        undoList->p_begin("enable attribute '" + toString(attr) + "'");
    }
    // enable attribute with undo/redo
    for (const auto& i : myAttributesEditorParent->getEditedACs()) {
        i->enableAttribute(attr, undoList);
    }
    // end undoList
    undoList->p_end();
    // refresh Attributes edito parent
    refreshAttributeEditorFlow();
    return 1;
}


void
GNEFrameAttributesModuls::AttributesEditorFlow::refreshEnd() {
    // first we need to check if all attributes are enabled or disabled
    int allAttributesEnabledOrDisabled = 0;
    for (const auto& i : myAttributesEditorParent->getEditedACs()) {
        allAttributesEnabledOrDisabled += i->isAttributeEnabled(SUMO_ATTR_END);
    }
    if (allAttributesEnabledOrDisabled == (int)myAttributesEditorParent->getEditedACs().size()) {
        // Declare a set of occuring values and insert attribute's values of item
        std::set<std::string> occuringValues;
        for (const auto& values : myAttributesEditorParent->getEditedACs()) {
            occuringValues.insert(values->getAttribute(SUMO_ATTR_END));
        }
        // get current value
        std::ostringstream endValue;
        for (auto it_val = occuringValues.begin(); it_val != occuringValues.end(); it_val++) {
            if (it_val != occuringValues.begin()) {
                endValue << " ";
            }
            endValue << *it_val;
        }
        // set radio button and text field
        myValueEndTextField->enable();
        myValueEndTextField->setText(endValue.str().c_str());
        myAttributeEndRadioButton->setCheck(TRUE);
    } else {
        // disable radio button and text field
        myValueEndTextField->disable();
        // check if we set an special value in textField
        if ((allAttributesEnabledOrDisabled > 0) && (myAttributesEditorParent->getEditedACs().size() > 1)) {
            myValueEndTextField->setText("Different flow attributes");
        } else if (myAttributesEditorParent->getEditedACs().size() == 1) {
            myValueEndTextField->setText(myAttributesEditorParent->getEditedACs().front()->getAlternativeValueForDisabledAttributes(SUMO_ATTR_END).c_str());
        } else {
            myValueEndTextField->setText("");
        }
        myAttributeEndRadioButton->setCheck(FALSE);
    }
}


void
GNEFrameAttributesModuls::AttributesEditorFlow::refreshNumber() {
    // first we need to check if all attributes are enabled or disabled
    int allAttributesEnabledOrDisabled = 0;
    for (const auto& i : myAttributesEditorParent->getEditedACs()) {
        allAttributesEnabledOrDisabled += i->isAttributeEnabled(SUMO_ATTR_NUMBER);
    }
    if (allAttributesEnabledOrDisabled == (int)myAttributesEditorParent->getEditedACs().size()) {
        // Declare a set of occuring values and insert attribute's values of item
        std::set<std::string> occuringValues;
        for (const auto& values : myAttributesEditorParent->getEditedACs()) {
            occuringValues.insert(values->getAttribute(SUMO_ATTR_NUMBER));
        }
        // get current value
        std::ostringstream numberValues;
        for (auto it_val = occuringValues.begin(); it_val != occuringValues.end(); it_val++) {
            if (it_val != occuringValues.begin()) {
                numberValues << " ";
            }
            numberValues << *it_val;
        }
        // set radio button and text field
        myValueNumberTextField->enable();
        myValueNumberTextField->setText(numberValues.str().c_str());
        myAttributeNumberRadioButton->setCheck(TRUE);
    } else {
        // disable radio button
        myValueNumberTextField->disable();
        // check if we set an special value in textField
        if ((allAttributesEnabledOrDisabled > 0) && (myAttributesEditorParent->getEditedACs().size() > 1)) {
            myValueNumberTextField->setText("Different flow attributes");
        } else if (myAttributesEditorParent->getEditedACs().size() == 1) {
            myValueNumberTextField->setText(myAttributesEditorParent->getEditedACs().front()->getAlternativeValueForDisabledAttributes(SUMO_ATTR_NUMBER).c_str());
        } else {
            myValueNumberTextField->setText("");
        }
        myAttributeNumberRadioButton->setCheck(FALSE);
    }
}


void
GNEFrameAttributesModuls::AttributesEditorFlow::refreshVehsPerHour() {
    // declare attribute
    SumoXMLAttr attr = SUMO_ATTR_VEHSPERHOUR;
    // first change attribute
    if (myAttributesEditorParent->getEditedACs().front()->getTagProperty().hasAttribute(SUMO_ATTR_PERSONSPERHOUR)) {
        attr = SUMO_ATTR_PERSONSPERHOUR;
    }
    // update radio button
    myAttributeVehsPerHourRadioButton->setText(toString(attr).c_str());
    // we need to check if all attributes are enabled or disabled
    int allAttributesEnabledOrDisabled = 0;
    for (const auto& i : myAttributesEditorParent->getEditedACs()) {
        allAttributesEnabledOrDisabled += i->isAttributeEnabled(attr);
    }
    if (allAttributesEnabledOrDisabled == (int)myAttributesEditorParent->getEditedACs().size()) {
        // Declare a set of occuring values and insert attribute's values of item
        std::set<std::string> occuringValues;
        for (const auto& values : myAttributesEditorParent->getEditedACs()) {
            occuringValues.insert(values->getAttribute(attr));
        }
        // get current value
        std::ostringstream vehsPerHourValues;
        for (auto it_val = occuringValues.begin(); it_val != occuringValues.end(); it_val++) {
            if (it_val != occuringValues.begin()) {
                vehsPerHourValues << " ";
            }
            vehsPerHourValues << *it_val;
        }
        // set radio button and text field
        myValueVehsPerHourTextField->enable();
        myValueVehsPerHourTextField->setText(vehsPerHourValues.str().c_str());
        myAttributeVehsPerHourRadioButton->setCheck(TRUE);
    } else {
        // disable radio button
        myValueVehsPerHourTextField->disable();
        // check if we set an special value in textField
        if ((allAttributesEnabledOrDisabled > 0) && (myAttributesEditorParent->getEditedACs().size() > 1)) {
            myValueVehsPerHourTextField->setText("Different flow attributes");
        } else if (myAttributesEditorParent->getEditedACs().size() == 1) {
            myValueVehsPerHourTextField->setText(myAttributesEditorParent->getEditedACs().front()->getAlternativeValueForDisabledAttributes(attr).c_str());
        } else {
            myValueVehsPerHourTextField->setText("");
        }
        myAttributeVehsPerHourRadioButton->setCheck(FALSE);
    }
}


void
GNEFrameAttributesModuls::AttributesEditorFlow::refreshPeriod() {
    // first we need to check if all attributes are enabled or disabled
    int allAttributesEnabledOrDisabled = 0;
    for (const auto& i : myAttributesEditorParent->getEditedACs()) {
        allAttributesEnabledOrDisabled += i->isAttributeEnabled(SUMO_ATTR_PERIOD);
    }
    if (allAttributesEnabledOrDisabled == (int)myAttributesEditorParent->getEditedACs().size()) {
        // Declare a set of occuring values and insert attribute's values of item
        std::set<std::string> occuringValues;
        for (const auto& values : myAttributesEditorParent->getEditedACs()) {
            occuringValues.insert(values->getAttribute(SUMO_ATTR_PERIOD));
        }
        // get current value
        std::ostringstream periodValues;
        for (auto it_val = occuringValues.begin(); it_val != occuringValues.end(); it_val++) {
            if (it_val != occuringValues.begin()) {
                periodValues << " ";
            }
            periodValues << *it_val;
        }
        // set radio button and text field
        myValuePeriodTextField->enable();
        myValuePeriodTextField->setText(periodValues.str().c_str());
        myAttributePeriodRadioButton->setCheck(TRUE);
    } else {
        // disable radio button and text field
        myValuePeriodTextField->disable();
        // check if we set an special value in textField
        if ((allAttributesEnabledOrDisabled > 0) && (myAttributesEditorParent->getEditedACs().size() > 1)) {
            myValuePeriodTextField->setText("Different flow attributes");
        } else if (myAttributesEditorParent->getEditedACs().size() == 1) {
            myValuePeriodTextField->setText(myAttributesEditorParent->getEditedACs().front()->getAlternativeValueForDisabledAttributes(SUMO_ATTR_PERIOD).c_str());
        } else {
            myValuePeriodTextField->setText("");
        }
        myAttributePeriodRadioButton->setCheck(FALSE);
    }
}


void
GNEFrameAttributesModuls::AttributesEditorFlow::refreshProbability() {
    // first we need to check if all attributes are enabled or disabled
    int allAttributesEnabledOrDisabled = 0;
    for (const auto& i : myAttributesEditorParent->getEditedACs()) {
        allAttributesEnabledOrDisabled += i->isAttributeEnabled(SUMO_ATTR_PROB);
    }
    if (allAttributesEnabledOrDisabled == (int)myAttributesEditorParent->getEditedACs().size()) {
        // Declare a set of occuring values and insert attribute's values of item
        std::set<std::string> occuringValues;
        for (const auto& values : myAttributesEditorParent->getEditedACs()) {
            occuringValues.insert(values->getAttribute(SUMO_ATTR_PROB));
        }
        // get current value
        std::ostringstream probabilityValues;
        for (auto it_val = occuringValues.begin(); it_val != occuringValues.end(); it_val++) {
            if (it_val != occuringValues.begin()) {
                probabilityValues << " ";
            }
            probabilityValues << *it_val;
        }
        // set radio button and text field
        myValueProbabilityTextField->enable();
        myValueProbabilityTextField->setText(probabilityValues.str().c_str());
        myAttributeProbabilityRadioButton->enable();
        myAttributeProbabilityRadioButton->setCheck(TRUE);
    } else {
        // disable radio button and text field
        myValueProbabilityTextField->disable();
        // check if we set an special value in textField
        if ((allAttributesEnabledOrDisabled > 0) && (myAttributesEditorParent->getEditedACs().size() > 1)) {
            myValueProbabilityTextField->setText("Different flow attributes");
        } else if (myAttributesEditorParent->getEditedACs().size() == 1) {
            myValueProbabilityTextField->setText(myAttributesEditorParent->getEditedACs().front()->getAlternativeValueForDisabledAttributes(SUMO_ATTR_PROB).c_str());
        } else {
            myValueProbabilityTextField->setText("");
        }
        myAttributeProbabilityRadioButton->setCheck(FALSE);
    }
}

// ---------------------------------------------------------------------------
// GNEFrameAttributesModuls::AttributesEditorExtended- methods
// ---------------------------------------------------------------------------

GNEFrameAttributesModuls::AttributesEditorExtended::AttributesEditorExtended(GNEFrame* frameParent) :
    FXGroupBox(frameParent->myContentFrame, "Extended attributes", GUIDesignGroupBoxFrame),
    myFrameParent(frameParent) {
    // Create open dialog button
    new FXButton(this, "Open attributes editor", nullptr, this, MID_GNE_SET_ATTRIBUTE_DIALOG, GUIDesignButton);
}


GNEFrameAttributesModuls::AttributesEditorExtended::~AttributesEditorExtended() {}


void
GNEFrameAttributesModuls::AttributesEditorExtended::showAttributesEditorExtendedModul() {
    show();
}


void
GNEFrameAttributesModuls::AttributesEditorExtended::hideAttributesEditorExtendedModul() {
    hide();
}


long
GNEFrameAttributesModuls::AttributesEditorExtended::onCmdOpenDialog(FXObject*, FXSelector, void*) {
    // open AttributesCreator extended dialog
    myFrameParent->attributesEditorExtendedDialogOpened();
    return 1;
}

// ---------------------------------------------------------------------------
// GNEFrameAttributesModuls::ParametersEditor - methods
// ---------------------------------------------------------------------------

GNEFrameAttributesModuls::ParametersEditor::ParametersEditor(GNEFrame* inspectorFrameParent, std::string title) :
    FXGroupBox(inspectorFrameParent->myContentFrame, title.c_str(), GUIDesignGroupBoxFrame),
    myFrameParent(inspectorFrameParent),
    myAC(nullptr),
    myAttrType(Parameterised::ParameterisedAttrType::STRING) {
    // set first letter upper
    title[0] = (char)tolower(title[0]);
    // create textfield and buttons
    myTextFieldParameters = new FXTextField(this, GUIDesignTextFieldNCol, this, MID_GNE_SET_ATTRIBUTE, GUIDesignTextField);
    myButtonEditParameters = new FXButton(this, ("Edit " + title).c_str(), nullptr, this, MID_GNE_SET_ATTRIBUTE_DIALOG, GUIDesignButton);
}


GNEFrameAttributesModuls::ParametersEditor::~ParametersEditor() {}


void
GNEFrameAttributesModuls::ParametersEditor::showParametersEditor(GNEAttributeCarrier* AC, std::string title) {
    if ((AC != nullptr) && AC->getTagProperty().hasParameters() && (title.size() > 0)) {
        // set AC
        myAC = AC;
        myACs.clear();
        // obtain a copy of AC parameters
        if (myAC) {
            // update flag
            if (myAC->getTagProperty().hasDoubleParameters()) {
                myAttrType = Parameterised::ParameterisedAttrType::DOUBLE;
            } else {
                myAttrType = Parameterised::ParameterisedAttrType::STRING;
            }
            // obtain string
            std::string parametersStr = myAC->getAttribute(GNE_ATTR_PARAMETERS);
            // clear parameters
            myParameters.clear();
            // separate value in a vector of string using | as separator
            StringTokenizer parameters(parametersStr, "|", true);
            // iterate over all values
            while (parameters.hasNext()) {
                // obtain key and value and save it in myParameters
                const std::vector<std::string> keyValue = StringTokenizer(parameters.next(), "=", true).getVector();
                if (keyValue.size() == 2) {
                    myParameters[keyValue.front()] = keyValue.back();
                }
            }
        }
        // set title and button
        myButtonEditParameters->setText(("Edit " + title).c_str());
        // set first letter upper
        title[0] = (char)toupper(title[0]);
        // change
        setText(title.c_str());
        // refresh ParametersEditor
        refreshParametersEditor();
        // show groupbox
        show();
    } else {
        hide();
    }
}


void
GNEFrameAttributesModuls::ParametersEditor::showParametersEditor(std::vector<GNEAttributeCarrier*> ACs, std::string title) {
    if ((ACs.size() > 0) && ACs.front()->getTagProperty().hasParameters()) {
        myAC = nullptr;
        myACs = ACs;
        // check if parameters are different
        bool differentsParameters = false;
        std::string firstParameters = myACs.front()->getAttribute(GNE_ATTR_PARAMETERS);
        for (auto i : myACs) {
            if (firstParameters != i->getAttribute(GNE_ATTR_PARAMETERS)) {
                differentsParameters = true;
            }
        }
        // set parameters editor
        if (differentsParameters) {
            myParameters.clear();
        } else {
            // update flag
            if (myACs.front()->getTagProperty().hasDoubleParameters()) {
                myAttrType = Parameterised::ParameterisedAttrType::DOUBLE;
            } else {
                myAttrType = Parameterised::ParameterisedAttrType::STRING;
            }
            // obtain string
            std::string parametersStr = myACs.front()->getAttribute(GNE_ATTR_PARAMETERS);
            // clear parameters
            myParameters.clear();
            // separate value in a vector of string using | as separator
            std::vector<std::string> parameters = StringTokenizer(parametersStr, "|", true).getVector();
            // iterate over all values
            for (const auto& i : parameters) {
                // obtain key and value and save it in myParameters
                std::vector<std::string> keyValue = StringTokenizer(i, "=", true).getVector();
                myParameters[keyValue.front()] = keyValue.back();
            }
        }
        // set title and button
        myButtonEditParameters->setText(("Edit " + title).c_str());
        // set first letter upper
        title[0] = (char)toupper(title[0]);
        // change
        setText(title.c_str());
        // refresh ParametersEditor
        refreshParametersEditor();
        // show groupbox
        show();
    } else {
        hide();
    }
}


void
GNEFrameAttributesModuls::ParametersEditor::hideParametersEditor() {
    myAC = nullptr;
    // hide groupbox
    hide();
}


void
GNEFrameAttributesModuls::ParametersEditor::refreshParametersEditor() {
    // update text field depending of AC
    if (myAC) {
        myTextFieldParameters->setText(myAC->getAttribute(GNE_ATTR_PARAMETERS).c_str());
        myTextFieldParameters->setTextColor(FXRGB(0, 0, 0));
        // disable myTextFieldParameters if Tag correspond to an network element but we're in demand mode (or vice versa), disable all elements
        if (isSupermodeValid(myFrameParent->myViewNet, myAC)) {
            myTextFieldParameters->enable();
            myButtonEditParameters->enable();
        } else {
            myTextFieldParameters->disable();
            myButtonEditParameters->disable();
        }
    } else if (myACs.size() > 0) {
        // check if parameters of all inspected ACs are different
        std::string parameters = myACs.front()->getAttribute(GNE_ATTR_PARAMETERS);
        for (auto i : myACs) {
            if (parameters != i->getAttribute(GNE_ATTR_PARAMETERS)) {
                parameters = "different parameters";
            }
        }
        myTextFieldParameters->setText(parameters.c_str());
        myTextFieldParameters->setTextColor(FXRGB(0, 0, 0));
        // disable myTextFieldParameters if we're in demand mode and inspected AC isn't a demand element (or viceversa)
        if (isSupermodeValid(myFrameParent->myViewNet, myACs.front())) {
            myTextFieldParameters->enable();
            myButtonEditParameters->enable();
        } else {
            myTextFieldParameters->disable();
            myButtonEditParameters->disable();
        }
    }
}


const std::map<std::string, std::string>&
GNEFrameAttributesModuls::ParametersEditor::getParametersMap() const {
    return myParameters;
}


std::string
GNEFrameAttributesModuls::ParametersEditor::getParametersStr() const {
    std::string result;
    // Generate an string using the following structure: "key1=value1|key2=value2|...
    for (const auto& i : myParameters) {
        result += i.first + "=" + i.second + "|";
    }
    // remove the last "|"
    if (!result.empty()) {
        result.pop_back();
    }
    return result;
}

std::vector<std::pair<std::string, std::string> >
GNEFrameAttributesModuls::ParametersEditor::getParametersVectorStr() const {
    std::vector<std::pair<std::string, std::string> > result;
    // Generate an vector string using the following structure: "<key1,value1>, <key2, value2>,...
    for (const auto& i : myParameters) {
        result.push_back(std::make_pair(i.first, i.second));
    }
    return result;
}


void
GNEFrameAttributesModuls::ParametersEditor::setParameters(const std::vector<std::pair<std::string, std::string> >& parameters) {
    // declare result string
    std::string result;
    // Generate an string using the following structure: "key1=value1|key2=value2|...
    for (const auto& i : parameters) {
        result += i.first + "=" + i.second + "|";
    }
    // remove the last "|"
    if (!result.empty()) {
        result.pop_back();
    }
    // set result in textField (and call onCmdEditParameters)
    myTextFieldParameters->setText(result.c_str(), TRUE);
}


GNEFrame*
GNEFrameAttributesModuls::ParametersEditor::getFrameParent() const {
    return myFrameParent;
}


Parameterised::ParameterisedAttrType
GNEFrameAttributesModuls::ParametersEditor::getAttrType() const {
    return myAttrType;
}


long
GNEFrameAttributesModuls::ParametersEditor::onCmdEditParameters(FXObject*, FXSelector, void*) {
    // write debug information
    WRITE_DEBUG("Open parameters dialog");
    // edit parameters using dialog
    if (GNEParametersDialog(this).execute()) {
        // write debug information
        WRITE_DEBUG("Close parameters dialog");
        // set values edited in Parameter dialog in Edited AC
        if (myAC) {
            myAC->setAttribute(GNE_ATTR_PARAMETERS, getParametersStr(), myFrameParent->myViewNet->getUndoList());
        } else if (myACs.size() > 0) {
            myFrameParent->myViewNet->getUndoList()->p_begin("Change multiple parameters");
            for (auto i : myACs) {
                i->setAttribute(GNE_ATTR_PARAMETERS, getParametersStr(), myFrameParent->myViewNet->getUndoList());
            }
            myFrameParent->myViewNet->getUndoList()->p_end();
            // update frame parent after attribute sucesfully set
            myFrameParent->attributeUpdated();
        }
        // Refresh parameter editor
        refreshParametersEditor();
    } else {
        // write debug information
        WRITE_DEBUG("Cancel parameters dialog");
    }
    return 1;
}


long
GNEFrameAttributesModuls::ParametersEditor::onCmdSetParameters(FXObject*, FXSelector, void*) {
    // check if current given string is valid
    if (Parameterised::areParametersValid(myTextFieldParameters->getText().text(), true, myAttrType)) {
        // parsed parameters ok, then set text field black and continue
        myTextFieldParameters->setTextColor(FXRGB(0, 0, 0));
        myTextFieldParameters->killFocus();
        // obtain parameters "key=value"
        std::vector<std::string> parameters = StringTokenizer(myTextFieldParameters->getText().text(), "|", true).getVector();
        // clear current existent parameters and set parsed parameters
        myParameters.clear();
        // iterate over parameters
        for (const auto& parameter : parameters) {
            // obtain key, value
            std::vector<std::string> keyParam = StringTokenizer(parameter, "=", true).getVector();
            // save it in myParameters
            myParameters[keyParam.front()] = keyParam.back();
        }
        // overwritte myTextFieldParameters (to remove duplicated parameters
        myTextFieldParameters->setText(getParametersStr().c_str(), FALSE);
        // if we're editing parameters of an AttributeCarrier, set it
        if (myAC) {
            // begin undo list
            myFrameParent->myViewNet->getUndoList()->p_begin("change parameters");
            // set parameters
            myAC->setAttribute(GNE_ATTR_PARAMETERS, getParametersStr(), myFrameParent->myViewNet->getUndoList());
            // end undo list
            myFrameParent->myViewNet->getUndoList()->p_end();
        } else if (myACs.size() > 0) {
            // begin undo list
            myFrameParent->myViewNet->getUndoList()->p_begin("change multiple parameters");
            // set parameters in all ACs
            for (const auto& i : myACs) {
                i->setAttribute(GNE_ATTR_PARAMETERS, getParametersStr(), myFrameParent->myViewNet->getUndoList());
            }
            // end undo list
            myFrameParent->myViewNet->getUndoList()->p_end();
            // update frame parent after attribute sucesfully set
            myFrameParent->attributeUpdated();
        }
    } else {
        myTextFieldParameters->setTextColor(FXRGB(255, 0, 0));
    }
    return 1;
}

// ---------------------------------------------------------------------------
// GNEFrameAttributesModuls::DrawingShape - methods
// ---------------------------------------------------------------------------

GNEFrameAttributesModuls::DrawingShape::DrawingShape(GNEFrame* frameParent) :
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
            << "  draws shape boundary.\n"
            << "- 'Stop drawing' or ENTER\n"
            << "  creates shape.\n"
            << "- 'Shift + Click'removes\n"
            << "  last created point.\n"
            << "- 'Abort drawing' or ESC\n"
            << "  removes drawed shape.";
    myInformationLabel = new FXLabel(this, information.str().c_str(), 0, GUIDesignLabelFrameInformation);
    // disable stop and abort functions as init
    myStopDrawingButton->disable();
    myAbortDrawingButton->disable();
}


GNEFrameAttributesModuls::DrawingShape::~DrawingShape() {}


void GNEFrameAttributesModuls::DrawingShape::showDrawingShape() {
    // abort current drawing before show
    abortDrawing();
    // show FXGroupBox
    FXGroupBox::show();
}


void GNEFrameAttributesModuls::DrawingShape::hideDrawingShape() {
    // abort current drawing before hide
    abortDrawing();
    // show FXGroupBox
    FXGroupBox::hide();
}


void
GNEFrameAttributesModuls::DrawingShape::startDrawing() {
    // Only start drawing if DrawingShape modul is shown
    if (shown()) {
        // change buttons
        myStartDrawingButton->disable();
        myStopDrawingButton->enable();
        myAbortDrawingButton->enable();
    }
}


void
GNEFrameAttributesModuls::DrawingShape::stopDrawing() {
    // try to build shape
    if (myFrameParent->shapeDrawed()) {
        // clear created points
        myTemporalShapeShape.clear();
        myFrameParent->myViewNet->updateViewNet();
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
GNEFrameAttributesModuls::DrawingShape::abortDrawing() {
    // clear created points
    myTemporalShapeShape.clear();
    myFrameParent->myViewNet->updateViewNet();
    // change buttons
    myStartDrawingButton->enable();
    myStopDrawingButton->disable();
    myAbortDrawingButton->disable();
}


void
GNEFrameAttributesModuls::DrawingShape::addNewPoint(const Position& P) {
    if (myStopDrawingButton->isEnabled()) {
        myTemporalShapeShape.push_back(P);
    } else {
        throw ProcessError("A new point cannot be added if drawing wasn't started");
    }
}


void
GNEFrameAttributesModuls::DrawingShape::removeLastPoint() {

}


const PositionVector&
GNEFrameAttributesModuls::DrawingShape::getTemporalShape() const {
    return myTemporalShapeShape;
}


bool
GNEFrameAttributesModuls::DrawingShape::isDrawing() const {
    return myStopDrawingButton->isEnabled();
}


void
GNEFrameAttributesModuls::DrawingShape::setDeleteLastCreatedPoint(bool value) {
    myDeleteLastCreatedPoint = value;
}


bool
GNEFrameAttributesModuls::DrawingShape::getDeleteLastCreatedPoint() {
    return myDeleteLastCreatedPoint;
}


long
GNEFrameAttributesModuls::DrawingShape::onCmdStartDrawing(FXObject*, FXSelector, void*) {
    startDrawing();
    return 0;
}


long
GNEFrameAttributesModuls::DrawingShape::onCmdStopDrawing(FXObject*, FXSelector, void*) {
    stopDrawing();
    return 0;
}


long
GNEFrameAttributesModuls::DrawingShape::onCmdAbortDrawing(FXObject*, FXSelector, void*) {
    abortDrawing();
    return 0;
}

// ---------------------------------------------------------------------------
// GNEFrameAttributesModuls::NeteditAttributes- methods
// ---------------------------------------------------------------------------

GNEFrameAttributesModuls::NeteditAttributes::NeteditAttributes(GNEFrame* frameParent) :
    FXGroupBox(frameParent->myContentFrame, "Netedit attributes", GUIDesignGroupBoxFrame),
    myFrameParent(frameParent),
    myCurrentLengthValid(true),
    myActualAdditionalReferencePoint(GNE_ADDITIONALREFERENCEPOINT_LEFT) {
    // Create FXListBox for the reference points and fill it
    myReferencePointMatchBox = new FXComboBox(this, GUIDesignComboBoxNCol, this, MID_GNE_SET_ATTRIBUTE, GUIDesignComboBox);
    myReferencePointMatchBox->appendItem("reference left");
    myReferencePointMatchBox->appendItem("reference right");
    myReferencePointMatchBox->appendItem("reference center");
    // Create Frame for Length Label and textField
    myLengthFrame = new FXHorizontalFrame(this, GUIDesignAuxiliarHorizontalFrame);
    new FXLabel(myLengthFrame, toString(SUMO_ATTR_LENGTH).c_str(), 0, GUIDesignLabelAttribute);
    myLengthTextField = new FXTextField(myLengthFrame, GUIDesignTextFieldNCol, this, MID_GNE_SET_ATTRIBUTE, GUIDesignTextField);
    myLengthTextField->setText("10");
    // Create Frame for block movement label and checkBox (By default disabled)
    myBlockMovementFrame = new FXHorizontalFrame(this, GUIDesignAuxiliarHorizontalFrame);
    new FXLabel(myBlockMovementFrame, "block move", 0, GUIDesignLabelAttribute);
    myBlockMovementCheckButton = new FXCheckButton(myBlockMovementFrame, "false", this, MID_GNE_SET_ATTRIBUTE, GUIDesignCheckButton);
    myBlockMovementCheckButton->setCheck(false);
    // Create Frame for block shape label and checkBox (By default disabled)
    myBlockShapeFrame = new FXHorizontalFrame(this, GUIDesignAuxiliarHorizontalFrame);
    new FXLabel(myBlockShapeFrame, "block shape", 0, GUIDesignLabelAttribute);
    myBlockShapeCheckButton = new FXCheckButton(myBlockShapeFrame, "false", this, MID_GNE_SET_ATTRIBUTE, GUIDesignCheckButton);
    // Create Frame for block close polygon and checkBox (By default disabled)
    myCloseShapeFrame = new FXHorizontalFrame(this, GUIDesignAuxiliarHorizontalFrame);
    new FXLabel(myCloseShapeFrame, "Close shape", 0, GUIDesignLabelAttribute);
    myCloseShapeCheckButton = new FXCheckButton(myCloseShapeFrame, "false", this, MID_GNE_SET_ATTRIBUTE, GUIDesignCheckButton);
    myBlockShapeCheckButton->setCheck(false);
    // Create Frame for center element after creation (By default enabled)
    myCenterViewAfterCreationFrame = new FXHorizontalFrame(this, GUIDesignAuxiliarHorizontalFrame);
    new FXLabel(myCenterViewAfterCreationFrame, "Center view", 0, GUIDesignLabelAttribute);
    myCenterViewAfterCreationButton = new FXCheckButton(myCenterViewAfterCreationFrame, "false", this, MID_GNE_SET_ATTRIBUTE, GUIDesignCheckButton);
    myCenterViewAfterCreationButton->setCheck(true);
    // Create help button
    helpReferencePoint = new FXButton(this, "Help", 0, this, MID_HELP, GUIDesignButtonRectangular);
    // Set visible items
    myReferencePointMatchBox->setNumVisible((int)myReferencePointMatchBox->getNumItems());
}


GNEFrameAttributesModuls::NeteditAttributes::~NeteditAttributes() {}


void
GNEFrameAttributesModuls::NeteditAttributes::showNeteditAttributesModul(const GNETagProperties& tagProperty) {
    // we assume that frame will not be show
    bool showFrame = false;
    // check if length text field has to be showed
    if (tagProperty.canMaskStartEndPos()) {
        myLengthFrame->show();
        myReferencePointMatchBox->show();
        showFrame = true;
    } else {
        myLengthFrame->hide();
        myReferencePointMatchBox->hide();
    }
    // check if block movement check button has to be show
    if (tagProperty.canBlockMovement()) {
        myBlockMovementFrame->show();
        showFrame = true;
    } else {
        myBlockMovementFrame->hide();
    }
    // check if block shape check button has to be show
    if (tagProperty.canBlockShape()) {
        myBlockShapeFrame->show();
        showFrame = true;
    } else {
        myBlockShapeFrame->hide();
    }
    // check if close shape check button has to be show
    if (tagProperty.canCloseShape()) {
        myCloseShapeFrame->show();
        showFrame = true;
    } else {
        myCloseShapeFrame->hide();
    }
    // check if center camera after creation check button has to be show
    if (tagProperty.canCenterCameraAfterCreation()) {
        myCenterViewAfterCreationFrame->show();
        showFrame = true;
    } else {
        myCenterViewAfterCreationFrame->hide();
    }
    // if at least one element is show, show modul
    if (showFrame) {
        recalc();
        show();
    } else {
        hide();
    }
}


void
GNEFrameAttributesModuls::NeteditAttributes::hideNeteditAttributesModul() {
    hide();
}


bool
GNEFrameAttributesModuls::NeteditAttributes::getNeteditAttributesAndValues(std::map<SumoXMLAttr, std::string>& valuesMap, const GNELane* lane) const {
    // check if we need to obtain a start and end position over an edge
    if (myReferencePointMatchBox->shown()) {
        // we need a valid lane to calculate position over lane
        if (lane == nullptr) {
            return false;
        } else if (myCurrentLengthValid) {
            // Obtain position of the mouse over lane (limited over grid)
            double mousePositionOverLane = lane->getLaneShape().nearest_offset_to_point2D(myFrameParent->myViewNet->snapToActiveGrid(myFrameParent->myViewNet->getPositionInformation())) / lane->getLengthGeometryFactor();
            // check if current reference point is valid
            if (myActualAdditionalReferencePoint == GNE_ADDITIONALREFERENCEPOINT_INVALID) {
                std::string errorMessage = "Current selected reference point isn't valid";
                myFrameParent->myViewNet->setStatusBarText(errorMessage);
                // Write Warning in console if we're in testing mode
                WRITE_DEBUG(errorMessage);
                return false;
            } else {
                // obtain length
                double length = GNEAttributeCarrier::parse<double>(myLengthTextField->getText().text());
                // set start and end position
                valuesMap[SUMO_ATTR_STARTPOS] = toString(setStartPosition(mousePositionOverLane, length));
                valuesMap[SUMO_ATTR_ENDPOS] = toString(setEndPosition(mousePositionOverLane, length));
            }
        } else {
            return false;
        }
    }
    // Save block value if element can be blocked
    if (myBlockMovementCheckButton->shown()) {
        if (myBlockMovementCheckButton->getCheck() == 1) {
            valuesMap[GNE_ATTR_BLOCK_MOVEMENT] = "1";
        } else {
            valuesMap[GNE_ATTR_BLOCK_MOVEMENT] = "0";
        }
    }
    // Save block shape value if shape's element can be blocked
    if (myBlockShapeCheckButton->shown()) {
        if (myBlockShapeCheckButton->getCheck() == 1) {
            valuesMap[GNE_ATTR_BLOCK_SHAPE] = "1";
        } else {
            valuesMap[GNE_ATTR_BLOCK_SHAPE] = "0";
        }
    }
    // Save close shape value if shape's element can be closed
    if (myCloseShapeCheckButton->shown()) {
        if (myCloseShapeCheckButton->getCheck() == 1) {
            valuesMap[GNE_ATTR_CLOSE_SHAPE] = "1";
        } else {
            valuesMap[GNE_ATTR_CLOSE_SHAPE] = "0";
        }
    }
    // check center element after creation
    if (myCenterViewAfterCreationButton->shown() && (myCenterViewAfterCreationButton->getCheck() == 1)) {
        valuesMap[GNE_ATTR_CENTER_AFTER_CREATION] = "1";
    }
    // all ok, then return true to continue creating element
    return true;
}


long
GNEFrameAttributesModuls::NeteditAttributes::onCmdSetNeteditAttribute(FXObject* obj, FXSelector, void*) {
    if (obj == myBlockMovementCheckButton) {
        if (myBlockMovementCheckButton->getCheck()) {
            myBlockMovementCheckButton->setText("true");
        } else {
            myBlockMovementCheckButton->setText("false");
        }
    } else if (obj == myBlockShapeCheckButton) {
        if (myBlockShapeCheckButton->getCheck()) {
            myBlockShapeCheckButton->setText("true");
        } else {
            myBlockShapeCheckButton->setText("false");
        }
    } else if (obj == myCloseShapeCheckButton) {
        if (myCloseShapeCheckButton->getCheck()) {
            myCloseShapeCheckButton->setText("true");
        } else {
            myCloseShapeCheckButton->setText("false");
        }
    } else if (obj == myCenterViewAfterCreationButton) {
        if (myCenterViewAfterCreationButton->getCheck()) {
            myCenterViewAfterCreationButton->setText("true");
        } else {
            myCenterViewAfterCreationButton->setText("false");
        }
    } else if (obj == myLengthTextField) {
        // change color of text field depending of the input length
        if (GNEAttributeCarrier::canParse<double>(myLengthTextField->getText().text()) &&
                GNEAttributeCarrier::parse<double>(myLengthTextField->getText().text()) > 0) {
            myLengthTextField->setTextColor(FXRGB(0, 0, 0));
            myLengthTextField->killFocus();
            myCurrentLengthValid = true;
        } else {
            myLengthTextField->setTextColor(FXRGB(255, 0, 0));
            myCurrentLengthValid = false;
        }
        // Update aditional frame
        update();
    } else if (obj == myReferencePointMatchBox) {
        // Cast actual reference point type
        if (myReferencePointMatchBox->getText() == "reference left") {
            myReferencePointMatchBox->setTextColor(FXRGB(0, 0, 0));
            myActualAdditionalReferencePoint = GNE_ADDITIONALREFERENCEPOINT_LEFT;
            myLengthTextField->enable();
        } else if (myReferencePointMatchBox->getText() == "reference right") {
            myReferencePointMatchBox->setTextColor(FXRGB(0, 0, 0));
            myActualAdditionalReferencePoint = GNE_ADDITIONALREFERENCEPOINT_RIGHT;
            myLengthTextField->enable();
        } else if (myReferencePointMatchBox->getText() == "reference center") {
            myLengthTextField->enable();
            myReferencePointMatchBox->setTextColor(FXRGB(0, 0, 0));
            myActualAdditionalReferencePoint = GNE_ADDITIONALREFERENCEPOINT_CENTER;
            myLengthTextField->enable();
        } else {
            myReferencePointMatchBox->setTextColor(FXRGB(255, 0, 0));
            myActualAdditionalReferencePoint = GNE_ADDITIONALREFERENCEPOINT_INVALID;
            myLengthTextField->disable();
        }
    }

    return 1;
}


long
GNEFrameAttributesModuls::NeteditAttributes::onCmdHelp(FXObject*, FXSelector, void*) {
    // Create dialog box
    FXDialogBox* additionalNeteditAttributesHelpDialog = new FXDialogBox(this, "Netedit Parameters Help", GUIDesignDialogBox);
    additionalNeteditAttributesHelpDialog->setIcon(GUIIconSubSys::getIcon(GUIIcon::MODEADDITIONAL));
    // set help text
    std::ostringstream help;
    help
            << "- Referece point: Mark the initial position of the additional element.\n"
            << "  Example: If you want to create a busStop with a length of 30 in the point 100 of the lane:\n"
            << "  - Reference Left will create it with startPos = 70 and endPos = 100.\n"
            << "  - Reference Right will create it with startPos = 100 and endPos = 130.\n"
            << "  - Reference Center will create it with startPos = 85 and endPos = 115.\n"
            << "\n"
            << "- Block movement: if is enabled, the created additional element will be blocked. i.e. cannot be moved with\n"
            << "  the mouse. This option can be modified inspecting element.\n"
            << "- Center view: if is enabled, view will be center over created element.";
    // Create label with the help text
    new FXLabel(additionalNeteditAttributesHelpDialog, help.str().c_str(), 0, GUIDesignLabelFrameInformation);
    // Create horizontal separator
    new FXHorizontalSeparator(additionalNeteditAttributesHelpDialog, GUIDesignHorizontalSeparator);
    // Create frame for OK Button
    FXHorizontalFrame* myHorizontalFrameOKButton = new FXHorizontalFrame(additionalNeteditAttributesHelpDialog, GUIDesignAuxiliarHorizontalFrame);
    // Create Button Close (And two more horizontal frames to center it)
    new FXHorizontalFrame(myHorizontalFrameOKButton, GUIDesignAuxiliarHorizontalFrame);
    new FXButton(myHorizontalFrameOKButton, "OK\t\tclose", GUIIconSubSys::getIcon(GUIIcon::ACCEPT), additionalNeteditAttributesHelpDialog, FXDialogBox::ID_ACCEPT, GUIDesignButtonOK);
    new FXHorizontalFrame(myHorizontalFrameOKButton, GUIDesignAuxiliarHorizontalFrame);
    // Write Warning in console if we're in testing mode
    WRITE_DEBUG("Opening NeteditAttributes help dialog");
    // create Dialog
    additionalNeteditAttributesHelpDialog->create();
    // show in the given position
    additionalNeteditAttributesHelpDialog->show(PLACEMENT_CURSOR);
    // refresh APP
    getApp()->refresh();
    // open as modal dialog (will block all windows until stop() or stopModal() is called)
    getApp()->runModalFor(additionalNeteditAttributesHelpDialog);
    // Write Warning in console if we're in testing mode
    WRITE_DEBUG("Closing NeteditAttributes help dialog");
    return 1;
    /**********
    help from PolygonFrame
            << "- Block movement: If enabled, the created polygon element will be blocked. i.e. cannot be moved with\n"
            << "  the mouse. This option can be modified inspecting element.\n"
            << "\n"
            << "- Block shape: If enabled, the shape of created polygon element will be blocked. i.e. their geometry points\n"
            << "  cannot be edited be moved with the mouse. This option can be modified inspecting element.\n"
            << "\n"
            << "- Close shape: If enabled, the created polygon element will be closed. i.e. the last created geometry point\n"
            << "  will be connected with the first geometry point automatically. This option can be modified inspecting element.";

    ****************/
}


double
GNEFrameAttributesModuls::NeteditAttributes::setStartPosition(double positionOfTheMouseOverLane, double lengthOfAdditional) const {
    switch (myActualAdditionalReferencePoint) {
        case GNE_ADDITIONALREFERENCEPOINT_LEFT:
            return positionOfTheMouseOverLane;
        case GNE_ADDITIONALREFERENCEPOINT_RIGHT:
            return positionOfTheMouseOverLane - lengthOfAdditional;
        case GNE_ADDITIONALREFERENCEPOINT_CENTER:
            return positionOfTheMouseOverLane - lengthOfAdditional / 2;
        default:
            throw InvalidArgument("Reference Point invalid");
    }
}


double
GNEFrameAttributesModuls::NeteditAttributes::setEndPosition(double positionOfTheMouseOverLane, double lengthOfAdditional)  const {
    switch (myActualAdditionalReferencePoint) {
        case GNE_ADDITIONALREFERENCEPOINT_LEFT:
            return positionOfTheMouseOverLane + lengthOfAdditional;
        case GNE_ADDITIONALREFERENCEPOINT_RIGHT:
            return positionOfTheMouseOverLane;
        case GNE_ADDITIONALREFERENCEPOINT_CENTER:
            return positionOfTheMouseOverLane + lengthOfAdditional / 2;
        default:
            throw InvalidArgument("Reference Point invalid");
    }
}


bool
GNEFrameAttributesModuls::isSupermodeValid(const GNEViewNet* viewNet, const GNEAttributeCarrier* AC) {
    if (viewNet->getEditModes().isCurrentSupermodeNetwork() && (
                AC->getTagProperty().isNetworkElement() ||
                AC->getTagProperty().isAdditionalElement() ||
                AC->getTagProperty().isShape() ||
                AC->getTagProperty().isTAZElement())) {
        return true;
    } else if (viewNet->getEditModes().isCurrentSupermodeDemand() &&
               AC->getTagProperty().isDemandElement()) {
        return true;
    } else if (viewNet->getEditModes().isCurrentSupermodeData() &&
               AC->getTagProperty().isDataElement()) {
        return true;
    } else {
        return false;
    }
}


bool
GNEFrameAttributesModuls::isSupermodeValid(const GNEViewNet* viewNet, const GNEAttributeProperties& ACAttr) {
    if (ACAttr.getTagPropertyParent().isNetworkElement() || ACAttr.getTagPropertyParent().isAdditionalElement() ||
            ACAttr.getTagPropertyParent().isShape() || ACAttr.getTagPropertyParent().isTAZElement()) {
        return (viewNet->getEditModes().isCurrentSupermodeNetwork());
    } else if (ACAttr.getTagPropertyParent().isDemandElement()) {
        return (viewNet->getEditModes().isCurrentSupermodeDemand());
    } else if (ACAttr.getTagPropertyParent().isDataElement()) {
        return (viewNet->getEditModes().isCurrentSupermodeData());
    } else {
        return false;
    }
}

/****************************************************************************/
