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
/// @file    GUIInductLoop.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Aug 2003
///
// The gui-version of the MSInductLoop, together with the according
/****************************************************************************/
#include <config.h>

#include <utils/gui/globjects/GUIGlObject.h>
#include <utils/geom/PositionVector.h>
#include "GUIInductLoop.h"
#include <utils/gui/div/GLHelper.h>
#include <utils/gui/div/GUIParameterTableWindow.h>
#include <microsim/logging/FunctionBinding.h>
#include <microsim/logging/FuncBinding_IntParam.h>
#include <microsim/MSLane.h>
#include <microsim/output/MSInductLoop.h>
#include "GUIEdge.h"
#include <utils/gui/globjects/GLIncludes.h>


// ===========================================================================
// method definitions
// ===========================================================================
/* -------------------------------------------------------------------------
 * GUIInductLoop-methods
 * ----------------------------------------------------------------------- */
GUIInductLoop::GUIInductLoop(const std::string& id, MSLane* const lane,
                             double position, const std::string& vTypes, bool show) :
    MSInductLoop(id, lane, position, vTypes, true),
    myWrapper(nullptr),
    myShow(show)
{}


GUIInductLoop::~GUIInductLoop() {}


GUIDetectorWrapper*
GUIInductLoop::buildDetectorGUIRepresentation() {
    // caller (GUINet) takes responsibility for pointer
    myWrapper = new MyWrapper(*this, myPosition);
    return myWrapper;
}


void
GUIInductLoop::setSpecialColor(const RGBColor* color) {
    if (myWrapper != nullptr) {
        myWrapper->setSpecialColor(color);
    }
}


// -------------------------------------------------------------------------
// GUIInductLoop::MyWrapper-methods
// -------------------------------------------------------------------------

GUIInductLoop::MyWrapper::MyWrapper(GUIInductLoop& detector, double pos) :
    GUIDetectorWrapper(GLO_E1DETECTOR, detector.getID()),
    myDetector(detector), myPosition(pos),
    mySpecialColor(nullptr) {
    myFGPosition = detector.getLane()->geometryPositionAtOffset(pos);
    myBoundary.add(myFGPosition.x() + (double) 5.5, myFGPosition.y() + (double) 5.5);
    myBoundary.add(myFGPosition.x() - (double) 5.5, myFGPosition.y() - (double) 5.5);
    myFGRotation = -detector.getLane()->getShape().rotationDegreeAtOffset(pos);
}


GUIInductLoop::MyWrapper::~MyWrapper() {}


Boundary
GUIInductLoop::MyWrapper::getCenteringBoundary() const {
    Boundary b(myBoundary);
    b.grow(20);
    return b;
}



GUIParameterTableWindow*
GUIInductLoop::MyWrapper::getParameterWindow(GUIMainWindow& app,
        GUISUMOAbstractView& /*parent !!! recheck this - never needed?*/) {
    GUIParameterTableWindow* ret = new GUIParameterTableWindow(app, *this);
    // add items
    // parameter
    ret->mkItem("position [m]", false, myPosition);
    ret->mkItem("lane", false, myDetector.getLane()->getID());
    // values
    ret->mkItem("entered vehicles [#]", true,
                new FuncBinding_IntParam<GUIInductLoop, double>(&myDetector, &GUIInductLoop::getEnteredNumber, 0));
    ret->mkItem("speed [m/s]", true,
                new FuncBinding_IntParam<GUIInductLoop, double>(&myDetector, &GUIInductLoop::getSpeed, 0));
    ret->mkItem("occupancy [%]", true,
                new FunctionBinding<GUIInductLoop, double>(&myDetector, &GUIInductLoop::getOccupancy));
    ret->mkItem("vehicle length [m]", true,
                new FuncBinding_IntParam<GUIInductLoop, double>(&myDetector, &GUIInductLoop::getVehicleLength, 0));
    ret->mkItem("empty time [s]", true,
                new FunctionBinding<GUIInductLoop, double>(&myDetector, &GUIInductLoop::getTimeSinceLastDetection));
    // close building
    ret->closeBuilding();
    return ret;
}


void
GUIInductLoop::MyWrapper::drawGL(const GUIVisualizationSettings& s) const {
    if (!myDetector.isVisible()) {
        return;
    }
    glPushName(getGlID());
    double width = (double) 2.0 * s.scale;
    glLineWidth(1.0);
    const double exaggeration = s.addSize.getExaggeration(s, this);
    // shape
    glColor3d(1, 1, 0);
    glPushMatrix();
    glTranslated(0, 0, getType());
    glTranslated(myFGPosition.x(), myFGPosition.y(), 0);
    glRotated(myFGRotation, 0, 0, 1);
    glScaled(exaggeration, exaggeration, 1);
    glBegin(GL_QUADS);
    glVertex2d(0 - 1.0, 2);
    glVertex2d(-1.0, -2);
    glVertex2d(1.0, -2);
    glVertex2d(1.0, 2);
    glEnd();
    glTranslated(0, 0, .01);
    glBegin(GL_LINES);
    glVertex2d(0, 2 - .1);
    glVertex2d(0, -2 + .1);
    glEnd();

    if (mySpecialColor == nullptr) {
        glColor3d(1, 1, 1);
    } else {
        GLHelper::setColor(*mySpecialColor);
    }

    // outline
    if (width * exaggeration > 1) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glBegin(GL_QUADS);
        glVertex2f(0 - 1.0, 2);
        glVertex2f(-1.0, -2);
        glVertex2f(1.0, -2);
        glVertex2f(1.0, 2);
        glEnd();
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    // position indicator
    if (width * exaggeration > 1) {
        glRotated(90, 0, 0, -1);
        glBegin(GL_LINES);
        glVertex2d(0, 1.7);
        glVertex2d(0, -1.7);
        glEnd();
    }
    glPopMatrix();
    drawName(getCenteringBoundary().getCenter(), s.scale, s.addName);
    glPopName();
}


/****************************************************************************/
