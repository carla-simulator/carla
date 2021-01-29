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
/// @file    GUIContainerStop.cpp
/// @author  Melanie Weber
/// @author  Andreas Kendziorra
/// @date    Wed, 01.08.2014
///
// A lane area vehicles can halt at (gui-version)
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
#include "GUIContainer.h"
#include "GUIContainerStop.h"
#include <utils/gui/globjects/GUIGLObjectPopupMenu.h>
#include <utils/gui/windows/GUIAppEnum.h>
#include <gui/GUIGlobals.h>
#include <utils/gui/div/GUIParameterTableWindow.h>
#include <gui/GUIApplicationWindow.h>
#include <microsim/logging/FunctionBinding.h>
#include <utils/gui/div/GUIGlobalSelection.h>
#include <utils/geom/GeomHelper.h>
#include <guisim/GUIContainerStop.h>
#include <utils/gui/globjects/GLIncludes.h>
#include <foreign/fontstash/fontstash.h>


// ===========================================================================
// method definitions
// ===========================================================================
GUIContainerStop::GUIContainerStop(const std::string& id, const std::vector<std::string>& lines, MSLane& lane,
                                   double frompos, double topos, const std::string& name, int containerCapacity, double parkingLength) :
    MSStoppingPlace(id, lines, lane, frompos, topos, name, containerCapacity, parkingLength),
    GUIGlObject_AbstractAdd(GLO_CONTAINER_STOP, id) {
    const double offsetSign = MSGlobals::gLefthand ? -1 : 1;
    myFGShape = lane.getShape();
    myFGShape.move2side(1.65 * offsetSign);
    myFGShape = myFGShape.getSubpart(
                    lane.interpolateLanePosToGeometryPos(frompos),
                    lane.interpolateLanePosToGeometryPos(topos));
    myFGShapeRotations.reserve(myFGShape.size() - 1);
    myFGShapeLengths.reserve(myFGShape.size() - 1);
    int e = (int) myFGShape.size() - 1;
    for (int i = 0; i < e; ++i) {
        const Position& f = myFGShape[i];
        const Position& s = myFGShape[i + 1];
        myFGShapeLengths.push_back(f.distanceTo(s));
        myFGShapeRotations.push_back((double) atan2((s.x() - f.x()), (f.y() - s.y())) * (double) 180.0 / (double) M_PI);
    }
    PositionVector tmp = myFGShape;
    tmp.move2side(1.5 * offsetSign);
    myFGSignPos = tmp.getLineCenter();
    myFGSignRot = 0;
    if (tmp.length() != 0) {
        myFGSignRot = myFGShape.rotationDegreeAtOffset(double((myFGShape.length() / 2.)));
        myFGSignRot -= 90;
    }
}


GUIContainerStop::~GUIContainerStop() {}


GUIGLObjectPopupMenu*
GUIContainerStop::getPopUpMenu(GUIMainWindow& app,
                               GUISUMOAbstractView& parent) {
    GUIGLObjectPopupMenu* ret = new GUIGLObjectPopupMenu(app, parent, *this);
    buildPopupHeader(ret, app);
    buildCenterPopupEntry(ret);
    buildNameCopyPopupEntry(ret);
    buildSelectionPopupEntry(ret);
    buildShowParamsPopupEntry(ret);
    buildPositionCopyEntry(ret, false);
    return ret;
}


GUIParameterTableWindow*
GUIContainerStop::getParameterWindow(GUIMainWindow& app,
                                     GUISUMOAbstractView&) {
    GUIParameterTableWindow* ret =
        new GUIParameterTableWindow(app, *this);
    // add items
    ret->mkItem("name", false, getMyName());
    ret->mkItem("begin position [m]", false, myBegPos);
    ret->mkItem("end position [m]", false, myEndPos);
    ret->mkItem("container number [#]", true, new FunctionBinding<GUIContainerStop, int>(this, &MSStoppingPlace::getTransportableNumber));
    ret->mkItem("stopped vehicles[#]", true, new FunctionBinding<GUIContainerStop, int>(this, &MSStoppingPlace::getStoppedVehicleNumber));
    ret->mkItem("last free pos[m]", true, new FunctionBinding<GUIContainerStop, double>(this, &MSStoppingPlace::getLastFreePos));
    // close building
    ret->closeBuilding();
    return ret;
}


void
GUIContainerStop::drawGL(const GUIVisualizationSettings& s) const {
    glPushName(getGlID());
    glPushMatrix();
    // draw the area
    glTranslated(0, 0, getType());
    GLHelper::setColor(s.stoppingPlaceSettings.containerStopColor);
    const double exaggeration = s.addSize.getExaggeration(s, this);
    GLHelper::drawBoxLines(myFGShape, myFGShapeRotations, myFGShapeLengths, 1.0);
    // draw details unless zoomed out to far
    if (s.drawDetail(s.detailSettings.stoppingPlaceDetails, exaggeration)) {
        glPushMatrix();
        // draw the lines
        const double rotSign = MSGlobals::gLefthand ? -1 : 1;
        // Iterate over every line
        for (int i = 0; i < (int)myLines.size(); ++i) {
            // push a new matrix for every line
            glPushMatrix();
            // traslate and rotate
            glTranslated(myFGSignPos.x(), myFGSignPos.y(), 0);
            glRotated(rotSign * myFGSignRot, 0, 0, 1);
            // draw line
            GLHelper::drawText(myLines[i].c_str(), Position(1.2, (double)i), .1, 1.f, s.stoppingPlaceSettings.containerStopColor, 0, FONS_ALIGN_LEFT);
            // pop matrix for every line
            glPopMatrix();
        }
        // draw the sign
        glTranslated(myFGSignPos.x(), myFGSignPos.y(), 0);
        int noPoints = 9;
        if (s.scale * exaggeration > 25) {
            noPoints = MIN2((int)(9.0 + (s.scale * exaggeration) / 10.0), 36);
        }
        glScaled(exaggeration, exaggeration, 1);
        GLHelper::drawFilledCircle((double) 1.1, noPoints);
        glTranslated(0, 0, .1);
        GLHelper::setColor(s.stoppingPlaceSettings.containerStopColorSign);
        GLHelper::drawFilledCircle((double) 0.9, noPoints);
        if (s.drawDetail(s.detailSettings.stoppingPlaceText, exaggeration)) {
            GLHelper::drawText("C", Position(), .1, 1.6, s.stoppingPlaceSettings.containerStopColor, myFGSignRot);
        }
        glPopMatrix();
    }
    glPopMatrix();
    glPopName();
    drawName(getCenteringBoundary().getCenter(), s.scale, s.addName);
    //for (std::vector<MSTransportable*>::const_iterator i = myWaitingTransportables.begin(); i != myWaitingTransportables.end(); ++i) {
    //    glTranslated(0, 1, 0); // make multiple containers viewable
    //    static_cast<GUIContainer*>(*i)->drawGL(s);
    //}
}


Boundary
GUIContainerStop::getCenteringBoundary() const {
    Boundary b = myFGShape.getBoxBoundary();
    b.grow(20);
    return b;
}

const std::string
GUIContainerStop::getOptionalName() const {
    return myName;
}


/****************************************************************************/
