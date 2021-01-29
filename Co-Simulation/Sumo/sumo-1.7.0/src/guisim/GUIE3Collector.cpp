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
/// @file    GUIE3Collector.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Jan 2004
///
// The gui-version of a MSE3Collector
/****************************************************************************/
#include <config.h>

#include "GUIE3Collector.h"
#include "GUIEdge.h"
#include <utils/gui/div/GUIParameterTableWindow.h>
#include <utils/gui/div/GLHelper.h>
#include <microsim/logging/FunctionBinding.h>
#include <microsim/MSLane.h>
#include <utils/gui/globjects/GLIncludes.h>


// ===========================================================================
// method definitions
// ===========================================================================

// -------------------------------------------------------------------------
// GUIE3Collector::MyWrapper-methods
// -------------------------------------------------------------------------

GUIE3Collector::MyWrapper::MyWrapper(GUIE3Collector& detector) :
    GUIDetectorWrapper(GLO_E3DETECTOR, detector.getID()),
    myDetector(detector) {
    const CrossSectionVector& entries = detector.getEntries();
    const CrossSectionVector& exits = detector.getExits();
    CrossSectionVectorConstIt i;
    for (i = entries.begin(); i != entries.end(); ++i) {
        SingleCrossingDefinition def = buildDefinition(*i);
        myBoundary.add(def.myFGPosition);
        myEntryDefinitions.push_back(def);
    }
    for (i = exits.begin(); i != exits.end(); ++i) {
        SingleCrossingDefinition def = buildDefinition(*i);
        myBoundary.add(def.myFGPosition);
        myExitDefinitions.push_back(def);
    }
}


GUIE3Collector::MyWrapper::~MyWrapper() {}


GUIE3Collector::MyWrapper::SingleCrossingDefinition
GUIE3Collector::MyWrapper::buildDefinition(const MSCrossSection& section) {
    SingleCrossingDefinition def;
    def.myFGPosition = section.myLane->geometryPositionAtOffset(section.myPosition);
    def.myFGRotation = -section.myLane->getShape().rotationDegreeAtOffset(section.myPosition);
    return def;
}


GUIParameterTableWindow*
GUIE3Collector::MyWrapper::getParameterWindow(GUIMainWindow& app,
        GUISUMOAbstractView&) {
    GUIParameterTableWindow* ret =
        new GUIParameterTableWindow(app, *this);
    // add items
    // values
    ret->mkItem("vehicles within [#]", true,
                new FunctionBinding<MSE3Collector, int>(&myDetector, &MSE3Collector::getVehiclesWithin));
    ret->mkItem("mean speed [m/s]", true,
                new FunctionBinding<MSE3Collector, double>(&myDetector, &MSE3Collector::getCurrentMeanSpeed));
    ret->mkItem("haltings [#]", true,
                new FunctionBinding<MSE3Collector, int>(&myDetector, &MSE3Collector::getCurrentHaltingNumber));
    // close building
    ret->closeBuilding();
    return ret;
}


void
GUIE3Collector::MyWrapper::drawGL(const GUIVisualizationSettings& s) const {
    glPushName(getGlID());
    glPushMatrix();
    glTranslated(0, 0, GLO_JUNCTION + 0.4); // do not draw on top of linkRules
    typedef std::vector<SingleCrossingDefinition> CrossingDefinitions;
    CrossingDefinitions::const_iterator i;
    GLHelper::setColor(s.detectorSettings.E3EntryColor);
    const double exaggeration = s.addSize.getExaggeration(s, this);
    for (i = myEntryDefinitions.begin(); i != myEntryDefinitions.end(); ++i) {
        drawSingleCrossing((*i).myFGPosition, (*i).myFGRotation, exaggeration);
    }
    GLHelper::setColor(s.detectorSettings.E3ExitColor);
    for (i = myExitDefinitions.begin(); i != myExitDefinitions.end(); ++i) {
        drawSingleCrossing((*i).myFGPosition, (*i).myFGRotation, exaggeration);
    }
    glPopMatrix();
    drawName(getCenteringBoundary().getCenter(), s.scale, s.addName);
    glPopName();
}


void
GUIE3Collector::MyWrapper::drawSingleCrossing(const Position& pos,
        double rot, double upscale) const {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glPushMatrix();
    glTranslated(pos.x(), pos.y(), 0);
    glRotated(rot, 0, 0, 1);
    glScaled(upscale, upscale, 1);
    glBegin(GL_LINES);
    glVertex2d(1.7, 0);
    glVertex2d(-1.7, 0);
    glEnd();
    glBegin(GL_QUADS);
    glVertex2d(-1.7, .5);
    glVertex2d(-1.7, -.5);
    glVertex2d(1.7, -.5);
    glVertex2d(1.7, .5);
    glEnd();
    // arrows
    glTranslated(1.5, 0, 0);
    GLHelper::drawBoxLine(Position(0, 4), 0, 2, .05);
    GLHelper::drawTriangleAtEnd(Position(0, 4), Position(0, 1), (double) 1, (double) .25);
    glTranslated(-3, 0, 0);
    GLHelper::drawBoxLine(Position(0, 4), 0, 2, .05);
    GLHelper::drawTriangleAtEnd(Position(0, 4), Position(0, 1), (double) 1, (double) .25);
    glPopMatrix();
}


Boundary
GUIE3Collector::MyWrapper::getCenteringBoundary() const {
    Boundary b(myBoundary);
    b.grow(20);
    return b;
}


GUIE3Collector&
GUIE3Collector::MyWrapper::getDetector() {
    return myDetector;
}


/* -------------------------------------------------------------------------
 * GUIE3Collector-methods
 * ----------------------------------------------------------------------- */
GUIE3Collector::GUIE3Collector(const std::string& id,
                               const CrossSectionVector& entries,  const CrossSectionVector& exits,
                               double haltingSpeedThreshold,
                               SUMOTime haltingTimeThreshold, const std::string& vTypes, bool openEntry)
    : MSE3Collector(id, entries,  exits, haltingSpeedThreshold, haltingTimeThreshold, vTypes, openEntry) {}


GUIE3Collector::~GUIE3Collector() {}


const CrossSectionVector&
GUIE3Collector::getEntries() const {
    return myEntries;
}


const CrossSectionVector&
GUIE3Collector::getExits() const {
    return myExits;
}



GUIDetectorWrapper*
GUIE3Collector::buildDetectorGUIRepresentation() {
    return new MyWrapper(*this);
}


/****************************************************************************/
