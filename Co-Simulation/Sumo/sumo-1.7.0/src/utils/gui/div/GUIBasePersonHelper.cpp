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
/// @file    GUIBasePersonHelper.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Feb 2018
///
// Functions used in GUIPerson and GNEPerson
/****************************************************************************/
#include <config.h>

#include <microsim/transportables/MSPModel_Striping.h>
#include <utils/gui/globjects/GLIncludes.h>
#include <utils/gui/images/GUITexturesHelper.h>

#include "GLHelper.h"
#include "GUIBasePersonHelper.h"

// ===========================================================================
// method definitions
// ===========================================================================

void
GUIBasePersonHelper::drawAction_drawAsTriangle(const double angle, const double length, const double width) {
    // draw triangle pointing forward
    glRotated(RAD2DEG(angle), 0, 0, 1);
    glScaled(length, width, 1);
    glBegin(GL_TRIANGLES);
    glVertex2d(0., 0.);
    glVertex2d(-1, -0.5);
    glVertex2d(-1, 0.5);
    glEnd();
    // draw a smaller triangle to indicate facing
    GLHelper::setColor(GLHelper::getColor().changedBrightness(-64));
    glTranslated(0, 0, .045);
    glBegin(GL_TRIANGLES);
    glVertex2d(0., 0.);
    glVertex2d(-0.5, -0.25);
    glVertex2d(-0.5, 0.25);
    glEnd();
    glTranslated(0, 0, -.045);
}


void
GUIBasePersonHelper::drawAction_drawAsCircle(const double length, const double width) {
    glScaled(length, width, 1);
    GLHelper::drawFilledCircle(0.8);
}


void
GUIBasePersonHelper::drawAction_drawAsPoly(const double angle, const double length, const double width) {
    // draw pedestrian shape
    glRotated(RAD2DEG(angle), 0, 0, 1);
    glScaled(length, width, 1);
    RGBColor lighter = GLHelper::getColor().changedBrightness(51);
    glTranslated(0, 0, .045);
    // head
    glScaled(1, 0.5, 1.);
    GLHelper::drawFilledCircle(0.5);
    // nose
    glBegin(GL_TRIANGLES);
    glVertex2d(0.0, -0.2);
    glVertex2d(0.0, 0.2);
    glVertex2d(0.6, 0.0);
    glEnd();
    glTranslated(0, 0, -.045);
    // body
    glScaled(0.9, 2.0, 1);
    glTranslated(0, 0, .04);
    GLHelper::setColor(lighter);
    GLHelper::drawFilledCircle(0.5);
    glTranslated(0, 0, -.04);
}


void
GUIBasePersonHelper::drawAction_drawAsImage(const double angle, const double length, const double width, const std::string& file,
        const SUMOVehicleShape guiShape, const double exaggeration) {
    // first check if filename isn't empty
    if (file != "") {
        if (guiShape == SVS_PEDESTRIAN) {
            glRotated(RAD2DEG(angle + M_PI / 2.), 0, 0, 1);
        }
        int textureID = GUITexturesHelper::getTextureID(file);
        if (textureID > 0) {
            const double halfLength = length / 2.0 * exaggeration;
            const double halfWidth = width / 2.0 * exaggeration;
            GUITexturesHelper::drawTexturedBox(textureID, -halfWidth, -halfLength, halfWidth, halfLength);
        }
    } else {
        // fallback if no image is defined
        drawAction_drawAsPoly(angle, length, width);
    }
}


/****************************************************************************/
