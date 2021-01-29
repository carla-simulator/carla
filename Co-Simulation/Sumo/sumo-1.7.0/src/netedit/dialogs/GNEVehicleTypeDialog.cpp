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
/// @file    GNEVehicleTypeDialog.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Jan 2019
///
// Dialog for edit vehicleTypes
/****************************************************************************/
#include <config.h>

#include <netedit/GNENet.h>
#include <netedit/GNEUndoList.h>
#include <netedit/GNEViewNet.h>
#include <netedit/changes/GNEChange_DemandElement.h>
#include <netedit/elements/demand/GNEVehicleType.h>
#include <utils/emissions/PollutantsInterface.h>
#include <utils/gui/div/GUIDesigns.h>
#include <utils/gui/windows/GUIAppEnum.h>

#include "GNEVehicleTypeDialog.h"


// ===========================================================================
// FOX callback mapping
// ===========================================================================

FXDEFMAP(GNEVehicleTypeDialog::VTypeAtributes) VTypeAtributesMap[] = {
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_SET_ATTRIBUTE,          GNEVehicleTypeDialog::VTypeAtributes::onCmdSetAttribute),
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_SET_ATTRIBUTE_DIALOG,   GNEVehicleTypeDialog::VTypeAtributes::onCmdSetAttributeDialog)
};

FXDEFMAP(GNEVehicleTypeDialog::CarFollowingModelParameters) CarFollowingModelParametersMap[] = {
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_SET_ATTRIBUTE,  GNEVehicleTypeDialog::CarFollowingModelParameters::onCmdSetVariable),
};

// Object implementation
FXIMPLEMENT(GNEVehicleTypeDialog::VTypeAtributes,               FXVerticalFrame,    VTypeAtributesMap,              ARRAYNUMBER(VTypeAtributesMap))
FXIMPLEMENT(GNEVehicleTypeDialog::CarFollowingModelParameters,  FXGroupBox,         CarFollowingModelParametersMap, ARRAYNUMBER(CarFollowingModelParametersMap))

// ===========================================================================
// member method definitions
// ===========================================================================

// ---------------------------------------------------------------------------
// GNEVehicleTypeDialog::VClassRow - methods
// ---------------------------------------------------------------------------

GNEVehicleTypeDialog::VTypeAtributes::VClassRow::VClassRow(VTypeAtributes* VTypeAtributesParent, FXVerticalFrame* column) :
    FXHorizontalFrame(column, GUIDesignAuxiliarHorizontalFrame),
    myVTypeAtributesParent(VTypeAtributesParent) {
    // create two auxiliar frames
    FXVerticalFrame* verticalFrameLabelAndComboBox = new FXVerticalFrame(this, GUIDesignAuxiliarVerticalFrame);
    // create FXComboBox for VClass
    new FXLabel(verticalFrameLabelAndComboBox, toString(SUMO_ATTR_VCLASS).c_str(), nullptr, GUIDesignLabelAttribute150);
    myComboBoxVClass = new FXComboBox(verticalFrameLabelAndComboBox, GUIDesignComboBoxNCol,
                                      VTypeAtributesParent, MID_GNE_SET_ATTRIBUTE, GUIDesignComboBox);
    myComboBoxVClassLabelImage = new FXLabel(this, "", nullptr, GUIDesignLabelTickedIcon180x46);
    myComboBoxVClassLabelImage->setBackColor(FXRGBA(255, 255, 255, 255));
    // fill combo Box with all allowed VClass for the current edited VType
    for (const auto& i : myVTypeAtributesParent->myVehicleTypeDialog->getEditedDemandElement()->getTagProperty().getAttributeProperties(SUMO_ATTR_VCLASS).getDiscreteValues()) {
        if (i != SumoVehicleClassStrings.getString(SVC_IGNORING)) {
            myComboBoxVClass->appendItem(i.c_str());
        }
    }
    // only show as maximum 10 VClasses
    if (myComboBoxVClass->getNumItems() > 10) {
        myComboBoxVClass->setNumVisible(10);
    } else {
        myComboBoxVClass->setNumVisible(myComboBoxVClass->getNumItems());
    }
}


SUMOVehicleClass
GNEVehicleTypeDialog::VTypeAtributes::VClassRow::setVariable() {
    // set color of myComboBoxVClass, depending if current value is valid or not
    myComboBoxVClass->setTextColor(FXRGB(0, 0, 0));
    if (myVTypeAtributesParent->myVehicleTypeDialog->myEditedDemandElement->isValid(SUMO_ATTR_VCLASS, myComboBoxVClass->getText().text())) {
        myComboBoxVClass->setTextColor(FXRGB(0, 0, 0));
        // check if  VType has to be updated
        if (myVTypeAtributesParent->myVehicleTypeDialog->myEditedDemandElement->getAttribute(SUMO_ATTR_VCLASS) != myComboBoxVClass->getText().text()) {
            // update VClass in VType
            myVTypeAtributesParent->myVehicleTypeDialog->myEditedDemandElement->setAttribute(SUMO_ATTR_VCLASS, myComboBoxVClass->getText().text(),
                    myVTypeAtributesParent->myVehicleTypeDialog->myEditedDemandElement->getNet()->getViewNet()->getUndoList());
            // update label image
            setVClassLabelImage();
            // obtain default vType parameters
            SUMOVTypeParameter::VClassDefaultValues defaultVTypeParameters(myVTypeAtributesParent->myVehicleTypeDialog->myEditedDemandElement->getVClass());
            // check if mutable rows haben to be updated
            if (!myVTypeAtributesParent->myVehicleTypeDialog->myEditedDemandElement->isAttributeEnabled(SUMO_ATTR_LENGTH)) {
                myVTypeAtributesParent->myLength->updateValue(toString(defaultVTypeParameters.length));
            }
            if (!myVTypeAtributesParent->myVehicleTypeDialog->myEditedDemandElement->isAttributeEnabled(SUMO_ATTR_MINGAP)) {
                myVTypeAtributesParent->myMinGap->updateValue(toString(defaultVTypeParameters.minGap));
            }
            if (!myVTypeAtributesParent->myVehicleTypeDialog->myEditedDemandElement->isAttributeEnabled(SUMO_ATTR_MAXSPEED)) {
                myVTypeAtributesParent->myMaxSpeed->updateValue(toString(defaultVTypeParameters.maxSpeed));
            }
            if (!myVTypeAtributesParent->myVehicleTypeDialog->myEditedDemandElement->isAttributeEnabled(SUMO_ATTR_SPEEDFACTOR)) {
                myVTypeAtributesParent->mySpeedFactor->updateValue(toString(defaultVTypeParameters.speedFactor.getParameter()[0]));
            }
            if (!myVTypeAtributesParent->myVehicleTypeDialog->myEditedDemandElement->isAttributeEnabled(SUMO_ATTR_SPEEDDEV)) {
                myVTypeAtributesParent->mySpeedDev->updateValue(toString(defaultVTypeParameters.speedFactor.getParameter()[1]));
            }
            if (!myVTypeAtributesParent->myVehicleTypeDialog->myEditedDemandElement->isAttributeEnabled(SUMO_ATTR_EMISSIONCLASS)) {
                myVTypeAtributesParent->myEmissionClass->updateValue(toString(defaultVTypeParameters.emissionClass));
            }
            if (!myVTypeAtributesParent->myVehicleTypeDialog->myEditedDemandElement->isAttributeEnabled(SUMO_ATTR_WIDTH)) {
                myVTypeAtributesParent->myWidth->updateValue(toString(defaultVTypeParameters.width));
            }
            if (!myVTypeAtributesParent->myVehicleTypeDialog->myEditedDemandElement->isAttributeEnabled(SUMO_ATTR_HEIGHT)) {
                myVTypeAtributesParent->myHeight->updateValue(toString(defaultVTypeParameters.height));
            }
            if (!myVTypeAtributesParent->myVehicleTypeDialog->myEditedDemandElement->isAttributeEnabled(SUMO_ATTR_OSGFILE)) {
                myVTypeAtributesParent->myOSGFile->updateValue(toString(defaultVTypeParameters.osgFile));
            }
            if (!myVTypeAtributesParent->myVehicleTypeDialog->myEditedDemandElement->isAttributeEnabled(SUMO_ATTR_PERSON_CAPACITY)) {
                myVTypeAtributesParent->myPersonCapacity->updateValue(toString(defaultVTypeParameters.personCapacity));
            }
            if (!myVTypeAtributesParent->myVehicleTypeDialog->myEditedDemandElement->isAttributeEnabled(SUMO_ATTR_CONTAINER_CAPACITY)) {
                myVTypeAtributesParent->myContainerCapacity->updateValue(toString(defaultVTypeParameters.containerCapacity));
            }
            if (!myVTypeAtributesParent->myVehicleTypeDialog->myEditedDemandElement->isAttributeEnabled(SUMO_ATTR_CARRIAGE_LENGTH)) {
                myVTypeAtributesParent->myCarriageLength->updateValue(toString(defaultVTypeParameters.containerCapacity));
            }
            if (!myVTypeAtributesParent->myVehicleTypeDialog->myEditedDemandElement->isAttributeEnabled(SUMO_ATTR_LOCOMOTIVE_LENGTH)) {
                myVTypeAtributesParent->myLocomotiveLength->updateValue(toString(defaultVTypeParameters.containerCapacity));
            }
        }
    } else {
        myComboBoxVClass->setTextColor(FXRGB(255, 0, 0));
        myVTypeAtributesParent->myVehicleTypeDialog->myVehicleTypeValid = false;
        myVTypeAtributesParent->myVehicleTypeDialog->myInvalidAttr = SUMO_ATTR_VCLASS;
    }
    return myVTypeAtributesParent->myVehicleTypeDialog->myEditedDemandElement->getVClass();
}


SUMOVehicleClass
GNEVehicleTypeDialog::VTypeAtributes::VClassRow::updateValue() {
    myComboBoxVClass->setText(myVTypeAtributesParent->myVehicleTypeDialog->myEditedDemandElement->getAttribute(SUMO_ATTR_VCLASS).c_str());
    setVClassLabelImage();
    return myVTypeAtributesParent->myVehicleTypeDialog->myEditedDemandElement->getVClass();
}


void
GNEVehicleTypeDialog::VTypeAtributes::VClassRow::setVClassLabelImage() {
    // by default vclass is passenger
    if (myVTypeAtributesParent->myVehicleTypeDialog->myEditedDemandElement->getAttribute(SUMO_ATTR_VCLASS).empty()) {
        myComboBoxVClassLabelImage->setIcon(GUIIconSubSys::getIcon(GUIIcon::VCLASS_PASSENGER));
    } else {
        // set Icon in label depending of current VClass
        switch (myVTypeAtributesParent->myVehicleTypeDialog->myEditedDemandElement->getVClass()) {
            case SVC_PRIVATE:
                myComboBoxVClassLabelImage->setIcon(GUIIconSubSys::getIcon(GUIIcon::VCLASS_PRIVATE));
                break;
            case SVC_EMERGENCY:
                myComboBoxVClassLabelImage->setIcon(GUIIconSubSys::getIcon(GUIIcon::VCLASS_EMERGENCY));
                break;
            case SVC_AUTHORITY:
                myComboBoxVClassLabelImage->setIcon(GUIIconSubSys::getIcon(GUIIcon::VCLASS_AUTHORITY));
                break;
            case SVC_ARMY:
                myComboBoxVClassLabelImage->setIcon(GUIIconSubSys::getIcon(GUIIcon::VCLASS_ARMY));
                break;
            case SVC_VIP:
                myComboBoxVClassLabelImage->setIcon(GUIIconSubSys::getIcon(GUIIcon::VCLASS_VIP));
                break;
            case SVC_PASSENGER:
                myComboBoxVClassLabelImage->setIcon(GUIIconSubSys::getIcon(GUIIcon::VCLASS_PASSENGER));
                break;
            case SVC_HOV:
                myComboBoxVClassLabelImage->setIcon(GUIIconSubSys::getIcon(GUIIcon::VCLASS_HOV));
                break;
            case SVC_TAXI:
                myComboBoxVClassLabelImage->setIcon(GUIIconSubSys::getIcon(GUIIcon::VCLASS_TAXI));
                break;
            case SVC_BUS:
                myComboBoxVClassLabelImage->setIcon(GUIIconSubSys::getIcon(GUIIcon::VCLASS_BUS));
                break;
            case SVC_COACH:
                myComboBoxVClassLabelImage->setIcon(GUIIconSubSys::getIcon(GUIIcon::VCLASS_COACH));
                break;
            case SVC_DELIVERY:
                myComboBoxVClassLabelImage->setIcon(GUIIconSubSys::getIcon(GUIIcon::VCLASS_DELIVERY));
                break;
            case SVC_TRUCK:
                myComboBoxVClassLabelImage->setIcon(GUIIconSubSys::getIcon(GUIIcon::VCLASS_TRUCK));
                break;
            case SVC_TRAILER:
                myComboBoxVClassLabelImage->setIcon(GUIIconSubSys::getIcon(GUIIcon::VCLASS_TRAILER));
                break;
            case SVC_TRAM:
                myComboBoxVClassLabelImage->setIcon(GUIIconSubSys::getIcon(GUIIcon::VCLASS_TRAM));
                break;
            case SVC_RAIL_URBAN:
                myComboBoxVClassLabelImage->setIcon(GUIIconSubSys::getIcon(GUIIcon::VCLASS_RAIL_URBAN));
                break;
            case SVC_RAIL:
                myComboBoxVClassLabelImage->setIcon(GUIIconSubSys::getIcon(GUIIcon::VCLASS_RAIL));
                break;
            case SVC_RAIL_ELECTRIC:
                myComboBoxVClassLabelImage->setIcon(GUIIconSubSys::getIcon(GUIIcon::VCLASS_RAIL_ELECTRIC));
                break;
            case SVC_RAIL_FAST:
                myComboBoxVClassLabelImage->setIcon(GUIIconSubSys::getIcon(GUIIcon::VCLASS_RAIL_ELECTRIC));
                break;
            case SVC_MOTORCYCLE:
                myComboBoxVClassLabelImage->setIcon(GUIIconSubSys::getIcon(GUIIcon::VCLASS_MOTORCYCLE));
                break;
            case SVC_MOPED:
                myComboBoxVClassLabelImage->setIcon(GUIIconSubSys::getIcon(GUIIcon::VCLASS_MOPED));
                break;
            case SVC_BICYCLE:
                myComboBoxVClassLabelImage->setIcon(GUIIconSubSys::getIcon(GUIIcon::VCLASS_BICYCLE));
                break;
            case SVC_PEDESTRIAN:
                myComboBoxVClassLabelImage->setIcon(GUIIconSubSys::getIcon(GUIIcon::VCLASS_PEDESTRIAN));
                break;
            case SVC_E_VEHICLE:
                myComboBoxVClassLabelImage->setIcon(GUIIconSubSys::getIcon(GUIIcon::VCLASS_EVEHICLE));
                break;
            case SVC_SHIP:
                myComboBoxVClassLabelImage->setIcon(GUIIconSubSys::getIcon(GUIIcon::VCLASS_SHIP));
                break;
            case SVC_CUSTOM1:
                myComboBoxVClassLabelImage->setIcon(GUIIconSubSys::getIcon(GUIIcon::VCLASS_CUSTOM1));
                break;
            case SVC_CUSTOM2:
                myComboBoxVClassLabelImage->setIcon(GUIIconSubSys::getIcon(GUIIcon::VCLASS_CUSTOM2));
                break;
            default:
                myComboBoxVClassLabelImage->setIcon(GUIIconSubSys::getIcon(GUIIcon::VCLASS_IGNORING));
                break;
        }
    }
}

// ---------------------------------------------------------------------------
// GNEVehicleTypeDialog::VShapeRow - methods
// ---------------------------------------------------------------------------

GNEVehicleTypeDialog::VTypeAtributes::VShapeRow::VShapeRow(VTypeAtributes* VTypeAtributesParent, FXVerticalFrame* column) :
    FXHorizontalFrame(column, GUIDesignAuxiliarHorizontalFrame),
    myVTypeAtributesParent(VTypeAtributesParent) {
    // create two auxiliar frames
    FXVerticalFrame* verticalFrameLabelAndComboBox = new FXVerticalFrame(this, GUIDesignAuxiliarVerticalFrame);
    // create combo for for vehicle shapes
    new FXLabel(verticalFrameLabelAndComboBox, toString(SUMO_ATTR_GUISHAPE).c_str(), nullptr, GUIDesignLabelAttribute150);
    myComboBoxShape = new FXComboBox(verticalFrameLabelAndComboBox, GUIDesignComboBoxNCol,
                                     VTypeAtributesParent, MID_GNE_SET_ATTRIBUTE, GUIDesignComboBox);
    myComboBoxShapeLabelImage = new FXLabel(this, "", nullptr, GUIDesignLabelTickedIcon180x46);
    myComboBoxShapeLabelImage->setBackColor(FXRGBA(255, 255, 255, 255));
    // fill combo Box with all vehicle shapes
    std::vector<std::string> VShapeStrings = SumoVehicleShapeStrings.getStrings();
    for (auto i : VShapeStrings) {
        if (i != SumoVehicleShapeStrings.getString(SVS_UNKNOWN)) {
            myComboBoxShape->appendItem(i.c_str());
        }
    }
    // only show 10 Shapes
    myComboBoxShape->setNumVisible(10);
}


void
GNEVehicleTypeDialog::VTypeAtributes::VShapeRow::setVariable() {
    // set color of myComboBoxShape, depending if current value is valid or not
    if (myVTypeAtributesParent->myVehicleTypeDialog->myEditedDemandElement->isValid(SUMO_ATTR_GUISHAPE, myComboBoxShape->getText().text())) {
        myComboBoxShape->setTextColor(FXRGB(0, 0, 0));
        myVTypeAtributesParent->myVehicleTypeDialog->myEditedDemandElement->setAttribute(SUMO_ATTR_GUISHAPE, myComboBoxShape->getText().text(),
                myVTypeAtributesParent->myVehicleTypeDialog->myEditedDemandElement->getNet()->getViewNet()->getUndoList());
        setVShapeLabelImage();
    } else {
        myComboBoxShape->setTextColor(FXRGB(255, 0, 0));
        myVTypeAtributesParent->myVehicleTypeDialog->myVehicleTypeValid = false;
        myVTypeAtributesParent->myVehicleTypeDialog->myInvalidAttr = SUMO_ATTR_GUISHAPE;
    }
}


void
GNEVehicleTypeDialog::VTypeAtributes::VShapeRow::updateValues() {
    myComboBoxShape->setText(myVTypeAtributesParent->myVehicleTypeDialog->myEditedDemandElement->getAttribute(SUMO_ATTR_GUISHAPE).c_str());
    setVShapeLabelImage();
}


void
GNEVehicleTypeDialog::VTypeAtributes::VShapeRow::setVShapeLabelImage() {
    // set Icon in label depending of current VClass
    switch (getVehicleShapeID(myVTypeAtributesParent->myVehicleTypeDialog->myEditedDemandElement->getAttribute(SUMO_ATTR_GUISHAPE))) {
        case SVS_UNKNOWN:
            myComboBoxShapeLabelImage->setIcon(GUIIconSubSys::getIcon(GUIIcon::VSHAPE_UNKNOWN));
            break;
        case SVS_PEDESTRIAN:
            myComboBoxShapeLabelImage->setIcon(GUIIconSubSys::getIcon(GUIIcon::VSHAPE_PEDESTRIAN));
            break;
        case SVS_BICYCLE:
            myComboBoxShapeLabelImage->setIcon(GUIIconSubSys::getIcon(GUIIcon::VSHAPE_BICYCLE));
            break;
        case SVS_MOPED:
            myComboBoxShapeLabelImage->setIcon(GUIIconSubSys::getIcon(GUIIcon::VSHAPE_MOPED));
            break;
        case SVS_MOTORCYCLE:
            myComboBoxShapeLabelImage->setIcon(GUIIconSubSys::getIcon(GUIIcon::VSHAPE_MOTORCYCLE));
            break;
        case SVS_PASSENGER:
            myComboBoxShapeLabelImage->setIcon(GUIIconSubSys::getIcon(GUIIcon::VSHAPE_PASSENGER));
            break;
        case SVS_PASSENGER_SEDAN:
            myComboBoxShapeLabelImage->setIcon(GUIIconSubSys::getIcon(GUIIcon::VSHAPE_PASSENGER_SEDAN));
            break;
        case SVS_PASSENGER_HATCHBACK:
            myComboBoxShapeLabelImage->setIcon(GUIIconSubSys::getIcon(GUIIcon::VSHAPE_PASSENGER_HATCHBACK));
            break;
        case SVS_PASSENGER_WAGON:
            myComboBoxShapeLabelImage->setIcon(GUIIconSubSys::getIcon(GUIIcon::VSHAPE_PASSENGER_WAGON));
            break;
        case SVS_PASSENGER_VAN:
            myComboBoxShapeLabelImage->setIcon(GUIIconSubSys::getIcon(GUIIcon::VSHAPE_PASSENGER_VAN));
            break;
        case SVS_DELIVERY:
            myComboBoxShapeLabelImage->setIcon(GUIIconSubSys::getIcon(GUIIcon::VSHAPE_DELIVERY));
            break;
        case SVS_TRUCK:
            myComboBoxShapeLabelImage->setIcon(GUIIconSubSys::getIcon(GUIIcon::VSHAPE_TRUCK));
            break;
        case SVS_TRUCK_SEMITRAILER:
            myComboBoxShapeLabelImage->setIcon(GUIIconSubSys::getIcon(GUIIcon::VSHAPE_TRUCK_SEMITRAILER));
            break;
        case SVS_TRUCK_1TRAILER:
            myComboBoxShapeLabelImage->setIcon(GUIIconSubSys::getIcon(GUIIcon::VSHAPE_TRUCK_1TRAILER));
            break;
        case SVS_BUS:
            myComboBoxShapeLabelImage->setIcon(GUIIconSubSys::getIcon(GUIIcon::VSHAPE_BUS));
            break;
        case SVS_BUS_COACH:
            myComboBoxShapeLabelImage->setIcon(GUIIconSubSys::getIcon(GUIIcon::VSHAPE_BUS_COACH));
            break;
        case SVS_BUS_FLEXIBLE:
            myComboBoxShapeLabelImage->setIcon(GUIIconSubSys::getIcon(GUIIcon::VSHAPE_BUS_FLEXIBLE));
            break;
        case SVS_BUS_TROLLEY:
            myComboBoxShapeLabelImage->setIcon(GUIIconSubSys::getIcon(GUIIcon::VSHAPE_BUS_TROLLEY));
            break;
        case SVS_RAIL:
            myComboBoxShapeLabelImage->setIcon(GUIIconSubSys::getIcon(GUIIcon::VSHAPE_RAIL));
            break;
        case SVS_RAIL_CAR:
            myComboBoxShapeLabelImage->setIcon(GUIIconSubSys::getIcon(GUIIcon::VSHAPE_RAIL_CAR));
            break;
        case SVS_RAIL_CARGO:
            myComboBoxShapeLabelImage->setIcon(GUIIconSubSys::getIcon(GUIIcon::VSHAPE_RAIL_CARGO));
            break;
        case SVS_E_VEHICLE:
            myComboBoxShapeLabelImage->setIcon(GUIIconSubSys::getIcon(GUIIcon::VSHAPE_E_VEHICLE));
            break;
        case SVS_ANT:
            myComboBoxShapeLabelImage->setIcon(GUIIconSubSys::getIcon(GUIIcon::VSHAPE_ANT));
            break;
        case SVS_SHIP:
            myComboBoxShapeLabelImage->setIcon(GUIIconSubSys::getIcon(GUIIcon::VSHAPE_SHIP));
            break;
        case SVS_EMERGENCY:
        case SVS_FIREBRIGADE:
            myComboBoxShapeLabelImage->setIcon(GUIIconSubSys::getIcon(GUIIcon::VSHAPE_EMERGENCY));
            break;
        case SVS_POLICE:
            myComboBoxShapeLabelImage->setIcon(GUIIconSubSys::getIcon(GUIIcon::VSHAPE_POLICE));
            break;
        case SVS_RICKSHAW:
            myComboBoxShapeLabelImage->setIcon(GUIIconSubSys::getIcon(GUIIcon::VSHAPE_RICKSHAW));
            break;
        default:
            myComboBoxShapeLabelImage->setIcon(GUIIconSubSys::getIcon(GUIIcon::VCLASS_IGNORING));
            break;
    }
}

// ---------------------------------------------------------------------------
// GNEVehicleTypeDialog::VTypeAtributes - methods
// ---------------------------------------------------------------------------

GNEVehicleTypeDialog::VTypeAtributes::VTypeAttributeRow::VTypeAttributeRow(VTypeAtributes* VTypeAtributesParent, FXVerticalFrame* verticalFrame, const SumoXMLAttr attr, const RowAttrType rowAttrType, const  std::vector<std::string>& values) :
    FXHorizontalFrame(verticalFrame, GUIDesignAuxiliarHorizontalFrame),
    myVTypeAtributesParent(VTypeAtributesParent),
    myAttr(attr),
    myRowAttrType(rowAttrType),
    myButton(nullptr),
    myTextField(nullptr),
    myComboBox(nullptr) {
    // first check if we have to create a button or a label
    if ((rowAttrType == ROWTYPE_COLOR) || (rowAttrType == ROWTYPE_FILENAME)) {
        myButton = new FXButton(this, filterAttributeName(attr), nullptr, VTypeAtributesParent, MID_GNE_SET_ATTRIBUTE_DIALOG, GUIDesignButtonRectangular150x23);
    } else {
        new FXLabel(this, filterAttributeName(attr), nullptr, GUIDesignLabelAttribute150);
    }
    // now check if we have to create a textfield or a ComboBox
    if ((rowAttrType == ROWTYPE_STRING) || (rowAttrType == ROWTYPE_COLOR)) {
        myTextField = new FXTextField(this, GUIDesignTextFieldNCol, VTypeAtributesParent, MID_GNE_SET_ATTRIBUTE, GUIDesignTextFielWidth180);
    } else if (rowAttrType == ROWTYPE_FILENAME) {
        myTextField = new FXTextField(this, GUIDesignTextFieldNCol, VTypeAtributesParent, MID_GNE_SET_ATTRIBUTE, GUIDesignTextFielWidth180);
    } else if (rowAttrType == ROWTYPE_COMBOBOX) {
        myComboBox = new FXComboBox(this, GUIDesignComboBoxNCol, VTypeAtributesParent, MID_GNE_SET_ATTRIBUTE, GUIDesignComboBoxWidth180);
        // fill combo Box with values
        for (auto i : values) {
            myComboBox->appendItem(i.c_str());
        }
        // set 10 visible elements as maximum
        if (myComboBox->getNumItems() < 10) {
            myComboBox->setNumVisible(myComboBox->getNumItems());
        } else {
            myComboBox->setNumVisible(10);
        }
    } else {
        throw ProcessError("Invalid row type");
    }
}


void
GNEVehicleTypeDialog::VTypeAtributes::VTypeAttributeRow::setVariable() {
    if (myRowAttrType == ROWTYPE_COMBOBOX) {
        // set color of myComboBox, depending if current value is valid or not
        if (myVTypeAtributesParent->myVehicleTypeDialog->myEditedDemandElement->isValid(myAttr, myComboBox->getText().text())) {
            myVTypeAtributesParent->myVehicleTypeDialog->myEditedDemandElement->setAttribute(myAttr, myComboBox->getText().text(),
                    myVTypeAtributesParent->myVehicleTypeDialog->myEditedDemandElement->getNet()->getViewNet()->getUndoList());
            // update value after setting it
            updateValue();
        } else {
            myComboBox->setTextColor(FXRGB(255, 0, 0));
            // mark VType as invalid
            myVTypeAtributesParent->myVehicleTypeDialog->myVehicleTypeValid = false;
            myVTypeAtributesParent->myVehicleTypeDialog->myInvalidAttr = myAttr;
        }
    } else if (myRowAttrType == ROWTYPE_COLOR) {
        // set color of myTextFieldColor, depending if current value is valid or not
        if (myVTypeAtributesParent->myVehicleTypeDialog->myEditedDemandElement->isValid(SUMO_ATTR_COLOR, myTextField->getText().text())) {
            // set color depending if is a default value
            if (myVTypeAtributesParent->myVehicleTypeDialog->myEditedDemandElement->getTagProperty().getDefaultValue(SUMO_ATTR_COLOR) != myTextField->getText().text()) {
                myTextField->setTextColor(FXRGB(0, 0, 0));
            } else {
                myTextField->setTextColor(FXRGB(195, 195, 195));
            }
            myVTypeAtributesParent->myVehicleTypeDialog->myEditedDemandElement->setAttribute(SUMO_ATTR_COLOR, myTextField->getText().text(), myVTypeAtributesParent->myVehicleTypeDialog->myEditedDemandElement->getNet()->getViewNet()->getUndoList());
        } else {
            myTextField->setTextColor(FXRGB(255, 0, 0));
            myVTypeAtributesParent->myVehicleTypeDialog->myVehicleTypeValid = false;
            myVTypeAtributesParent->myVehicleTypeDialog->myInvalidAttr = SUMO_ATTR_COLOR;
        }
    } else {
        // set color of textField, depending if current value is valid or not
        if (myVTypeAtributesParent->myVehicleTypeDialog->myEditedDemandElement->isValid(myAttr, myTextField->getText().text())) {
            myVTypeAtributesParent->myVehicleTypeDialog->myEditedDemandElement->setAttribute(myAttr, myTextField->getText().text(),
                    myVTypeAtributesParent->myVehicleTypeDialog->myEditedDemandElement->getNet()->getViewNet()->getUndoList());
            // update value after setting it
            updateValue();
        } else {
            myTextField->setTextColor(FXRGB(255, 0, 0));
            // mark VType as invalid
            myVTypeAtributesParent->myVehicleTypeDialog->myVehicleTypeValid = false;
            myVTypeAtributesParent->myVehicleTypeDialog->myInvalidAttr = myAttr;
        }
    }
}


void
GNEVehicleTypeDialog::VTypeAtributes::VTypeAttributeRow::setVariable(const std::string& defaultValue) {
    if (myComboBox) {
        // set color of myComboBox, depending if current value is valid or not
        if (myVTypeAtributesParent->myVehicleTypeDialog->myEditedDemandElement->isValid(myAttr, myComboBox->getText().text())) {
            myVTypeAtributesParent->myVehicleTypeDialog->myEditedDemandElement->setAttribute(myAttr, myComboBox->getText().text(),
                    myVTypeAtributesParent->myVehicleTypeDialog->myEditedDemandElement->getNet()->getViewNet()->getUndoList());
            // update value after setting it
            updateValue(defaultValue);
        } else {
            myComboBox->setTextColor(FXRGB(255, 0, 0));
            // mark VType as invalid
            myVTypeAtributesParent->myVehicleTypeDialog->myVehicleTypeValid = false;
            myVTypeAtributesParent->myVehicleTypeDialog->myInvalidAttr = myAttr;
        }
    } else {
        // set color of textField, depending if current value is valid or not
        if (myVTypeAtributesParent->myVehicleTypeDialog->myEditedDemandElement->isValid(myAttr, myTextField->getText().text())) {
            myVTypeAtributesParent->myVehicleTypeDialog->myEditedDemandElement->setAttribute(myAttr, myTextField->getText().text(),
                    myVTypeAtributesParent->myVehicleTypeDialog->myEditedDemandElement->getNet()->getViewNet()->getUndoList());
            // update value after setting it
            updateValue(defaultValue);
        } else {
            myTextField->setTextColor(FXRGB(255, 0, 0));
            // mark VType as invalid
            myVTypeAtributesParent->myVehicleTypeDialog->myVehicleTypeValid = false;
            myVTypeAtributesParent->myVehicleTypeDialog->myInvalidAttr = myAttr;
        }
    }
}


void
GNEVehicleTypeDialog::VTypeAtributes::VTypeAttributeRow::updateValue() {
    if (myRowAttrType == ROWTYPE_COMBOBOX) {
        // set text of myComboBox using current value of VType
        myComboBox->setText(myVTypeAtributesParent->myVehicleTypeDialog->myEditedDemandElement->getAttribute(myAttr).c_str());
        // set color depending if is a default value
        if (myVTypeAtributesParent->myVehicleTypeDialog->myEditedDemandElement->getTagProperty().getDefaultValue(myAttr) != myComboBox->getText().text()) {
            myComboBox->setTextColor(FXRGB(0, 0, 0));
        } else {
            myComboBox->setTextColor(FXRGB(195, 195, 195));
        }
    } else if (myRowAttrType == ROWTYPE_COLOR) {
        // set field color
        myTextField->setText(myVTypeAtributesParent->myVehicleTypeDialog->myEditedDemandElement->getAttribute(myAttr).c_str());
        // set color depending if is a default value
        if (myVTypeAtributesParent->myVehicleTypeDialog->myEditedDemandElement->getTagProperty().getDefaultValue(myAttr) != myTextField->getText().text()) {
            myTextField->setTextColor(FXRGB(0, 0, 0));
        } else {
            myTextField->setTextColor(FXRGB(195, 195, 195));
        }
    } else {
        // set text of myTextField using current value of VType
        myTextField->setText(myVTypeAtributesParent->myVehicleTypeDialog->myEditedDemandElement->getAttribute(myAttr).c_str());
        // set color depending if is a default value
        if (myVTypeAtributesParent->myVehicleTypeDialog->myEditedDemandElement->getTagProperty().getDefaultValue(myAttr) != myTextField->getText().text()) {
            myTextField->setTextColor(FXRGB(0, 0, 0));
        } else {
            myTextField->setTextColor(FXRGB(195, 195, 195));
        }
    }
}


void
GNEVehicleTypeDialog::VTypeAtributes::VTypeAttributeRow::updateValue(const std::string& defaultValue) {
    if (myComboBox) {
        // set text of myComboBox using current value of VType
        myComboBox->setText(myVTypeAtributesParent->myVehicleTypeDialog->myEditedDemandElement->getAttribute(myAttr).c_str());
        // set color depending if is a default value
        if (defaultValue != myComboBox->getText().text()) {
            myComboBox->setTextColor(FXRGB(0, 0, 0));
        } else {
            myComboBox->setTextColor(FXRGB(195, 195, 195));
        }
    } else {
        // set text of myTextField using current value of VType
        myTextField->setText(myVTypeAtributesParent->myVehicleTypeDialog->myEditedDemandElement->getAttribute(myAttr).c_str());
        // set color depending if is a default value
        if (defaultValue != myTextField->getText().text()) {
            myTextField->setTextColor(FXRGB(0, 0, 0));
        } else {
            myTextField->setTextColor(FXRGB(195, 195, 195));
        }
    }
}


const FXButton*
GNEVehicleTypeDialog::VTypeAtributes::VTypeAttributeRow::getButton() const {
    return myButton;
}


void
GNEVehicleTypeDialog::VTypeAtributes::VTypeAttributeRow::openColorDialog() {
    // create FXColorDialog
    FXColorDialog colordialog(this, tr("Color Dialog"));
    colordialog.setTarget(this);
    // If previous attribute wasn't correct, set black as default color
    if (GNEAttributeCarrier::canParse<RGBColor>(myTextField->getText().text())) {
        colordialog.setRGBA(MFXUtils::getFXColor(RGBColor::parseColor(myTextField->getText().text())));
    } else {
        colordialog.setRGBA(MFXUtils::getFXColor(RGBColor::BLACK));
    }
    // execute dialog to get a new color
    if (colordialog.execute()) {
        std::string newValue = toString(MFXUtils::getRGBColor(colordialog.getRGBA()));
        myTextField->setText(newValue.c_str());
        if (myVTypeAtributesParent->myVehicleTypeDialog->myEditedDemandElement->isValid(myAttr, newValue)) {
            myVTypeAtributesParent->myVehicleTypeDialog->myEditedDemandElement->setAttribute(myAttr, newValue, myVTypeAtributesParent->myVehicleTypeDialog->myEditedDemandElement->getNet()->getViewNet()->getUndoList());
            // If previously value was incorrect, change font color to black
            myTextField->setTextColor(FXRGB(0, 0, 0));
            myTextField->killFocus();
        }
    }
}


void
GNEVehicleTypeDialog::VTypeAtributes::VTypeAttributeRow::openImageFileDialog() {
    // get the new image file
    FXFileDialog opendialog(this, "Open Image");
    opendialog.setIcon(GUIIconSubSys::getIcon(GUIIcon::VTYPE));
    opendialog.setSelectMode(SELECTFILE_EXISTING);
    opendialog.setPatternList("All files (*)");
    if (gCurrentFolder.length() != 0) {
        opendialog.setDirectory(gCurrentFolder);
    }
    if (opendialog.execute()) {
        // update global current folder
        gCurrentFolder = opendialog.getDirectory();
        // get image path
        std::string imagePath = opendialog.getFilename().text();
        // check if image is valid
        if (myVTypeAtributesParent->myVehicleTypeDialog->myEditedDemandElement->isValid(myAttr, imagePath)) {
            myVTypeAtributesParent->myVehicleTypeDialog->myEditedDemandElement->setAttribute(myAttr, imagePath, myVTypeAtributesParent->myVehicleTypeDialog->myEditedDemandElement->getNet()->getViewNet()->getUndoList());
            myTextField->setText(imagePath.c_str());
            // If previously value was incorrect, change font color to black
            myTextField->setTextColor(FXRGB(0, 0, 0));
            myTextField->killFocus();
        }
    }
}


void
GNEVehicleTypeDialog::VTypeAtributes::VTypeAttributeRow::openOSGFileDialog() {
    // get the new file name
    FXFileDialog opendialog(this, "Open OSG File");
    opendialog.setIcon(GUIIconSubSys::getIcon(GUIIcon::VTYPE));
    opendialog.setSelectMode(SELECTFILE_EXISTING);
    opendialog.setPatternList("OSG file (*.obj)");
    if (gCurrentFolder.length() != 0) {
        opendialog.setDirectory(gCurrentFolder);
    }
    if (opendialog.execute()) {
        // update global current folder
        gCurrentFolder = opendialog.getDirectory();
        // get image path
        std::string imagePath = opendialog.getFilename().text();
        // check if image is valid
        if (myVTypeAtributesParent->myVehicleTypeDialog->myEditedDemandElement->isValid(myAttr, imagePath)) {
            myVTypeAtributesParent->myVehicleTypeDialog->myEditedDemandElement->setAttribute(myAttr, imagePath, myVTypeAtributesParent->myVehicleTypeDialog->myEditedDemandElement->getNet()->getViewNet()->getUndoList());
            myTextField->setText(imagePath.c_str());
            // If previously value was incorrect, change font color to black
            myTextField->setTextColor(FXRGB(0, 0, 0));
            myTextField->killFocus();
        }
    }
}


FXString
GNEVehicleTypeDialog::VTypeAtributes::VTypeAttributeRow::filterAttributeName(const SumoXMLAttr attr) const {
    switch (attr) {
        // JM
        case SUMO_ATTR_JM_CROSSING_GAP:
            return "crossingGap";
        case SUMO_ATTR_JM_DRIVE_AFTER_YELLOW_TIME:
            return "driveAfterYellowTime";
        case SUMO_ATTR_JM_DRIVE_AFTER_RED_TIME:
            return "driveAfterRedTime";
        case SUMO_ATTR_JM_DRIVE_RED_SPEED:
            return "driveRedSpeed";
        case SUMO_ATTR_JM_IGNORE_KEEPCLEAR_TIME:
            return "ignoreKeepClearTime";
        case SUMO_ATTR_JM_IGNORE_FOE_SPEED:
            return "ignoreFoeSpeed";
        case SUMO_ATTR_JM_IGNORE_FOE_PROB:
            return "ignoreFoeProb";
        case SUMO_ATTR_JM_SIGMA_MINOR:
            return "sigmaMinor";
        case SUMO_ATTR_JM_TIMEGAP_MINOR:
            return "timegapMinor";
        // LCM
        case SUMO_ATTR_LCA_STRATEGIC_PARAM:
            return "strategic";
        case SUMO_ATTR_LCA_COOPERATIVE_PARAM:
            return "cooperative";
        case SUMO_ATTR_LCA_SPEEDGAIN_PARAM:
            return "speedGain";
        case SUMO_ATTR_LCA_KEEPRIGHT_PARAM:
            return "keepRight";
        case SUMO_ATTR_LCA_SUBLANE_PARAM:
            return "sublane";
        case SUMO_ATTR_LCA_OPPOSITE_PARAM:
            return "opposite";
        case SUMO_ATTR_LCA_PUSHY:
            return "pushy";
        case SUMO_ATTR_LCA_PUSHYGAP:
            return "pushyGap";
        case SUMO_ATTR_LCA_ASSERTIVE:
            return "assertive";
        case SUMO_ATTR_LCA_IMPATIENCE:
            return "impatience";
        case SUMO_ATTR_LCA_TIME_TO_IMPATIENCE:
            return "timeToImpatience";
        case SUMO_ATTR_LCA_ACCEL_LAT:
            return "accelLat";
        case SUMO_ATTR_LCA_LOOKAHEADLEFT:
            return "lookaheadLeft";
        case SUMO_ATTR_LCA_SPEEDGAINRIGHT:
            return "speedGainRight";
        case SUMO_ATTR_LCA_MAXSPEEDLATSTANDING:
            return "maxSpeedLatStanding";
        case SUMO_ATTR_LCA_MAXSPEEDLATFACTOR:
            return "maxSpeedLatFactor";
        case SUMO_ATTR_LCA_TURN_ALIGNMENT_DISTANCE:
            return "turnAlignDistance";
        case SUMO_ATTR_LCA_OVERTAKE_RIGHT:
            return "overtakeRight";
        /* case SUMO_ATTR_LCA_EXPERIMENTAL1:
            return "experimental1";
        */
        default:
            return toString(attr).c_str();
    }
}

// ---------------------------------------------------------------------------
// GNEVehicleTypeDialog::VTypeAtributes - methods
// ---------------------------------------------------------------------------

GNEVehicleTypeDialog::VTypeAtributes::VTypeAtributes(GNEVehicleTypeDialog* vehicleTypeDialog, FXHorizontalFrame* column) :
    FXVerticalFrame(column, GUIDesignAuxiliarVerticalFrame),
    myVehicleTypeDialog(vehicleTypeDialog) {
    // declare two auxiliar horizontal frames
    FXHorizontalFrame* firstAuxiliarHorizontalFrame = new FXHorizontalFrame(this, GUIDesignAuxiliarHorizontalFrame);
    FXVerticalFrame* firstAuxiliarVerticalFrame = new FXVerticalFrame(firstAuxiliarHorizontalFrame, GUIDesignAuxiliarVerticalFrame);
    // create attributes for common attributes
    FXGroupBox* commonAttributes = new FXGroupBox(firstAuxiliarVerticalFrame, "Vehicle Type attributes", GUIDesignGroupBoxFrame);
    // create horizontal frame for columns of attributes
    FXHorizontalFrame* columnsBasicVTypeAttributes = new FXHorizontalFrame(commonAttributes, GUIDesignAuxiliarHorizontalFrame);
    // build left attributes
    buildAttributesA(new FXVerticalFrame(columnsBasicVTypeAttributes, GUIDesignAuxiliarFrame));
    // build right attributes
    buildAttributesB(new FXVerticalFrame(columnsBasicVTypeAttributes, GUIDesignAuxiliarFrame));
    // create GroupBox for Junction Model Attributes
    FXGroupBox* JMAttributes = new FXGroupBox(firstAuxiliarVerticalFrame, "Junction Model attributes", GUIDesignGroupBoxFrame);
    // create horizontal frame for columns of Junction Model attributes
    FXHorizontalFrame* columnsJMVTypeAttributes = new FXHorizontalFrame(JMAttributes, GUIDesignAuxiliarHorizontalFrame);
    // build left attributes
    buildJunctionModelAttributesA(new FXVerticalFrame(columnsJMVTypeAttributes, GUIDesignAuxiliarFrame));
    // build right attributes
    buildJunctionModelAttributesB(new FXVerticalFrame(columnsJMVTypeAttributes, GUIDesignAuxiliarFrame));
    // create GroupBox for Junction Model Attributes
    FXGroupBox* LCMAttributes = new FXGroupBox(firstAuxiliarHorizontalFrame, "Lane Change Model attributes", GUIDesignGroupBoxFrame);
    // create Lane Change Model Attributes
    buildLaneChangeModelAttributes(new FXVerticalFrame(LCMAttributes, GUIDesignAuxiliarFrame));
}


void
GNEVehicleTypeDialog::VTypeAtributes::buildAttributesA(FXVerticalFrame* column) {
    // 01 Create VClassRow
    myVClassRow = new VClassRow(this, column);

    // 02 create FXTextField and Label for vehicleTypeID
    FXHorizontalFrame* row = new FXHorizontalFrame(column, GUIDesignAuxiliarHorizontalFrame);
    new FXLabel(row, toString(SUMO_ATTR_ID).c_str(), nullptr, GUIDesignLabelAttribute150);
    myTextFieldVehicleTypeID = new FXTextField(row, GUIDesignTextFieldNCol, this, MID_GNE_SET_ATTRIBUTE, GUIDesignTextFielWidth180);

    // 03 create FXTextField and Button for Color
    myColor = new VTypeAttributeRow(this, column, SUMO_ATTR_COLOR, VTypeAttributeRow::RowAttrType::ROWTYPE_COLOR);

    // 04 create FXTextField and Label for Length
    myLength = new VTypeAttributeRow(this, column, SUMO_ATTR_LENGTH, VTypeAttributeRow::RowAttrType::ROWTYPE_STRING);

    // 05 create FXTextField and Label for MinGap
    myMinGap = new VTypeAttributeRow(this, column, SUMO_ATTR_MINGAP, VTypeAttributeRow::RowAttrType::ROWTYPE_STRING);

    // 06 create FXTextField and Label for MaxSpeed
    myMaxSpeed = new VTypeAttributeRow(this, column, SUMO_ATTR_MAXSPEED, VTypeAttributeRow::RowAttrType::ROWTYPE_STRING);

    // 07 create FXTextField and Label for SpeedFactor
    mySpeedFactor = new VTypeAttributeRow(this, column, SUMO_ATTR_SPEEDFACTOR, VTypeAttributeRow::RowAttrType::ROWTYPE_STRING);

    // 08 create FXTextField and Label for SpeedDev
    mySpeedDev = new VTypeAttributeRow(this, column, SUMO_ATTR_SPEEDDEV, VTypeAttributeRow::RowAttrType::ROWTYPE_STRING);

    // 09 create FXTextField and Label for EmissionClass
    myEmissionClass = new VTypeAttributeRow(this, column, SUMO_ATTR_EMISSIONCLASS, VTypeAttributeRow::RowAttrType::ROWTYPE_COMBOBOX, PollutantsInterface::getAllClassesStr());

    // 10 create FXTextField and Label for Width
    myWidth = new VTypeAttributeRow(this, column, SUMO_ATTR_WIDTH, VTypeAttributeRow::RowAttrType::ROWTYPE_STRING);

    // 11 create FXTextField and Label for Height
    myHeight = new VTypeAttributeRow(this, column, SUMO_ATTR_HEIGHT, VTypeAttributeRow::RowAttrType::ROWTYPE_STRING);

    // 12 create FXTextField and Label for Filename
    myFilename = new VTypeAttributeRow(this, column, SUMO_ATTR_IMGFILE, VTypeAttributeRow::RowAttrType::ROWTYPE_FILENAME);

    // 13 create FXTextField and Label for Filename
    myOSGFile = new VTypeAttributeRow(this, column, SUMO_ATTR_OSGFILE, VTypeAttributeRow::RowAttrType::ROWTYPE_FILENAME);

    // 14 create VTypeAttributeRow and Label for Probability
    myProbability = new VTypeAttributeRow(this, column, SUMO_ATTR_PROB, VTypeAttributeRow::RowAttrType::ROWTYPE_STRING);
}


void
GNEVehicleTypeDialog::VTypeAtributes::buildAttributesB(FXVerticalFrame* column) {
    // 01 Create VShapeRow
    myVShapeRow = new VShapeRow(this, column);

    // 02 create VTypeAttributeRow and Label for LaneChangeModel
    myLaneChangeModel = new VTypeAttributeRow(this, column, SUMO_ATTR_LANE_CHANGE_MODEL, VTypeAttributeRow::RowAttrType::ROWTYPE_COMBOBOX, SUMOXMLDefinitions::LaneChangeModels.getStrings());

    // 03 create VTypeAttributeRow and Label for PersonCapacity
    myPersonCapacity = new VTypeAttributeRow(this, column, SUMO_ATTR_PERSON_CAPACITY, VTypeAttributeRow::RowAttrType::ROWTYPE_STRING);

    // 04 create VTypeAttributeRow and Label for ContainerCapacity
    myContainerCapacity = new VTypeAttributeRow(this, column, SUMO_ATTR_CONTAINER_CAPACITY, VTypeAttributeRow::RowAttrType::ROWTYPE_STRING);

    // 05 create VTypeAttributeRow and Label for BoardingDuration
    myBoardingDuration = new VTypeAttributeRow(this, column, SUMO_ATTR_BOARDING_DURATION, VTypeAttributeRow::RowAttrType::ROWTYPE_STRING);

    // 06 create VTypeAttributeRow and Label for LoadingDuration
    myLoadingDuration = new VTypeAttributeRow(this, column, SUMO_ATTR_LOADING_DURATION, VTypeAttributeRow::RowAttrType::ROWTYPE_STRING);

    // 07 create ComboBox and Label for LatAlignment
    myLatAlignment = new VTypeAttributeRow(this, column, SUMO_ATTR_LATALIGNMENT, VTypeAttributeRow::RowAttrType::ROWTYPE_COMBOBOX, SUMOXMLDefinitions::LateralAlignments.getStrings());

    // 08 create VTypeAttributeRow and Label for MinGapLat
    myMinGapLat = new VTypeAttributeRow(this, column, SUMO_ATTR_MINGAP_LAT, VTypeAttributeRow::RowAttrType::ROWTYPE_STRING);

    // 09 create VTypeAttributeRow and Label for MaxSpeedLat
    myMaxSpeedLat = new VTypeAttributeRow(this, column, SUMO_ATTR_MAXSPEED_LAT, VTypeAttributeRow::RowAttrType::ROWTYPE_STRING);

    // 10 create VTypeAttributeRow and Label for ActionStepLength
    myActionStepLength = new VTypeAttributeRow(this, column, SUMO_ATTR_ACTIONSTEPLENGTH, VTypeAttributeRow::RowAttrType::ROWTYPE_STRING);

    // 11 create FXTextField and Label for Carriage length
    myCarriageLength = new VTypeAttributeRow(this, column, SUMO_ATTR_CARRIAGE_LENGTH, VTypeAttributeRow::RowAttrType::ROWTYPE_STRING);

    // 12 create FXTextField and Label for Locomotive length
    myLocomotiveLength = new VTypeAttributeRow(this, column, SUMO_ATTR_LOCOMOTIVE_LENGTH, VTypeAttributeRow::RowAttrType::ROWTYPE_STRING);

    // 13 create FXTextField and Label for carriage GAP
    myCarriageGap = new VTypeAttributeRow(this, column, SUMO_ATTR_CARRIAGE_GAP, VTypeAttributeRow::RowAttrType::ROWTYPE_STRING);
}


void
GNEVehicleTypeDialog::VTypeAtributes::buildJunctionModelAttributesA(FXVerticalFrame* column) {
    // 01 create VTypeAttributeRow and Label for JMCrossingGap
    myJMCrossingGap = new VTypeAttributeRow(this, column, SUMO_ATTR_JM_CROSSING_GAP, VTypeAttributeRow::RowAttrType::ROWTYPE_STRING);

    // 02 create VTypeAttributeRow and Label for JMIgnoreKeepclearTime
    myJMIgnoreKeepclearTime = new VTypeAttributeRow(this, column, SUMO_ATTR_JM_IGNORE_KEEPCLEAR_TIME, VTypeAttributeRow::RowAttrType::ROWTYPE_STRING);

    // 03 create VTypeAttributeRow and Label for JMDriveAfterYellowTime
    myJMDriveAfterYellowTime = new VTypeAttributeRow(this, column, SUMO_ATTR_JM_DRIVE_AFTER_YELLOW_TIME, VTypeAttributeRow::RowAttrType::ROWTYPE_STRING);

    // 04 create VTypeAttributeRow and Label for JMDriveAfterRedTime
    myJMDriveAfterRedTime = new VTypeAttributeRow(this, column, SUMO_ATTR_JM_DRIVE_AFTER_RED_TIME, VTypeAttributeRow::RowAttrType::ROWTYPE_STRING);

    // 05 create VTypeAttributeRow and Label for JMDriveRedSpeed
    myJMDriveRedSpeed = new VTypeAttributeRow(this, column, SUMO_ATTR_JM_DRIVE_RED_SPEED, VTypeAttributeRow::RowAttrType::ROWTYPE_STRING);
}


void
GNEVehicleTypeDialog::VTypeAtributes::buildJunctionModelAttributesB(FXVerticalFrame* column) {
    // 01 create VTypeAttributeRow and Label for JMIgnoreFoeProb
    myJMIgnoreFoeProb = new VTypeAttributeRow(this, column, SUMO_ATTR_JM_IGNORE_FOE_PROB, VTypeAttributeRow::RowAttrType::ROWTYPE_STRING);

    // 02 create VTypeAttributeRow and Label for JMIgnoreFoeSpeed
    myJMIgnoreFoeSpeed = new VTypeAttributeRow(this, column, SUMO_ATTR_JM_IGNORE_FOE_SPEED, VTypeAttributeRow::RowAttrType::ROWTYPE_STRING);

    // 03 create VTypeAttributeRow and Label for JMSigmaMinor
    myJMSigmaMinor = new VTypeAttributeRow(this, column, SUMO_ATTR_JM_SIGMA_MINOR, VTypeAttributeRow::RowAttrType::ROWTYPE_STRING);

    // 04 create VTypeAttributeRow and Label for JMTimeGapMinor
    myJMTimeGapMinor = new VTypeAttributeRow(this, column, SUMO_ATTR_JM_TIMEGAP_MINOR, VTypeAttributeRow::RowAttrType::ROWTYPE_STRING);

    // 05 create VTypeAttributeRow and Label for Impatience
    myJMImpatience = new VTypeAttributeRow(this, column, SUMO_ATTR_IMPATIENCE, VTypeAttributeRow::RowAttrType::ROWTYPE_STRING);
}


void
GNEVehicleTypeDialog::VTypeAtributes::buildLaneChangeModelAttributes(FXVerticalFrame* column) {
    // 01 create VTypeAttributeRow and Label for strategic param
    myLCAStrategicParam = new VTypeAttributeRow(this, column, SUMO_ATTR_LCA_STRATEGIC_PARAM, VTypeAttributeRow::RowAttrType::ROWTYPE_STRING);

    // 02 create VTypeAttributeRow and Label for cooperative param
    myLCACooperativeParam = new VTypeAttributeRow(this, column, SUMO_ATTR_LCA_COOPERATIVE_PARAM, VTypeAttributeRow::RowAttrType::ROWTYPE_STRING);

    // 03 create VTypeAttributeRow and Label for speed gain param
    myLCASpeedgainParam = new VTypeAttributeRow(this, column, SUMO_ATTR_LCA_SPEEDGAIN_PARAM, VTypeAttributeRow::RowAttrType::ROWTYPE_STRING);

    // 04 create VTypeAttributeRow and Label for keepright param
    myLCAKeeprightParam = new VTypeAttributeRow(this, column, SUMO_ATTR_LCA_KEEPRIGHT_PARAM, VTypeAttributeRow::RowAttrType::ROWTYPE_STRING);

    // 05 create VTypeAttributeRow and Label for sublane param
    myLCASublaneParam = new VTypeAttributeRow(this, column, SUMO_ATTR_LCA_SUBLANE_PARAM, VTypeAttributeRow::RowAttrType::ROWTYPE_STRING);

    // 06 create VTypeAttributeRow and Label for opposite param
    myLCAOppositeParam = new VTypeAttributeRow(this, column, SUMO_ATTR_LCA_OPPOSITE_PARAM, VTypeAttributeRow::RowAttrType::ROWTYPE_STRING);

    // 07 create VTypeAttributeRow and Label for pushy
    myLCAPushy = new VTypeAttributeRow(this, column, SUMO_ATTR_LCA_PUSHY, VTypeAttributeRow::RowAttrType::ROWTYPE_STRING);

    // 08 create VTypeAttributeRow and Label for pushy gap
    myLCAPushygap = new VTypeAttributeRow(this, column, SUMO_ATTR_LCA_PUSHYGAP, VTypeAttributeRow::RowAttrType::ROWTYPE_STRING);

    // 09 create VTypeAttributeRow and Label for assertive
    myLCAAssertive = new VTypeAttributeRow(this, column, SUMO_ATTR_LCA_ASSERTIVE, VTypeAttributeRow::RowAttrType::ROWTYPE_STRING);

    // 10 create VTypeAttributeRow and Label for impatience
    myLCAImpatience = new VTypeAttributeRow(this, column, SUMO_ATTR_LCA_IMPATIENCE, VTypeAttributeRow::RowAttrType::ROWTYPE_STRING);

    // 11 create VTypeAttributeRow and Label for time to impatience
    myLCATimeToImpatience = new VTypeAttributeRow(this, column, SUMO_ATTR_LCA_TIME_TO_IMPATIENCE, VTypeAttributeRow::RowAttrType::ROWTYPE_STRING);

    // 12 create VTypeAttributeRow and Label for accel lat
    myLCAAccelLat = new VTypeAttributeRow(this, column, SUMO_ATTR_LCA_ACCEL_LAT, VTypeAttributeRow::RowAttrType::ROWTYPE_STRING);

    // 13 create VTypeAttributeRow and Label for look ahead lefth
    myLCALookAheadLeft = new VTypeAttributeRow(this, column, SUMO_ATTR_LCA_LOOKAHEADLEFT, VTypeAttributeRow::RowAttrType::ROWTYPE_STRING);

    // 14 create VTypeAttributeRow and Label for speed gain right
    myLCASpeedGainRight = new VTypeAttributeRow(this, column, SUMO_ATTR_LCA_SPEEDGAINRIGHT, VTypeAttributeRow::RowAttrType::ROWTYPE_STRING);

    // 15 create VTypeAttributeRow and Label for max speed lat standing
    myLCAMaxSpeedLatStanding = new VTypeAttributeRow(this, column, SUMO_ATTR_LCA_MAXSPEEDLATSTANDING, VTypeAttributeRow::RowAttrType::ROWTYPE_STRING);

    // 16 create VTypeAttributeRow and Label for max speed lat factor
    myLCAMaxSpeedLatFactor = new VTypeAttributeRow(this, column, SUMO_ATTR_LCA_MAXSPEEDLATFACTOR, VTypeAttributeRow::RowAttrType::ROWTYPE_STRING);

    // 17 create VTypeAttributeRow and Label for turn alignment distance
    myLCATurnAlignmentDistance = new VTypeAttributeRow(this, column, SUMO_ATTR_LCA_TURN_ALIGNMENT_DISTANCE, VTypeAttributeRow::RowAttrType::ROWTYPE_STRING);

    // 18 create VTypeAttributeRow and Label for overtake right
    myLCAOvertakeRight = new VTypeAttributeRow(this, column, SUMO_ATTR_LCA_OVERTAKE_RIGHT, VTypeAttributeRow::RowAttrType::ROWTYPE_STRING);

    // 19 create VTypeAttributeRow and Label for experimental
    /* myLCAExperimental = new VTypeAttributeRow(this, column, SUMO_ATTR_LCA_EXPERIMENTAL1, VTypeAttributeRow::RowAttrType::ROWTYPE_STRING); */
}


void
GNEVehicleTypeDialog::VTypeAtributes::updateValues() {
    //set values of myEditedDemandElement into fields
    myTextFieldVehicleTypeID->setText(myVehicleTypeDialog->myEditedDemandElement->getAttribute(SUMO_ATTR_ID).c_str());
    // set variables of special rows VClass and VShape
    SUMOVTypeParameter::VClassDefaultValues defaultVTypeParameters(myVClassRow->updateValue());
    myVShapeRow->updateValues();
    // update rows
    myLaneChangeModel->updateValue();
    myLatAlignment->updateValue();
    myColor->updateValue();
    myLength->updateValue(toString(defaultVTypeParameters.length));
    myMinGap->updateValue(toString(defaultVTypeParameters.minGap));
    myMaxSpeed->updateValue(toString(defaultVTypeParameters.maxSpeed));
    mySpeedFactor->updateValue(toString(defaultVTypeParameters.speedFactor.getParameter()[0]));
    mySpeedDev->updateValue(toString(defaultVTypeParameters.speedFactor.getParameter()[1]));
    myEmissionClass->updateValue(toString(defaultVTypeParameters.emissionClass));
    myWidth->updateValue(toString(defaultVTypeParameters.width));
    myHeight->updateValue(toString(defaultVTypeParameters.height));
    myFilename->updateValue();
    myOSGFile->updateValue(toString(defaultVTypeParameters.osgFile));
    myPersonCapacity->updateValue(toString(defaultVTypeParameters.personCapacity));
    myContainerCapacity->updateValue(toString(defaultVTypeParameters.containerCapacity));
    myCarriageLength->updateValue(toString(defaultVTypeParameters.carriageLength));
    myLocomotiveLength->updateValue(toString(defaultVTypeParameters.locomotiveLength));
    myBoardingDuration->updateValue();
    myLoadingDuration->updateValue();
    myMinGapLat->updateValue();
    myMaxSpeedLat->updateValue();
    myActionStepLength->updateValue();
    myProbability->updateValue();
    myCarriageGap->updateValue();
    // JM Parameters
    myJMCrossingGap->updateValue();
    myJMIgnoreKeepclearTime->updateValue();
    myJMDriveAfterYellowTime->updateValue();
    myJMDriveAfterRedTime->updateValue();
    myJMDriveRedSpeed->updateValue();
    myJMIgnoreFoeProb->updateValue();
    myJMIgnoreFoeSpeed->updateValue();
    myJMSigmaMinor->updateValue();
    myJMTimeGapMinor->updateValue();
    myJMImpatience->updateValue();
    // LCM Attributes
    myLCAStrategicParam->updateValue();
    myLCACooperativeParam->updateValue();
    myLCASpeedgainParam->updateValue();
    myLCAKeeprightParam->updateValue();
    myLCASublaneParam->updateValue();
    myLCAOppositeParam->updateValue();
    myLCAPushy->updateValue();
    myLCAPushygap->updateValue();
    myLCAAssertive->updateValue();
    myLCAImpatience->updateValue();
    myLCATimeToImpatience->updateValue();
    myLCAAccelLat->updateValue();
    myLCALookAheadLeft->updateValue();
    myLCASpeedGainRight->updateValue();
    myLCAMaxSpeedLatStanding->updateValue();
    myLCAMaxSpeedLatFactor->updateValue();
    myLCATurnAlignmentDistance->updateValue();
    myLCAOvertakeRight->updateValue();
    /* myLCAExperimental->updateValue(); */
}


long
GNEVehicleTypeDialog::VTypeAtributes::onCmdSetAttribute(FXObject*, FXSelector, void*) {
    // At start we assumed, that all values are valid
    myVehicleTypeDialog->myVehicleTypeValid = true;
    myVehicleTypeDialog->myInvalidAttr = SUMO_ATTR_NOTHING;
    // set color of myTextFieldVehicleTypeID, depending if current value is valid or not
    if (myVehicleTypeDialog->myEditedDemandElement->isValid(SUMO_ATTR_ID, myTextFieldVehicleTypeID->getText().text())) {
        myTextFieldVehicleTypeID->setTextColor(FXRGB(0, 0, 0));
        myVehicleTypeDialog->myEditedDemandElement->setAttribute(SUMO_ATTR_ID, myTextFieldVehicleTypeID->getText().text(), myVehicleTypeDialog->myEditedDemandElement->getNet()->getViewNet()->getUndoList());
    } else if (myVehicleTypeDialog->myEditedDemandElement->getAttribute(SUMO_ATTR_ID) == myTextFieldVehicleTypeID->getText().text()) {
        myTextFieldVehicleTypeID->setTextColor(FXRGB(0, 0, 0));
        myVehicleTypeDialog->myEditedDemandElement->setAttribute(SUMO_ATTR_ID, myTextFieldVehicleTypeID->getText().text(), myVehicleTypeDialog->myEditedDemandElement->getNet()->getViewNet()->getUndoList());
    } else {
        myTextFieldVehicleTypeID->setTextColor(FXRGB(255, 0, 0));
        myVehicleTypeDialog->myVehicleTypeValid = false;
        myVehicleTypeDialog->myInvalidAttr = SUMO_ATTR_ID;
    }
    // set variables of special rows VClass and VShape
    SUMOVTypeParameter::VClassDefaultValues defaultVTypeParameters(myVClassRow->setVariable());
    // set variables of special rows VShape
    myVShapeRow->setVariable();
    // set attributes in rest rows
    myColor->setVariable();
    myLength->setVariable(toString(defaultVTypeParameters.length));
    myMinGap->setVariable(toString(defaultVTypeParameters.minGap));
    myMaxSpeed->setVariable(toString(defaultVTypeParameters.maxSpeed));
    mySpeedFactor->setVariable(toString(defaultVTypeParameters.speedFactor.getParameter()[0]));
    mySpeedDev->setVariable(toString(defaultVTypeParameters.speedFactor.getParameter()[1]));
    myEmissionClass->setVariable(toString(defaultVTypeParameters.emissionClass));
    myWidth->setVariable(toString(defaultVTypeParameters.width));
    myHeight->setVariable(toString(defaultVTypeParameters.height));
    myOSGFile->setVariable(toString(defaultVTypeParameters.osgFile));
    // set attributes in rows
    myLaneChangeModel->setVariable();
    myLatAlignment->setVariable();
    myLength->setVariable(toString(defaultVTypeParameters.length));
    myMinGap->setVariable(toString(defaultVTypeParameters.minGap));
    myMaxSpeed->setVariable(toString(defaultVTypeParameters.maxSpeed));
    mySpeedFactor->setVariable(toString(defaultVTypeParameters.speedFactor.getParameter()[0]));
    mySpeedDev->setVariable(toString(defaultVTypeParameters.speedFactor.getParameter()[1]));
    myEmissionClass->setVariable(toString(defaultVTypeParameters.emissionClass));
    myWidth->setVariable(toString(defaultVTypeParameters.width));
    myHeight->setVariable(toString(defaultVTypeParameters.height));
    myFilename->setVariable();
    myOSGFile->setVariable(toString(defaultVTypeParameters.osgFile));
    myPersonCapacity->setVariable(toString(defaultVTypeParameters.personCapacity));
    myContainerCapacity->setVariable(toString(defaultVTypeParameters.containerCapacity));
    myCarriageLength->setVariable(toString(defaultVTypeParameters.carriageLength));
    myLocomotiveLength->setVariable(toString(defaultVTypeParameters.locomotiveLength));
    myBoardingDuration->setVariable();
    myLoadingDuration->setVariable();
    myMinGapLat->setVariable();
    myMaxSpeedLat->setVariable();
    myActionStepLength->setVariable();
    myProbability->setVariable();
    myCarriageGap->setVariable();
    // JM Variables
    myJMCrossingGap->setVariable();
    myJMIgnoreKeepclearTime->setVariable();
    myJMDriveAfterYellowTime->setVariable();
    myJMDriveAfterRedTime->setVariable();
    myJMDriveRedSpeed->setVariable();
    myJMIgnoreFoeProb->setVariable();
    myJMIgnoreFoeSpeed->setVariable();
    myJMSigmaMinor->setVariable();
    myJMTimeGapMinor->setVariable();
    myJMImpatience->setVariable();
    // LCM Attributes
    myLCAStrategicParam->setVariable();
    myLCACooperativeParam->setVariable();
    myLCASpeedgainParam->setVariable();
    myLCAKeeprightParam->setVariable();
    myLCASublaneParam->setVariable();
    myLCAOppositeParam->setVariable();
    myLCAPushy->setVariable();
    myLCAPushygap->setVariable();
    myLCAAssertive->setVariable();
    myLCAImpatience->setVariable();
    myLCATimeToImpatience->setVariable();
    myLCAAccelLat->setVariable();
    myLCALookAheadLeft->setVariable();
    myLCASpeedGainRight->setVariable();
    myLCAMaxSpeedLatStanding->setVariable();
    myLCAMaxSpeedLatFactor->setVariable();
    myLCATurnAlignmentDistance->setVariable();
    myLCAOvertakeRight->setVariable();
    /* myLCAExperimental->setVariable(); */
    return true;
}


long
GNEVehicleTypeDialog::VTypeAtributes::onCmdSetAttributeDialog(FXObject* obj, FXSelector, void*) {
    // check what dialog has to be opened
    if (obj == myColor->getButton()) {
        myColor->openColorDialog();
    } else if (obj == myFilename->getButton()) {
        myFilename->openImageFileDialog();
    } else if (obj == myOSGFile->getButton()) {
        myFilename->openOSGFileDialog();
    }
    return 1;
}

// ---------------------------------------------------------------------------
// GNEVehicleTypeDialog::VShapeRow - methods
// ---------------------------------------------------------------------------

GNEVehicleTypeDialog::CarFollowingModelParameters::CarFollowingModelParameters(GNEVehicleTypeDialog* vehicleTypeDialog, FXHorizontalFrame* column) :
    FXGroupBox(column, "Car Following Model attributes", GUIDesignGroupBoxFrame),
    myVehicleTypeDialog(vehicleTypeDialog) {

    // create vertical frame for rows
    myVerticalFrameRows = new FXVerticalFrame(this, GUIDesignAuxiliarFrame);

    // declare combo box
    FXHorizontalFrame* row = new FXHorizontalFrame(myVerticalFrameRows, GUIDesignAuxiliarHorizontalFrame);
    new FXLabel(row, "Algorithm", nullptr, GUIDesignLabelAttribute150);
    myComboBoxCarFollowModel = new FXComboBox(row, GUIDesignComboBoxNCol, this, MID_GNE_SET_ATTRIBUTE, GUIDesignComboBox);

    // fill combo Box with all Car following models
    std::vector<std::string> CFModels = SUMOXMLDefinitions::CarFollowModels.getStrings();
    for (auto i : CFModels) {
        myComboBoxCarFollowModel->appendItem(i.c_str());
    }
    myComboBoxCarFollowModel->setNumVisible(10);

    // 01 create FX and Label for Accel
    myAccelRow = new CarFollowingModelRow(this, myVerticalFrameRows, SUMO_ATTR_ACCEL);
    myRows.push_back(myAccelRow);

    // 02 create FX and Label for Decel
    myDecelRow = new CarFollowingModelRow(this, myVerticalFrameRows, SUMO_ATTR_DECEL);
    myRows.push_back(myDecelRow);

    // 03 create FX and Label for Apparent decel
    myApparentDecelRow = new CarFollowingModelRow(this, myVerticalFrameRows, SUMO_ATTR_APPARENTDECEL);
    myRows.push_back(myApparentDecelRow);

    // 04 create FX and Label for emergency decel
    myEmergencyDecelRow = new CarFollowingModelRow(this, myVerticalFrameRows, SUMO_ATTR_EMERGENCYDECEL);
    myRows.push_back(myEmergencyDecelRow);

    // 05 create FX and Label for Sigma
    mySigmaRow = new CarFollowingModelRow(this, myVerticalFrameRows, SUMO_ATTR_SIGMA);
    myRows.push_back(mySigmaRow);

    // 06 create FX and Label for Tau
    myTauRow = new CarFollowingModelRow(this, myVerticalFrameRows, SUMO_ATTR_TAU);
    myRows.push_back(myTauRow);

    // 07 myMinGapFactor FX and Label for MinGapFactor
    myMinGapFactorRow = new CarFollowingModelRow(this, myVerticalFrameRows, SUMO_ATTR_COLLISION_MINGAP_FACTOR);
    myRows.push_back(myMinGapFactorRow);

    // 08 create FX and Label for K
    myKRow = new CarFollowingModelRow(this, myVerticalFrameRows, SUMO_ATTR_K);
    myRows.push_back(myKRow);

    // 09 create FX and Label for PHI
    myPhiRow = new CarFollowingModelRow(this, myVerticalFrameRows, SUMO_ATTR_CF_KERNER_PHI);
    myRows.push_back(myPhiRow);

    // 10 create FX and Label for Deleta
    myDeltaRow = new CarFollowingModelRow(this, myVerticalFrameRows, SUMO_ATTR_CF_IDM_DELTA);
    myRows.push_back(myDeltaRow);

    // 11 create FX and Label for Stepping
    mySteppingRow = new CarFollowingModelRow(this, myVerticalFrameRows, SUMO_ATTR_CF_IDM_STEPPING);
    myRows.push_back(mySteppingRow);

    // 12 create FX and Label for Security
    mySecurityRow = new CarFollowingModelRow(this, myVerticalFrameRows, SUMO_ATTR_CF_WIEDEMANN_SECURITY);
    myRows.push_back(mySecurityRow);

    // 13 create FX and Label for Estimation
    myEstimationRow = new CarFollowingModelRow(this, myVerticalFrameRows, SUMO_ATTR_CF_WIEDEMANN_ESTIMATION);
    myRows.push_back(myEstimationRow);

    // 14 create FX and Label for TMP1
    myTmp1Row = new CarFollowingModelRow(this, myVerticalFrameRows, SUMO_ATTR_TMP1);
    myRows.push_back(myTmp1Row);

    // 15 create FX and Label for TMP2
    myTmp2Row = new CarFollowingModelRow(this, myVerticalFrameRows, SUMO_ATTR_TMP2);
    myRows.push_back(myTmp2Row);

    // 16 create FX and Label for TMP3
    myTmp3Row = new CarFollowingModelRow(this, myVerticalFrameRows, SUMO_ATTR_TMP3);
    myRows.push_back(myTmp3Row);

    // 17 create FX and Label for TMP4
    myTmp4Row = new CarFollowingModelRow(this, myVerticalFrameRows, SUMO_ATTR_TMP4);
    myRows.push_back(myTmp4Row);

    // 18 create FX and Label for TMP5
    myTmp5Row = new CarFollowingModelRow(this, myVerticalFrameRows, SUMO_ATTR_TMP5);
    myRows.push_back(myTmp5Row);

    // 19 create FX and Label for trainType (allow strings)
    myTrainTypeRow = new CarFollowingModelRow(this, myVerticalFrameRows, SUMO_ATTR_TRAIN_TYPE);
    myRows.push_back(myTrainTypeRow);

    // 20 create FX and Label for Tau Last
    myTrauLastRow = new CarFollowingModelRow(this, myVerticalFrameRows, SUMO_ATTR_CF_PWAGNER2009_TAULAST);
    myRows.push_back(myTrauLastRow);

    // 21 create FX and Label for Aprob
    myAprobRow = new CarFollowingModelRow(this, myVerticalFrameRows, SUMO_ATTR_CF_PWAGNER2009_APPROB);
    myRows.push_back(myAprobRow);

    // 22 create FX and Label for Adapt Factor
    myAdaptFactorRow = new CarFollowingModelRow(this, myVerticalFrameRows, SUMO_ATTR_CF_IDMM_ADAPT_FACTOR);
    myRows.push_back(myAdaptFactorRow);

    // 23 create FX and Label for Adapt Time
    myAdaptTimeRow = new CarFollowingModelRow(this, myVerticalFrameRows, SUMO_ATTR_CF_IDMM_ADAPT_TIME);
    myRows.push_back(myAdaptTimeRow);

    // create myLabelIncompleteAttribute
    myLabelIncompleteAttribute = new FXLabel(myVerticalFrameRows, "Some attributes wasn't\nimplemented yet", nullptr, GUIDesignLabelAboutInfoCenter);
    myLabelIncompleteAttribute->hide();

    // show or hidde ComboBox depending of current selected CFM
    refreshCFMFields();
}


void
GNEVehicleTypeDialog::CarFollowingModelParameters::refreshCFMFields() {
    // start hidding all rows
    for (const auto& i : myRows) {
        i->hide();
    }
    // hide myLabelIncompleteAttribute
    myLabelIncompleteAttribute->hide();
    // show textfield depending of current CFM
    if (SUMOXMLDefinitions::CarFollowModels.hasString(myComboBoxCarFollowModel->getText().text())) {
        // show textfield depending of selected CFM
        switch (SUMOXMLDefinitions::CarFollowModels.get(myComboBoxCarFollowModel->getText().text())) {
            case SUMO_TAG_CF_KRAUSS:
            case SUMO_TAG_CF_KRAUSS_ORIG1:
            case SUMO_TAG_CF_KRAUSS_PLUS_SLOPE:
                myTauRow->show();
                myAccelRow->show();
                myDecelRow->show();
                myApparentDecelRow->show();
                myEmergencyDecelRow->show();
                mySigmaRow->show();
                break;
            case SUMO_TAG_CF_KRAUSSX:
                myTauRow->show();
                myTmp1Row->show();
                myTmp2Row->show();
                myTmp3Row->show();
                myTmp4Row->show();
                myTmp5Row->show();
                break;
            case SUMO_TAG_CF_SMART_SK:
            case SUMO_TAG_CF_DANIEL1:
                myTauRow->show();
                myAccelRow->show();
                myDecelRow->show();
                myEmergencyDecelRow->show();
                mySigmaRow->show();
                myMinGapFactorRow->show();
                myTmp1Row->show();
                myTmp2Row->show();
                myTmp3Row->show();
                myTmp4Row->show();
                myTmp5Row->show();
                break;
            case SUMO_TAG_CF_PWAGNER2009:
                myTauRow->show();
                myAccelRow->show();
                myDecelRow->show();
                myEmergencyDecelRow->show();
                mySigmaRow->show();
                myMinGapFactorRow->show();
                myTrauLastRow->show();
                myAprobRow->show();
                break;
            case SUMO_TAG_CF_IDM:
                myTauRow->show();
                myAccelRow->show();
                myDecelRow->show();
                myEmergencyDecelRow->show();
                mySteppingRow->show();
                myMinGapFactorRow->show();
                break;
            case SUMO_TAG_CF_IDMM:
                myTauRow->show();
                myAccelRow->show();
                myDecelRow->show();
                myEmergencyDecelRow->show();
                mySteppingRow->show();
                myMinGapFactorRow->show();
                myAdaptFactorRow->show();
                myAdaptTimeRow->show();
                break;
            case SUMO_TAG_CF_BKERNER:
                myTauRow->show();
                myAccelRow->show();
                myDecelRow->show();
                myEmergencyDecelRow->show();
                myKRow->show();
                myPhiRow->show();
                myMinGapFactorRow->show();
                break;
            case SUMO_TAG_CF_WIEDEMANN:
                myTauRow->show();
                myAccelRow->show();
                myDecelRow->show();
                myEmergencyDecelRow->show();
                myMinGapFactorRow->show();
                mySecurityRow->show();
                myEstimationRow->show();
                break;
            case SUMO_TAG_CF_RAIL:
                myTauRow->show();
                myTrainTypeRow->show();
                break;
            case SUMO_TAG_CF_ACC:
                myTauRow->show();
                myAccelRow->show();
                myDecelRow->show();
                myEmergencyDecelRow->show();
                myMinGapFactorRow->show();
                // show myLabelIncompleteAttribute
                myLabelIncompleteAttribute->show();
                /**
                The follow parameters has to be added:
                    SUMO_ATTR_SC_GAIN
                    SUMO_ATTR_GCC_GAIN_SPEED
                    SUMO_ATTR_GCC_GAIN_SPACE
                    SUMO_ATTR_GC_GAIN_SPEED
                    SUMO_ATTR_GC_GAIN_SPACE
                    SUMO_ATTR_CA_GAIN_SPEED
                    SUMO_ATTR_CA_GAIN_SPACE
                */
                break;
            case SUMO_TAG_CF_CACC:
                myTauRow->show();
                myAccelRow->show();
                myDecelRow->show();
                myEmergencyDecelRow->show();
                myMinGapFactorRow->show();
                // show myLabelIncompleteAttribute
                myLabelIncompleteAttribute->show();
                /**
                The follow parameters has to be added:
                    SUMO_ATTR_SC_GAIN_CACC
                    SUMO_ATTR_GCC_GAIN_GAP_CACC
                    SUMO_ATTR_GCC_GAIN_GAP_DOT_CACC
                    SUMO_ATTR_GC_GAIN_GAP_CACC
                    SUMO_ATTR_GC_GAIN_GAP_DOT_CACC
                    SUMO_ATTR_CA_GAIN_GAP_CACC
                    SUMO_ATTR_CA_GAIN_GAP_DOT_CACC
                    SUMO_ATTR_GCC_GAIN_SPEED
                    SUMO_ATTR_GCC_GAIN_SPACE
                    SUMO_ATTR_GC_GAIN_SPEED
                    SUMO_ATTR_GC_GAIN_SPACE
                    SUMO_ATTR_CA_GAIN_SPEED
                    SUMO_ATTR_CA_GAIN_SPACE
                */
                break;
            case SUMO_TAG_CF_CC:
                myTauRow->show();
                myAccelRow->show();
                myDecelRow->show();
                // show myLabelIncompleteAttribute
                myLabelIncompleteAttribute->show();
                /**
                The follow parameters has to be added:
                    SUMO_ATTR_CF_CC_C1
                    SUMO_ATTR_CF_CC_CCDECEL
                    SUMO_ATTR_CF_CC_CONSTSPACING
                    SUMO_ATTR_CF_CC_KP
                    SUMO_ATTR_CF_CC_LAMBDA
                    SUMO_ATTR_CF_CC_OMEGAN
                    SUMO_ATTR_CF_CC_TAU
                    SUMO_ATTR_CF_CC_XI
                    SUMO_ATTR_CF_CC_LANES_COUNT
                    SUMO_ATTR_CF_CC_CCACCEL
                    SUMO_ATTR_CF_CC_PLOEG_KP
                    SUMO_ATTR_CF_CC_PLOEG_KD
                    SUMO_ATTR_CF_CC_PLOEG_H
                    SUMO_ATTR_CF_CC_FLATBED_KA
                    SUMO_ATTR_CF_CC_FLATBED_KV
                    SUMO_ATTR_CF_CC_FLATBED_KP
                    SUMO_ATTR_CF_CC_FLATBED_D
                    SUMO_ATTR_CF_CC_FLATBED_H
                */
                break;
            default:
                break;
        }
    }
    myVerticalFrameRows->recalc();
    update();
}


void
GNEVehicleTypeDialog::CarFollowingModelParameters::updateValues() {
    //set values of myEditedDemandElement into fields
    if (myVehicleTypeDialog->myEditedDemandElement->getAttribute(SUMO_ATTR_CAR_FOLLOW_MODEL).empty()) {
        myComboBoxCarFollowModel->setCurrentItem(0);
    } else {
        myComboBoxCarFollowModel->setText(myVehicleTypeDialog->myEditedDemandElement->getAttribute(SUMO_ATTR_CAR_FOLLOW_MODEL).c_str());
    }
    // refresh fields
    refreshCFMFields();
    // update value in all Rows
    for (const auto& i : myRows) {
        i->updateValue();
    }
}


long
GNEVehicleTypeDialog::CarFollowingModelParameters::onCmdSetVariable(FXObject*, FXSelector, void*) {
    // At start we assumed, that all values are valid
    myVehicleTypeDialog->myVehicleTypeValid = true;
    myVehicleTypeDialog->myInvalidAttr = SUMO_ATTR_NOTHING;
    // set color of myTextFieldCarFollowModel, depending if current value is valid or not
    if (myVehicleTypeDialog->myEditedDemandElement->isValid(SUMO_ATTR_CAR_FOLLOW_MODEL, myComboBoxCarFollowModel->getText().text())) {
        myComboBoxCarFollowModel->setTextColor(FXRGB(0, 0, 0));
        myVehicleTypeDialog->myEditedDemandElement->setAttribute(SUMO_ATTR_CAR_FOLLOW_MODEL, myComboBoxCarFollowModel->getText().text(), myVehicleTypeDialog->myEditedDemandElement->getNet()->getViewNet()->getUndoList());
    } else {
        myComboBoxCarFollowModel->setTextColor(FXRGB(255, 0, 0));
        myVehicleTypeDialog->myVehicleTypeValid = false;
        myVehicleTypeDialog->myInvalidAttr = SUMO_ATTR_CAR_FOLLOW_MODEL;
    }
    // set variable in all Rows
    for (const auto& i : myRows) {
        i->setVariable();
    }
    // refresh fields
    refreshCFMFields();
    return true;
}

// ---------------------------------------------------------------------------
// GNEVehicleTypeDialog - public methods
// ---------------------------------------------------------------------------

GNEVehicleTypeDialog::GNEVehicleTypeDialog(GNEDemandElement* editedVehicleType, bool updatingElement) :
    GNEDemandElementDialog(editedVehicleType, updatingElement, /*1022*/ /*1322*/ 1372, 575),
    myVehicleTypeValid(true),
    myInvalidAttr(SUMO_ATTR_NOTHING) {

    // change default header
    changeDemandElementDialogHeader(updatingElement ? "Edit " + myEditedDemandElement->getTagStr() : "Create " + myEditedDemandElement->getTagStr());

    // Create auxiliar frames for values
    FXHorizontalFrame* columns = new FXHorizontalFrame(myContentFrame, GUIDesignAuxiliarHorizontalFrame);

    // create vehicle type attributes
    myVTypeAtributes = new VTypeAtributes(this, columns);

    // create car following model parameters
    myCarFollowingModelParameters = new CarFollowingModelParameters(this, columns);

    // start a undo list for editing local to this additional
    initChanges();

    // add element if we aren't updating an existent element
    if (myUpdatingElement == false) {
        myEditedDemandElement->getNet()->getViewNet()->getUndoList()->add(new GNEChange_DemandElement(myEditedDemandElement, true), true);
    }

    // update values of Vehicle Type common attributes
    myVTypeAtributes->updateValues();

    // update values of Car Following Model Parameters
    myCarFollowingModelParameters->updateValues();

    // open as modal dialog
    openAsModalDialog();
}


GNEVehicleTypeDialog::~GNEVehicleTypeDialog() {}


long
GNEVehicleTypeDialog::onCmdAccept(FXObject*, FXSelector, void*) {
    if (myVehicleTypeValid == false) {
        // write warning if netedit is running in testing mode
        WRITE_DEBUG("Opening FXMessageBox of type 'warning'");
        std::string operation1 = myUpdatingElement ? ("updating") : ("creating");
        std::string operation2 = myUpdatingElement ? ("updated") : ("created");
        std::string tagString = myEditedDemandElement->getTagStr();
        // open warning dialogBox
        FXMessageBox::warning(getApp(), MBOX_OK,
                              ("Error " + operation1 + " " + tagString).c_str(), "%s",
                              (tagString + " cannot be " + operation2 +
                               " because parameter " + toString(myInvalidAttr) +
                               " is invalid.").c_str());
        // write warning if netedit is running in testing mode
        WRITE_DEBUG("Closed FXMessageBox of type 'warning' with 'OK'");
        return 0;
    } else {
        // accept changes before closing dialog
        acceptChanges();
        // stop dialgo sucesfully
        getApp()->stopModal(this, TRUE);
        return 1;
    }
}


long
GNEVehicleTypeDialog::onCmdCancel(FXObject*, FXSelector, void*) {
    // cancel changes
    cancelChanges();
    // Stop Modal
    getApp()->stopModal(this, FALSE);
    return 1;
}


long
GNEVehicleTypeDialog::onCmdReset(FXObject*, FXSelector, void*) {
    // reset changes
    resetChanges();
    // update values of Vehicle Type common attributes
    myVTypeAtributes->updateValues();
    // update values of Car Following Model Parameters
    myCarFollowingModelParameters->updateValues();
    return 1;
}

// ---------------------------------------------------------------------------
// GNEVehicleTypeDialog - private methods
// ---------------------------------------------------------------------------

GNEVehicleTypeDialog::CarFollowingModelParameters::CarFollowingModelRow::CarFollowingModelRow(CarFollowingModelParameters* carFollowingModelParametersParent, FXVerticalFrame* verticalFrame, SumoXMLAttr attr) :
    FXHorizontalFrame(verticalFrame, GUIDesignAuxiliarHorizontalFrame),
    myCarFollowingModelParametersParent(carFollowingModelParametersParent),
    myAttr(attr),
    myTextField(nullptr) {
    new FXLabel(this, toString(attr).c_str(), nullptr, GUIDesignLabelAttribute150);
    myTextField = new FXTextField(this, GUIDesignTextFieldNCol, carFollowingModelParametersParent, MID_GNE_SET_ATTRIBUTE, GUIDesignTextFielWidth180);
}


void
GNEVehicleTypeDialog::CarFollowingModelParameters::CarFollowingModelRow::setVariable() {
    // set color of textField, depending if current value is valid or not
    if (myCarFollowingModelParametersParent->myVehicleTypeDialog->myEditedDemandElement->isValid(myAttr, myTextField->getText().text())) {
        // set color depending if is a default value
        if (myCarFollowingModelParametersParent->myVehicleTypeDialog->myEditedDemandElement->getTagProperty().getDefaultValue(myAttr) != myTextField->getText().text()) {
            myTextField->setTextColor(FXRGB(0, 0, 0));
        } else {
            myTextField->setTextColor(FXRGB(195, 195, 195));
        }
        myCarFollowingModelParametersParent->myVehicleTypeDialog->myEditedDemandElement->setAttribute(myAttr, myTextField->getText().text(),
                myCarFollowingModelParametersParent->myVehicleTypeDialog->myEditedDemandElement->getNet()->getViewNet()->getUndoList());
        // update value after setting it
        updateValue();
    } else {
        myTextField->setTextColor(FXRGB(255, 0, 0));
        // mark VType as invalid
        myCarFollowingModelParametersParent->myVehicleTypeDialog->myVehicleTypeValid = false;
        myCarFollowingModelParametersParent->myVehicleTypeDialog->myInvalidAttr = myAttr;
    }
}


void
GNEVehicleTypeDialog::CarFollowingModelParameters::CarFollowingModelRow::updateValue() {
    // set text of myTextField using current value of VType
    myTextField->setText(myCarFollowingModelParametersParent->myVehicleTypeDialog->myEditedDemandElement->getAttribute(myAttr).c_str());
    // set color depending if is a default value
    if (myCarFollowingModelParametersParent->myVehicleTypeDialog->myEditedDemandElement->getTagProperty().getDefaultValue(myAttr) != myTextField->getText().text()) {
        myTextField->setTextColor(FXRGB(0, 0, 0));
    } else {
        myTextField->setTextColor(FXRGB(195, 195, 195));
    }
}


/****************************************************************************/
