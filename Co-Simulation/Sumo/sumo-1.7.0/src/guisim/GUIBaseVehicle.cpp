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
/// @file    GUIBaseVehicle.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @author  Laura Bieker-Walz
/// @date    Sept 2002
///
// A MSVehicle extended by some values for usage within the gui
/****************************************************************************/
#include <config.h>

#include <cmath>
#include <vector>
#include <string>
#include <functional>
#include <utils/common/StringUtils.h>
#include <utils/common/StringTokenizer.h>
#include <utils/geom/GeomHelper.h>
#include <utils/vehicle/SUMOVehicleParameter.h>
#include <utils/emissions/PollutantsInterface.h>
#include <utils/gui/globjects/GLIncludes.h>
#include <utils/gui/windows/GUISUMOAbstractView.h>
#include <utils/gui/windows/GUIAppEnum.h>
#include <utils/gui/images/GUITexturesHelper.h>
#include <utils/gui/div/GUIGlobalSelection.h>
#include <utils/gui/div/GLHelper.h>
#include <utils/gui/div/GUIGlobalSelection.h>
#include <utils/gui/div/GUIBaseVehicleHelper.h>
#include <mesosim/MEVehicle.h>
#include <mesosim/MELoop.h>
#include <microsim/MSVehicle.h>
#include <microsim/MSLane.h>
#include <microsim/logging/CastingFunctionBinding.h>
#include <microsim/logging/FunctionBinding.h>
#include <microsim/MSVehicleControl.h>
#include <microsim/lcmodels/MSAbstractLaneChangeModel.h>
#include <microsim/devices/MSDevice_Vehroutes.h>
#include <microsim/devices/MSDevice_Transportable.h>
#include <microsim/devices/MSDevice_BTreceiver.h>
#include <gui/GUIApplicationWindow.h>
#include <gui/GUIGlobals.h>

#include "GUIBaseVehicle.h"
#include "GUIPerson.h"
#include "GUIContainer.h"
#include "GUINet.h"
#include "GUIEdge.h"
#include "GUILane.h"

//#define DRAW_BOUNDING_BOX

// ===========================================================================
// FOX callback mapping
// ===========================================================================
FXDEFMAP(GUIBaseVehicle::GUIBaseVehiclePopupMenu) GUIBaseVehiclePopupMenuMap[] = {
    FXMAPFUNC(SEL_COMMAND, MID_SHOW_ALLROUTES, GUIBaseVehicle::GUIBaseVehiclePopupMenu::onCmdShowAllRoutes),
    FXMAPFUNC(SEL_COMMAND, MID_HIDE_ALLROUTES, GUIBaseVehicle::GUIBaseVehiclePopupMenu::onCmdHideAllRoutes),
    FXMAPFUNC(SEL_COMMAND, MID_SHOW_CURRENTROUTE, GUIBaseVehicle::GUIBaseVehiclePopupMenu::onCmdShowCurrentRoute),
    FXMAPFUNC(SEL_COMMAND, MID_HIDE_CURRENTROUTE, GUIBaseVehicle::GUIBaseVehiclePopupMenu::onCmdHideCurrentRoute),
    FXMAPFUNC(SEL_COMMAND, MID_SHOW_FUTUREROUTE, GUIBaseVehicle::GUIBaseVehiclePopupMenu::onCmdShowFutureRoute),
    FXMAPFUNC(SEL_COMMAND, MID_HIDE_FUTUREROUTE, GUIBaseVehicle::GUIBaseVehiclePopupMenu::onCmdHideFutureRoute),
    FXMAPFUNC(SEL_COMMAND, MID_SHOW_ROUTE_NOLOOPS, GUIBaseVehicle::GUIBaseVehiclePopupMenu::onCmdShowRouteNoLoops),
    FXMAPFUNC(SEL_COMMAND, MID_HIDE_ROUTE_NOLOOPS, GUIBaseVehicle::GUIBaseVehiclePopupMenu::onCmdHideRouteNoLoops),
    FXMAPFUNC(SEL_COMMAND, MID_SHOW_BEST_LANES, GUIBaseVehicle::GUIBaseVehiclePopupMenu::onCmdShowBestLanes),
    FXMAPFUNC(SEL_COMMAND, MID_HIDE_BEST_LANES, GUIBaseVehicle::GUIBaseVehiclePopupMenu::onCmdHideBestLanes),
    FXMAPFUNC(SEL_COMMAND, MID_START_TRACK, GUIBaseVehicle::GUIBaseVehiclePopupMenu::onCmdStartTrack),
    FXMAPFUNC(SEL_COMMAND, MID_STOP_TRACK, GUIBaseVehicle::GUIBaseVehiclePopupMenu::onCmdStopTrack),
    FXMAPFUNC(SEL_COMMAND, MID_SHOW_LFLINKITEMS, GUIBaseVehicle::GUIBaseVehiclePopupMenu::onCmdShowLFLinkItems),
    FXMAPFUNC(SEL_COMMAND, MID_HIDE_LFLINKITEMS, GUIBaseVehicle::GUIBaseVehiclePopupMenu::onCmdHideLFLinkItems),
    FXMAPFUNC(SEL_COMMAND, MID_SHOW_FOES, GUIBaseVehicle::GUIBaseVehiclePopupMenu::onCmdShowFoes),
    FXMAPFUNC(SEL_COMMAND, MID_REMOVE_OBJECT, GUIBaseVehicle::GUIBaseVehiclePopupMenu::onCmdRemoveObject),
    FXMAPFUNC(SEL_COMMAND, MID_TOGGLE_STOP, GUIBaseVehicle::GUIBaseVehiclePopupMenu::onCmdToggleStop),
};

// Object implementation
FXIMPLEMENT(GUIBaseVehicle::GUIBaseVehiclePopupMenu, GUIGLObjectPopupMenu, GUIBaseVehiclePopupMenuMap, ARRAYNUMBER(GUIBaseVehiclePopupMenuMap))

// ===========================================================================
// method definitions
// ===========================================================================
/* -------------------------------------------------------------------------
 * GUIBaseVehicle::GUIBaseVehiclePopupMenu - methods
 * ----------------------------------------------------------------------- */
GUIBaseVehicle::GUIBaseVehiclePopupMenu::GUIBaseVehiclePopupMenu(
    GUIMainWindow& app, GUISUMOAbstractView& parent, GUIGlObject& o)
    : GUIGLObjectPopupMenu(app, parent, o) {
}


GUIBaseVehicle::GUIBaseVehiclePopupMenu::~GUIBaseVehiclePopupMenu() {}


long
GUIBaseVehicle::GUIBaseVehiclePopupMenu::onCmdShowAllRoutes(FXObject*, FXSelector, void*) {
    assert(myObject->getType() == GLO_VEHICLE);
    if (!static_cast<GUIBaseVehicle*>(myObject)->hasActiveAddVisualisation(myParent, VO_SHOW_ALL_ROUTES)) {
        static_cast<GUIBaseVehicle*>(myObject)->addActiveAddVisualisation(myParent, VO_SHOW_ALL_ROUTES);
    }
    return 1;
}

long
GUIBaseVehicle::GUIBaseVehiclePopupMenu::onCmdHideAllRoutes(FXObject*, FXSelector, void*) {
    assert(myObject->getType() == GLO_VEHICLE);
    static_cast<GUIBaseVehicle*>(myObject)->removeActiveAddVisualisation(myParent, VO_SHOW_ALL_ROUTES);
    return 1;
}


long
GUIBaseVehicle::GUIBaseVehiclePopupMenu::onCmdShowCurrentRoute(FXObject*, FXSelector, void*) {
    assert(myObject->getType() == GLO_VEHICLE);
    if (!static_cast<GUIBaseVehicle*>(myObject)->hasActiveAddVisualisation(myParent, VO_SHOW_ROUTE)) {
        static_cast<GUIBaseVehicle*>(myObject)->addActiveAddVisualisation(myParent, VO_SHOW_ROUTE);
    }
    return 1;
}

long
GUIBaseVehicle::GUIBaseVehiclePopupMenu::onCmdHideCurrentRoute(FXObject*, FXSelector, void*) {
    assert(myObject->getType() == GLO_VEHICLE);
    static_cast<GUIBaseVehicle*>(myObject)->removeActiveAddVisualisation(myParent, VO_SHOW_ROUTE);
    return 1;
}


long
GUIBaseVehicle::GUIBaseVehiclePopupMenu::onCmdShowFutureRoute(FXObject*, FXSelector, void*) {
    assert(myObject->getType() == GLO_VEHICLE);
    if (!static_cast<GUIBaseVehicle*>(myObject)->hasActiveAddVisualisation(myParent, VO_SHOW_FUTURE_ROUTE)) {
        static_cast<GUIBaseVehicle*>(myObject)->addActiveAddVisualisation(myParent, VO_SHOW_FUTURE_ROUTE);
    }
    return 1;
}

long
GUIBaseVehicle::GUIBaseVehiclePopupMenu::onCmdHideFutureRoute(FXObject*, FXSelector, void*) {
    assert(myObject->getType() == GLO_VEHICLE);
    static_cast<GUIBaseVehicle*>(myObject)->removeActiveAddVisualisation(myParent, VO_SHOW_FUTURE_ROUTE);
    return 1;
}


long
GUIBaseVehicle::GUIBaseVehiclePopupMenu::onCmdShowRouteNoLoops(FXObject*, FXSelector, void*) {
    assert(myObject->getType() == GLO_VEHICLE);
    if (!static_cast<GUIBaseVehicle*>(myObject)->hasActiveAddVisualisation(myParent, VO_SHOW_ROUTE_NOLOOP)) {
        static_cast<GUIBaseVehicle*>(myObject)->addActiveAddVisualisation(myParent, VO_SHOW_ROUTE_NOLOOP);
    }
    return 1;
}

long
GUIBaseVehicle::GUIBaseVehiclePopupMenu::onCmdHideRouteNoLoops(FXObject*, FXSelector, void*) {
    assert(myObject->getType() == GLO_VEHICLE);
    static_cast<GUIBaseVehicle*>(myObject)->removeActiveAddVisualisation(myParent, VO_SHOW_ROUTE_NOLOOP);
    return 1;
}


long
GUIBaseVehicle::GUIBaseVehiclePopupMenu::onCmdShowBestLanes(FXObject*, FXSelector, void*) {
    assert(myObject->getType() == GLO_VEHICLE);
    if (!static_cast<GUIBaseVehicle*>(myObject)->hasActiveAddVisualisation(myParent, VO_SHOW_BEST_LANES)) {
        static_cast<GUIBaseVehicle*>(myObject)->addActiveAddVisualisation(myParent, VO_SHOW_BEST_LANES);
    }
    return 1;
}

long
GUIBaseVehicle::GUIBaseVehiclePopupMenu::onCmdHideBestLanes(FXObject*, FXSelector, void*) {
    assert(myObject->getType() == GLO_VEHICLE);
    static_cast<GUIBaseVehicle*>(myObject)->removeActiveAddVisualisation(myParent, VO_SHOW_BEST_LANES);
    return 1;
}


long
GUIBaseVehicle::GUIBaseVehiclePopupMenu::onCmdStartTrack(FXObject*, FXSelector, void*) {
    assert(myObject->getType() == GLO_VEHICLE);
    if (myParent->getTrackedID() != static_cast<GUIBaseVehicle*>(myObject)->getGlID()) {
        myParent->startTrack(static_cast<GUIBaseVehicle*>(myObject)->getGlID());
    }
    return 1;
}

long
GUIBaseVehicle::GUIBaseVehiclePopupMenu::onCmdStopTrack(FXObject*, FXSelector, void*) {
    assert(myObject->getType() == GLO_VEHICLE);
    myParent->stopTrack();
    return 1;
}


long
GUIBaseVehicle::GUIBaseVehiclePopupMenu::onCmdShowLFLinkItems(FXObject*, FXSelector, void*) {
    assert(myObject->getType() == GLO_VEHICLE);
    if (!static_cast<GUIBaseVehicle*>(myObject)->hasActiveAddVisualisation(myParent, VO_SHOW_LFLINKITEMS)) {
        static_cast<GUIBaseVehicle*>(myObject)->addActiveAddVisualisation(myParent, VO_SHOW_LFLINKITEMS);
    }
    return 1;
}

long
GUIBaseVehicle::GUIBaseVehiclePopupMenu::onCmdHideLFLinkItems(FXObject*, FXSelector, void*) {
    assert(myObject->getType() == GLO_VEHICLE);
    static_cast<GUIBaseVehicle*>(myObject)->removeActiveAddVisualisation(myParent, VO_SHOW_LFLINKITEMS);
    return 1;
}

long
GUIBaseVehicle::GUIBaseVehiclePopupMenu::onCmdShowFoes(FXObject*, FXSelector, void*) {
    assert(myObject->getType() == GLO_VEHICLE);
    static_cast<GUIBaseVehicle*>(myObject)->selectBlockingFoes();
    myParent->update();
    return 1;
}


long
GUIBaseVehicle::GUIBaseVehiclePopupMenu::onCmdRemoveObject(FXObject*, FXSelector, void*) {
    GUIBaseVehicle* baseVeh = static_cast<GUIBaseVehicle*>(myObject);
    MSVehicle* microVeh = dynamic_cast<MSVehicle*>(&baseVeh->myVehicle);
    if (microVeh != nullptr) {
        microVeh->onRemovalFromNet(MSMoveReminder::NOTIFICATION_VAPORIZED_GUI);
        if (microVeh->getLane() != nullptr) {
            microVeh->getLane()->removeVehicle(microVeh, MSMoveReminder::NOTIFICATION_VAPORIZED_GUI);
        }
    } else {
        MEVehicle* mesoVeh = dynamic_cast<MEVehicle*>(&baseVeh->myVehicle);
        MSGlobals::gMesoNet->vaporizeCar(mesoVeh, MSMoveReminder::NOTIFICATION_VAPORIZED_GUI);
    }
    MSNet::getInstance()->getVehicleControl().scheduleVehicleRemoval(&baseVeh->myVehicle);
    myParent->update();
    return 1;
}


long
GUIBaseVehicle::GUIBaseVehiclePopupMenu::onCmdToggleStop(FXObject*, FXSelector, void*) {
    GUIBaseVehicle* baseVeh = static_cast<GUIBaseVehicle*>(myObject);
    MSVehicle* microVeh = dynamic_cast<MSVehicle*>(&baseVeh->myVehicle);
    if (microVeh != nullptr) {
        if (microVeh->isStopped()) {
            microVeh->resumeFromStopping();
        } else {
            std::string errorOut;
            const double brakeGap = microVeh->getCarFollowModel().brakeGap(microVeh->getSpeed());
            std::pair<const MSLane*, double> stopPos = microVeh->getLanePosAfterDist(brakeGap);
            if (stopPos.first != nullptr) {
                SUMOVehicleParameter::Stop stop;
                stop.lane = stopPos.first->getID();
                stop.startPos = stopPos.second;
                stop.endPos = stopPos.second + POSITION_EPS;
                stop.duration = TIME2STEPS(3600);
                microVeh->addTraciStop(stop, errorOut);
                if (errorOut != "") {
                    WRITE_WARNING(errorOut);
                }
            }
        }
    } else {
        WRITE_WARNING("GUI-triggered stop not implemented for meso");
    }
    myParent->update();
    return 1;
}


/* -------------------------------------------------------------------------
 * GUIBaseVehicle - methods
 * ----------------------------------------------------------------------- */

GUIBaseVehicle::GUIBaseVehicle(MSBaseVehicle& vehicle) :
    GUIGlObject(GLO_VEHICLE, vehicle.getID()),
    myVehicle(vehicle),
    myPopup(nullptr) {
    // as it is possible to show all vehicle routes, we have to store them... (bug [ 2519761 ])
    myRoutes = MSDevice_Vehroutes::buildVehicleDevices(myVehicle, myVehicle.myDevices, 5);
    myVehicle.myMoveReminders.push_back(std::make_pair(myRoutes, 0.));
    mySeatPositions.push_back(Seat()); // ensure length 1
    myContainerPositions.push_back(Seat()); // ensure length 1
}


GUIBaseVehicle::~GUIBaseVehicle() {
    myLock.lock();
    for (std::map<GUISUMOAbstractView*, int>::iterator i = myAdditionalVisualizations.begin(); i != myAdditionalVisualizations.end(); ++i) {
        while (i->first->removeAdditionalGLVisualisation(this));
    }
    myLock.unlock();
    delete myRoutes;
    if (myPopup != nullptr) {
        myPopup->getParentView()->destroyPopup();
    }
}


GUIGLObjectPopupMenu*
GUIBaseVehicle::getPopUpMenu(GUIMainWindow& app,
                             GUISUMOAbstractView& parent) {
    GUIGLObjectPopupMenu* ret = new GUIBaseVehiclePopupMenu(app, parent, *this);
    buildPopupHeader(ret, app);
    buildCenterPopupEntry(ret);
    buildNameCopyPopupEntry(ret);
    buildSelectionPopupEntry(ret);
    //
    if (hasActiveAddVisualisation(&parent, VO_SHOW_ROUTE)) {
        new FXMenuCommand(ret, "Hide Current Route", nullptr, ret, MID_HIDE_CURRENTROUTE);
    } else {
        new FXMenuCommand(ret, "Show Current Route", nullptr, ret, MID_SHOW_CURRENTROUTE);
    }
    if (hasActiveAddVisualisation(&parent, VO_SHOW_FUTURE_ROUTE)) {
        new FXMenuCommand(ret, "Hide Future Route", nullptr, ret, MID_HIDE_FUTUREROUTE);
    } else {
        new FXMenuCommand(ret, "Show Future Route", nullptr, ret, MID_SHOW_FUTUREROUTE);
    }
    if (hasActiveAddVisualisation(&parent, VO_SHOW_ALL_ROUTES)) {
        new FXMenuCommand(ret, "Hide All Routes", nullptr, ret, MID_HIDE_ALLROUTES);
    } else {
        new FXMenuCommand(ret, "Show All Routes", nullptr, ret, MID_SHOW_ALLROUTES);
    }
    if (hasActiveAddVisualisation(&parent, VO_SHOW_ROUTE_NOLOOP)) {
        FXMenuCheck* showLoops = new FXMenuCheck(ret, "Draw looped routes", ret, MID_HIDE_ROUTE_NOLOOPS);
        showLoops->setCheck(false);
    } else {
        FXMenuCheck* showLoops = new FXMenuCheck(ret, "Draw looped routes", ret, MID_SHOW_ROUTE_NOLOOPS);
        showLoops->setCheck(true);
    }
    if (hasActiveAddVisualisation(&parent, VO_SHOW_BEST_LANES)) {
        new FXMenuCommand(ret, "Hide Best Lanes", nullptr, ret, MID_HIDE_BEST_LANES);
    } else {
        new FXMenuCommand(ret, "Show Best Lanes", nullptr, ret, MID_SHOW_BEST_LANES);
    }
    if (hasActiveAddVisualisation(&parent, VO_SHOW_LFLINKITEMS)) {
        new FXMenuCommand(ret, "Hide Link Items", nullptr, ret, MID_HIDE_LFLINKITEMS);
    } else {
        new FXMenuCommand(ret, "Show Link Items", nullptr, ret, MID_SHOW_LFLINKITEMS);
    }
    new FXMenuSeparator(ret);
    if (parent.getTrackedID() != getGlID()) {
        new FXMenuCommand(ret, "Start Tracking", nullptr, ret, MID_START_TRACK);
    } else {
        new FXMenuCommand(ret, "Stop Tracking", nullptr, ret, MID_STOP_TRACK);
    }
    new FXMenuCommand(ret, "Select Foes", nullptr, ret, MID_SHOW_FOES);

    new FXMenuCommand(ret, myVehicle.isStopped() ? "Abort stop" : "Stop", nullptr, ret, MID_TOGGLE_STOP);
    new FXMenuCommand(ret, "Remove", nullptr, ret, MID_REMOVE_OBJECT);

    new FXMenuSeparator(ret);
    //
    buildShowParamsPopupEntry(ret, false);
    buildShowTypeParamsPopupEntry(ret);
    buildPositionCopyEntry(ret, false);
    myPopup = ret;
    return ret;
}

void
GUIBaseVehicle::removedPopupMenu() {
    myPopup = nullptr;
}

Boundary
GUIBaseVehicle::getCenteringBoundary() const {
    Boundary b;
    b.add(getPosition());
    b.grow(myVehicle.getVehicleType().getLength());
    return b;
}


const std::string
GUIBaseVehicle::getOptionalName() const {
    return myVehicle.getParameter().getParameter("name", "");
}


void
GUIBaseVehicle::drawOnPos(const GUIVisualizationSettings& s, const Position& pos, const double angle) const {
    glPushName(getGlID());
    glPushMatrix();
    Position p1 = pos;
    const double degAngle = RAD2DEG(angle + M_PI / 2.);
    const double length = getVType().getLength();
    glTranslated(p1.x(), p1.y(), getType());
    glRotated(degAngle, 0, 0, 1);
    // set lane color
    setColor(s);
    // scale
    const double upscale = s.vehicleSize.getExaggeration(s, this);
    double upscaleLength = upscale;
    if (upscale > 1 && length > 5) {
        // reduce the length/width ratio because this is not usefull at high zoom
        const double widthLengthFactor = length / getVType().getWidth();
        const double shrinkFactor = MIN2(widthLengthFactor, sqrt(upscaleLength));
        upscaleLength /= shrinkFactor;
    }
    glScaled(upscale, upscaleLength, 1);
    /*
        MSLCM_DK2004 &m2 = static_cast<MSLCM_DK2004&>(veh->getLaneChangeModel());
        if((m2.getState()&LCA_URGENT)!=0) {
            glColor3d(1, .4, .4);
        } else if((m2.getState()&LCA_SPEEDGAIN)!=0) {
            glColor3d(.4, .4, 1);
        } else {
            glColor3d(.4, 1, .4);
        }
        */
    // draw the vehicle
    bool drawCarriages = false;
    switch (s.vehicleQuality) {
        case 0:
            GUIBaseVehicleHelper::drawAction_drawVehicleAsTrianglePlus(getVType().getWidth(), getVType().getLength());
            break;
        case 1:
            GUIBaseVehicleHelper::drawAction_drawVehicleAsBoxPlus(getVType().getWidth(), getVType().getLength());
            break;
        case 2:
            drawCarriages = drawAction_drawVehicleAsPolyWithCarriagges(s);
            // draw flashing blue light for emergency vehicles
            if (getVType().getGuiShape() == SVS_EMERGENCY) {
                glTranslated(0, 0, .1);
                drawAction_drawVehicleBlueLight();
            }
            break;
        case 3:
        default:
            drawCarriages = drawAction_drawVehicleAsPolyWithCarriagges(s, true);
            break;
    }
    if (s.drawMinGap) {
        const double minGap = -getVType().getMinGap();
        glColor3d(0., 1., 0.);
        glBegin(GL_LINES);
        glVertex2d(0., 0);
        glVertex2d(0., minGap);
        glVertex2d(-.5, minGap);
        glVertex2d(.5, minGap);
        glEnd();
    }
    if (s.drawBrakeGap && !MSGlobals::gUseMesoSim) {
        const double brakeGap = -static_cast<MSVehicle&>(myVehicle).getCarFollowModel().brakeGap(myVehicle.getSpeed());
        glColor3d(1., 0., 0.);
        glBegin(GL_LINES);
        glVertex2d(0., 0);
        glVertex2d(0., brakeGap);
        glVertex2d(-.5, brakeGap);
        glVertex2d(.5, brakeGap);
        glEnd();
    }
    MSDevice_BTreceiver* dev = static_cast<MSDevice_BTreceiver*>(myVehicle.getDevice(typeid(MSDevice_BTreceiver)));
    if (dev != nullptr && s.showBTRange) {
        glColor3d(1., 0., 0.);
        GLHelper::drawOutlineCircle(dev->getRange(), dev->getRange() - .2, 32);
    }
    // draw the blinker and brakelights if wished
    if (s.showBlinker) {
        glTranslated(0, 0, .1);
        switch (getVType().getGuiShape()) {
            case SVS_PEDESTRIAN:
            case SVS_BICYCLE:
            case SVS_ANT:
            case SVS_SHIP:
            case SVS_RAIL:
            case SVS_RAIL_CARGO:
            case SVS_RAIL_CAR:
                break;
            case SVS_MOTORCYCLE:
            case SVS_MOPED:
                drawAction_drawVehicleBlinker(length);
                drawAction_drawVehicleBrakeLight(length, true);
                break;
            default:
                // only SVS_RAIL_CAR has blinkers and brake lights but they are drawn along with the carriages
                if (!drawCarriages) {
                    drawAction_drawVehicleBlinker(length);
                    drawAction_drawVehicleBrakeLight(length);
                }
                break;
        }
    }
    // draw the wish to change the lane
    if (s.drawLaneChangePreference) {
        /*
                if(gSelected.isSelected(GLO_VEHICLE, veh->getGlID())) {
                MSLCM_DK2004 &m = static_cast<MSLCM_DK2004&>(veh->getLaneChangeModel());
                glColor3d(.5, .5, 1);
                glBegin(GL_LINES);
                glVertex2f(0, 0);
                glVertex2f(m.getChangeProbability(), .5);
                glEnd();

                glColor3d(1, 0, 0);
                glBegin(GL_LINES);
                glVertex2f(0.1, 0);
                glVertex2f(0.1, m.myMaxJam1);
                glEnd();

                glColor3d(0, 1, 0);
                glBegin(GL_LINES);
                glVertex2f(-0.1, 0);
                glVertex2f(-0.1, m.myTDist);
                glEnd();
                }
                */
    }
    // draw best lanes
    /*
    if (true) {
        const MSLane &l = veh->getLane();
        double r1 = veh->allowedContinuationsLength(&l, 0);
        double r2 = l.getLeftLane()!=0 ? veh->allowedContinuationsLength(l.getLeftLane(), 0) : 0;
        double r3 = l.getRightLane()!=0 ? veh->allowedContinuationsLength(l.getRightLane(), 0) : 0;
        double mmax = MAX3(r1, r2, r3);
        glBegin(GL_LINES);
        glVertex2f(0, 0);
        glVertex2f(0, r1/mmax/2.);
        glEnd();
        glBegin(GL_LINES);
        glVertex2f(.4, 0);
        glVertex2f(.4, r2/mmax/2.);
        glEnd();
        glBegin(GL_LINES);
        glVertex2f(-.4, 0);
        glVertex2f(-.4, r3/mmax/2.);
        glEnd();
    }
    */
    glTranslated(0, MIN2(length / 2, double(5)), -getType()); // drawing name at GLO_MAX fails unless translating z
    glScaled(1 / upscale, 1 / upscaleLength, 1);
    glRotated(-degAngle, 0, 0, 1);
    drawName(Position(0, 0), s.scale, s.vehicleName, s.angle);
    if (s.vehicleName.show && myVehicle.getParameter().line != "") {
        glRotated(-s.angle, 0, 0, 1);
        glTranslated(0, 0.7 * s.vehicleName.scaledSize(s.scale), 0);
        glRotated(s.angle, 0, 0, 1);
        GLHelper::drawTextSettings(s.vehicleName, "line:" + myVehicle.getParameter().line, Position(0, 0), s.scale, s.angle);
    }
    if (s.vehicleValue.show) {
        glRotated(-s.angle, 0, 0, 1);
        glTranslated(0, 0.7 * s.vehicleName.scaledSize(s.scale), 0);
        glRotated(s.angle, 0, 0, 1);
        const double value = getColorValue(s, s.vehicleColorer.getActive());
        GLHelper::drawTextSettings(s.vehicleValue, toString(value), Position(0, 0), s.scale, s.angle);
    }
    if (s.vehicleText.show) {
        std::string error;
        std::string value = myVehicle.getPrefixedParameter(s.vehicleTextParam, error);
        if (value != "") {
            auto lines = StringTokenizer(value, StringTokenizer::NEWLINE).getVector();
            glRotated(-s.angle, 0, 0, 1);
            glTranslated(0, 0.7 * s.vehicleText.scaledSize(s.scale) * lines.size(), 0);
            glRotated(s.angle, 0, 0, 1);
            for (std::string& line : lines) {
                GLHelper::drawTextSettings(s.vehicleText, line, Position(0, 0), s.scale, s.angle);
                glRotated(-s.angle, 0, 0, 1);
                glTranslated(0, -0.7 * s.vehicleText.scaledSize(s.scale), 0);
                glRotated(s.angle, 0, 0, 1);
            }
        }
    }

    if (!drawCarriages) {
        mySeatPositions.clear();
        myContainerPositions.clear();
        int requiredSeats = getNumPassengers();
        int requiredContainerPositions = getNumContainers();
        const Position back = (p1 + Position(-length * upscaleLength, 0)).rotateAround2D(angle, p1);
        computeSeats(p1, back, SUMO_const_waitingPersonWidth, getVType().getPersonCapacity(), upscale, requiredSeats, mySeatPositions);
        computeSeats(p1, back, SUMO_const_waitingContainerWidth, getVType().getContainerCapacity(), upscale, requiredContainerPositions, myContainerPositions);
    }

    glPopMatrix();
    glPopName();
    drawAction_drawPersonsAndContainers(s);
}


void
GUIBaseVehicle::drawGL(const GUIVisualizationSettings& s) const {
    drawOnPos(s, getPosition(), getAngle());
}


void
GUIBaseVehicle::drawGLAdditional(GUISUMOAbstractView* const parent, const GUIVisualizationSettings& s) const {
    if (!myVehicle.isOnRoad()) {
        drawGL(s);
    }
    glPushName(getGlID());
    glPushMatrix();
    glTranslated(0, 0, getType() - .1); // don't draw on top of other cars
    if (hasActiveAddVisualisation(parent, VO_SHOW_BEST_LANES)) {
        drawBestLanes();
    }
    bool noLoop = hasActiveAddVisualisation(parent, VO_SHOW_ROUTE_NOLOOP);
    if (hasActiveAddVisualisation(parent, VO_SHOW_ROUTE)) {
        drawRoute(s, 0, 0.25, false, noLoop);
    }
    if (hasActiveAddVisualisation(parent, VO_SHOW_FUTURE_ROUTE)) {
        drawRoute(s, 0, 0.25, true, noLoop);
    }
    if (hasActiveAddVisualisation(parent, VO_SHOW_ALL_ROUTES)) {
        if (myVehicle.getNumberReroutes() > 0) {
            const int noReroutePlus1 = myVehicle.getNumberReroutes() + 1;
            for (int i = noReroutePlus1 - 1; i >= 0; i--) {
                double darken = double(0.4) / double(noReroutePlus1) * double(i);
                drawRoute(s, i, darken);
            }
        } else {
            drawRoute(s, 0, 0.25, false, noLoop);
        }
    }
    if (hasActiveAddVisualisation(parent, VO_SHOW_LFLINKITEMS)) {
        drawAction_drawLinkItems(s);
    }
    glPopMatrix();
    glPopName();
}


void
GUIBaseVehicle::drawLinkItem(const Position& pos, SUMOTime arrivalTime, SUMOTime leaveTime, double exagerate) {
    glTranslated(pos.x(), pos.y(), -.1);
    GLHelper::drawFilledCircle(1);
    std::string times = toString(STEPS2TIME(arrivalTime)) + "/" + toString(STEPS2TIME(leaveTime));
    GLHelper::drawText(times.c_str(), Position(), .1, 1.6 * exagerate, RGBColor::GREEN, 0);
    glTranslated(-pos.x(), -pos.y(), .1);
}


RGBColor
GUIBaseVehicle::setColor(const GUIVisualizationSettings& s) const {
    RGBColor col;
    const GUIColorer& c = s.vehicleColorer;
    if (!setFunctionalColor(c.getActive(), &myVehicle, col)) {
        col = c.getScheme().getColor(getColorValue(s, c.getActive()));
    }
    GLHelper::setColor(col);
    return col;
}


bool
GUIBaseVehicle::setFunctionalColor(int activeScheme, const MSBaseVehicle* veh, RGBColor& col) {
    switch (activeScheme) {
        case 0: {
            //test for emergency vehicle
            if (veh->getVehicleType().getGuiShape() == SVS_EMERGENCY) {
                col = RGBColor::WHITE;
                return true;
            }
            //test for firebrigade
            if (veh->getVehicleType().getGuiShape() == SVS_FIREBRIGADE) {
                col = RGBColor::RED;
                return true;
            }
            //test for police car
            if (veh->getVehicleType().getGuiShape() == SVS_POLICE) {
                col = RGBColor::BLUE;
                return true;
            }
            if (veh->getParameter().wasSet(VEHPARS_COLOR_SET)) {
                col = veh->getParameter().color;
                return true;
            }
            if (veh->getVehicleType().wasSet(VTYPEPARS_COLOR_SET)) {
                col = veh->getVehicleType().getColor();
                return true;
            }
            if (&(veh->getRoute().getColor()) != &RGBColor::DEFAULT_COLOR) {
                col = veh->getRoute().getColor();
                return true;
            }
            return false;
        }
        case 2: {
            if (veh->getParameter().wasSet(VEHPARS_COLOR_SET)) {
                col = veh->getParameter().color;
                return true;
            }
            return false;
        }
        case 3: {
            if (veh->getVehicleType().wasSet(VTYPEPARS_COLOR_SET)) {
                col = veh->getVehicleType().getColor();
                return true;
            }
            return false;
        }
        case 4: {
            if (&(veh->getRoute().getColor()) != &RGBColor::DEFAULT_COLOR) {
                col = veh->getRoute().getColor();
                return true;
            }
            return false;
        }
        case 5: {
            Position p = veh->getRoute().getEdges()[0]->getLanes()[0]->getShape()[0];
            const Boundary& b = ((GUINet*) MSNet::getInstance())->getBoundary();
            Position center = b.getCenter();
            double hue = 180. + atan2(center.x() - p.x(), center.y() - p.y()) * 180. / M_PI;
            double sat = p.distanceTo(center) / center.distanceTo(Position(b.xmin(), b.ymin()));
            col = RGBColor::fromHSV(hue, sat, 1.);
            return true;
        }
        case 6: {
            Position p = veh->getRoute().getEdges().back()->getLanes()[0]->getShape()[-1];
            const Boundary& b = ((GUINet*) MSNet::getInstance())->getBoundary();
            Position center = b.getCenter();
            double hue = 180. + atan2(center.x() - p.x(), center.y() - p.y()) * 180. / M_PI;
            double sat = p.distanceTo(center) / center.distanceTo(Position(b.xmin(), b.ymin()));
            col = RGBColor::fromHSV(hue, sat, 1.);
            return true;
        }
        case 7: {
            Position pb = veh->getRoute().getEdges()[0]->getLanes()[0]->getShape()[0];
            Position pe = veh->getRoute().getEdges().back()->getLanes()[0]->getShape()[-1];
            const Boundary& b = ((GUINet*) MSNet::getInstance())->getBoundary();
            double hue = 180. + atan2(pb.x() - pe.x(), pb.y() - pe.y()) * 180. / M_PI;
            Position minp(b.xmin(), b.ymin());
            Position maxp(b.xmax(), b.ymax());
            double sat = pb.distanceTo(pe) / minp.distanceTo(maxp);
            col = RGBColor::fromHSV(hue, sat, 1.);
            return true;
        }
        case 32: { // color randomly (by pointer hash)
            std::hash<const MSBaseVehicle*> ptr_hash;
            const double hue = (double)(ptr_hash(veh) % 360); // [0-360]
            const double sat = ((ptr_hash(veh) / 360) % 67) / 100.0 + 0.33; // [0.33-1]
            col = RGBColor::fromHSV(hue, sat, 1.);
            return true;
        }
        case 33: { // color by angle
            double hue = GeomHelper::naviDegree(veh->getAngle());
            col = RGBColor::fromHSV(hue, 1., 1.);
            return true;
        }
    }
    return false;
}


// ------------ Additional visualisations
bool
GUIBaseVehicle::hasActiveAddVisualisation(GUISUMOAbstractView* const parent, int which) const {
    return myAdditionalVisualizations.find(parent) != myAdditionalVisualizations.end() && (myAdditionalVisualizations.find(parent)->second & which) != 0;
}


void
GUIBaseVehicle::addActiveAddVisualisation(GUISUMOAbstractView* const parent, int which) {
    if (myAdditionalVisualizations.find(parent) == myAdditionalVisualizations.end()) {
        myAdditionalVisualizations[parent] = 0;
    }
    myAdditionalVisualizations[parent] |= which;
    parent->addAdditionalGLVisualisation(this);
}


void
GUIBaseVehicle::removeActiveAddVisualisation(GUISUMOAbstractView* const parent, int which) {
    myAdditionalVisualizations[parent] &= ~which;
    if (myAdditionalVisualizations[parent] == 0) {
        myAdditionalVisualizations.erase(parent);
    }
    parent->removeAdditionalGLVisualisation(this);
}


void
GUIBaseVehicle::drawRoute(const GUIVisualizationSettings& s, int routeNo, double darken, bool future, bool noLoop) const {
    RGBColor darker = setColor(s).changedBrightness((int)(darken * -255));
    GLHelper::setColor(darker);
    if (routeNo == 0) {
        drawRouteHelper(s, myVehicle.getRoute(), future, noLoop, darker);
        return;
    }
    const MSRoute* route = myRoutes->getRoute(routeNo - 1); // only prior routes are stored
    if (route != nullptr) {
        drawRouteHelper(s, *route, future, noLoop, darker);
    }
}


const GUIBaseVehicle::Seat&
GUIBaseVehicle::getSeatPosition(int personIndex) const {
    /// if there are not enough seats in the vehicle people have to squeeze onto the last seat
    return mySeatPositions[MIN2(personIndex, (int)mySeatPositions.size() - 1)];
}

const GUIBaseVehicle::Seat&
GUIBaseVehicle::getContainerPosition(int containerIndex) const {
    /// if there are not enough positions in the vehicle containers have to squeeze onto the last position
    return myContainerPositions[MIN2(containerIndex, (int)myContainerPositions.size() - 1)];
}


void
GUIBaseVehicle::drawAction_drawPersonsAndContainers(const GUIVisualizationSettings& s) const {
    if (myVehicle.myPersonDevice != nullptr) {
        const std::vector<MSTransportable*>& ps = myVehicle.myPersonDevice->getTransportables();
        int personIndex = 0;
        for (std::vector<MSTransportable*>::const_iterator i = ps.begin(); i != ps.end(); ++i) {
            GUIPerson* person = dynamic_cast<GUIPerson*>(*i);
            assert(person != 0);
            person->setPositionInVehicle(getSeatPosition(personIndex++));
            person->drawGL(s);
        }
    }
    if (myVehicle.myContainerDevice != nullptr) {
        const std::vector<MSTransportable*>& cs = myVehicle.myContainerDevice->getTransportables();
        int containerIndex = 0;
        for (std::vector<MSTransportable*>::const_iterator i = cs.begin(); i != cs.end(); ++i) {
            GUIContainer* container = dynamic_cast<GUIContainer*>(*i);
            assert(container != 0);
            container->setPositionInVehicle(getContainerPosition(containerIndex++));
            container->drawGL(s);
        }
    }
#ifdef DRAW_BOUNDING_BOX
    glPushName(getGlID());
    glPushMatrix();
    glTranslated(0, 0, getType());
    PositionVector boundingBox = getBoundingBox();
    boundingBox.push_back(boundingBox.front());
    PositionVector smallBB = getBoundingPoly();
    glColor3d(0, .8, 0);
    GLHelper::drawLine(boundingBox);
    glColor3d(0.5, .8, 0);
    GLHelper::drawLine(smallBB);
    //GLHelper::drawBoxLines(getBoundingBox(), 0.5);
    glPopMatrix();
    glPopName();
#endif
}


bool
GUIBaseVehicle::drawAction_drawVehicleAsPolyWithCarriagges(const GUIVisualizationSettings& s, bool asImage) const {
    if (getVType().getParameter().carriageLength > 0) {
        drawAction_drawCarriageClass(s, asImage);
        return true;
    } else {
        if (asImage && GUIBaseVehicleHelper::drawAction_drawVehicleAsImage(
                    s, getVType().getImgFile(), this, getVType().getWidth(), getVType().getLength())) {
            return false;
        }
        GUIBaseVehicleHelper::drawAction_drawVehicleAsPoly(s, getVType().getGuiShape(), getVType().getWidth(), getVType().getLength());
        return false;
    }
}


int
GUIBaseVehicle::getNumPassengers() const {
    if (myVehicle.getPersonDevice() != nullptr) {
        return (int)myVehicle.getPersonDevice()->size();
    }
    return 0;
}


int
GUIBaseVehicle::getNumContainers() const {
    if (myVehicle.getContainerDevice() != nullptr) {
        return (int)myVehicle.getContainerDevice()->size();
    }
    return 0;
}

std::string
GUIBaseVehicle::getDeviceDescription() {
    std::vector<std::string> devs;
    for (MSDevice* d : myVehicle.getDevices()) {
        devs.push_back(d->deviceName());
    }
    return joinToString(devs, " ");
}


void
GUIBaseVehicle::computeSeats(const Position& front, const Position& back, double seatOffset, int maxSeats, double exaggeration, int& requiredSeats, Seats& into) const {
    if (requiredSeats <= 0) {
        return;
    }
    maxSeats = MAX2(maxSeats, 1); // compute at least one seat
    seatOffset *= exaggeration;
    const double vehWidth = getVType().getWidth() * exaggeration;
    const double length = front.distanceTo2D(back);
    const int rowSize = MAX2(1, (int)floor(vehWidth / seatOffset));
    const double rowOffset = (length - 1) / ceil((double)maxSeats / rowSize);
    const double sideOffset = (rowSize - 1) / 2.0 * seatOffset;
    double rowPos = 1 - rowOffset;
    double angle = back.angleTo2D(front);
    //if (myVehicle.getID() == "v0") std::cout << SIMTIME << " seatOffset=" << seatOffset << " max=" << maxSeats << " ex=" << exaggeration << " req=" << requiredSeats << " rowSize=" << rowSize << " sideOffset=" << sideOffset << " front=" << front << " back=" << back << " a=" << angle << " da=" << RAD2DEG(angle) << "\n";
    for (int i = 0; requiredSeats > 0 && i < maxSeats; i++) {
        int seat = (i % rowSize);
        if (seat == 0) {
            rowPos += rowOffset;
        }
        into.push_back(Seat(PositionVector::positionAtOffset2D(front, back, rowPos, seat * seatOffset - sideOffset), angle));
        requiredSeats--;
    }
}


/****************************************************************************/
