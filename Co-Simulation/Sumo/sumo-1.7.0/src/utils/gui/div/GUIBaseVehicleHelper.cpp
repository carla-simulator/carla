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
/// @file    GUIBaseVehicleHelper.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Feb 2018
///
// Functions used in GUIBaseVehicleHelper and GNEVehicle
/****************************************************************************/
#include <config.h>
#include <fx.h>

#include <utils/gui/globjects/GLIncludes.h>
#include <utils/gui/images/GUITexturesHelper.h>
#include <utils/common/SUMOVehicleClass.h>
#include <utils/gui/settings/GUIVisualizationSettings.h>
#include "GLHelper.h"
#include "GUIBaseVehicleHelper.h"


// ===========================================================================
// data definitions
// ===========================================================================
/* -------------------------------------------------------------------------
 * drawed shapes
 * ----------------------------------------------------------------------- */
double vehiclePoly_PassengerCarBody[] = { .5, 0,  0, 0,  0, .3,  0.08, .44,  0.25, .5,  0.95, .5,  1., .4,  1., -.4,  0.95, -.5,  0.25, -.5,  0.08, -.44,  0, -.3,  0, 0,  -10000 };
double vehiclePoly_PassengerCarBodyFront[] = { 0.1, 0,  0.025, 0,  0.025, 0.25,  0.27, 0.4,  0.27, -.4,  0.025, -0.25,  0.025, 0,  -10000 };
double vehiclePoly_PassengerFrontGlass[] = { 0.35, 0,  0.3, 0,  0.3, 0.4,  0.43, 0.3,  0.43, -0.3,  0.3, -0.4,  0.3, 0,  -10000 };
double vehiclePoly_PassengerSedanRightGlass[] = { 0.36, -.43,  0.34, -.47,  0.77, -.47,  0.67, -.37,  0.45, -.37,  0.34, -.47,  -10000 };
double vehiclePoly_PassengerSedanLeftGlass[] = { 0.36, .43,  0.34, .47,  0.77, .47,  0.67, .37,  0.45, .37,  0.34, .47,  -10000 };
double vehiclePoly_PassengerSedanBackGlass[] = { 0.80, 0,  0.70, 0,  0.70, 0.3,  0.83, 0.4,  0.83, -.4,  0.70, -.3,  0.70, 0,  -10000 };
double vehiclePoly_PassengerHatchbackRightGlass[] = { 0.36, -.43,  0.34, -.47,  0.94, -.47,  0.80, -.37,  0.45, -.37,  0.34, -.47,  -10000 };
double vehiclePoly_PassengerHatchbackLeftGlass[] = { 0.36, .43,  0.34, .47,  0.94, .47,  0.80, .37,  0.45, .37,  0.34, .47,  -10000 };
double vehiclePoly_PassengerHatchbackBackGlass[] = { 0.92, 0,  0.80, 0,  0.80, 0.3,  0.95, 0.4,  0.95, -.4,  0.80, -.3,  0.80, 0,  -10000 };
double vehiclePoly_PassengerWagonRightGlass[] = { 0.36, -.43,  0.34, -.47,  0.94, -.47,  0.87, -.37,  0.45, -.37,  0.34, -.47,  -10000 };
double vehiclePoly_PassengerWagonLeftGlass[] = { 0.36, .43,  0.34, .47,  0.94, .47,  0.87, .37,  0.45, .37,  0.34, .47,  -10000 };
double vehiclePoly_PassengerWagonBackGlass[] = { 0.92, 0,  0.90, 0,  0.90, 0.3,  0.95, 0.4,  0.95, -.4,  0.90, -.3,  0.90, 0,  -10000 };

double vehiclePoly_PassengerVanBody[] = { .5, 0,  0, 0,  0, .4,  0.1, .5,  0.97, .5,  1., .47,  1., -.47,  0.97, -.5,  0.1, -.5,  0, -.4,  0, 0,  -10000 };
double vehiclePoly_PassengerVanBodyFront[] = { 0.1, 0,  0.025, 0,  0.025, 0.25,  0.13, 0.4,  0.13, -.4,  0.025, -0.25,  0.025, 0,  -10000 };
double vehiclePoly_PassengerVanFrontGlass[] = { 0.21, 0,  0.16, 0,  0.16, 0.4,  0.29, 0.3,  0.29, -0.3,  0.16, -0.4,  0.16, 0,  -10000 };
double vehiclePoly_PassengerVanRightGlass[] = { 0.36, -.43,  0.20, -.47,  0.98, -.47,  0.91, -.37,  0.31, -.37,  0.20, -.47,  -10000 };
double vehiclePoly_PassengerVanLeftGlass[] = { 0.36, .43,  0.20, .47,  0.98, .47,  0.91, .37,  0.31, .37,  0.20, .47,  -10000 };
double vehiclePoly_PassengerVanBackGlass[] = { 0.95, 0,  0.94, 0,  0.94, 0.3,  0.98, 0.4,  0.98, -.4,  0.94, -.3,  0.94, 0,  -10000 };

double vehiclePoly_DeliveryMediumRightGlass[] = { 0.21, -.43,  0.20, -.47,  0.38, -.47,  0.38, -.37,  0.31, -.37,  0.20, -.47,  -10000 };
double vehiclePoly_DeliveryMediumLeftGlass[] = { 0.21, .43,  0.20, .47,  0.38, .47,  0.38, .37,  0.31, .37,  0.20, .47,  -10000 };

double vehiclePoly_TransportBody[] = { .5, 0,  0, 0,  0, .45,  0.05, .5,  2.25, .5,  2.25, -.5,  0.05, -.5,  0, -.45,  0, 0,  -10000 };
double vehiclePoly_TransportFrontGlass[] = { 0.1, 0,  0.05, 0,  0.05, 0.45,  0.25, 0.4,  0.25, -.4,  0.05, -0.45,  0.05, 0,  -10000 };
double vehiclePoly_TransportRightGlass[] = { 0.36, -.47,  0.10, -.48,  1.25, -.48,  1.25, -.4,  0.3, -.4,  0.10, -.48,  -10000 };
double vehiclePoly_TransportLeftGlass[] = { 0.36, .47,  0.10, .48,  1.25, .48,  1.25, .4,  0.3, .4,  0.10, .48,  -10000 };

double vehiclePoly_EVehicleBody[] = { .5, 0,  0, 0,  0, .3,  0.08, .44,  0.25, .5,  0.75, .5,  .92, .44,  1, .3,  1, -.3,  .92, -.44,  .75, -.5,  .25, -.5,  0.08, -.44,  0, -.3,  0, 0,  -1000 };
double vehiclePoly_EVehicleFrontGlass[] = { .5, 0,  0.05, .05,  0.05, .25,  0.13, .39,  0.3, .45,  0.70, .45,  .87, .39,  .95, .25,  .95, -.25,  .87, -.39,  .70, -.45,  .3, -.45,  0.13, -.39,  0.05, -.25,  0.05, 0.05,  -1000 };
//double vehiclePoly_EVehicleFrontGlass[] = { 0.35,0,  0.1,0,  0.1,0.4,  0.43,0.3,  0.43,-0.3,  0.1,-0.4,  0.1,0,  -10000 };
double vehiclePoly_EVehicleBackGlass[] =  { 0.65, 0,  0.9, 0,  0.9, 0.4,  0.57, 0.3,  0.57, -0.3,  0.9, -0.4,  0.9, 0,  -10000 };

double vehiclePoly_Ship[] =  { 0.25, 0,  0, 0,  0.1, 0.25, 0.2, 0.45, 0.25, 0.5,  0.95, 0.5, 1.0, 0.45,   1.0, -0.45, 0.95, -0.5,  0.25, -0.5, 0.2, -0.45,  0.1, -0.25, 0, 0,   -10000 };
double vehiclePoly_ShipDeck[] =  { 0.5, 0,  0.25, 0.4,  0.95, 0.4, 0.95, -0.4, 0.25, -0.4, 0.25, 0.4, -10000 };
double vehiclePoly_ShipSuperStructure[] =  { 0.8, 0,  0.5, 0.3,  0.85, 0.3,  0.85, -0.3, 0.5, -0.3,  0.5, 0.3,  -10000 };

double vehiclePoly_Cyclist[] =  { 0.5, 0,  0.25, 0.45,  0.25, 0.5, 0.8, 0.15,     0.8, -0.15, 0.25, -0.5, 0.25, -0.45,     -10000 };

double vehiclePoly_EmergencySign[] =   { .2, .5,  -.2, .5,  -.2, -.5,  .2, -.5, -10000 };
double vehiclePoly_Emergency[] =   { .1, .1,  -.1, .1,  -.1, -.1,  .1, -.1, -10000 };
double vehiclePoly_Emergency2[] =   { .04, .3,  -.04, .3,  -.04, -.3,  .04, -.3, -10000 };

double vehiclePoly_EmergencyLadder[] =   { -.5, .3, .5, .3, .5, .2, -.5, .2, -10000 };
double vehiclePoly_EmergencyLadder2[] =   { -.5, -.3, .5, -.3, .5, -.2, -.5, -.2, -10000 };
double vehiclePoly_EmergencyLadder3[] =   { -.45, .3, -.4, .3, -.4, -.3, -.45, -.3, -10000 };
double vehiclePoly_EmergencyLadder4[] =   { .45, .3, .4, .3, .4, -.3, .45, -.3, -10000 };
double vehiclePoly_EmergencyLadder5[] =   { .05, .3, .0, .3, .0, -.3, .05, -.3, -10000 };
double vehiclePoly_EmergencyLadder6[] =   { -.25, .3, -.2, .3, -.2, -.3, -.25, -.3, -10000 };
double vehiclePoly_EmergencyLadder7[] =   { .25, .3, .2, .3, .2, -.3, .25, -.3, -10000 };

double vehiclePoly_Rickshaw[] = { 0.5, 0,  0.25, 0.45,  0.25, 0.5, 0.8, 0.15,     0.8, -0.15, 0.25, -0.5, 0.25, -0.45,     -10000 };

// ===========================================================================
// method definitions
// ===========================================================================

void
GUIBaseVehicleHelper::drawPoly(const double* poses, const double offset) {
    glPushMatrix();
    glTranslated(0, 0, offset * .1);
    glPolygonOffset(0, (GLfloat) - offset);
    glBegin(GL_TRIANGLE_FAN);
    int i = 0;
    while (poses[i] > -999) {
        glVertex2d(poses[i], poses[i + 1]);
        i = i + 2;
    }
    glEnd();
    glPopMatrix();
}

void
GUIBaseVehicleHelper::drawAction_drawVehicleAsBoxPlus(const double width, const double length) {
    glPushMatrix();
    glScaled(width, length, 1.);
    glBegin(GL_TRIANGLE_STRIP);
    glVertex2d(0., 0.);
    glVertex2d(-.5, .15);
    glVertex2d(.5, .15);
    glVertex2d(-.5, 1.);
    glVertex2d(.5, 1.);
    glEnd();
    glPopMatrix();
}


void
GUIBaseVehicleHelper::drawAction_drawVehicleAsTrianglePlus(const double width, const double length) {
    if (length >= 8.) {
        drawAction_drawVehicleAsBoxPlus(width, length);
        return;
    }
    glPushMatrix();
    glScaled(width, length, 1.);
    glBegin(GL_TRIANGLES);
    glVertex2d(0., 0.);
    glVertex2d(-.5, 1.);
    glVertex2d(.5, 1.);
    glEnd();
    glPopMatrix();
}


void
GUIBaseVehicleHelper::drawAction_drawVehicleAsPoly(const GUIVisualizationSettings& s, const SUMOVehicleShape shape, const double width, const double length,
        int carriageIndex) {
    UNUSED_PARAMETER(s);
    RGBColor current = GLHelper::getColor();
    RGBColor lighter = current.changedBrightness(51);
    RGBColor darker = current.changedBrightness(-51);
    glPushMatrix();
    glRotated(90, 0, 0, 1);
    glScaled(length, width, 1.);

    // draw main body
    switch (shape) {
        case SVS_UNKNOWN:
            drawPoly(vehiclePoly_PassengerCarBody, 4);
            GLHelper::setColor(lighter);
            drawPoly(vehiclePoly_PassengerCarBodyFront, 4.5);
            glColor3d(0, 0, 0);
            drawPoly(vehiclePoly_PassengerFrontGlass, 4.5);
            break;
        case SVS_PEDESTRIAN:
            glTranslated(0, 0, .045);
            GLHelper::drawFilledCircle(0.25);
            glTranslated(0, 0, -.045);
            glScaled(.7, 2, 1);
            glTranslated(0, 0, .04);
            GLHelper::setColor(lighter);
            GLHelper::drawFilledCircle(0.25);
            glTranslated(0, 0, -.04);
            break;
        case SVS_BICYCLE:
        case SVS_MOPED:
        case SVS_MOTORCYCLE: {
            RGBColor darker = current.changedBrightness(-50);
            // body
            drawPoly(vehiclePoly_Cyclist, 4);
            // head
            glPushMatrix();
            glTranslated(0.4, 0, .5);
            glScaled(0.1, 0.2, 1);
            GLHelper::setColor(darker);
            GLHelper::drawFilledCircle(1);
            glPopMatrix();
            // bike frame
            GLHelper::setColor(RGBColor::GREY);
            glPushMatrix();
            glTranslated(0.5, 0, .3);
            glScaled(0.5, 0.05, 1);
            GLHelper::drawFilledCircle(1);
            glPopMatrix();
            // handle bar
            glPushMatrix();
            glTranslated(0.25, 0, .3);
            glScaled(0.02, 0.5, 1);
            GLHelper::drawFilledCircle(1);
            glPopMatrix();
        }
        break;
        case SVS_PASSENGER:
        case SVS_PASSENGER_SEDAN:
        case SVS_PASSENGER_HATCHBACK:
        case SVS_PASSENGER_WAGON:
            drawPoly(vehiclePoly_PassengerCarBody, 4);
            GLHelper::setColor(lighter);
            drawPoly(vehiclePoly_PassengerCarBodyFront, 4.5);
            glColor3d(0, 0, 0);
            drawPoly(vehiclePoly_PassengerFrontGlass, 4.5);
            break;
        case SVS_PASSENGER_VAN:
            drawPoly(vehiclePoly_PassengerVanBody, 4);
            GLHelper::setColor(lighter);
            drawPoly(vehiclePoly_PassengerVanBodyFront, 4.5);
            glColor3d(0, 0, 0);
            drawPoly(vehiclePoly_PassengerVanFrontGlass, 4.5);
            drawPoly(vehiclePoly_PassengerVanRightGlass, 4.5);
            drawPoly(vehiclePoly_PassengerVanLeftGlass, 4.5);
            drawPoly(vehiclePoly_PassengerVanBackGlass, 4.5);
            break;
        case SVS_DELIVERY:
            drawPoly(vehiclePoly_PassengerVanBody, 4);
            GLHelper::setColor(lighter);
            drawPoly(vehiclePoly_PassengerVanBodyFront, 4.5);
            glColor3d(0, 0, 0);
            drawPoly(vehiclePoly_PassengerVanFrontGlass, 4.5);
            drawPoly(vehiclePoly_DeliveryMediumRightGlass, 4.5);
            drawPoly(vehiclePoly_DeliveryMediumLeftGlass, 4.5);
            break;
        case SVS_TRUCK:
        case SVS_TRUCK_SEMITRAILER:
        case SVS_TRUCK_1TRAILER:
            if (carriageIndex < 1) {
                glScaled(1. / (length), 1, 1.);
                drawPoly(vehiclePoly_TransportBody, 4);
                glColor3d(0, 0, 0);
                drawPoly(vehiclePoly_TransportFrontGlass, 4.5);
                drawPoly(vehiclePoly_TransportRightGlass, 4.5);
                drawPoly(vehiclePoly_TransportLeftGlass, 4.5);
            }
            break;
        case SVS_BUS:
        case SVS_BUS_COACH:
        case SVS_BUS_TROLLEY: {
            double ml = length;
            glScaled(1. / (length), 1, 1.);
            glTranslated(0, 0, .04);
            glBegin(GL_TRIANGLE_FAN);
            glVertex2d(ml / 2., 0);
            glVertex2d(0, 0);
            glVertex2d(0, -.45);
            glVertex2d(0 + .05, -.5);
            glVertex2d(ml - .05, -.5);
            glVertex2d(ml, -.45);
            glVertex2d(ml, .45);
            glVertex2d(ml - .05, .5);
            glVertex2d(0 + .05, .5);
            glVertex2d(0, .45);
            glVertex2d(0, 0);
            glEnd();
            glTranslated(0, 0, -.04);

            glTranslated(0, 0, .045);
            glColor3d(0, 0, 0);
            glBegin(GL_QUADS);
            glVertex2d(0 + .05, .48);
            glVertex2d(0 + .05, -.48);
            glVertex2d(0 + .15, -.48);
            glVertex2d(0 + .15, .48);

            glVertex2d(ml - .1, .45);
            glVertex2d(ml - .1, -.45);
            glVertex2d(ml - .05, -.45);
            glVertex2d(ml - .05, .45);

            glVertex2d(0 + .20, .49);
            glVertex2d(0 + .20, .45);
            glVertex2d(ml - .20, .45);
            glVertex2d(ml - .20, .49);

            glVertex2d(0 + .20, -.49);
            glVertex2d(0 + .20, -.45);
            glVertex2d(ml - .20, -.45);
            glVertex2d(ml - .20, -.49);

            glEnd();
            glTranslated(0, 0, -.045);
        }
        break;
        case SVS_BUS_FLEXIBLE:
        case SVS_RAIL:
        case SVS_RAIL_CAR:
        case SVS_RAIL_CARGO:
        case SVS_E_VEHICLE:
            drawPoly(vehiclePoly_EVehicleBody, 4);
            glColor3d(0, 0, 0);
            drawPoly(vehiclePoly_EVehicleFrontGlass, 4.5);
            glTranslated(0, 0, .048);
            GLHelper::setColor(current);
            glBegin(GL_QUADS);
            glVertex2d(.3, .5);
            glVertex2d(.35, .5);
            glVertex2d(.35, -.5);
            glVertex2d(.3, -.5);

            glVertex2d(.3, -.05);
            glVertex2d(.7, -.05);
            glVertex2d(.7, .05);
            glVertex2d(.3, .05);

            glVertex2d(.7, .5);
            glVertex2d(.65, .5);
            glVertex2d(.65, -.5);
            glVertex2d(.7, -.5);
            glEnd();
            glTranslated(0, 0, -.048);
            //drawPoly(vehiclePoly_EVehicleBackGlass, 4.5);
            break;
        case SVS_ANT:
            glPushMatrix();
            // ant is stretched via vehicle length
            GLHelper::setColor(darker);
            // draw left side
            GLHelper::drawBoxLine(Position(-0.2, -.10), 350, 0.5, .02);
            GLHelper::drawBoxLine(Position(-0.3, -.50), 240, 0.4, .03);
            GLHelper::drawBoxLine(Position(0.3, -.10), 340, 0.8, .03);
            GLHelper::drawBoxLine(Position(0.05, -.80), 290, 0.6, .04);
            GLHelper::drawBoxLine(Position(0.4, -.10),  20, 0.8, .03);
            GLHelper::drawBoxLine(Position(0.65, -.80),  75, 0.6, .04);
            GLHelper::drawBoxLine(Position(0.5, -.10),  55, 0.8, .04);
            GLHelper::drawBoxLine(Position(1.1, -.55),  90, 0.6, .04);
            // draw right side
            GLHelper::drawBoxLine(Position(-0.2,  .10), 190, 0.5, .02);
            GLHelper::drawBoxLine(Position(-0.3,  .50), 300, 0.4, .03);
            GLHelper::drawBoxLine(Position(0.3,  .10), 200, 0.8, .03);
            GLHelper::drawBoxLine(Position(0.05, .80), 250, 0.6, .04);
            GLHelper::drawBoxLine(Position(0.4,  .10), 160, 0.8, .03);
            GLHelper::drawBoxLine(Position(0.65, .80), 105, 0.6, .04);
            GLHelper::drawBoxLine(Position(0.5,  .10), 125, 0.8, .04);
            GLHelper::drawBoxLine(Position(1.1,  .55),  90, 0.6, .04);
            // draw body
            GLHelper::setColor(current);
            glTranslated(0, 0, 0.1);
            GLHelper::drawFilledCircle(.25, 16);
            glTranslated(.4, 0, 0);
            GLHelper::drawFilledCircle(.2, 16);
            glTranslated(.4, 0, 0);
            GLHelper::drawFilledCircle(.3, 16);
            glPopMatrix();
            break;
        case SVS_SHIP: {
            RGBColor darker = current.changedBrightness(-30);
            RGBColor darker2 = current.changedBrightness(-70);
            drawPoly(vehiclePoly_Ship, 4);
            GLHelper::setColor(darker);
            drawPoly(vehiclePoly_ShipDeck, 5);
            GLHelper::setColor(darker2);
            drawPoly(vehiclePoly_ShipSuperStructure, 6);
            break;
        }
        case SVS_EMERGENCY: // similar to delivery
            drawPoly(vehiclePoly_PassengerVanBody, 4);
            GLHelper::setColor(darker);
            drawPoly(vehiclePoly_PassengerVanBodyFront, 4.5);
            glColor3d(0, 0, 0);
            drawPoly(vehiclePoly_PassengerVanFrontGlass, 4.5);
            drawPoly(vehiclePoly_DeliveryMediumRightGlass, 4.5);
            drawPoly(vehiclePoly_DeliveryMediumLeftGlass, 4.5);
            // first aid sign
            glTranslated(0.7, 0, 0);
            glColor3d(.18, .55, .34);
            drawPoly(vehiclePoly_EmergencySign, 4.5);
            glColor3d(1, 1, 1);
            drawPoly(vehiclePoly_Emergency, 5);
            drawPoly(vehiclePoly_Emergency2, 5);
            break;
        case SVS_FIREBRIGADE: // similar to delivery in red orange
            drawPoly(vehiclePoly_PassengerVanBody, 4);
            GLHelper::setColor(lighter);
            drawPoly(vehiclePoly_PassengerVanBodyFront, 4.5);
            glColor3d(0, 0, 0);
            drawPoly(vehiclePoly_PassengerVanFrontGlass, 4.5);
            drawPoly(vehiclePoly_DeliveryMediumRightGlass, 4.5);
            drawPoly(vehiclePoly_DeliveryMediumLeftGlass, 4.5);
            // draw ladder
            glTranslated(0.7, 0, 0);
            glColor3d(1, .5, 0);
            drawPoly(vehiclePoly_EmergencySign, 4.5);
            glColor3d(.5, .5, .5);
            drawPoly(vehiclePoly_EmergencyLadder, 5);
            drawPoly(vehiclePoly_EmergencyLadder2, 5);
            drawPoly(vehiclePoly_EmergencyLadder3, 5);
            drawPoly(vehiclePoly_EmergencyLadder4, 5);
            drawPoly(vehiclePoly_EmergencyLadder5, 5);
            drawPoly(vehiclePoly_EmergencyLadder6, 5);
            drawPoly(vehiclePoly_EmergencyLadder7, 5);
            break;
        case SVS_POLICE: // similar to passenger grey with blue
            drawPoly(vehiclePoly_PassengerCarBody, 4);
            GLHelper::setColor(lighter);
            drawPoly(vehiclePoly_PassengerCarBodyFront, 4.5);
            glColor3d(0, 0, 0);
            drawPoly(vehiclePoly_PassengerFrontGlass, 4.5);
            // first aid sign
            glTranslated(0.7, 0, 0);
            glColor3d(.5, .5, .5);
            drawPoly(vehiclePoly_EmergencySign, 4.5);
            glColor3d(0, 0, 1);
            drawPoly(vehiclePoly_Emergency, 5);
            drawPoly(vehiclePoly_Emergency2, 5);
            break;
        case SVS_RICKSHAW: // Rickshaw
            drawPoly(vehiclePoly_PassengerCarBody, 4);
            // wheels
            GLHelper::setColor(darker);
            glPushMatrix();
            glTranslated(.5, .5, -0.1);
            GLHelper::drawFilledCircle(.3, 6);
            glPopMatrix();
            //other wheel
            glPushMatrix();
            glTranslated(.5, -.5, -0.1);
            GLHelper::drawFilledCircle(.3, 6);
            glPopMatrix();
            break;
        default: // same as passenger
            drawPoly(vehiclePoly_PassengerCarBody, 4);
            glColor3d(1, 1, 1);
            drawPoly(vehiclePoly_PassengerCarBodyFront, 4.5);
            glColor3d(0, 0, 0);
            drawPoly(vehiclePoly_PassengerFrontGlass, 4.5);
            break;
    }

    // draw decorations
    switch (shape) {
        case SVS_PEDESTRIAN:
            break;
        case SVS_BICYCLE:
            //glScaled(length, 1, 1.);
            glBegin(GL_TRIANGLE_FAN);
            glVertex2d(1 / 2., 0);
            glVertex2d(0, 0);
            glVertex2d(0, -.03);
            glVertex2d(0 + .05, -.05);
            glVertex2d(1 - .05, -.05);
            glVertex2d(1, -.03);
            glVertex2d(1, .03);
            glVertex2d(1 - .05, .05);
            glVertex2d(0 + .05, .05);
            glVertex2d(0, .03);
            glVertex2d(0, 0);
            glEnd();
            break;
        case SVS_MOPED:
        case SVS_MOTORCYCLE:
            //glScaled(length, 1, 1.);
            glBegin(GL_TRIANGLE_FAN);
            glVertex2d(1 / 2., 0);
            glVertex2d(0, 0);
            glVertex2d(0, -.03);
            glVertex2d(0 + .05, -.2);
            glVertex2d(1 - .05, -.2);
            glVertex2d(1, -.03);
            glVertex2d(1, .03);
            glVertex2d(1 - .05, .2);
            glVertex2d(0 + .05, .2);
            glVertex2d(0, .03);
            glVertex2d(0, 0);
            glEnd();
            break;
        case SVS_PASSENGER:
        case SVS_PASSENGER_SEDAN:
            drawPoly(vehiclePoly_PassengerSedanRightGlass, 4.5);
            drawPoly(vehiclePoly_PassengerSedanLeftGlass, 4.5);
            drawPoly(vehiclePoly_PassengerSedanBackGlass, 4.5);
            break;
        case SVS_PASSENGER_HATCHBACK:
            drawPoly(vehiclePoly_PassengerHatchbackRightGlass, 4.5);
            drawPoly(vehiclePoly_PassengerHatchbackLeftGlass, 4.5);
            drawPoly(vehiclePoly_PassengerHatchbackBackGlass, 4.5);
            break;
        case SVS_PASSENGER_WAGON:
            drawPoly(vehiclePoly_PassengerWagonRightGlass, 4.5);
            drawPoly(vehiclePoly_PassengerWagonLeftGlass, 4.5);
            drawPoly(vehiclePoly_PassengerWagonBackGlass, 4.5);
            break;
        case SVS_PASSENGER_VAN:
        case SVS_DELIVERY:
            break;
        case SVS_TRUCK:
            GLHelper::setColor(current);
            GLHelper::drawBoxLine(Position(2.3, 0), 90., length - 2.3, .5);
            break;
        case SVS_TRUCK_SEMITRAILER:
            if (carriageIndex < 0) {
                GLHelper::setColor(current);
                GLHelper::drawBoxLine(Position(2.8, 0), 90., length - 2.8, .5);
            }
            break;
        case SVS_TRUCK_1TRAILER: {
            GLHelper::setColor(current);
            double l = length - 2.3;
            if (carriageIndex != 0) {
                l = l / 2.;
                GLHelper::drawBoxLine(Position(2.3, 0), 90., l, .5);
                GLHelper::drawBoxLine(Position(2.3 + l + .5, 0), 90., l - .5, .5);
            } else {
                GLHelper::drawBoxLine(Position(2.3, 0), 90., l, .5);
            }
            break;
        }
        case SVS_BUS_TROLLEY:
            glPushMatrix();
            glTranslated(0, 0, .1);
            GLHelper::setColor(darker);
            GLHelper::drawBoxLine(Position(3.8, 0), 90., 1, .3);
            glTranslated(0, 0, .1);
            glColor3d(0, 0, 0);
            GLHelper::drawBoxLine(Position(4.3, .2), 90., 1, .06);
            GLHelper::drawBoxLine(Position(4.3, -.2), 90., 1, .06);
            GLHelper::drawBoxLine(Position(5.3, .2), 90., 3, .03);
            GLHelper::drawBoxLine(Position(5.3, -.2), 90., 3, .03);
            glPopMatrix();
            break;
        case SVS_BUS:
        case SVS_BUS_COACH:
        case SVS_BUS_FLEXIBLE:
        case SVS_RAIL:
        case SVS_RAIL_CAR:
        case SVS_RAIL_CARGO:
        case SVS_E_VEHICLE:
        case SVS_ANT:
        case SVS_SHIP:
        case SVS_EMERGENCY:
        case SVS_FIREBRIGADE:
        case SVS_POLICE:
        case SVS_RICKSHAW:
            break;
        default: // same as passenger/sedan
            drawPoly(vehiclePoly_PassengerSedanRightGlass, 4.5);
            drawPoly(vehiclePoly_PassengerSedanLeftGlass, 4.5);
            drawPoly(vehiclePoly_PassengerSedanBackGlass, 4.5);
            break;
    }
    /*
    glBegin(GL_TRIANGLE_FAN);
    glVertex2d(.5,.5); // center - strip begin
    glVertex2d(0,    .5); // center, front
    glVertex2d(0,    .8); // ... clockwise ... (vehicle right side)
    glVertex2d(0.08, .94);
    glVertex2d(0.25, 1.);
    glVertex2d(0.95, 1.);
    glVertex2d(1.,   .9);
    glVertex2d(1.,   .1); // (vehicle left side)
    glVertex2d(0.95, 0.);
    glVertex2d(0.25, 0.);
    glVertex2d(0.08, .06);
    glVertex2d(0,    .2); //
    glVertex2d(0,    .5); // center, front (close)
    glEnd();

    glPolygonOffset(0, -4.5);
    glColor3d(1, 1, 1); // front
    glBegin(GL_TRIANGLE_FAN);
    glVertex2d(0.1,0.5);
    glVertex2d(0.025,0.5);
    glVertex2d(0.025,0.75);
    glVertex2d(0.27,0.9);
    glVertex2d(0.27,0.1);
    glVertex2d(0.025,0.25);
    glVertex2d(0.025,0.5);
    glEnd();

    glColor3d(0, 0, 0); // front glass
    glBegin(GL_TRIANGLE_FAN);
    glVertex2d(0.35,0.5);
    glVertex2d(0.3,0.5);
    glVertex2d(0.3,0.9);
    glVertex2d(0.43,0.8);
    glVertex2d(0.43,0.2);
    glVertex2d(0.3,0.1);
    glVertex2d(0.3,0.5);
    glEnd();

    glBegin(GL_TRIANGLE_FAN); // back glass
    glVertex2d(0.92,0.5);
    glVertex2d(0.90,0.5);
    glVertex2d(0.90,0.8);
    glVertex2d(0.95,0.9);
    glVertex2d(0.95,0.1);
    glVertex2d(0.90,0.2);
    glVertex2d(0.90,0.5);
    glEnd();

    glBegin(GL_TRIANGLE_FAN); // right glass
    glVertex2d(0.36,0.07);
    glVertex2d(0.34,0.03);
    glVertex2d(0.94,0.03);
    glVertex2d(0.87,0.13);
    glVertex2d(0.45,0.13);
    glVertex2d(0.34,0.03);
    glEnd();

    glBegin(GL_TRIANGLE_FAN); // left glass
    glVertex2d(0.36,1.-0.07);
    glVertex2d(0.34,1.-0.03);
    glVertex2d(0.94,1.-0.03);
    glVertex2d(0.87,1.-0.13);
    glVertex2d(0.45,1.-0.13);
    glVertex2d(0.34,1.-0.03);
    glEnd();
    */

    glPopMatrix();
}


bool
GUIBaseVehicleHelper::drawAction_drawVehicleAsImage(const GUIVisualizationSettings& s, const std::string& file, const GUIGlObject* o, const double width, double length) {
    if (file != "") {
        int textureID = GUITexturesHelper::getTextureID(file);
        if (textureID > 0) {
            const double exaggeration = s.vehicleSize.getExaggeration(s, o);
            const double halfWidth = width / 2.0 * exaggeration;
            GUITexturesHelper::drawTexturedBox(textureID, -halfWidth, 0, halfWidth, length * exaggeration);
            return true;
        }
    }
    return false;
}


/****************************************************************************/
