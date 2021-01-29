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
/// @file    GUIViewTraffic.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Christian Roessel
/// @author  Michael Behrisch
/// @author  Andreas Gaubatz
/// @date    Sept 2002
///
// A view on the simulation; this view is a microscopic one
/****************************************************************************/
#include <config.h>

#ifdef HAVE_FFMPEG
#include <utils/gui/div/GUIVideoEncoder.h>
#endif

#include <iostream>
#include <utility>
#include <cmath>
#include <limits>
#include <guisim/GUINet.h>
#include <guisim/GUIEdge.h>
#include <guisim/GUILane.h>
#include <guisim/GUIVehicle.h>
#include <guisim/GUIVehicleControl.h>
#include <microsim/MSGlobals.h>
#include <microsim/MSEdge.h>
#include <microsim/MSLane.h>
#include <microsim/MSJunctionControl.h>
#include <microsim/MSStoppingPlace.h>
#include <microsim/traffic_lights/MSTLLogicControl.h>
#include <microsim/traffic_lights/MSSimpleTrafficLightLogic.h>
#include <utils/common/RGBColor.h>
#include <utils/geom/PositionVector.h>
#include <utils/shapes/ShapeContainer.h>
#include "GUISUMOViewParent.h"
#include "GUIViewTraffic.h"
#include <utils/gui/windows/GUISUMOAbstractView.h>
#include <utils/gui/windows/GUIPerspectiveChanger.h>
#include <utils/gui/windows/GUIAppEnum.h>
#include <utils/foxtools/MFXCheckableButton.h>
#include <utils/gui/images/GUIIconSubSys.h>
#include <gui/GUIApplicationWindow.h>
#include <utils/gui/windows/GUIDialog_ViewSettings.h>
#include <utils/gui/settings/GUICompleteSchemeStorage.h>
#include <utils/foxtools/MFXImageHelper.h>
#include <utils/gui/globjects/GUIGlObjectStorage.h>
#include <foreign/rtree/SUMORTree.h>
#include <utils/gui/div/GLHelper.h>
#include <utils/gui/div/GUIGlobalSelection.h>
#include <utils/gui/globjects/GLIncludes.h>

/* -------------------------------------------------------------------------
 * GUIViewTraffic - FOX callback mapping
 * ----------------------------------------------------------------------- */
FXDEFMAP(GUIViewTraffic) GUIViewTrafficMap[] = {
    FXMAPFUNC(SEL_COMMAND, MID_CLOSE_LANE, GUIViewTraffic::onCmdCloseLane),
    FXMAPFUNC(SEL_COMMAND, MID_CLOSE_EDGE, GUIViewTraffic::onCmdCloseEdge),
    FXMAPFUNC(SEL_COMMAND, MID_ADD_REROUTER, GUIViewTraffic::onCmdAddRerouter),
    FXMAPFUNC(SEL_COMMAND, MID_REACHABILITY, GUIViewTraffic::onCmdShowReachability),
};


FXIMPLEMENT_ABSTRACT(GUIViewTraffic, GUISUMOAbstractView, GUIViewTrafficMap, ARRAYNUMBER(GUIViewTrafficMap))


// ===========================================================================
// member method definitions
// ===========================================================================
GUIViewTraffic::GUIViewTraffic(
    FXComposite* p,
    GUIMainWindow& app,
    GUISUMOViewParent* parent,
    GUINet& net, FXGLVisual* glVis,
    FXGLCanvas* share) :
    GUISUMOAbstractView(p, app, parent, net.getVisualisationSpeedUp(), glVis, share),
    myTrackedID(GUIGlObject::INVALID_ID),
    myTLSGame(OptionsCont::getOptions().getString("game.mode") == "tls")
#ifdef HAVE_FFMPEG
    , myCurrentVideo(nullptr)
#endif
{}


GUIViewTraffic::~GUIViewTraffic() {
    endSnapshot();
}


void
GUIViewTraffic::buildViewToolBars(GUIGlChildWindow* v) {
    // build coloring tools
    {
        const std::vector<std::string>& names = gSchemeStorage.getNames();
        for (std::vector<std::string>::const_iterator i = names.begin(); i != names.end(); ++i) {
            v->getColoringSchemesCombo()->appendItem(i->c_str());
            if ((*i) == myVisualizationSettings->name) {
                v->getColoringSchemesCombo()->setCurrentItem(v->getColoringSchemesCombo()->getNumItems() - 1);
            }
        }
        v->getColoringSchemesCombo()->setNumVisible(MAX2(5, (int)names.size() + 1));
    }
    // for junctions
    new FXButton(v->getLocatorPopup(),
                 "\tLocate Junction\tLocate a junction within the network.",
                 GUIIconSubSys::getIcon(GUIIcon::LOCATEJUNCTION), v, MID_LOCATEJUNCTION,
                 ICON_ABOVE_TEXT | FRAME_THICK | FRAME_RAISED);
    // for edges
    new FXButton(v->getLocatorPopup(),
                 "\tLocate Street\tLocate a street within the network.",
                 GUIIconSubSys::getIcon(GUIIcon::LOCATEEDGE), v, MID_LOCATEEDGE,
                 ICON_ABOVE_TEXT | FRAME_THICK | FRAME_RAISED);
    // for vehicles
    new FXButton(v->getLocatorPopup(),
                 "\tLocate Vehicle\tLocate a vehicle within the network.",
                 GUIIconSubSys::getIcon(GUIIcon::LOCATEVEHICLE), v, MID_LOCATEVEHICLE,
                 ICON_ABOVE_TEXT | FRAME_THICK | FRAME_RAISED);
    // for persons
    new FXButton(v->getLocatorPopup(),
                 "\tLocate Person\tLocate a person within the network.",
                 GUIIconSubSys::getIcon(GUIIcon::LOCATEPERSON), v, MID_LOCATEPERSON,
                 ICON_ABOVE_TEXT | FRAME_THICK | FRAME_RAISED);
    // for containers
    new FXButton(v->getLocatorPopup(),
                 "\tLocate Container\tLocate a container within the network.",
                 GUIIconSubSys::getIcon(GUIIcon::LOCATECONTAINER), v, MID_LOCATECONTAINER,
                 ICON_ABOVE_TEXT | FRAME_THICK | FRAME_RAISED);
    // for tls
    new FXButton(v->getLocatorPopup(),
                 "\tLocate TLS\tLocate a tls within the network.",
                 GUIIconSubSys::getIcon(GUIIcon::LOCATETLS), v, MID_LOCATETLS,
                 ICON_ABOVE_TEXT | FRAME_THICK | FRAME_RAISED);
    // for additional stuff
    new FXButton(v->getLocatorPopup(),
                 "\tLocate Additional\tLocate an additional structure within the network.",
                 GUIIconSubSys::getIcon(GUIIcon::LOCATEADD), v, MID_LOCATEADD,
                 ICON_ABOVE_TEXT | FRAME_THICK | FRAME_RAISED);
    // for pois
    new FXButton(v->getLocatorPopup(),
                 "\tLocate PoI\tLocate a PoI within the network.",
                 GUIIconSubSys::getIcon(GUIIcon::LOCATEPOI), v, MID_LOCATEPOI,
                 ICON_ABOVE_TEXT | FRAME_THICK | FRAME_RAISED);
    // for polygons
    new FXButton(v->getLocatorPopup(),
                 "\tLocate Polygon\tLocate a Polygon within the network.",
                 GUIIconSubSys::getIcon(GUIIcon::LOCATEPOLY), v, MID_LOCATEPOLY,
                 ICON_ABOVE_TEXT | FRAME_THICK | FRAME_RAISED);
}


bool
GUIViewTraffic::setColorScheme(const std::string& name) {
    if (!gSchemeStorage.contains(name)) {
        return false;
    }
    if (myVisualizationChanger != nullptr) {
        if (myVisualizationChanger->getCurrentScheme() != name) {
            myVisualizationChanger->setCurrentScheme(name);
        }
    }
    myVisualizationSettings = &gSchemeStorage.get(name.c_str());
    myVisualizationSettings->gaming = myApp->isGaming();
    update();
    return true;
}


void
GUIViewTraffic::buildColorRainbow(const GUIVisualizationSettings& s, GUIColorScheme& scheme, int active, GUIGlObjectType objectType,
                                  bool hide, double hideThreshold) {
    assert(!scheme.isFixed());
    double minValue = std::numeric_limits<double>::infinity();
    double maxValue = -std::numeric_limits<double>::infinity();
    // retrieve range
    if (objectType == GLO_LANE) {
        // XXX (see #3409) multi-colors are not currently handled. this is a quick hack
        if (active == 22) {
            active = 21; // segment height, fall back to start height
        } else if (active == 24) {
            active = 23; // segment incline, fall back to total incline
        }
        const MSEdgeVector& edges = MSEdge::getAllEdges();
        for (MSEdgeVector::const_iterator it = edges.begin(); it != edges.end(); ++it) {
            if (MSGlobals::gUseMesoSim) {
                const double val = static_cast<GUIEdge*>(*it)->getColorValue(s, active);
                if (val == s.MISSING_DATA) {
                    continue;
                }
                minValue = MIN2(minValue, val);
                maxValue = MAX2(maxValue, val);
            } else {
                const std::vector<MSLane*>& lanes = (*it)->getLanes();
                for (std::vector<MSLane*>::const_iterator it_l = lanes.begin(); it_l != lanes.end(); it_l++) {
                    const double val = static_cast<GUILane*>(*it_l)->getColorValue(s, active);
                    if (val == s.MISSING_DATA) {
                        continue;
                    }
                    minValue = MIN2(minValue, val);
                    maxValue = MAX2(maxValue, val);
                }
            }
        }
    } else if (objectType == GLO_JUNCTION) {
        if (active == 3) {
            std::set<const MSJunction*> junctions;
            for (MSEdge* edge : MSEdge::getAllEdges()) {
                junctions.insert(edge->getFromJunction());
                junctions.insert(edge->getToJunction());
            }
            for (const MSJunction* junction : junctions) {
                minValue = MIN2(minValue, junction->getPosition().z());
                maxValue = MAX2(maxValue, junction->getPosition().z());
            }
        }
    }
    if (scheme.getName() == GUIVisualizationSettings::SCHEME_NAME_PERMISSION_CODE) {
        scheme.clear();
        // add threshold for every distinct value
        std::set<SVCPermissions> codes;
        for (MSEdge* edge : MSEdge::getAllEdges()) {
            for (MSLane* lane : edge->getLanes()) {
                codes.insert(lane->getPermissions());
            }
        }
        int step = MAX2(1, 360 / (int)codes.size());
        int hue = 0;
        for (SVCPermissions p : codes) {
            scheme.addColor(RGBColor::fromHSV(hue, 1, 1), p);
            hue = (hue + step) % 360;
        }
        return;
    }

    if (minValue != std::numeric_limits<double>::infinity()) {
        scheme.clear();
        // add new thresholds
        if (scheme.getName() == GUIVisualizationSettings::SCHEME_NAME_EDGEDATA_NUMERICAL) {
            scheme.addColor(RGBColor(204, 204, 204), std::numeric_limits<double>::max(), "missing data");
        }
        if (hide) {
            const double rawRange = maxValue - minValue;
            minValue = MAX2(hideThreshold + MIN2(1.0, rawRange / 100.0), minValue);
            scheme.addColor(RGBColor(204, 204, 204), hideThreshold);
        }
        double range = maxValue - minValue;
        scheme.addColor(RGBColor::RED, (minValue));
        scheme.addColor(RGBColor::ORANGE, (minValue + range * 1 / 6.0));
        scheme.addColor(RGBColor::YELLOW, (minValue + range * 2 / 6.0));
        scheme.addColor(RGBColor::GREEN, (minValue + range * 3 / 6.0));
        scheme.addColor(RGBColor::CYAN, (minValue + range * 4 / 6.0));
        scheme.addColor(RGBColor::BLUE, (minValue + range * 5 / 6.0));
        scheme.addColor(RGBColor::MAGENTA, (maxValue));
    }
}


std::vector<std::string>
GUIViewTraffic::getEdgeDataAttrs() const {
    if (GUINet::getGUIInstance() != nullptr) {
        return GUINet::getGUIInstance()->getEdgeDataAttrs();
    }
    return std::vector<std::string>();
}


std::vector<std::string>
GUIViewTraffic::getEdgeLaneParamKeys(bool edgeKeys) const {
    std::set<std::string> keys;
    for (const MSEdge* e : MSEdge::getAllEdges()) {
        if (edgeKeys) {
            for (const auto& item : e->getParametersMap()) {
                keys.insert(item.first);
            }
        } else {
            for (const auto lane : e->getLanes()) {
                for (const auto& item : lane->getParametersMap()) {
                    keys.insert(item.first);
                }
            }
        }
    }
    return std::vector<std::string>(keys.begin(), keys.end());
}


std::vector<std::string>
GUIViewTraffic::getVehicleParamKeys(bool /*vTypeKeys*/) const {
    std::set<std::string> keys;
    GUIVehicleControl* vc = GUINet::getGUIInstance()->getGUIVehicleControl();
    vc->secureVehicles();
    for (auto vehIt = vc->loadedVehBegin(); vehIt != vc->loadedVehEnd(); ++vehIt) {
        for (auto kv : vehIt->second->getParameter().getParametersMap()) {
            keys.insert(kv.first);
        }
    }
    vc->releaseVehicles();
    return std::vector<std::string>(keys.begin(), keys.end());
}

std::vector<std::string>
GUIViewTraffic::getPOIParamKeys() const {
    std::set<std::string> keys;
    const ShapeContainer::POIs& pois = static_cast<ShapeContainer&>(GUINet::getInstance()->getShapeContainer()).getPOIs();
    for (auto item : pois) {
        for (auto kv : item.second->getParametersMap()) {
            keys.insert(kv.first);
        }
    }
    return std::vector<std::string>(keys.begin(), keys.end());
}

int
GUIViewTraffic::doPaintGL(int mode, const Boundary& bound) {
    // (uncomment the next line to check select mode)
    //myVisualizationSettings->drawForRectangleSelection = true;
    // init view settings
    glRenderMode(mode);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_ALPHA_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);

    // draw decals (if not in grabbing mode)
    if (!myUseToolTips) {
        drawDecals();
        if (myVisualizationSettings->showGrid) {
            paintGLGrid();
        }
    }

    glLineWidth(1);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    const float minB[2] = { (float)bound.xmin(), (float)bound.ymin() };
    const float maxB[2] = { (float)bound.xmax(), (float)bound.ymax() };
    myVisualizationSettings->scale = myVisualizationSettings->drawForRectangleSelection ? myVisualizationSettings->scale : m2p(SUMO_const_laneWidth);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glEnable(GL_POLYGON_OFFSET_LINE);
    int hits2 = myGrid->Search(minB, maxB, *myVisualizationSettings);
    // Draw additional objects
    if (myAdditionallyDrawn.size() > 0) {
        glTranslated(0, 0, -.01);
        GUINet::getGUIInstance()->lock();
        for (auto i : myAdditionallyDrawn) {
            i.first->drawGLAdditional(this, *myVisualizationSettings);
        }
        GUINet::getGUIInstance()->unlock();
        glTranslated(0, 0, .01);
    }
    glPopMatrix();
    /*
    // draw legends
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslated(1.-.2, 1.-.5, 0.);
    glScaled(.2, .5, 1.);
    GUIColoringSchemesMap<GUILane> &sm = GUIViewTraffic::getLaneSchemesMap(); //!!!
    sm.getColorer(myVisualizationSettings->laneEdgeMode)->drawLegend();
    */
    return hits2;
}


void
GUIViewTraffic::startTrack(int id) {
    myTrackedID = id;
}


void
GUIViewTraffic::stopTrack() {
    myTrackedID = GUIGlObject::INVALID_ID;
}


GUIGlID
GUIViewTraffic::getTrackedID() const {
    return myTrackedID;
}


void
GUIViewTraffic::onGamingClick(Position pos) {
    if (myTLSGame) {
        MSTLLogicControl& tlsControl = MSNet::getInstance()->getTLSControl();
        const MSTrafficLightLogic* minTll = nullptr;
        double minDist = std::numeric_limits<double>::infinity();
        for (const MSTrafficLightLogic* const tll : tlsControl.getAllLogics()) {
            if (tlsControl.isActive(tll) && tll->getProgramID() != "off") {
                // get the links
                const MSTrafficLightLogic::LaneVector& lanes = tll->getLanesAt(0);
                if (lanes.size() > 0) {
                    const Position& endPos = lanes[0]->getShape().back();
                    if (endPos.distanceTo(pos) < minDist) {
                        minDist = endPos.distanceTo(pos);
                        minTll = tll;
                    }
                }
            }
        }
        if (minTll != nullptr) {
            const MSTLLogicControl::TLSLogicVariants& vars = tlsControl.get(minTll->getID());
            const std::vector<MSTrafficLightLogic*> logics = vars.getAllLogics();
            if (logics.size() > 1) {
                MSSimpleTrafficLightLogic* l = (MSSimpleTrafficLightLogic*) logics[0];
                for (int i = 0; i < (int)logics.size() - 1; ++i) {
                    if (minTll->getProgramID() == logics[i]->getProgramID()) {
                        l = (MSSimpleTrafficLightLogic*) logics[i + 1];
                        tlsControl.switchTo(minTll->getID(), l->getProgramID());
                    }
                }
                if (l == logics[0]) {
                    tlsControl.switchTo(minTll->getID(), l->getProgramID());
                }
                l->changeStepAndDuration(tlsControl, MSNet::getInstance()->getCurrentTimeStep(), 0, l->getPhase(0).duration);
                update();
            }
        }
    } else {
        // DRT game
        if (MSGlobals::gUseMesoSim) {
            return;
        }
        const std::set<GUIGlID>& sel = gSelected.getSelected(GLO_VEHICLE);
        if (sel.size() == 0) {
            // find closest pt vehicle
            double minDist = std::numeric_limits<double>::infinity();
            GUIVehicle* closest = nullptr;
            MSVehicleControl& vc = MSNet::getInstance()->getVehicleControl();
            MSVehicleControl::constVehIt it = vc.loadedVehBegin();
            MSVehicleControl::constVehIt end = vc.loadedVehEnd();
            for (it = vc.loadedVehBegin(); it != end; ++it) {
                GUIVehicle* veh = dynamic_cast<GUIVehicle*>(it->second);
                assert(veh != 0);
                if (veh->getParameter().line != "") {
                    const double dist = veh->getPosition().distanceTo2D(pos);
                    if (dist < minDist) {
                        minDist = dist;
                        closest = veh;
                    }
                }
            }
            if (closest != nullptr) {
                gSelected.select(closest->getGlID());
                closest->addActiveAddVisualisation(this, GUIBaseVehicle::VO_SHOW_FUTURE_ROUTE);
            }
        } else {
            // find closest pt stop
            double minDist = std::numeric_limits<double>::infinity();
            MSStoppingPlace* closestStop = nullptr;
            const NamedObjectCont<MSStoppingPlace*>& stops = MSNet::getInstance()->getStoppingPlaces(SUMO_TAG_BUS_STOP);
            for (auto it = stops.begin(); it != stops.end(); ++it) {
                MSStoppingPlace* stop = it->second;
                const double dist = pos.distanceTo2D(stop->getLane().geometryPositionAtOffset(stop->getEndLanePosition()));
                if (dist < minDist) {
                    minDist = dist;
                    closestStop = stop;
                }
            }
            if (closestStop != 0) {
                GUIGlID id = *sel.begin();
                GUIVehicle* veh = dynamic_cast<GUIVehicle*>(GUIGlObjectStorage::gIDStorage.getObjectBlocking(id));
                assert(veh != 0);
                MSLane* lane = veh->getLane();
                lane->getVehiclesSecure();
                veh->rerouteDRTStop(closestStop);
                GUIGlObjectStorage::gIDStorage.unblockObject(id);
                lane->releaseVehicles();
            }
        }
    }
}


void
GUIViewTraffic::onGamingRightClick(Position /*pos*/) {
    const std::set<GUIGlID>& sel = gSelected.getSelected(GLO_VEHICLE);
    if (sel.size() > 0) {
        GUIGlID id = *sel.begin();
        GUIVehicle* veh = dynamic_cast<GUIVehicle*>(GUIGlObjectStorage::gIDStorage.getObjectBlocking(id));
        if (veh != 0) {
            veh->removeActiveAddVisualisation(this, GUIBaseVehicle::VO_SHOW_FUTURE_ROUTE);
        }
        GUIGlObjectStorage::gIDStorage.unblockObject(id);
    }
    gSelected.clear();
}


SUMOTime
GUIViewTraffic::getCurrentTimeStep() const {
    return MSNet::getInstance()->getCurrentTimeStep();
}


GUILane*
GUIViewTraffic::getLaneUnderCursor() {
    if (makeCurrent()) {
        int id = getObjectUnderCursor();
        if (id != 0) {
            GUIGlObject* o = GUIGlObjectStorage::gIDStorage.getObjectBlocking(id);
            if (o != nullptr) {
                return dynamic_cast<GUILane*>(o);
            }
        }
        makeNonCurrent();
    }
    return nullptr;
}

long
GUIViewTraffic::onCmdCloseLane(FXObject*, FXSelector, void*) {
    GUILane* lane = getLaneUnderCursor();
    if (lane != nullptr) {
        lane->closeTraffic();
        GUIGlObjectStorage::gIDStorage.unblockObject(lane->getGlID());
        update();
    }
    return 1;
}


long
GUIViewTraffic::onCmdCloseEdge(FXObject*, FXSelector, void*) {
    GUILane* lane = getLaneUnderCursor();
    if (lane != nullptr) {
        dynamic_cast<GUIEdge*>(&lane->getEdge())->closeTraffic(lane);
        GUIGlObjectStorage::gIDStorage.unblockObject(lane->getGlID());
        update();
    }
    return 1;
}


long
GUIViewTraffic::onCmdAddRerouter(FXObject*, FXSelector, void*) {
    GUILane* lane = getLaneUnderCursor();
    if (lane != nullptr) {
        dynamic_cast<GUIEdge*>(&lane->getEdge())->addRerouter();
        GUIGlObjectStorage::gIDStorage.unblockObject(lane->getGlID());
        update();
    }
    return 1;
}


long
GUIViewTraffic::onCmdShowReachability(FXObject* menu, FXSelector, void*) {
    GUILane* lane = getLaneUnderCursor();
    if (lane != nullptr) {
        // reset
        const double UNREACHABLE = -1;
        gSelected.clear();
        for (const MSEdge* const e : MSEdge::getAllEdges()) {
            for (MSLane* const l : e->getLanes()) {
                GUILane* gLane = dynamic_cast<GUILane*>(l);
                gLane->setReachability(UNREACHABLE);
            }
        }
        // prepare
        FXMenuCommand* mc = dynamic_cast<FXMenuCommand*>(menu);
        const SUMOVehicleClass svc = SumoVehicleClassStrings.get(mc->getText().text());
        const double defaultMaxSpeed = SUMOVTypeParameter::VClassDefaultValues(svc).maxSpeed;
        // find reachable
        std::map<MSEdge*, double> reachableEdges;
        reachableEdges[&lane->getEdge()] = 0;
        MSEdgeVector check;
        check.push_back(&lane->getEdge());
        while (check.size() > 0) {
            MSEdge* e = check.front();
            check.erase(check.begin());
            double traveltime = reachableEdges[e];
            for (MSLane* const l : e->getLanes()) {
                if (l->allowsVehicleClass(svc)) {
                    GUILane* gLane = dynamic_cast<GUILane*>(l);
                    gSelected.select(gLane->getGlID());
                    gLane->setReachability(traveltime);
                }
            }
            traveltime += e->getLength() / MIN2(e->getSpeedLimit(), defaultMaxSpeed);
            for (MSEdge* const nextEdge : e->getSuccessors(svc)) {
                if (reachableEdges.count(nextEdge) == 0 ||
                        // revisit edge via faster path
                        reachableEdges[nextEdge] > traveltime) {
                    reachableEdges[nextEdge] = traveltime;
                    check.push_back(nextEdge);
                }
            }
        }
        // switch to 'color by selection' unless coloring 'by reachability'
        if (myVisualizationSettings->laneColorer.getActive() != 36) {
            myVisualizationSettings->laneColorer.setActive(1);
        }
        update();
    }
    return 1;
}


long
GUIViewTraffic::onDoubleClicked(FXObject*, FXSelector, void*) {
    // leave fullscreen mode
    if (myApp->isFullScreen()) {
        myApp->onCmdFullScreen(nullptr, 0, nullptr);
    } else {
        stopTrack();
    }
    return 1;
}



void
GUIViewTraffic::saveFrame(const std::string& destFile, FXColor* buf) {
#ifdef HAVE_FFMPEG
    if (myCurrentVideo == nullptr) {
        myCurrentVideo = new GUIVideoEncoder(destFile.c_str(), getWidth(), getHeight(), myApp->getDelay());
    }
    myCurrentVideo->writeFrame((uint8_t*)buf);
#else
    UNUSED_PARAMETER(destFile);
    UNUSED_PARAMETER(buf);
#endif
}


void
GUIViewTraffic::endSnapshot() {
#ifdef HAVE_FFMPEG
    if (myCurrentVideo != nullptr) {
        delete myCurrentVideo;
        myCurrentVideo = nullptr;
    }
#endif
}


void
GUIViewTraffic::checkSnapshots() {
#ifdef HAVE_FFMPEG
    if (myCurrentVideo != nullptr) {
        addSnapshot(getCurrentTimeStep() - DELTA_T, "");
    }
#endif
    GUISUMOAbstractView::checkSnapshots();
}


const std::vector<SUMOTime>
GUIViewTraffic::retrieveBreakpoints() const {
    return myApp->retrieveBreakpoints();
}


/****************************************************************************/
