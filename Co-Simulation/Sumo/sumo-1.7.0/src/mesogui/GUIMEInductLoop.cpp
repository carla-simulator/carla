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
/// @file    GUIMEInductLoop.cpp
/// @author  Daniel Krajzewicz
/// @date    Thu, 21.07.2005
///
// The gui-version of the MEInductLoop
/****************************************************************************/
#include <config.h>


#include <utils/gui/globjects/GLIncludes.h>
#include <utils/gui/globjects/GUIGlObject.h>
#include <utils/geom/PositionVector.h>
#include <guisim/GUILane.h>
#include <utils/gui/div/GLHelper.h>
#include <utils/gui/div/GUIParameterTableWindow.h>
#include <microsim/logging/FuncBinding_IntParam.h>
#include <microsim/logging/FunctionBinding.h>
#include <mesosim/MEInductLoop.h>
#include <mesosim/MESegment.h>
#include "GUIMEInductLoop.h"


// ===========================================================================
// method definitions
// ===========================================================================
/* -------------------------------------------------------------------------
 * GUIMEInductLoop-methods
 * ----------------------------------------------------------------------- */
GUIMEInductLoop::GUIMEInductLoop(const std::string& id, MESegment* s,
                                 double position, const std::string& vTypes)
    : MEInductLoop(id, s, position, vTypes) {}


GUIMEInductLoop::~GUIMEInductLoop() {}


GUIDetectorWrapper*
GUIMEInductLoop::buildDetectorGUIRepresentation() {
    return new MyWrapper(*this, myPosition);
}

// -----------------------------------------------------------------------
// GUIMEInductLoop::MyWrapper-methods
// -----------------------------------------------------------------------

GUIMEInductLoop::MyWrapper::MyWrapper(GUIMEInductLoop& detector, double pos)
    : GUIDetectorWrapper(GLO_E1DETECTOR_ME, detector.getID()),
      myDetector(detector), myPosition(pos) {
    const MSLane* lane = detector.mySegment->getEdge().getLanes()[0];
    myFGPosition = lane->geometryPositionAtOffset(pos);
    myBoundary.add(myFGPosition.x() + (double) 5.5, myFGPosition.y() + (double) 5.5);
    myBoundary.add(myFGPosition.x() - (double) 5.5, myFGPosition.y() - (double) 5.5);
    myFGRotation = -lane->getShape().rotationDegreeAtOffset(pos);
}


GUIMEInductLoop::MyWrapper::~MyWrapper() {}


Boundary
GUIMEInductLoop::MyWrapper::getCenteringBoundary() const {
    Boundary b(myBoundary);
    b.grow(20);
    return b;
}



GUIParameterTableWindow*
GUIMEInductLoop::MyWrapper::getParameterWindow(GUIMainWindow& app,
        GUISUMOAbstractView& /* parent */) {
    GUIParameterTableWindow* ret = new GUIParameterTableWindow(app, *this);
    // add items
    /*
    ret->mkItem("flow [veh/h]", true,
        new FuncBinding_IntParam<GUIMEInductLoop, double>(
            &(getLoop()), &GUIMEInductLoop::getFlow, 1));
    ret->mkItem("mean speed [m/s]", true,
        new FuncBinding_IntParam<GUIMEInductLoop, double>(
            &(getLoop()), &GUIMEInductLoop::getMeanSpeed, 1));
    ret->mkItem("occupancy [%]", true,
        new FuncBinding_IntParam<GUIMEInductLoop, double>(
            &(getLoop()), &GUIMEInductLoop::getOccupancy, 1));
    ret->mkItem("mean vehicle length [m]", true,
        new FuncBinding_IntParam<GUIMEInductLoop, double>(
            &(getLoop()), &GUIMEInductLoop::getMeanVehicleLength, 1));
    ret->mkItem("empty time [s]", true,
        new FunctionBinding<GUIMEInductLoop, double>(
            &(getLoop()), &GUIMEInductLoop::getTimeSinceLastDetection));
            */
    //
    ret->mkItem("position [m]", false, myPosition);
    ret->mkItem("lane", false, myDetector.mySegment->getID());
    // close building
    ret->closeBuilding();
    return ret;
}


void
GUIMEInductLoop::MyWrapper::drawGL(const GUIVisualizationSettings& s) const {
    glPushName(getGlID());
    glPolygonOffset(0, -2);
    double width = (double) 2.0 * s.scale;
    glLineWidth(1.0);
    const double exaggeration = s.addSize.getExaggeration(s, this);
    // shape
    glColor3d(1, 1, 0);
    glPushMatrix();
    glTranslated(myFGPosition.x(), myFGPosition.y(), getType());
    glRotated(myFGRotation, 0, 0, 1);
    glScaled(exaggeration, exaggeration, exaggeration);
    glBegin(GL_QUADS);
    glVertex2d(0 - 1.0, 2);
    glVertex2d(-1.0, -2);
    glVertex2d(1.0, -2);
    glVertex2d(1.0, 2);
    glEnd();
    glBegin(GL_LINES);
    // without the substracted offsets, lines are partially longer
    //  than the boxes
    glVertex2d(0, 2 - .1);
    glVertex2d(0, -2 + .1);
    glEnd();

    // outline
    if (width * exaggeration > 1) {
        glColor3d(1, 1, 1);
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
        glColor3d(1, 1, 1);
        glBegin(GL_LINES);
        glVertex2d(0, 1.7);
        glVertex2d(0, -1.7);
        glEnd();
    }
    glPopMatrix();
    drawName(getCenteringBoundary().getCenter(), s.scale, s.addName);
    glPopName();
}


GUIMEInductLoop&
GUIMEInductLoop::MyWrapper::getLoop() {
    return myDetector;
}


/****************************************************************************/
