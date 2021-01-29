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
/// @file    GUIMEVehicle.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// A MSVehicle extended by some values for usage within the gui
/****************************************************************************/
#include <config.h>

#include <utils/gui/globjects/GLIncludes.h>
#include <utils/gui/div/GLHelper.h>
#include <utils/gui/div/GUIParameterTableWindow.h>
#include <utils/gui/div/GUIGlobalSelection.h>
#include <utils/gui/div/GUIBaseVehicleHelper.h>
#include <utils/emissions/PollutantsInterface.h>
#include <utils/gui/settings/GUIVisualizationSettings.h>
#include <microsim/logging/CastingFunctionBinding.h>
#include <microsim/logging/FunctionBinding.h>
#include <microsim/devices/MSVehicleDevice.h>
#include <guisim/GUILane.h>

#include "GUIMEVehicle.h"


// ===========================================================================
// method definitions
// ===========================================================================
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4355)
#endif
GUIMEVehicle::GUIMEVehicle(SUMOVehicleParameter* pars, const MSRoute* route,
                           MSVehicleType* type, const double speedFactor) :
    MEVehicle(pars, route, type, speedFactor),
    GUIBaseVehicle((MSBaseVehicle&) * this) {
}
#ifdef _MSC_VER
#pragma warning(pop)
#endif


GUIMEVehicle::~GUIMEVehicle() { }


GUIParameterTableWindow*
GUIMEVehicle::getParameterWindow(GUIMainWindow& app,
                                 GUISUMOAbstractView&) {
    GUIParameterTableWindow* ret = new GUIParameterTableWindow(app, *this);
    // add items
    ret->mkItem("edge [id]", true, new FunctionBindingString<GUIMEVehicle>(this, &GUIMEVehicle::getEdgeID));
    ret->mkItem("segment [#]", true,  new FunctionBinding<GUIMEVehicle, int>(this, &GUIMEVehicle::getSegmentIndex));
    ret->mkItem("queue [#]", true,  new FunctionBinding<GUIMEVehicle, int>(this, &GUIMEVehicle::getQueIndex));
    ret->mkItem("position [m]", true, new FunctionBinding<GUIMEVehicle, double>(this, &MEVehicle::getPositionOnLane));
    ret->mkItem("speed [m/s]", true, new FunctionBinding<GUIMEVehicle, double>(this, &MEVehicle::getSpeed));
    ret->mkItem("angle [degree]", true, new FunctionBinding<GUIMEVehicle, double>(this, &GUIBaseVehicle::getNaviDegree));
    ret->mkItem("waiting time [s]", true,
                new FunctionBinding<GUIMEVehicle, double>(this, &MEVehicle::getWaitingSeconds));
    ret->mkItem("speed factor", false, getChosenSpeedFactor());
    //ret->mkItem("time gap [s]", true,
    //            new FunctionBinding<GUIMEVehicle, double>(this, &MSVehicle::getTimeGap));
    //ret->mkItem("waiting time [s]", true,
    //            new FunctionBinding<GUIMEVehicle, double>(this, &MSVehicle::getWaitingSeconds));
    //ret->mkItem("impatience", true,
    //            new FunctionBinding<GUIMEVehicle, double>(this, &MSVehicle::getImpatience));
    //ret->mkItem("last lane change [s]", true,
    //            new FunctionBinding<GUIMEVehicle, double>(this, &GUIMEVehicle::getLastLaneChangeOffset));
    ret->mkItem("desired depart [s]", false, time2string(getParameter().depart));
    ret->mkItem("depart delay [s]", false, time2string(getDepartDelay()));
    ret->mkItem("odometer [m]", true,
                new FunctionBinding<GUIMEVehicle, double>(this, &MSBaseVehicle::getOdometer));
    if (getParameter().repetitionNumber < std::numeric_limits<int>::max()) {
        ret->mkItem("remaining [#]", false, (int) getParameter().repetitionNumber - getParameter().repetitionsDone);
    }
    if (getParameter().repetitionOffset > 0) {
        ret->mkItem("insertion period [s]", false, time2string(getParameter().repetitionOffset));
    }
    if (getParameter().repetitionProbability > 0) {
        ret->mkItem("insertion probability", false, getParameter().repetitionProbability);
    }
    //ret->mkItem("stop info", false, getStopInfo());
    ret->mkItem("line", false, myParameter->line);
    //ret->mkItem("CO2 [mg/s]", true,
    //            new FunctionBinding<GUIMEVehicle, double>(this, &GUIMEVehicle::getCO2Emissions));
    //ret->mkItem("CO [mg/s]", true,
    //            new FunctionBinding<GUIMEVehicle, double>(this, &GUIMEVehicle::getCOEmissions));
    //ret->mkItem("HC [mg/s]", true,
    //            new FunctionBinding<GUIMEVehicle, double>(this, &GUIMEVehicle::getHCEmissions));
    //ret->mkItem("NOx [mg/s]", true,
    //            new FunctionBinding<GUIMEVehicle, double>(this, &GUIMEVehicle::getNOxEmissions));
    //ret->mkItem("PMx [mg/s]", true,
    //            new FunctionBinding<GUIMEVehicle, double>(this, &GUIMEVehicle::getPMxEmissions));
    //ret->mkItem("fuel [ml/s]", true,
    //            new FunctionBinding<GUIMEVehicle, double>(this, &GUIMEVehicle::getFuelConsumption));
    //ret->mkItem("noise (Harmonoise) [dB]", true,
    //            new FunctionBinding<GUIMEVehicle, double>(this, &GUIMEVehicle::getHarmonoise_NoiseEmissions));
    ret->mkItem("devices", false, getDeviceDescription());
    //ret->mkItem("persons", true,
    //            new FunctionBinding<GUIMEVehicle, int>(this, &GUIMEVehicle::getPersonNumber));
    //ret->mkItem("containers", true,
    //            new FunctionBinding<GUIMEVehicle, int>(this, &GUIMEVehicle::getContainerNumber));
    // meso specific values
    ret->mkItem("event time [s]", true, new FunctionBinding<GUIMEVehicle, double>(this, &MEVehicle::getEventTimeSeconds));
    ret->mkItem("entry time [s]", true, new FunctionBinding<GUIMEVehicle, double>(this, &MEVehicle::getLastEntryTimeSeconds));
    ret->mkItem("block time [s]", true, new FunctionBinding<GUIMEVehicle, double>(this, &MEVehicle::getBlockTimeSeconds));
    ret->mkItem("link penalty [s]", true, new FunctionBinding<GUIMEVehicle, double>(this, &MEVehicle::getCurrentLinkPenaltySeconds));
    ret->mkItem("stop time [s]", true, new FunctionBinding<GUIMEVehicle, double>(this, &MEVehicle::getCurrentStoppingTimeSeconds));
    // close building
    ret->closeBuilding(&getParameter());
    return ret;
}


GUIParameterTableWindow*
GUIMEVehicle::getTypeParameterWindow(GUIMainWindow& app,
                                     GUISUMOAbstractView&) {
    GUIParameterTableWindow* ret = new GUIParameterTableWindow(app, *this);
    // add items
    ret->mkItem("Type Information:", false, "");
    ret->mkItem("type [id]", false, myType->getID());
    ret->mkItem("length", false, myType->getLength());
    ret->mkItem("minGap", false, myType->getMinGap());
    ret->mkItem("vehicle class", false, SumoVehicleClassStrings.getString(myType->getVehicleClass()));
    ret->mkItem("emission class", false, PollutantsInterface::getName(myType->getEmissionClass()));
    ret->mkItem("maximum speed [m/s]", false, getMaxSpeed());
    //ret->mkItem("maximum acceleration [m/s^2]", false, getCarFollowModel().getMaxAccel());
    //ret->mkItem("maximum deceleration [m/s^2]", false, getCarFollowModel().getMaxDecel());
    //ret->mkItem("imperfection (sigma)", false, getCarFollowModel().getImperfection());
    //ret->mkItem("desired headway (tau)", false, getCarFollowModel().getHeadwayTime());
    ret->mkItem("person capacity", false, myType->getPersonCapacity());
    ret->mkItem("container capacity", false, myType->getContainerCapacity());
    // close building
    ret->closeBuilding(&(myType->getParameter()));
    return ret;
}


void
GUIMEVehicle::drawAction_drawCarriageClass(const GUIVisualizationSettings& /* s */, bool /* asImage */) const {
    GUIBaseVehicleHelper::drawAction_drawVehicleAsBoxPlus(getVType().getWidth(), getVType().getLength());
}


double
GUIMEVehicle::getColorValue(const GUIVisualizationSettings& /* s */, int activeScheme) const {
    switch (activeScheme) {
        case 8:
            return getSpeed();
        case 9:
            return 0; // by actionStep
        case 10:
            return getWaitingSeconds();
        case 11:
            return 0; // getAccumulatedWaitingSeconds
        case 12:
            return 0; // invalid getLastLaneChangeOffset();
        case 13:
            return getSegment()->getEdge().getVehicleMaxSpeed(this);
        case 14:
            return 0; // invalid getCO2Emissions();
        case 15:
            return 0; // invalid getCOEmissions();
        case 16:
            return 0; // invalid getPMxEmissions();
        case 17:
            return 0; // invalid  getNOxEmissions();
        case 18:
            return 0; // invalid getHCEmissions();
        case 19:
            return 0; // invalid getFuelConsumption();
        case 20:
            return 0; // invalid getHarmonoise_NoiseEmissions();
        case 21: // reroute number
            if (getNumberReroutes() == 0) {
                return -1;
            }
            return getNumberReroutes();
        case 22:
            return gSelected.isSelected(GLO_VEHICLE, getGlID());
        case 23:
            return 0; // invalid getBestLaneOffset();
        case 24:
            return 0; // invalid getAcceleration();
        case 25:
            return 0; // invalid getTimeGapOnLane();
        case 26:
            return STEPS2TIME(getDepartDelay());
        case 27:
            return 0; // electricityConsumption
        case 28:
            return 0; // timeLossSeconds
        case 29:
            return 0; // getSpeedLat
    }
    return 0;
}



void
GUIMEVehicle::drawRouteHelper(const GUIVisualizationSettings& s, const MSRoute& r, bool future, bool noLoop, const RGBColor& /*col*/) const {
    const double exaggeration = s.vehicleSize.getExaggeration(s, this);
    MSRouteIterator start = future ? myCurrEdge : r.begin();
    MSRouteIterator i = start;
    for (; i != r.end(); ++i) {
        const GUILane* lane = static_cast<GUILane*>((*i)->getLanes()[0]);
        GLHelper::drawBoxLines(lane->getShape(), lane->getShapeRotations(), lane->getShapeLengths(), 1.0);
        GLHelper::drawBoxLines(lane->getShape(), lane->getShapeRotations(), lane->getShapeLengths(), exaggeration);
        if (noLoop && i != start && (*i) == (*start)) {
            break;
        }
    }
}


double
GUIMEVehicle::getLastLaneChangeOffset() const {
    // @todo possibly we could compute something reasonable here
    return 0;
}


std::string
GUIMEVehicle::getStopInfo() const {
    std::string result = "";
    if (isParking()) {
        result += "parking";
    } else if (isStopped()) {
        result += "stopped";
    } else {
        return "";
    }
    return result;
}

std::string
GUIMEVehicle::getEdgeID() const {
    return getEdge()->getID();
}

int
GUIMEVehicle::getSegmentIndex() const {
    return getSegment() != nullptr ? getSegment()->getIndex() : -1;
}


void
GUIMEVehicle::selectBlockingFoes() const {
    // @todo possibly we could compute something reasonable here
}


/****************************************************************************/
