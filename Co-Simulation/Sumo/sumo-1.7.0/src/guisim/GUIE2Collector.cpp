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
/// @file    GUIE2Collector.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @author  Laura Bieker
/// @date    Okt 2003
///
// The gui-version of the MSE2Collector
/****************************************************************************/
#include <config.h>

#include <utils/gui/globjects/GUIGlObject.h>
#include <utils/geom/PositionVector.h>
#include "GUIE2Collector.h"
#include <utils/gui/div/GLHelper.h>
#include <utils/geom/GeomHelper.h>
#include <utils/gui/div/GUIParameterTableWindow.h>
#include <microsim/logging/FunctionBinding.h>
#include "GUIEdge.h"
#include <utils/gui/globjects/GLIncludes.h>


// ===========================================================================
// method definitions
// ===========================================================================
/* -------------------------------------------------------------------------
 * GUIE2Collector-methods
 * ----------------------------------------------------------------------- */
GUIE2Collector::GUIE2Collector(const std::string& id, DetectorUsage usage,
                               MSLane* lane, double startPos, double endPos, double detLength,
                               SUMOTime haltingTimeThreshold, double haltingSpeedThreshold,
                               double jamDistThreshold, const std::string& vTypes, bool showDetector)
    : MSE2Collector(id, usage, lane, startPos, endPos, detLength, haltingTimeThreshold,
                    haltingSpeedThreshold, jamDistThreshold, vTypes),
      myShow(showDetector) {}

GUIE2Collector::GUIE2Collector(const std::string& id, DetectorUsage usage,
                               std::vector<MSLane*> lanes, double startPos, double endPos,
                               SUMOTime haltingTimeThreshold, double haltingSpeedThreshold,
                               double jamDistThreshold, const std::string& vTypes, bool showDetector)
    : MSE2Collector(id, usage, lanes, startPos, endPos, haltingTimeThreshold,
                    haltingSpeedThreshold, jamDistThreshold, vTypes),
      myShow(showDetector) {}

GUIE2Collector::~GUIE2Collector() {}


GUIDetectorWrapper*
GUIE2Collector::buildDetectorGUIRepresentation() {
    return new MyWrapper(*this);
}

// -------------------------------------------------------------------------
// GUIE2Collector::MyWrapper-methods
// -------------------------------------------------------------------------

GUIE2Collector::MyWrapper::MyWrapper(GUIE2Collector& detector) :
    GUIDetectorWrapper(GLO_E2DETECTOR, detector.getID()),
    myDetector(detector) {
    // collect detector shape into one vector (v)
    const std::vector<MSLane*> lanes = detector.getLanes();
    for (std::vector<MSLane*>::const_iterator li = lanes.begin(); li != lanes.end(); ++li) {
        PositionVector shape = (*li)->getShape();
        double start = (li == lanes.begin() ? lanes.front()->interpolateLanePosToGeometryPos(detector.getStartPos()) : 0);
        double end = (li + 1 == lanes.end() ? lanes.back()->interpolateLanePosToGeometryPos(detector.getEndPos()) : shape.length());
        shape = shape.getSubpart(start, end);
        myFullGeometry.insert(myFullGeometry.end(), shape.begin(), shape.end());
    }
    //
    myShapeRotations.reserve(myFullGeometry.size() - 1);
    myShapeLengths.reserve(myFullGeometry.size() - 1);
    int e = (int) myFullGeometry.size() - 1;
    for (int i = 0; i < e; ++i) {
        const Position& f = myFullGeometry[i];
        const Position& s = myFullGeometry[i + 1];
        myShapeLengths.push_back(f.distanceTo(s));
        myShapeRotations.push_back((double) atan2((s.x() - f.x()), (f.y() - s.y())) * (double) 180.0 / (double) M_PI);
    }
    //
    myBoundary = myFullGeometry.getBoxBoundary();
}


GUIE2Collector::MyWrapper::~MyWrapper() {}


Boundary
GUIE2Collector::MyWrapper::getCenteringBoundary() const {
    Boundary b(myBoundary);
    b.grow(20);
    return b;
}


GUIParameterTableWindow*
GUIE2Collector::MyWrapper::getParameterWindow(GUIMainWindow& app,
        GUISUMOAbstractView&) {
    GUIParameterTableWindow* ret =
        new GUIParameterTableWindow(app, *this);
    // add items
    // parameter
    ret->mkItem("length [m]", false, myDetector.getLength());
    ret->mkItem("position [m]", false, myDetector.getStartPos());
    ret->mkItem("lane", false, myDetector.getLane()->getID());
    // values
    ret->mkItem("vehicles [#]", true,
                new FunctionBinding<MSE2Collector, int>(&myDetector, &MSE2Collector::getCurrentVehicleNumber));
    ret->mkItem("occupancy [%]", true,
                new FunctionBinding<MSE2Collector, double>(&myDetector, &MSE2Collector::getCurrentOccupancy));
    ret->mkItem("mean speed [m/s]", true,
                new FunctionBinding<MSE2Collector, double>(&myDetector, &MSE2Collector::getCurrentMeanSpeed));
    ret->mkItem("mean vehicle length [m]", true,
                new FunctionBinding<MSE2Collector, double>(&myDetector, &MSE2Collector::getCurrentMeanLength));
    ret->mkItem("jam number [#]", true,
                new FunctionBinding<MSE2Collector, int>(&myDetector, &MSE2Collector::getCurrentJamNumber));
    ret->mkItem("max jam length [veh]", true,
                new FunctionBinding<MSE2Collector, int>(&myDetector, &MSE2Collector::getCurrentMaxJamLengthInVehicles));
    ret->mkItem("max jam length [m]", true,
                new FunctionBinding<MSE2Collector, double>(&myDetector, &MSE2Collector::getCurrentMaxJamLengthInMeters));
    ret->mkItem("jam length sum [veh]", true,
                new FunctionBinding<MSE2Collector, int>(&myDetector, &MSE2Collector::getCurrentJamLengthInVehicles));
    ret->mkItem("jam length sum [m]", true,
                new FunctionBinding<MSE2Collector, double>(&myDetector, &MSE2Collector::getCurrentJamLengthInMeters));
    ret->mkItem("started halts [#]", true,
                new FunctionBinding<MSE2Collector, int>(&myDetector, &MSE2Collector::getCurrentStartedHalts));
    // close building
    ret->closeBuilding();
    return ret;
}


void
GUIE2Collector::MyWrapper::drawGL(const GUIVisualizationSettings& s) const {
    if (!myDetector.myShow) {
        return;
    }
    glPushName(getGlID());
    glPushMatrix();
    glTranslated(0, 0, getType());
    double dwidth = 1;
    const double exaggeration = s.addSize.getExaggeration(s, this);
    if (exaggeration > 0) {
        if (myDetector.getUsageType() == DU_TL_CONTROL) {
            dwidth = (double) 0.3;
            glColor3d(0, (double) .6, (double) .8);
        } else {
            glColor3d(0, (double) .8, (double) .8);
        }
        double width = (double) 2.0 * s.scale;
        if (width * exaggeration > 1.0) {
            GLHelper::drawBoxLines(myFullGeometry, myShapeRotations, myShapeLengths, dwidth * exaggeration);
        } else {
            int e = (int) myFullGeometry.size() - 1;
            for (int i = 0; i < e; ++i) {
                GLHelper::drawLine(myFullGeometry[i], myShapeRotations[i], myShapeLengths[i]);
            }
        }
    }
    glPopMatrix();
    drawName(getCenteringBoundary().getCenter(), s.scale, s.addName);
    glPopName();
}


GUIE2Collector&
GUIE2Collector::MyWrapper::getDetector() {
    return myDetector;
}


/****************************************************************************/
