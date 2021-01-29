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
/// @file    GUIOverheadWire.cpp
/// @author  Jakub Sevcik (RICE)
/// @author  Jan Prikryl (RICE)
/// @date    2019-11-25
///
// The gui-version of a MSOverheadWire
/****************************************************************************/
#include <config.h>

#include <string>
#include <utils/common/MsgHandler.h>
#include <utils/geom/PositionVector.h>
#include <utils/geom/Boundary.h>
#include <utils/gui/div/GLHelper.h>
#include <utils/common/ToString.h>
#include <microsim/MSNet.h>
#include <microsim/MSLane.h>
#include <microsim/MSEdge.h>
#include "GUINet.h"
#include "GUIEdge.h"
#include "GUIPerson.h"
#include "GUIOverheadWire.h"
#include <utils/gui/globjects/GUIGLObjectPopupMenu.h>
#include <utils/gui/windows/GUIAppEnum.h>
#include <gui/GUIGlobals.h>
#include <utils/gui/div/GUIParameterTableWindow.h>
#include <gui/GUIApplicationWindow.h>
#include <microsim/logging/FunctionBinding.h>
#include <utils/gui/div/GUIGlobalSelection.h>
#include <foreign/fontstash/fontstash.h>
#include <utils/geom/GeomHelper.h>
#include <utils/gui/globjects/GLIncludes.h>
#include <utils/vehicle/SUMOVehicle.h>
#include <microsim/MSVehicleType.h>


// ===========================================================================
// method definitions
// ===========================================================================
GUIOverheadWire::GUIOverheadWire(const std::string& id, MSLane& lane, double frompos, double topos, bool voltageSource) :
    MSOverheadWire(id, lane, frompos, topos, voltageSource),
    GUIGlObject_AbstractAdd(GLO_OVERHEAD_WIRE_SEGMENT, id) {
    myFGShape = lane.getShape();
    myFGShape = myFGShape.getSubpart(
                    lane.interpolateLanePosToGeometryPos(frompos),
                    lane.interpolateLanePosToGeometryPos(topos));
    myFGShapeRotations.reserve(myFGShape.size() - 1);
    myFGShapeLengths.reserve(myFGShape.size() - 1);
    int e = (int)myFGShape.size() - 1;
    for (int i = 0; i < e; ++i) {
        const Position& f = myFGShape[i];
        const Position& s = myFGShape[i + 1];
        myFGShapeLengths.push_back(f.distanceTo(s));
        myFGShapeRotations.push_back((double)atan2((s.x() - f.x()), (f.y() - s.y())) * (double) 180.0 / (double)M_PI);
    }
    PositionVector tmp = myFGShape;
    tmp.move2side(1.5);

    // position of the centre of the lane + move2side
    //myFGSignPos = tmp.getLineCenter();

    // position of beginning of the lane + move2side (2 equivallent commands ?)
    //myFGSignPos = tmp.positionAtOffset(double(0.0));
    myFGSignPos = tmp[0];

    myFGSignRot = 0;
    if (tmp.length() != 0) {
        myFGSignRot = myFGShape.rotationDegreeAtOffset(double((myFGShape.length() / 2.)));
        myFGSignRot -= 90;
    }
}


GUIOverheadWire::~GUIOverheadWire() {
}

GUIOverheadWireClamp::GUIOverheadWireClamp(const std::string& id, MSLane& lane_start, MSLane& lane_end) :
    GUIGlObject_AbstractAdd(GLO_OVERHEAD_WIRE_SEGMENT, id) {
    myFGShape.clear();
    myFGShape.push_back(lane_start.getShape().front());
    myFGShape.push_back(lane_end.getShape().back());
}

GUIOverheadWireClamp::~GUIOverheadWireClamp() {
}

GUIParameterTableWindow*
GUIOverheadWire::getParameterWindow(GUIMainWindow& app, GUISUMOAbstractView&) {
    // Create table items
    GUIParameterTableWindow* ret = new GUIParameterTableWindow(app, *this);

    // add items
    ret->mkItem("begin position [m]", false, myBegPos);
    ret->mkItem("end position [m]", false, myEndPos);
    //ret->mkItem("voltage [V]", false, myVoltage);

    // close building
    ret->closeBuilding();
    return ret;
}


GUIGLObjectPopupMenu*
GUIOverheadWire::getPopUpMenu(GUIMainWindow& app, GUISUMOAbstractView& parent) {
    GUIGLObjectPopupMenu* ret = new GUIGLObjectPopupMenu(app, parent, *this);
    buildPopupHeader(ret, app);
    buildCenterPopupEntry(ret);
    buildNameCopyPopupEntry(ret);
    buildSelectionPopupEntry(ret);
    buildShowParamsPopupEntry(ret);
    buildPositionCopyEntry(ret, false);
    return ret;
}

Boundary
GUIOverheadWire::getCenteringBoundary() const {
    Boundary b = myFGShape.getBoxBoundary();
    b.grow(20);
    return b;
}


void
GUIOverheadWire::drawGL(const GUIVisualizationSettings& s) const {
    // Draw overhead wire segment
    glPushName(getGlID());
    glPushMatrix();
    RGBColor lightgray(211, 211, 211, 255);
    RGBColor green(76, 170, 50, 255);
    RGBColor yellow(255, 235, 0, 255);
    RGBColor yellowCharge(255, 180, 0, 255);
    RGBColor redCharge(255, 51, 51, 255);
    RGBColor redChargeOverheadWire(180, 0, 0, 255);

    GUIColorScheme scheme = GUIColorScheme("by overhead wire current", RGBColor::BLACK, "road", true);
    double range = 200;
    scheme.clear();
    scheme.addColor(RGBColor::RED, (0));
    scheme.addColor(RGBColor::ORANGE, (range * 1 / 6.0));
    scheme.addColor(RGBColor::YELLOW, (range * 2 / 6.0));
    scheme.addColor(RGBColor::GREEN, (range * 3 / 6.0));
    scheme.addColor(RGBColor::CYAN, (range * 4 / 6.0));
    scheme.addColor(RGBColor::BLUE, (range * 5 / 6.0));
    scheme.addColor(RGBColor::MAGENTA, (200));

    // draw the area depending if the vehicle is charging
    glTranslated(0, 0, getType());

    //right catenary
    if (getElecHybridCount() > 0) {
        GLHelper::setColor(redChargeOverheadWire);
    } else if (myTractionSubstation != NULL && myTractionSubstation->getElecHybridCount() > 0) {
        //GLHelper::setColor(redCharge);
        GLHelper::setColor(yellowCharge);
    } else {
        //GLHelper::setColor(lightgray);
        GLHelper::setColor(green);
    }

    const double exaggeration = s.addSize.getExaggeration(s, this);
    //exaggeration - wide of line


    //right catenary
    double toPos = getEndLanePosition();
    double fromPos = 0;
    PositionVector myFGShape_aux = myFGShape;
    const MSLane& lane_aux = getLane();
    std::vector<double> myFGShapeRotations_aux;
    std::vector<double> myFGShapeLengths_aux;
    int e_aux = 0;
    Node* node = NULL;
    double voltage = 0;

    if (myCircuitStartNodePos != NULL) {
        voltage = myCircuitStartNodePos->getVoltage();
        GLHelper::setColor(scheme.getColor(MAX2(0.0, voltage - 400)));
    }

    //TODORICE
    //for (auto it = myChargingVehicles.begin(); it != myChargingVehicles.end(); ++it) {
    for (std::vector<SUMOVehicle*>::const_iterator it = myChargingVehicles.begin(); it != myChargingVehicles.end(); ++it) {
        fromPos = (*it)->getPositionOnLane() - ((*it)->getVehicleType().getLength() / 2);
        if (fromPos < 0) {
            fromPos = 0;
        };

        myFGShape_aux = myFGShape;
        myFGShape_aux = myFGShape_aux.getSubpart(
                            lane_aux.interpolateLanePosToGeometryPos(fromPos),
                            lane_aux.interpolateLanePosToGeometryPos(toPos));

        myFGShapeRotations_aux.clear();
        myFGShapeLengths_aux.clear();

        myFGShapeRotations_aux.reserve(myFGShape.size() - 1);
        myFGShapeLengths_aux.reserve(myFGShape.size() - 1);

        e_aux = (int)myFGShape_aux.size() - 1;
        for (int i = 0; i < e_aux; ++i) {
            const Position& f_aux = myFGShape_aux[i];
            const Position& s_aux = myFGShape_aux[i + 1];
            myFGShapeLengths_aux.push_back(f_aux.distanceTo(s_aux));
            myFGShapeRotations_aux.push_back((double)atan2((s_aux.x() - f_aux.x()), (f_aux.y() - s_aux.y())) * (double) 180.0 / (double)M_PI);
        }

        voltage = 0;
        if (getCircuit() != nullptr) {
            //TODORICE it causes crash of SUMO GUI often in debug mode and
            // vector "_STL_VERIFY(_Mycont->_Myfirst <= _Ptr && _Ptr < _Mycont->_Mylast,
            //"can't dereference out of range vector iterator"); "
            node = getCircuit()->getNode("pos_" + (*it)->getID());
            if (node != nullptr) {
                voltage = node->getVoltage();
            }
        }
        GLHelper::setColor(scheme.getColor(MAX2(0.0, voltage - 400)));
        GLHelper::drawBoxLines(myFGShape_aux, myFGShapeRotations_aux, myFGShapeLengths_aux, exaggeration / 8, 0, 0.5);

        toPos = fromPos;
    }

    myFGShape_aux = myFGShape;

    myFGShape_aux = myFGShape_aux.getSubpart(
                        lane_aux.interpolateLanePosToGeometryPos(getBeginLanePosition()),
                        lane_aux.interpolateLanePosToGeometryPos(toPos));

    myFGShapeRotations_aux.clear();
    myFGShapeLengths_aux.clear();

    myFGShapeRotations_aux.reserve(myFGShape.size() - 1);
    myFGShapeLengths_aux.reserve(myFGShape.size() - 1);

    e_aux = (int)myFGShape_aux.size() - 1;
    for (int i = 0; i < e_aux; ++i) {
        const Position& f_aux = myFGShape_aux[i];
        const Position& s_aux = myFGShape_aux[i + 1];
        myFGShapeLengths_aux.push_back(f_aux.distanceTo(s_aux));
        myFGShapeRotations_aux.push_back((double)atan2((s_aux.x() - f_aux.x()), (f_aux.y() - s_aux.y())) * (double) 180.0 / (double)M_PI);
    }

    //GLHelper::setColor(green);
    GLHelper::drawBoxLines(myFGShape_aux, myFGShapeRotations_aux, myFGShapeLengths_aux, exaggeration / 8, 0, 0.5);


    //left catenary
    if (getElecHybridCount() > 0) {
        //GLHelper::setColor(yellowCharge);
        GLHelper::setColor(redChargeOverheadWire);
    } else if (myTractionSubstation != NULL && myTractionSubstation->getElecHybridCount() > 0) {
        //GLHelper::setColor(yellow);
        GLHelper::setColor(yellowCharge);
    } else {
        //GLHelper::setColor(lightgray);
        GLHelper::setColor(green);
    }
    GLHelper::drawBoxLines(myFGShape, myFGShapeRotations, myFGShapeLengths, exaggeration / 8, 0, -0.5);

    //a catenary in the centre of lane
    //GLHelper::drawBoxLines(myFGShape, myFGShapeRotations, myFGShapeLengths, exaggeration / 4);

    // draw details unless zoomed out to far
    if (s.scale * exaggeration >= 10 && myVoltageSource) {

        // push charging power matrix
        glPushMatrix();
        // draw charging power
        GLHelper::drawText((toString(getTractionSubstation()->getSubstationVoltage()) + " V").c_str(), myFGSignPos + Position(1.2, 0), .1, 1.f, RGBColor(114, 210, 252), myFGSignRot, FONS_ALIGN_LEFT);
        // pop charging power matrix
        glPopMatrix();

        // draw the sign
        glTranslated(myFGSignPos.x(), myFGSignPos.y(), 0);
        int noPoints = 9;
        if (s.scale * exaggeration > 25) {
            noPoints = MIN2((int)(9.0 + (s.scale * exaggeration) / 10.0), 36);
        }

        glScaled(exaggeration, exaggeration, 1);
        GLHelper::drawFilledCircle((double) 1.1, noPoints);
        glTranslated(0, 0, .1);

        GLHelper::setColor(yellow);
        GLHelper::drawFilledCircle((double) 0.9, noPoints);

        if (s.scale * exaggeration >= 4.5) {
            GLHelper::drawText("C", Position(), .1, 1.6, lightgray, myFGSignRot);
        }

        glTranslated(5, 0, 0);

    }


    glPopMatrix();
    glPopName();
    drawName(getCenteringBoundary().getCenter(), s.scale, s.addName);
}


GUIParameterTableWindow*
GUIOverheadWireClamp::getParameterWindow(GUIMainWindow& app, GUISUMOAbstractView&) {
    // Create table items
    GUIParameterTableWindow* ret = new GUIParameterTableWindow(app, *this);

    // add items
    //ret->mkItem("begin position [m]", false, NAN);
    //ret->mkItem("end position [m]", false, NAN);
    //ret->mkItem("voltage [V]", false, NAN);

    // close building
    ret->closeBuilding();
    return ret;
}


GUIGLObjectPopupMenu*
GUIOverheadWireClamp::getPopUpMenu(GUIMainWindow& app, GUISUMOAbstractView& parent) {
    GUIGLObjectPopupMenu* ret = new GUIGLObjectPopupMenu(app, parent, *this);
    buildPopupHeader(ret, app);
    buildCenterPopupEntry(ret);
    buildNameCopyPopupEntry(ret);
    buildSelectionPopupEntry(ret);
    buildShowParamsPopupEntry(ret);
    buildPositionCopyEntry(ret, false);
    return ret;
}


Boundary
GUIOverheadWireClamp::getCenteringBoundary() const {
    Boundary b = myFGShape.getBoxBoundary();
    b.grow(20);
    return b;
}


void
GUIOverheadWireClamp::drawGL(const GUIVisualizationSettings& s) const {
    // Draw overhead wire segment
    glPushName(getGlID());
    glPushMatrix();
    RGBColor lightgray(211, 211, 211, 255);
    RGBColor green(76, 170, 50, 255);
    RGBColor yellow(255, 235, 0, 255);
    RGBColor yellowCharge(255, 180, 0, 255);
    RGBColor redCharge(255, 51, 51, 255);
    RGBColor redChargeOverheadWire(180, 0, 0, 255);

    // draw the area depending if the vehicle is charging
    glTranslated(0, 0, getType());


    GLHelper::setColor(redChargeOverheadWire);


    const double exaggeration = s.addSize.getExaggeration(s, this);
    //exaggeration - wide of line



    PositionVector myFGShape_aux = myFGShape;
    std::vector<double> myFGShapeRotations_aux;
    std::vector<double> myFGShapeLengths_aux;

    myFGShapeRotations_aux.clear();
    myFGShapeLengths_aux.clear();

    myFGShapeRotations_aux.reserve(myFGShape.size() - 1);
    myFGShapeLengths_aux.reserve(myFGShape.size() - 1);

    int e_aux = (int)myFGShape_aux.size() - 1;
    for (int i = 0; i < e_aux; ++i) {
        const Position& f_aux = myFGShape_aux[i];
        const Position& s_aux = myFGShape_aux[i + 1];
        myFGShapeLengths_aux.push_back(f_aux.distanceTo(s_aux));
        myFGShapeRotations_aux.push_back((double)atan2((s_aux.x() - f_aux.x()), (f_aux.y() - s_aux.y())) * (double) 180.0 / (double)M_PI);
    }

    //GLHelper::setColor(green);
    GLHelper::drawBoxLines(myFGShape_aux, myFGShapeRotations_aux, myFGShapeLengths_aux, exaggeration / 8, 0, 0.5);

    //GLHelper::drawBoxLines(myFGShape, myFGShapeRotations, myFGShapeLengths, exaggeration / 8, 0, -0.5);



    glPopMatrix();
    glPopName();
    drawName(getCenteringBoundary().getCenter(), s.scale, s.addName);
}


/****************************************************************************/
