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
/// @file    GNEAllowDisallow.cpp
/// @author  Pablo Alvarez Lopez
/// @date    April 2016
///
// Dialog for edit rerouters
/****************************************************************************/
#include <config.h>

#include <utils/gui/windows/GUIAppEnum.h>
#include <utils/gui/div/GUIDesigns.h>
#include <utils/common/StringTokenizer.h>
#include <netedit/elements/GNEAttributeCarrier.h>
#include <netedit/GNEViewNet.h>

#include "GNEAllowDisallow.h"



// ===========================================================================
// FOX callback mapping
// ===========================================================================

FXDEFMAP(GNEAllowDisallow) GNEAllowDisallowMap[] = {
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_ALLOWDISALLOW_CHANGE,           GNEAllowDisallow::onCmdValueChanged),
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_ALLOWDISALLOW_SELECTALL,        GNEAllowDisallow::onCmdSelectAll),
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_ALLOWDISALLOW_UNSELECTALL,      GNEAllowDisallow::onCmdUnselectAll),
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_ALLOWDISALLOW_SELECTONLYROAD,   GNEAllowDisallow::onCmdSelectOnlyRoad),
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_BUTTON_ACCEPT,                  GNEAllowDisallow::onCmdAccept),
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_BUTTON_CANCEL,                  GNEAllowDisallow::onCmdCancel),
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_BUTTON_RESET,                   GNEAllowDisallow::onCmdReset),
};

// Object implementation
FXIMPLEMENT(GNEAllowDisallow, FXDialogBox, GNEAllowDisallowMap, ARRAYNUMBER(GNEAllowDisallowMap))

// ===========================================================================
// member method definitions
// ===========================================================================

GNEAllowDisallow::GNEAllowDisallow(GNEViewNet* viewNet, GNEAttributeCarrier* AC) :
    FXDialogBox(viewNet->getApp(), ("Edit " + toString(SUMO_ATTR_ALLOW) + " " + toString(SUMO_ATTR_VCLASS) + "es").c_str(), GUIDesignDialogBox),
    myViewNet(viewNet),
    myAC(AC) {
    assert(AC->getTagProperty().hasAttribute(SUMO_ATTR_ALLOW));
    // set vehicle icon for this dialog
    setIcon(GUIIconSubSys::getIcon(GUIIcon::GREENVEHICLE));
    // create main frame
    FXVerticalFrame* mainFrame = new FXVerticalFrame(this, GUIDesignAuxiliarFrame);
    // create groupbox for options
    FXGroupBox* myGroupBoxOptions = new FXGroupBox(mainFrame, "Selection options", GUIDesignGroupBoxFrame);
    FXHorizontalFrame* myOptionsFrame = new FXHorizontalFrame(myGroupBoxOptions, GUIDesignAuxiliarHorizontalFrame);
    mySelectAllVClassButton = new FXButton(myOptionsFrame, "", GUIIconSubSys::getIcon(GUIIcon::OK), this, MID_GNE_ALLOWDISALLOW_SELECTALL, GUIDesignButtonIcon);
    new FXLabel(myOptionsFrame, "Allow all vehicles", nullptr, GUIDesignLabelLeftThick);
    mySelectOnlyRoadVClassButton = new FXButton(myOptionsFrame, "", GUIIconSubSys::getIcon(GUIIcon::OK), this, MID_GNE_ALLOWDISALLOW_SELECTONLYROAD, GUIDesignButtonIcon);
    new FXLabel(myOptionsFrame, "Allow only road vehicles", nullptr, GUIDesignLabelLeftThick);
    myUnselectAllVClassButton = new FXButton(myOptionsFrame, "", GUIIconSubSys::getIcon(GUIIcon::CANCEL), this, MID_GNE_ALLOWDISALLOW_UNSELECTALL, GUIDesignButtonIcon);
    new FXLabel(myOptionsFrame, "Disallow all vehicles", nullptr, GUIDesignLabelLeftThick);
    // create groupbox for vehicles
    FXGroupBox* myGroupBoxVehiclesFrame = new FXGroupBox(mainFrame, ("Select " + toString(SUMO_ATTR_VCLASS) + "es").c_str(), GUIDesignGroupBoxFrame);
    // Create frame for vehicles's columns
    FXHorizontalFrame* myVehiclesFrame = new FXHorizontalFrame(myGroupBoxVehiclesFrame, GUIDesignContentsFrame);
    // create left frame and fill it
    FXVerticalFrame* myContentLeftFrame = new FXVerticalFrame(myVehiclesFrame, GUIDesignAuxiliarFrame);
    buildVClass(myContentLeftFrame, SVC_PASSENGER, GUIIcon::VCLASS_PASSENGER, "Default vehicle class");
    buildVClass(myContentLeftFrame, SVC_PRIVATE, GUIIcon::VCLASS_PRIVATE, "A passenger car assigned for private use");
    buildVClass(myContentLeftFrame, SVC_TAXI, GUIIcon::VCLASS_TAXI, "Vehicle for hire with a driver");
    buildVClass(myContentLeftFrame, SVC_BUS, GUIIcon::VCLASS_BUS, "Urban line traffic");
    buildVClass(myContentLeftFrame, SVC_COACH, GUIIcon::VCLASS_COACH, "Overland transport");
    buildVClass(myContentLeftFrame, SVC_DELIVERY, GUIIcon::VCLASS_DELIVERY, "Vehicles specialized to deliver goods");
    buildVClass(myContentLeftFrame, SVC_TRUCK, GUIIcon::VCLASS_TRUCK, "Vehicle designed to transport cargo");
    buildVClass(myContentLeftFrame, SVC_TRAILER, GUIIcon::VCLASS_TRAILER, "Truck with trailer");
    buildVClass(myContentLeftFrame, SVC_EMERGENCY, GUIIcon::VCLASS_EMERGENCY, "Vehicle designated to respond to an emergency");
    // create center frame and fill it
    FXVerticalFrame* myContentCenterFrame = new FXVerticalFrame(myVehiclesFrame, GUIDesignAuxiliarFrame);
    buildVClass(myContentCenterFrame, SVC_MOTORCYCLE, GUIIcon::VCLASS_MOTORCYCLE, "Two- or three-wheeled motor vehicle");
    buildVClass(myContentCenterFrame, SVC_MOPED, GUIIcon::VCLASS_MOPED, "Motorcycle not allowed in motorways");
    buildVClass(myContentCenterFrame, SVC_BICYCLE, GUIIcon::VCLASS_BICYCLE, "Human-powered, pedal-driven vehicle");
    buildVClass(myContentCenterFrame, SVC_PEDESTRIAN, GUIIcon::VCLASS_PEDESTRIAN, "Person traveling on foot");
    buildVClass(myContentCenterFrame, SVC_TRAM, GUIIcon::VCLASS_TRAM, "Rail vehicle which runs on tracks");
    buildVClass(myContentCenterFrame, SVC_RAIL_ELECTRIC, GUIIcon::VCLASS_RAIL_ELECTRIC, "Rail electric vehicle");
    buildVClass(myContentCenterFrame, SVC_RAIL_FAST, GUIIcon::VCLASS_RAIL_ELECTRIC, "High-speed rail vehicle");
    buildVClass(myContentCenterFrame, SVC_RAIL_URBAN, GUIIcon::VCLASS_RAIL_URBAN, "Heavier than tram");
    buildVClass(myContentCenterFrame, SVC_RAIL, GUIIcon::VCLASS_RAIL, "Heavy rail vehicle");
    // create right frame and fill it  (8 vehicles)
    FXVerticalFrame* myContentRightFrame = new FXVerticalFrame(myVehiclesFrame, GUIDesignAuxiliarFrame);
    buildVClass(myContentRightFrame, SVC_E_VEHICLE, GUIIcon::VCLASS_EVEHICLE, "Future electric mobility vehicles");
    buildVClass(myContentRightFrame, SVC_ARMY, GUIIcon::VCLASS_ARMY, "Vehicle designed for military forces");
    buildVClass(myContentRightFrame, SVC_SHIP, GUIIcon::VCLASS_SHIP, "Basic class for navigating waterway");
    buildVClass(myContentRightFrame, SVC_AUTHORITY, GUIIcon::VCLASS_AUTHORITY, "Vehicle of a governmental security agency");
    buildVClass(myContentRightFrame, SVC_VIP, GUIIcon::VCLASS_VIP, "A civilian security armored car used by VIPs");
    buildVClass(myContentRightFrame, SVC_HOV, GUIIcon::VCLASS_HOV, "High-Occupancy Vehicle (two or more passengers)");
    buildVClass(myContentRightFrame, SVC_CUSTOM1, GUIIcon::VCLASS_CUSTOM1, "Reserved for user-defined semantics");
    buildVClass(myContentRightFrame, SVC_CUSTOM2, GUIIcon::VCLASS_CUSTOM2, "Reserved for user-defined semantics");
    // create dialog buttons bot centered
    FXHorizontalFrame* buttonsFrame = new FXHorizontalFrame(mainFrame, GUIDesignHorizontalFrame);
    new FXHorizontalFrame(buttonsFrame, GUIDesignAuxiliarHorizontalFrame);
    myAcceptButton = new FXButton(buttonsFrame, "accept\t\tclose", GUIIconSubSys::getIcon(GUIIcon::ACCEPT), this, MID_GNE_BUTTON_ACCEPT, GUIDesignButtonAccept);
    myCancelButton = new FXButton(buttonsFrame, "cancel\t\tclose", GUIIconSubSys::getIcon(GUIIcon::CANCEL), this, MID_GNE_BUTTON_CANCEL, GUIDesignButtonCancel);
    myResetButton = new FXButton(buttonsFrame,  "reset\t\tclose",  GUIIconSubSys::getIcon(GUIIcon::RESET), this, MID_GNE_BUTTON_RESET,  GUIDesignButtonReset);
    new FXHorizontalFrame(buttonsFrame, GUIDesignAuxiliarHorizontalFrame);
    // reset dialog
    onCmdReset(nullptr, 0, nullptr);
}


GNEAllowDisallow::~GNEAllowDisallow() {
}


long
GNEAllowDisallow::onCmdValueChanged(FXObject* obj, FXSelector, void*) {
    FXButton* buttonPressed = dynamic_cast<FXButton*>(obj);
    // change icon of button
    for (auto i = myVClassMap.begin(); i != myVClassMap.end(); i++) {
        if (i->second.first == buttonPressed) {
            if (buttonPressed->getIcon() == GUIIconSubSys::getIcon(GUIIcon::ACCEPT)) {
                buttonPressed->setIcon(GUIIconSubSys::getIcon(GUIIcon::CANCEL));
            } else {
                buttonPressed->setIcon(GUIIconSubSys::getIcon(GUIIcon::ACCEPT));
            }
            return 1;
        }
    }
    return 1;
}


long
GNEAllowDisallow::onCmdSelectAll(FXObject*, FXSelector, void*) {
    // change all icons to accept
    for (auto i : myVClassMap) {
        i.second.first->setIcon(GUIIconSubSys::getIcon(GUIIcon::ACCEPT));
    }
    return 1;
}


long
GNEAllowDisallow::onCmdUnselectAll(FXObject*, FXSelector, void*) {
    // change all icons to cancel
    for (auto i : myVClassMap) {
        i.second.first->setIcon(GUIIconSubSys::getIcon(GUIIcon::CANCEL));
    }
    return 1;
}


long
GNEAllowDisallow::onCmdSelectOnlyRoad(FXObject*, FXSelector, void*) {
    // change all non-road icons to disallow, and allow for the rest
    for (auto i : myVClassMap) {
        if ((i.first & (SVC_PEDESTRIAN | SVC_NON_ROAD)) == 0) {
            i.second.first->setIcon(GUIIconSubSys::getIcon(GUIIcon::ACCEPT));
        } else {
            i.second.first->setIcon(GUIIconSubSys::getIcon(GUIIcon::CANCEL));
        }
    }
    return 1;
}


long
GNEAllowDisallow::onCmdAccept(FXObject*, FXSelector, void*) {
    // clear allow and disallow VClasses
    std::vector<std::string> allowedVehicles;
    for (auto i : myVClassMap) {
        // check if vehicle is alloweddepending of the Icon
        if (i.second.first->getIcon() == GUIIconSubSys::getIcon(GUIIcon::ACCEPT)) {
            allowedVehicles.push_back(getVehicleClassNames(i.first));
        }
    }
    // chek if all vehicles are enabled and set new allowed vehicles
    myAC->setAttribute(SUMO_ATTR_ALLOW, joinToString(allowedVehicles, " "), myViewNet->getUndoList());
    // Stop Modal
    getApp()->stopModal(this, TRUE);
    return 1;
}


long
GNEAllowDisallow::onCmdCancel(FXObject*, FXSelector, void*) {
    // Stop Modal
    getApp()->stopModal(this, FALSE);
    return 1;
}


long
GNEAllowDisallow::onCmdReset(FXObject*, FXSelector, void*) {
    if (myAC->getAttribute(SUMO_ATTR_ALLOW) == "all") {
        // iterate over myVClassMap and set all icons as true
        for (auto i : myVClassMap) {
            i.second.first->setIcon(GUIIconSubSys::getIcon(GUIIcon::ACCEPT));
        }
    } else {
        // declare string vector for saving all vclasses
        const std::vector<std::string>& allowStringVector = StringTokenizer(myAC->getAttribute(SUMO_ATTR_ALLOW)).getVector();
        const std::set<std::string> allowSet(allowStringVector.begin(), allowStringVector.end());
        // iterate over myVClassMap and set icons
        for (auto i : myVClassMap) {
            if (allowSet.count(getVehicleClassNames(i.first)) > 0) {
                i.second.first->setIcon(GUIIconSubSys::getIcon(GUIIcon::ACCEPT));
            } else {
                i.second.first->setIcon(GUIIconSubSys::getIcon(GUIIcon::CANCEL));
            }
        }
    }
    return 1;
}


void
GNEAllowDisallow::buildVClass(FXVerticalFrame* contentsFrame, SUMOVehicleClass vclass, GUIIcon vclassIcon, const std::string& description) {
    // add frame for vehicle icons
    FXHorizontalFrame* vehicleFrame = new FXHorizontalFrame(contentsFrame, GUIDesignAuxiliarHorizontalFrame);
    FXLabel* labelVehicleIcon = new FXLabel(vehicleFrame, "", GUIIconSubSys::getIcon(vclassIcon), GUIDesignLabelIcon64x32Thicked);
    labelVehicleIcon->setBackColor(FXRGBA(255, 255, 255, 255));
    // create frame for information and button
    FXVerticalFrame* buttonAndInformationFrame = new FXVerticalFrame(vehicleFrame, GUIDesignAuxiliarHorizontalFrame);
    FXHorizontalFrame* buttonAndStatusFrame = new FXHorizontalFrame(buttonAndInformationFrame, GUIDesignAuxiliarHorizontalFrame);
    // create status and text button
    myVClassMap[vclass].first = new FXButton(buttonAndStatusFrame, "", GUIIconSubSys::getIcon(GUIIcon::EMPTY), this, MID_GNE_ALLOWDISALLOW_CHANGE, GUIDesignButtonIcon);
    myVClassMap[vclass].second = new FXLabel(buttonAndStatusFrame, toString(vclass).c_str(), nullptr, GUIDesignLabelLeftThick);
    // create label for description of vehicle
    new FXLabel(buttonAndInformationFrame, description.c_str(), nullptr, GUIDesignLabelLeftThick);
}


/****************************************************************************/
