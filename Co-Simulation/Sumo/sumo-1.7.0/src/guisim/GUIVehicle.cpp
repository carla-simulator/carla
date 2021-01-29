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
/// @file    GUIVehicle.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// A MSVehicle extended by some values for usage within the gui
/****************************************************************************/
#include <config.h>

#include <cmath>
#include <vector>
#include <string>
#include <utils/common/StringUtils.h>
#include <utils/vehicle/SUMOVehicleParameter.h>
#include <utils/emissions/PollutantsInterface.h>
#include <utils/geom/GeomHelper.h>
#include <utils/gui/globjects/GLIncludes.h>
#include <utils/gui/windows/GUISUMOAbstractView.h>
#include <utils/gui/windows/GUIAppEnum.h>
#include <utils/gui/images/GUITexturesHelper.h>
#include <utils/gui/div/GUIParameterTableWindow.h>
#include <utils/gui/div/GUIGlobalSelection.h>
#include <utils/gui/div/GLHelper.h>
#include <utils/gui/div/GLObjectValuePassConnector.h>
#include <utils/gui/div/GUIGlobalSelection.h>
#include <utils/gui/div/GUIBaseVehicleHelper.h>
#include <microsim/MSGlobals.h>
#include <microsim/MSVehicle.h>
#include <microsim/MSJunction.h>
#include <microsim/MSLane.h>
#include <microsim/logging/CastingFunctionBinding.h>
#include <microsim/logging/FunctionBinding.h>
#include <microsim/lcmodels/MSAbstractLaneChangeModel.h>
#include <microsim/devices/MSDevice_Vehroutes.h>
#include <microsim/devices/MSDevice_Routing.h>
#include <microsim/devices/MSRoutingEngine.h>
#include <microsim/devices/MSDevice_Transportable.h>
#include <microsim/devices/MSDevice_BTreceiver.h>
#include <microsim/devices/MSDevice_ElecHybrid.h>
#include <gui/GUIApplicationWindow.h>
#include <gui/GUIGlobals.h>

#include "GUIVehicle.h"
#include "GUIPerson.h"
#include "GUIContainer.h"
#include "GUINet.h"
#include "GUIEdge.h"
#include "GUILane.h"

//#define DEBUG_FOES

// ===========================================================================
// FOX callback mapping
// ===========================================================================

// Object implementation


/* -------------------------------------------------------------------------
 * GUIVehicle - methods
 * ----------------------------------------------------------------------- */
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4355)
#endif
GUIVehicle::GUIVehicle(SUMOVehicleParameter* pars, const MSRoute* route,
                       MSVehicleType* type, const double speedFactor) :
    MSVehicle(pars, route, type, speedFactor),
    GUIBaseVehicle((MSBaseVehicle&) * this) {
}
#ifdef _MSC_VER
#pragma warning(pop)
#endif


GUIVehicle::~GUIVehicle() {
}


GUIParameterTableWindow*
GUIVehicle::getParameterWindow(GUIMainWindow& app,
                               GUISUMOAbstractView&) {
    const bool isElecHybrid = getDevice(typeid(MSDevice_ElecHybrid)) != nullptr ? true : false;
    GUIParameterTableWindow* ret = new GUIParameterTableWindow(app, *this);
    // add items
    ret->mkItem("lane [id]", true, new FunctionBindingString<GUIVehicle>(this, &GUIVehicle::getLaneID));
    if (MSAbstractLaneChangeModel::haveLateralDynamics()) {
        ret->mkItem("shadow lane [id]", true, new FunctionBindingString<GUIVehicle>(this, &GUIVehicle::getShadowLaneID));
    }
    if (MSGlobals::gLateralResolution > 0) {
        ret->mkItem("target lane [id]", true, new FunctionBindingString<GUIVehicle>(this, &GUIVehicle::getTargetLaneID));
    }
    ret->mkItem("position [m]", true,
                new FunctionBinding<GUIVehicle, double>(this, &MSVehicle::getPositionOnLane));
    ret->mkItem("lateral offset [m]", true,
                new FunctionBinding<GUIVehicle, double>(this, &GUIVehicle::getLateralPositionOnLane));
    ret->mkItem("speed [m/s]", true,
                new FunctionBinding<GUIVehicle, double>(this, &MSVehicle::getSpeed));
    ret->mkItem("lateral speed [m/s]", true,
                new FunctionBinding<MSAbstractLaneChangeModel, double>(&getLaneChangeModel(), &MSAbstractLaneChangeModel::getSpeedLat));
    ret->mkItem("acceleration [m/s^2]", true,
                new FunctionBinding<GUIVehicle, double>(this, &MSVehicle::getAcceleration));
    ret->mkItem("angle [degree]", true,
                new FunctionBinding<GUIVehicle, double>(this, &GUIBaseVehicle::getNaviDegree));
    ret->mkItem("slope [degree]", true,
                new FunctionBinding<GUIVehicle, double>(this, &MSVehicle::getSlope));
    ret->mkItem("speed factor", false, getChosenSpeedFactor());
    ret->mkItem("time gap on lane [s]", true,
                new FunctionBinding<GUIVehicle, double>(this, &MSVehicle::getTimeGapOnLane));
    ret->mkItem("waiting time [s]", true,
                new FunctionBinding<GUIVehicle, double>(this, &MSVehicle::getWaitingSeconds));
    ret->mkItem(("waiting time (accumulated, " + time2string(MSGlobals::gWaitingTimeMemory) + "s) [s]").c_str(), true,
                new FunctionBinding<GUIVehicle, double>(this, &MSVehicle::getAccumulatedWaitingSeconds));
    ret->mkItem("time loss [s]", true,
                new FunctionBinding<GUIVehicle, double>(this, &MSVehicle::getTimeLossSeconds));
    ret->mkItem("impatience", true,
                new FunctionBinding<GUIVehicle, double>(this, &MSVehicle::getImpatience));
    ret->mkItem("last lane change [s]", true,
                new FunctionBinding<GUIVehicle, double>(this, &GUIVehicle::getLastLaneChangeOffset));
    ret->mkItem("desired depart [s]", false, time2string(getParameter().depart));
    ret->mkItem("depart delay [s]", false, time2string(getDepartDelay()));
    ret->mkItem("odometer [m]", true,
                new FunctionBinding<GUIVehicle, double>(this, &MSBaseVehicle::getOdometer));
    if (getParameter().repetitionNumber < std::numeric_limits<int>::max()) {
        ret->mkItem("remaining [#]", false, (int) getParameter().repetitionNumber - getParameter().repetitionsDone);
    }
    if (getParameter().repetitionOffset > 0) {
        ret->mkItem("insertion period [s]", false, time2string(getParameter().repetitionOffset));
    }
    if (getParameter().repetitionProbability > 0) {
        ret->mkItem("insertion probability", false, getParameter().repetitionProbability);
    }
    ret->mkItem("stop info", true, new FunctionBindingString<GUIVehicle>(this, &GUIVehicle::getStopInfo));
    ret->mkItem("line", false, myParameter->line);
    ret->mkItem("CO2 [mg/s]", true,
                new FunctionBinding<GUIVehicle, double>(this, &MSVehicle::getCO2Emissions));
    ret->mkItem("CO [mg/s]", true,
                new FunctionBinding<GUIVehicle, double>(this, &MSVehicle::getCOEmissions));
    ret->mkItem("HC [mg/s]", true,
                new FunctionBinding<GUIVehicle, double>(this, &MSVehicle::getHCEmissions));
    ret->mkItem("NOx [mg/s]", true,
                new FunctionBinding<GUIVehicle, double>(this, &MSVehicle::getNOxEmissions));
    ret->mkItem("PMx [mg/s]", true,
                new FunctionBinding<GUIVehicle, double>(this, &MSVehicle::getPMxEmissions));
    ret->mkItem("fuel [ml/s]", true,
                new FunctionBinding<GUIVehicle, double>(this, &MSVehicle::getFuelConsumption));
    ret->mkItem("electricity [Wh/s]", true,
                new FunctionBinding<GUIVehicle, double>(this, &MSVehicle::getElectricityConsumption));
    ret->mkItem("noise (Harmonoise) [dB]", true,
                new FunctionBinding<GUIVehicle, double>(this, &MSVehicle::getHarmonoise_NoiseEmissions));
    ret->mkItem("devices", false, getDeviceDescription());
    ret->mkItem("persons", true,
                new FunctionBinding<GUIVehicle, int>(this, &MSVehicle::getPersonNumber));
    ret->mkItem("containers", true,
                new FunctionBinding<GUIVehicle, int>(this, &MSVehicle::getContainerNumber));
    ret->mkItem("lcState right", true, new FunctionBindingString<GUIVehicle>(this, &GUIVehicle::getLCStateRight));
    ret->mkItem("lcState left", true, new FunctionBindingString<GUIVehicle>(this, &GUIVehicle::getLCStateLeft));
    // close building
    if (MSGlobals::gLateralResolution > 0) {
        ret->mkItem("lcState center", true, new FunctionBindingString<GUIVehicle>(this, &GUIVehicle::getLCStateCenter));
        ret->mkItem("right side on edge [m]", true, new FunctionBinding<GUIVehicle, double>(this, &GUIVehicle::getRightSideOnEdge2));
        ret->mkItem("left side on edge [m]", true, new FunctionBinding<GUIVehicle, double>(this, &GUIVehicle::getLeftSideOnEdge));
        ret->mkItem("rightmost edge sublane [#]", true, new FunctionBinding<GUIVehicle, int>(this, &GUIVehicle::getRightSublaneOnEdge));
        ret->mkItem("leftmost edge sublane [#]", true, new FunctionBinding<GUIVehicle, int>(this, &GUIVehicle::getLeftSublaneOnEdge));
        ret->mkItem("lane change maneuver distance [m]", true, new FunctionBinding<GUIVehicle, double>(this, &GUIVehicle::getManeuverDist));
    }
    if (isElecHybrid) {
        ret->mkItem("actual state of charge [Wh]", true,
                    new FunctionBinding<GUIVehicle, double>(this, &MSVehicle::getStateOfCharge));
        ret->mkItem("actual electric current [A]", true,
                    new FunctionBinding<GUIVehicle, double>(this, &MSVehicle::getElecHybridCurrent));
    }
    ret->closeBuilding(&getParameter());
    return ret;
}


GUIParameterTableWindow*
GUIVehicle::getTypeParameterWindow(GUIMainWindow& app,
                                   GUISUMOAbstractView&) {
    GUIParameterTableWindow* ret = new GUIParameterTableWindow(app, *this);
    // add items
    ret->mkItem("Type Information:", false, "");
    ret->mkItem("type [id]", false, myType->getID());
    ret->mkItem("length", false, myType->getLength());
    ret->mkItem("width", false, myType->getWidth());
    ret->mkItem("height", false, myType->getHeight());
    ret->mkItem("minGap", false, myType->getMinGap());
    ret->mkItem("vehicle class", false, SumoVehicleClassStrings.getString(myType->getVehicleClass()));
    ret->mkItem("emission class", false, PollutantsInterface::getName(myType->getEmissionClass()));
    ret->mkItem("carFollowModel", false, SUMOXMLDefinitions::CarFollowModels.getString((SumoXMLTag)getCarFollowModel().getModelID()));
    ret->mkItem("LaneChangeModel", false, SUMOXMLDefinitions::LaneChangeModels.getString(getLaneChangeModel().getModelID()));
    ret->mkItem("guiShape", false, getVehicleShapeName(myType->getGuiShape()));
    ret->mkItem("maximum speed [m/s]", false, getMaxSpeed());
    ret->mkItem("maximum acceleration [m/s^2]", false, getCarFollowModel().getMaxAccel());
    ret->mkItem("maximum deceleration [m/s^2]", false, getCarFollowModel().getMaxDecel());
    ret->mkItem("emergency deceleration [m/s^2]", false, getCarFollowModel().getEmergencyDecel());
    ret->mkItem("apparent deceleration [m/s^2]", false, getCarFollowModel().getApparentDecel());
    ret->mkItem("imperfection (sigma)", false, getCarFollowModel().getImperfection());
    ret->mkItem("desired headway (tau)", false, getCarFollowModel().getHeadwayTime());
    if (myType->getParameter().wasSet(VTYPEPARS_ACTIONSTEPLENGTH_SET)) {
        ret->mkItem("action step length [s]", false, myType->getActionStepLengthSecs());
    }
    ret->mkItem("person capacity", false, myType->getPersonCapacity());
    ret->mkItem("boarding time", false, STEPS2TIME(myType->getBoardingDuration()));
    ret->mkItem("container capacity", false, myType->getContainerCapacity());
    ret->mkItem("loading time", false, STEPS2TIME(myType->getLoadingDuration()));
    if (MSGlobals::gLateralResolution > 0) {
        ret->mkItem("minGapLat", false, myType->getMinGapLat());
        ret->mkItem("maxSpeedLat", false, myType->getMaxSpeedLat());
        ret->mkItem("latAlignment", false, toString(myType->getPreferredLateralAlignment()));
    } else if (MSGlobals::gLaneChangeDuration > 0) {
        ret->mkItem("maxSpeedLat", false, myType->getMaxSpeedLat());
    }
    for (auto item : myType->getParameter().lcParameter) {
        ret->mkItem(toString(item.first).c_str(), false, toString(item.second));
    }
    for (auto item : myType->getParameter().jmParameter) {
        ret->mkItem(toString(item.first).c_str(), false, toString(item.second));
    }
    if (MSGlobals::gModelParkingManoeuver) {
        ret->mkItem("manoeuver Angle vs Times", false, myType->getParameter().getManoeuverAngleTimesS());
    }

    // close building
    ret->closeBuilding(&(myType->getParameter()));
    return ret;
}



void
GUIVehicle::drawAction_drawLinkItems(const GUIVisualizationSettings& s) const {
    glTranslated(0, 0, getType() + .2); // draw on top of cars
    for (DriveItemVector::const_iterator i = myLFLinkLanes.begin(); i != myLFLinkLanes.end(); ++i) {
        if ((*i).myLink == nullptr) {
            continue;
        }
        MSLink* link = (*i).myLink;
        MSLane* via = link->getViaLaneOrLane();
        if (via != nullptr) {
            Position p = via->getShape()[0];
            if ((*i).mySetRequest) {
                glColor3d(0, .8, 0);
            } else {
                glColor3d(.8, 0, 0);
            }
            const SUMOTime leaveTime = (*i).myLink->getLeaveTime(
                                           (*i).myArrivalTime, (*i).myArrivalSpeed, (*i).getLeaveSpeed(), getVehicleType().getLength());
            drawLinkItem(p, (*i).myArrivalTime, leaveTime, s.vehicleName.size / s.scale);
            // the time slot that ego vehicle uses when checking opened may
            // differ from the one it requests in setApproaching
            MSLink::ApproachingVehicleInformation avi = (*i).myLink->getApproaching(this);
            assert(avi.arrivalTime == (*i).myArrivalTime && avi.leavingTime == leaveTime);
            UNUSED_PARAMETER(avi); // only used for assertion
        }
    }
    glTranslated(0, 0, getType() - .2); // draw on top of cars
}


void
GUIVehicle::drawAction_drawCarriageClass(const GUIVisualizationSettings& s, bool asImage) const {
    RGBColor current = GLHelper::getColor();
    RGBColor darker = current.changedBrightness(-51);
    const double exaggeration = s.vehicleSize.getExaggeration(s, this);
    const double totalLength = getVType().getLength();
    double upscaleLength = exaggeration;
    if (exaggeration > 1 && totalLength > 5) {
        // reduce the length/width ratio because this is not usefull at high zoom
        const double widthLengthFactor = totalLength / getVType().getWidth();
        const double shrinkFactor = MIN2(widthLengthFactor, sqrt(upscaleLength));
        upscaleLength /= shrinkFactor;
    }
    const double locomotiveLength = getVehicleType().getParameter().locomotiveLength * upscaleLength;
    if (exaggeration == 0) {
        return;
    }
    const double defaultLength = getVehicleType().getParameter().carriageLength * upscaleLength;
    const double carriageGap = getVehicleType().getParameter().carriageGap * upscaleLength;
    const double length = totalLength * upscaleLength;
    const double halfWidth = getVehicleType().getWidth() / 2.0 * exaggeration;
    glPopMatrix(); // undo initial translation and rotation
    const double xCornerCut = 0.3 * exaggeration;
    const double yCornerCut = 0.4 * exaggeration;
    // round to closest integer
    const int numCarriages = MAX2(1, 1 + (int)((length - locomotiveLength) / (defaultLength + carriageGap) + 0.5));
    assert(numCarriages > 0);
    double carriageLengthWithGap = length / numCarriages;
    double carriageLength = carriageLengthWithGap - carriageGap;
    double firstCarriageLength = carriageLength;
    if (defaultLength != locomotiveLength && numCarriages > 1) {
        firstCarriageLength = locomotiveLength;
        carriageLengthWithGap = (length - locomotiveLength) / (numCarriages - 1);
        carriageLength = carriageLengthWithGap - carriageGap;
    }
    const int firstPassengerCarriage = defaultLength == locomotiveLength || numCarriages == 1 || (getVClass() & SVC_RAIL_CLASSES) == 0 ? 0 : 1;
    const int firstContainerCarriage = numCarriages == 1 || getVehicleType().getGuiShape() == SVS_TRUCK_1TRAILER ? 0 : 1;
    const int seatsPerCarriage = (int)ceil(getVType().getPersonCapacity() / (numCarriages - firstPassengerCarriage));
    const int containersPerCarriage = (int)ceil(getVType().getContainerCapacity() / (numCarriages - firstContainerCarriage));
    // lane on which the carriage front is situated
    MSLane* lane = myLane;
    int furtherIndex = 0;
    // lane on which the carriage back is situated
    MSLane* backLane = myLane;
    int backFurtherIndex = furtherIndex;
    // offsets of front and back
    double carriageOffset = myState.pos();
    double carriageBackOffset = myState.pos() - firstCarriageLength;
    // handle seats
    int requiredSeats = getNumPassengers();
    int requiredPositions = getNumContainers();
    if (requiredSeats > 0) {
        mySeatPositions.clear();
    }
    if (requiredPositions > 0) {
        myContainerPositions.clear();
    }
    Position front, back;
    double angle = 0.;
    // draw individual carriages
    double curCLength = firstCarriageLength;
    //std::cout << SIMTIME << " veh=" << getID() << " curCLength=" << curCLength << " loc=" << locomotiveLength << " car=" << carriageLength << " tlen=" << totalLength << " len=" << length << "\n";
    for (int i = 0; i < numCarriages; ++i) {
        if (i > 0) {
            curCLength = carriageLength;
        }
        while (carriageOffset < 0) {
            MSLane* prev = getPreviousLane(lane, furtherIndex);
            if (prev != lane) {
                carriageOffset += prev->getLength();
            } else {
                // no lane available for drawing.
                carriageOffset = 0;
            }
            lane = prev;
        }
        while (carriageBackOffset < 0) {
            MSLane* prev = getPreviousLane(backLane, backFurtherIndex);
            if (prev != backLane) {
                carriageBackOffset += prev->getLength();
            } else {
                // no lane available for drawing.
                carriageBackOffset = 0;
            }
            backLane = prev;
        }
        front = lane->geometryPositionAtOffset(carriageOffset);
        back = backLane->geometryPositionAtOffset(carriageBackOffset);
        if (front == back) {
            // no place for drawing available
            continue;
        }
        const double drawnCarriageLength = front.distanceTo2D(back);
        angle = atan2((front.x() - back.x()), (back.y() - front.y())) * (double) 180.0 / (double) M_PI;
        if (i >= firstPassengerCarriage) {
            computeSeats(front, back, SUMO_const_waitingPersonWidth, seatsPerCarriage, exaggeration, requiredSeats, mySeatPositions);
        }
        if (i >= firstContainerCarriage) {
            computeSeats(front, back, SUMO_const_waitingContainerWidth, containersPerCarriage, exaggeration, requiredPositions, myContainerPositions);
        }
        glPushMatrix();
        glTranslated(front.x(), front.y(), getType());
        glRotated(angle, 0, 0, 1);
        if (!asImage || !GUIBaseVehicleHelper::drawAction_drawVehicleAsImage(s, getVType().getImgFile(), this, getVType().getWidth(), curCLength / exaggeration)) {
            switch (getVType().getGuiShape()) {
                case SVS_TRUCK_SEMITRAILER:
                case SVS_TRUCK_1TRAILER:
                    if (i == 0) {
                        GUIBaseVehicleHelper::drawAction_drawVehicleAsPoly(s, getVType().getGuiShape(), getVType().getWidth() * exaggeration, curCLength, i);
                    } else {
                        GLHelper::setColor(current);
                        GLHelper::drawBoxLine(Position(0, 0), 180, curCLength, halfWidth);
                    }
                    break;
                default: {
                    if (i == 0) {
                        GLHelper::setColor(darker);
                    } else {
                        GLHelper::setColor(current);
                    }
                    // generic rail carriage
                    glBegin(GL_TRIANGLE_FAN);
                    glVertex2d(-halfWidth + xCornerCut, 0);
                    glVertex2d(-halfWidth, yCornerCut);
                    glVertex2d(-halfWidth, drawnCarriageLength - yCornerCut);
                    glVertex2d(-halfWidth + xCornerCut, drawnCarriageLength);
                    glVertex2d(halfWidth - xCornerCut, drawnCarriageLength);
                    glVertex2d(halfWidth, drawnCarriageLength - yCornerCut);
                    glVertex2d(halfWidth, yCornerCut);
                    glVertex2d(halfWidth - xCornerCut, 0);
                    glEnd();
                }
            }
        }
        glPopMatrix();
        carriageOffset -= (curCLength + carriageGap);
        carriageBackOffset -= carriageLengthWithGap;
    }
    if (getVType().getGuiShape() == SVS_RAIL_CAR) {
        glPushMatrix();
        glTranslated(front.x(), front.y(), getType());
        glRotated(angle, 0, 0, 1);
        drawAction_drawVehicleBlinker(curCLength);
        drawAction_drawVehicleBrakeLight(curCLength);
        glPopMatrix();
    }
    // restore matrix
    glPushMatrix();
    front = getPosition();
    glTranslated(front.x(), front.y(), getType());
    const double degAngle = RAD2DEG(getAngle() + M_PI / 2.);
    glRotated(degAngle, 0, 0, 1);
    glScaled(exaggeration, upscaleLength, 1);
    if (mySeatPositions.size() == 0) {
        mySeatPositions.push_back(Seat(back, DEG2RAD(angle)));
    }
    if (myContainerPositions.size() == 0) {
        myContainerPositions.push_back(Seat(back, DEG2RAD(angle)));
    }
}

#define BLINKER_POS_FRONT .5
#define BLINKER_POS_BACK .5

inline void
drawAction_drawBlinker(double dir, double length) {
    glColor3d(1.f, .8f, 0);
    glPushMatrix();
    glTranslated(dir, BLINKER_POS_FRONT, -0.1);
    GLHelper::drawFilledCircle(.5, 6);
    glPopMatrix();
    glPushMatrix();
    glTranslated(dir, length - BLINKER_POS_BACK, -0.1);
    GLHelper::drawFilledCircle(.5, 6);
    glPopMatrix();
}


void
GUIVehicle::drawAction_drawVehicleBlinker(double length) const {
    if (!signalSet(MSVehicle::VEH_SIGNAL_BLINKER_RIGHT | MSVehicle::VEH_SIGNAL_BLINKER_LEFT | MSVehicle::VEH_SIGNAL_BLINKER_EMERGENCY)) {
        return;
    }
    const double offset = MAX2(.5 * getVehicleType().getWidth(), .4);
    if (signalSet(MSVehicle::VEH_SIGNAL_BLINKER_RIGHT)) {
        drawAction_drawBlinker(-offset, length);
    }
    if (signalSet(MSVehicle::VEH_SIGNAL_BLINKER_LEFT)) {
        drawAction_drawBlinker(offset, length);
    }
    if (signalSet(MSVehicle::VEH_SIGNAL_BLINKER_EMERGENCY)) {
        drawAction_drawBlinker(-offset, length);
        drawAction_drawBlinker(offset, length);
    }
}


inline void
GUIVehicle::drawAction_drawVehicleBrakeLight(double length, bool onlyOne) const {
    if (!signalSet(MSVehicle::VEH_SIGNAL_BRAKELIGHT)) {
        return;
    }
    glColor3f(1.f, .2f, 0);
    glPushMatrix();
    if (onlyOne) {
        glTranslated(0, length, -0.1);
        GLHelper::drawFilledCircle(.5, 6);
    } else {
        glTranslated(-getVehicleType().getWidth() * 0.5, length, -0.1);
        GLHelper::drawFilledCircle(.5, 6);
        glPopMatrix();
        glPushMatrix();
        glTranslated(getVehicleType().getWidth() * 0.5, length, -0.1);
        GLHelper::drawFilledCircle(.5, 6);
    }
    glPopMatrix();
}

inline void
GUIVehicle::drawAction_drawVehicleBlueLight() const {
    if (signalSet(MSVehicle::VEH_SIGNAL_EMERGENCY_BLUE)) {
        glPushMatrix();
        glTranslated(0, 2.5, .5);
        glColor3f(0, 0, 1);
        GLHelper::drawFilledCircle(.5, 6);
        glPopMatrix();
    }
}


double
GUIVehicle::getColorValue(const GUIVisualizationSettings& s, int activeScheme) const {
    switch (activeScheme) {
        case 8:
            return getSpeed();
        case 9:
            // color by action step
            if (isActionStep(SIMSTEP)) {
                // Upcoming simstep is actionstep (t was already increased before drawing)
                return 1.;
            } else if (isActive()) {
                // Completed simstep was actionstep
                return 2.;
            } else {
                // not active
                return 0.;
            }
        case 10:
            return getWaitingSeconds();
        case 11:
            return getAccumulatedWaitingSeconds();
        case 12:
            return getLastLaneChangeOffset();
        case 13:
            return getLane()->getVehicleMaxSpeed(this);
        case 14:
            return getCO2Emissions();
        case 15:
            return getCOEmissions();
        case 16:
            return getPMxEmissions();
        case 17:
            return getNOxEmissions();
        case 18:
            return getHCEmissions();
        case 19:
            return getFuelConsumption();
        case 20:
            return getHarmonoise_NoiseEmissions();
        case 21:
            return getNumberReroutes();
        case 22:
            return gSelected.isSelected(GLO_VEHICLE, getGlID());
        case 23:
            return getLaneChangeModel().isOpposite() ? -100 : getBestLaneOffset();
        case 24:
            return getAcceleration();
        case 25:
            return getTimeGapOnLane();
        case 26:
            return STEPS2TIME(getDepartDelay());
        case 27:
            return getElectricityConsumption();
        case 28:
            return getTimeLossSeconds();
        case 29:
            return getStopDelay();
        case 30:
            return getLaneChangeModel().getSpeedLat();
        case 31: // by numerical param value
            std::string error;
            std::string val = getPrefixedParameter(s.vehicleParam, error);
            try {
                if (val == "") {
                    return 0;
                } else {
                    return StringUtils::toDouble(val);
                }
            } catch (NumberFormatException&) {
                try {
                    return StringUtils::toBool(val);
                } catch (BoolFormatException&) {
                    WRITE_WARNING("Vehicle parameter '" + myParameter->getParameter(s.vehicleParam, "0") + "' key '" + s.vehicleParam + "' is not a number for vehicle '" + getID() + "'");
                    return -1;
                }
            }
    }
    return 0;
}


void
GUIVehicle::drawBestLanes() const {
    myLock.lock();
    std::vector<std::vector<MSVehicle::LaneQ> > bestLanes = myBestLanes;
    myLock.unlock();
    for (std::vector<std::vector<MSVehicle::LaneQ> >::iterator j = bestLanes.begin(); j != bestLanes.end(); ++j) {
        std::vector<MSVehicle::LaneQ>& lanes = *j;
        double gmax = -1;
        double rmax = -1;
        for (std::vector<MSVehicle::LaneQ>::const_iterator i = lanes.begin(); i != lanes.end(); ++i) {
            gmax = MAX2((*i).length, gmax);
            rmax = MAX2((*i).occupation, rmax);
        }
        for (std::vector<MSVehicle::LaneQ>::const_iterator i = lanes.begin(); i != lanes.end(); ++i) {
            const PositionVector& shape = (*i).lane->getShape();
            double g = (*i).length / gmax;
            double r = (*i).occupation / rmax;
            glColor3d(r, g, 0);
            double width = 0.5 / (1 + abs((*i).bestLaneOffset));
            GLHelper::drawBoxLines(shape, width);

            PositionVector s1 = shape;
            s1.move2side((double) .1);
            glColor3d(r, 0, 0);
            GLHelper::drawLine(s1);
            s1.move2side((double) - .2);
            glColor3d(0, g, 0);
            GLHelper::drawLine(s1);

            glColor3d(r, g, 0);
        }
    }
}


void
GUIVehicle::drawRouteHelper(const GUIVisualizationSettings& s, const MSRoute& r, bool future, bool noLoop, const RGBColor& col) const {
    const double exaggeration = s.vehicleSize.getExaggeration(s, this) * (s.gaming ? 0.5 : 1);
    MSRouteIterator start = future ? myCurrEdge : r.begin();
    MSRouteIterator i = start;
    const std::vector<MSLane*>& bestLaneConts = getBestLanesContinuation();
    // draw continuation lanes when drawing the current route where available
    int bestLaneIndex = (&r == myRoute ? 0 : (int)bestLaneConts.size());
    std::map<const MSLane*, int> repeatLane; // count repeated occurrences of the same edge
    const double textSize = s.vehicleName.size / s.scale;
    const GUILane* prevLane = nullptr;
    int reversalIndex = 0;
    const int indexDigits = (int)toString(r.size()).size();
    for (; i != r.end(); ++i) {
        const GUILane* lane;
        if (bestLaneIndex < (int)bestLaneConts.size() && bestLaneConts[bestLaneIndex] != 0 && (*i) == &(bestLaneConts[bestLaneIndex]->getEdge())) {
            lane = static_cast<GUILane*>(bestLaneConts[bestLaneIndex]);
            ++bestLaneIndex;
        } else {
            const std::vector<MSLane*>* allowed = (*i)->allowedLanes(getVClass());
            if (allowed != nullptr && allowed->size() != 0) {
                lane = static_cast<GUILane*>((*allowed)[0]);
            } else {
                lane = static_cast<GUILane*>((*i)->getLanes()[0]);
            }
        }
        GLHelper::setColor(col);
        GLHelper::drawBoxLines(lane->getShape(), lane->getShapeRotations(), lane->getShapeLengths(), exaggeration);
        if (prevLane != nullptr && lane->getBidiLane() == prevLane) {
            // indicate train reversal
            std::string label = "reverse:" + toString(reversalIndex++);
            Position pos = lane->geometryPositionAtOffset(lane->getLength() / 2) - Position(0, textSize * repeatLane[lane]);
            GLHelper::drawTextSettings(s.vehicleValue, label, pos, s.scale, s.angle, 1.0);
        }
        if (s.showRouteIndex) {
            std::string label = toString((int)(i - myCurrEdge));
            const double laneAngle = lane->getShape().angleAt2D(0);
            Position pos = lane->getShape().front() - Position(0, textSize * repeatLane[lane]) + Position(
                               (laneAngle >= -0.25 * M_PI && laneAngle < 0.75 * M_PI ? 1 : -1) * 0.4 * indexDigits * textSize, 0);
            //GLHelper::drawText(label, pos, 1.0, textSize, s.vehicleName.color);
            GLHelper::drawTextSettings(s.vehicleName, label, pos, s.scale, s.angle, 1.0);
        }
        repeatLane[lane]++;
        prevLane = lane;
        if (noLoop && i != start && (*i) == (*start)) {
            break;
        }
    }
    // draw stop labels
    // (vertical shift for repeated stops at the same position
    std::map<std::pair<const MSLane*, double>, int> repeat; // count repeated occurrences of the same position
    int stopIndex = 0;
    for (const Stop& stop : myStops) {
        double stopLanePos;
        if (stop.pars.speed > 0) {
            stopLanePos = stop.reached ? stop.pars.endPos : stop.pars.startPos;
        } else {
            stopLanePos = stop.reached ? getPositionOnLane() : MAX2(0.0, stop.getEndPos(*this));
        }
        Position pos = stop.lane->geometryPositionAtOffset(stopLanePos);
        GLHelper::setColor(col);
        GLHelper::drawBoxLines(stop.lane->getShape().getOrthogonal(pos, 10, true, stop.lane->getWidth()), 0.1);
        std::string label = stop.pars.speed > 0 ? "waypoint" : (stop.reached ? "stopped" : "stop " + toString(stopIndex));
#ifdef _DEBUG
        label += " (" + toString(stop.edge - myCurrEdge) + "e)";
#endif
        if (isStoppedTriggered()) {
            label += " triggered:";
            if (stop.triggered) {
                label += "person";
                if (stop.numExpectedPerson > 0) {
                    label += "(" + toString(stop.numExpectedPerson) + ")";
                }
            }
            if (stop.containerTriggered) {
                label += "container";
                if (stop.numExpectedContainer > 0) {
                    label += "(" + toString(stop.numExpectedContainer) + ")";
                }
            }
            if (stop.joinTriggered) {
                label += "join";
                if (stop.pars.join != "") {
                    label += "(" + stop.pars.join + ")";
                }
            }
        }
        if (stop.pars.until >= 0) {
            label += " until:" + time2string(stop.pars.until);
        }
        if (stop.duration >= 0 || stop.pars.duration > 0) {
            if (STEPS2TIME(stop.duration) > 3600 * 24) {
                label += " duration:1day+";
            } else {
                label += " duration:" + time2string(stop.duration);
            }
        }
        if (stop.pars.speed > 0) {
            label += " speed:" + toString(stop.pars.speed);
        }
        std::pair<const MSLane*, double> stopPos = std::make_pair(stop.lane, stopLanePos);
        const double nameSize = s.vehicleName.size / s.scale;
        Position pos2 = pos - Position(0, nameSize * repeat[stopPos]);
        if (noLoop && repeat[stopPos] > 0) {
            break;
        }
        GLHelper::drawTextSettings(s.vehicleText, label, pos2, s.scale, s.angle, 1.0);
        repeat[stopPos]++;
        stopIndex++;
    }
}



MSLane*
GUIVehicle::getPreviousLane(MSLane* current, int& furtherIndex) const {
    if (furtherIndex < (int)myFurtherLanes.size()) {
        return myFurtherLanes[furtherIndex++];
    } else {
        // try to use route information
        int routeIndex = getRoutePosition();
        bool resultInternal;
        if (MSGlobals::gUsingInternalLanes && MSNet::getInstance()->hasInternalLinks()) {
            if (myLane->isInternal()) {
                if (furtherIndex % 2 == 0) {
                    routeIndex -= (furtherIndex + 0) / 2;
                    resultInternal = false;
                } else {
                    routeIndex -= (furtherIndex + 1) / 2;
                    resultInternal = false;
                }
            } else {
                if (furtherIndex % 2 != 0) {
                    routeIndex -= (furtherIndex + 1) / 2;
                    resultInternal = false;
                } else {
                    routeIndex -= (furtherIndex + 2) / 2;
                    resultInternal = true;
                }
            }
        } else {
            routeIndex -= furtherIndex;
            resultInternal = false;
        }
        furtherIndex++;
        if (routeIndex >= 0) {
            if (resultInternal) {
                const MSEdge* prevNormal = myRoute->getEdges()[routeIndex];
                for (MSLane* cand : prevNormal->getLanes()) {
                    for (MSLink* link : cand->getLinkCont()) {
                        if (link->getLane() == current) {
                            if (link->getViaLane() != nullptr) {
                                return link->getViaLane();
                            } else {
                                return const_cast<MSLane*>(link->getLaneBefore());
                            }
                        }
                    }
                }
            } else {
                return myRoute->getEdges()[routeIndex]->getLanes()[0];
            }
        }
    }
    return current;
}


double
GUIVehicle::getLastLaneChangeOffset() const {
    return STEPS2TIME(getLaneChangeModel().getLastLaneChangeOffset());
}


std::string
GUIVehicle::getStopInfo() const {
    std::string result = "";
    if (isParking()) {
        result += "parking";
    } else if (isStopped()) {
        result += "stopped";
    } else if (hasStops()) {
        return "next: " + myStops.front().getDescription();
    } else {
        return "";
    }
    if (myStops.front().pars.triggered) {
        result += ", triggered";
    } else if (myStops.front().pars.containerTriggered) {
        result += ", containerTriggered";
    } else if (myStops.front().collision) {
        result += ", collision";
    } else if (myStops.front().pars.until != -1) {
        result += ", until=" + time2string(myStops.front().pars.until);
    } else {
        result += ", duration=" + time2string(myStops.front().duration);
    }
    return result;
}


void
GUIVehicle::selectBlockingFoes() const {
    double dist = myLane->getLength() - getPositionOnLane();
#ifdef DEBUG_FOES
    std::cout << SIMTIME << " selectBlockingFoes veh=" << getID() << " dist=" << dist << " numLinks=" << myLFLinkLanes.size() << "\n";
#endif
    for (DriveItemVector::const_iterator i = myLFLinkLanes.begin(); i != myLFLinkLanes.end(); ++i) {
        const DriveProcessItem& dpi = *i;
        if (dpi.myLink == nullptr) {
            /// XXX if the vehicle intends to stop on an intersection, there could be a relevant exitLink (see #4299)
            continue;
        }
        MSLink::BlockingFoes blockingFoes;
        std::vector<const MSPerson*> blockingPersons;
#ifdef DEBUG_FOES
        std::cout << "   foeLink=" << dpi.myLink->getViaLaneOrLane()->getID() << "\n";
        const bool isOpen =
#endif
            dpi.myLink->opened(dpi.myArrivalTime, dpi.myArrivalSpeed, dpi.getLeaveSpeed(), getVehicleType().getLength(),
                               getImpatience(), getCarFollowModel().getMaxDecel(), getWaitingTime(), getLateralPositionOnLane(), &blockingFoes);
#ifdef DEBUG_FOES
        if (!isOpen) {
            std::cout << "     closed due to:\n";
            for (const auto& item : blockingFoes) {
                std::cout << "   " << item->getID() << "\n";
            }
        }
#endif
        if (getLaneChangeModel().getShadowLane() != nullptr) {
            MSLink* parallelLink = dpi.myLink->getParallelLink(getLaneChangeModel().getShadowDirection());
            if (parallelLink != nullptr) {
                const double shadowLatPos = getLateralPositionOnLane() - getLaneChangeModel().getShadowDirection() * 0.5 * (
                                                myLane->getWidth() + getLaneChangeModel().getShadowLane()->getWidth());
#ifdef DEBUG_FOES
                const bool isShadowOpen =
#endif
                    parallelLink->opened(dpi.myArrivalTime, dpi.myArrivalSpeed, dpi.getLeaveSpeed(),
                                         getVehicleType().getLength(), getImpatience(),
                                         getCarFollowModel().getMaxDecel(),
                                         getWaitingTime(), shadowLatPos, &blockingFoes);
#ifdef DEBUG_FOES
                if (!isShadowOpen) {
                    std::cout <<  "    foes at shadow link=" << parallelLink->getViaLaneOrLane()->getID() << ":\n";
                    for (const auto& item : blockingFoes) {
                        std::cout << "   " << item->getID() << "\n";
                    }
                }
#endif
            }
        }
        for (const auto& item : blockingFoes) {
            gSelected.select(static_cast<const GUIVehicle*>(item)->getGlID());
        }
#ifdef DEBUG_FOES
        gDebugFlag1 = true;
#endif
        const MSLink::LinkLeaders linkLeaders = (dpi.myLink)->getLeaderInfo(this, dist, &blockingPersons);
#ifdef DEBUG_FOES
        gDebugFlag1 = false;
#endif
        for (MSLink::LinkLeaders::const_iterator it = linkLeaders.begin(); it != linkLeaders.end(); ++it) {
            // the vehicle to enter the junction first has priority
            const GUIVehicle* leader = dynamic_cast<const GUIVehicle*>(it->vehAndGap.first);
            if (leader != nullptr) {
                if (isLeader(dpi.myLink, leader)) {
                    gSelected.select(leader->getGlID());
#ifdef DEBUG_FOES
                    std::cout << "      linkLeader=" << leader->getID() << "\n";
#endif
                }
            } else {
                for (std::vector<const MSPerson*>::iterator it_p = blockingPersons.begin(); it_p != blockingPersons.end(); ++it_p) {
                    const GUIPerson* foe = dynamic_cast<const GUIPerson*>(*it_p);
                    if (foe != nullptr) {
                        gSelected.select(foe->getGlID());
                        //std::cout << SIMTIME << " veh=" << getID() << " is blocked on link " << dpi.myLink->getRespondIndex() << " to " << dpi.myLink->getViaLaneOrLane()->getID() << " by pedestrian. dist=" << it->second << "\n";
                    }
                }
            }
        }
        dist += dpi.myLink->getViaLaneOrLane()->getLength();
    }
}


void
GUIVehicle::drawOutsideNetwork(bool add) {
    GUIMainWindow* mw = GUIMainWindow::getInstance();
    GUISUMOAbstractView* view = mw->getActiveView();
    if (view != nullptr) {
        if (add) {
            if ((myAdditionalVisualizations[view] & VO_DRAW_OUTSIDE_NETWORK) == 0) {
                myAdditionalVisualizations[view] |= VO_DRAW_OUTSIDE_NETWORK;
                view->addAdditionalGLVisualisation(this);
            }
        } else {
            view->removeAdditionalGLVisualisation(this);
            myAdditionalVisualizations[view] &= ~VO_DRAW_OUTSIDE_NETWORK;
        }
    }
}

bool
GUIVehicle::isSelected() const {
    return gSelected.isSelected(GLO_VEHICLE, getGlID());
}

int
GUIVehicle::getRightSublaneOnEdge() const {
    const double rightSide = getRightSideOnEdge();
    const std::vector<double>& sublaneSides = myLane->getEdge().getSubLaneSides();
    for (int i = 0; i < (int)sublaneSides.size(); ++i) {
        if (sublaneSides[i] > rightSide) {
            return MAX2(i - 1, 0);
        }
    }
    return -1;
}

int
GUIVehicle::getLeftSublaneOnEdge() const {
    const double leftSide = getLeftSideOnEdge();
    const std::vector<double>& sublaneSides = myLane->getEdge().getSubLaneSides();
    for (int i = (int)sublaneSides.size() - 1; i >= 0; --i) {
        if (sublaneSides[i] < leftSide) {
            return i;
        }
    }
    return -1;
}


std::string
GUIVehicle::getLCStateRight() const {
    return toString((LaneChangeAction)getLaneChangeModel().getSavedState(-1).second);
}

std::string
GUIVehicle::getLCStateLeft() const {
    return toString((LaneChangeAction)getLaneChangeModel().getSavedState(1).second);
}

std::string
GUIVehicle::getLCStateCenter() const {
    return toString((LaneChangeAction)getLaneChangeModel().getSavedState(0).second);
}

std::string
GUIVehicle::getLaneID() const {
    return Named::getIDSecure(myLane, "n/a");
}

std::string
GUIVehicle::getShadowLaneID() const {
    return Named::getIDSecure(getLaneChangeModel().getShadowLane(), "");
}

std::string
GUIVehicle::getTargetLaneID() const {
    return Named::getIDSecure(getLaneChangeModel().getTargetLane(), "");
}

double
GUIVehicle::getManeuverDist() const {
    return getLaneChangeModel().getManeuverDist();
}

void
GUIVehicle::rerouteDRTStop(MSStoppingPlace* busStop) {
    SUMOTime intermediateDuration = TIME2STEPS(20);
    SUMOTime finalDuration = SUMOTime_MAX;
    if (myParameter->stops.size() >= 2) {
        // copy durations from the original stops
        intermediateDuration = myParameter->stops.front().duration;
        finalDuration = myParameter->stops.back().duration;
    }
    // if the stop is already in the list of stops, cancel all stops that come
    // after it and set the stop duration
    std::string line = "";
    int destinations = 0;
    bool add = true;
    for (auto it = myStops.begin(); it != myStops.end(); it++) {
        if (!it->reached && destinations < 2 && it->busstop != nullptr) {
            line += it->busstop->getID();
            destinations++;
        }
        if (it->busstop == busStop) {
            it->duration = finalDuration;
            myStops.erase(++it, myStops.end());
            add = false;
            break;
        } else {
            it->duration = MIN2(it->duration, intermediateDuration);
        }
    }
    if (destinations < 2) {
        line += busStop->getID();
    }
    if (add) {
        // create new stop
        SUMOVehicleParameter::Stop stopPar;
        stopPar.busstop = busStop->getID();
        stopPar.lane = busStop->getLane().getID();
        stopPar.startPos = busStop->getBeginLanePosition();
        stopPar.endPos = busStop->getEndLanePosition();
        stopPar.duration = finalDuration;
        stopPar.until = -1;
        stopPar.triggered = false;
        stopPar.containerTriggered = false;
        stopPar.parking = false;
        stopPar.index = STOP_INDEX_FIT;
        stopPar.parametersSet = STOP_START_SET | STOP_END_SET;
        // clean up prior route to improve visualisation, ensure that the stop can be added immediately
        ConstMSEdgeVector edges = myRoute->getEdges();
        edges.erase(edges.begin(), edges.begin() + getRoutePosition());
        edges.push_back(&busStop->getLane().getEdge());
        replaceRouteEdges(edges, -1, 0, "DRT.tmp", false, false, false);
        std::string errorMsg;
        // add stop
        addStop(stopPar, errorMsg);
    }
    const bool hasReroutingDevice = getDevice(typeid(MSDevice_Routing)) != nullptr;
    SUMOAbstractRouter<MSEdge, SUMOVehicle>& router = hasReroutingDevice
            ? MSRoutingEngine::getRouterTT(getRNGIndex(), getVClass())
            : MSNet::getInstance()->getRouterTT(getRNGIndex());
    // reroute to ensure the new stop is reached
    reroute(MSNet::getInstance()->getCurrentTimeStep(), "DRT", router);
    myParameter->line = line;
    assert(haveValidStopEdges());
}


/****************************************************************************/
