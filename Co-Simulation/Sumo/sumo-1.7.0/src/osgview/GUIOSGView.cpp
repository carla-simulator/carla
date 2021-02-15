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
/// @file    GUIOSGView.cpp
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    19.01.2012
///
// An OSG-based 3D view on the simulation
/****************************************************************************/
#include <config.h>

#ifdef HAVE_OSG

#include <iostream>
#include <utility>
#include <cmath>
#include <limits>
// osg may include windows.h somewhere so we need to guard against macro pollution
#ifdef WIN32
#define NOMINMAX
#pragma warning(push)
#pragma warning(disable: 4127) // do not warn about constant conditional expression
#pragma warning(disable: 4275) // do not warn about the DLL interface for OSG
#endif
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/NodeTrackerManipulator>
#include <osgDB/ReadFile>
#include <osg/PositionAttitudeTransform>
#include <osg/Vec4>
#include <osg/ShapeDrawable>
#ifdef WIN32
#undef NOMINMAX
#pragma warning(pop)
#endif
#include <utils/gui/windows/GUISUMOAbstractView.h>
#include <utils/gui/windows/GUIPerspectiveChanger.h>
#include <utils/gui/windows/GUIAppEnum.h>
#include <utils/foxtools/MFXCheckableButton.h>
#include <utils/gui/images/GUIIconSubSys.h>
#include <gui/GUIApplicationWindow.h>
#include <utils/gui/windows/GUIDialog_ViewSettings.h>
#include <utils/gui/windows/GUIDialog_EditViewport.h>
#include <utils/gui/settings/GUICompleteSchemeStorage.h>
#include <utils/gui/images/GUITexturesHelper.h>
#include <utils/foxtools/MFXImageHelper.h>
#include <utils/gui/globjects/GUIGlObjectStorage.h>
#include <foreign/rtree/SUMORTree.h>
#include <utils/gui/div/GLHelper.h>
#include <guisim/GUINet.h>
#include <guisim/GUIJunctionWrapper.h>
#include <guisim/GUIEdge.h>
#include <guisim/GUILane.h>
#include <guisim/GUIVehicle.h>
#include <microsim/MSEdge.h>
#include <microsim/MSEdgeControl.h>
#include <microsim/MSLane.h>
#include <microsim/MSJunctionControl.h>
#include <microsim/transportables/MSTransportableControl.h>
#include <microsim/MSVehicleControl.h>
#include <microsim/traffic_lights/MSTLLogicControl.h>
#include <microsim/traffic_lights/MSSimpleTrafficLightLogic.h>
#include <utils/common/RGBColor.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/StringUtils.h>
#include <utils/geom/PositionVector.h>
#include <gui/GUISUMOViewParent.h>
#include <utils/gui/globjects/GLIncludes.h>
#include "GUIOSGBuilder.h"
#include "GUIOSGView.h"


FXDEFMAP(GUIOSGView) GUIOSGView_Map[] = {
    //________Message_Type_________        ___ID___                        ________Message_Handler________
    FXMAPFUNC(SEL_CHORE,                MID_CHORE,        GUIOSGView::OnIdle)
};
FXIMPLEMENT(GUIOSGView, GUISUMOAbstractView, GUIOSGView_Map, ARRAYNUMBER(GUIOSGView_Map))


std::ostream&
operator<<(std::ostream& os, const osg::Vec3d& v) {
    return os << v.x() << "," << v.y() << "," << v.z();
}


// ===========================================================================
// GUIOSGView::Command_TLSChange member method definitions
// ===========================================================================
GUIOSGView::Command_TLSChange::Command_TLSChange(const MSLink* const link, osg::Switch* switchNode)
    : myLink(link), mySwitch(switchNode), myLastState(LINKSTATE_TL_OFF_NOSIGNAL) {
    execute();
}


GUIOSGView::Command_TLSChange::~Command_TLSChange() {}


void
GUIOSGView::Command_TLSChange::execute() {
    switch (myLink->getState()) {
        case LINKSTATE_TL_GREEN_MAJOR:
        case LINKSTATE_TL_GREEN_MINOR:
            mySwitch->setSingleChildOn(0);
            break;
        case LINKSTATE_TL_YELLOW_MAJOR:
        case LINKSTATE_TL_YELLOW_MINOR:
            mySwitch->setSingleChildOn(1);
            break;
        case LINKSTATE_TL_RED:
            mySwitch->setSingleChildOn(2);
            break;
        case LINKSTATE_TL_REDYELLOW:
            mySwitch->setSingleChildOn(3);
            break;
        default:
            mySwitch->setAllChildrenOff();
    }
    myLastState = myLink->getState();
}



// ===========================================================================
// GUIOSGView member method definitions
// ===========================================================================
GUIOSGView::GUIOSGView(
    FXComposite* p,
    GUIMainWindow& app,
    GUISUMOViewParent* parent,
    GUINet& net, FXGLVisual* glVis,
    FXGLCanvas* share) :
    GUISUMOAbstractView(p, app, parent, net.getVisualisationSpeedUp(), glVis, share),
    myTracked(0), myCameraManipulator(new SUMOTerrainManipulator()), myLastUpdate(-1) {

    //FXGLVisual* glVisual=new FXGLVisual(getApp(),VISUAL_DOUBLEBUFFER|VISUAL_STEREO);

    //m_gwFox = new GraphicsWindowFOX(this, glVisual, NULL, NULL, LAYOUT_FILL_X|LAYOUT_FILL_Y, x, y, w, h );

    int w = getWidth();
    int h = getHeight();
    myAdapter = new FXOSGAdapter(this, new FXCursor(parent->getApp(), CURSOR_CROSS));

    myViewer = new osgViewer::Viewer();
    myViewer->getCamera()->setGraphicsContext(myAdapter);
    myViewer->getCamera()->setViewport(0, 0, w, h);
    myViewer->setThreadingModel(osgViewer::Viewer::SingleThreaded);

    const char* sumoPath = getenv("SUMO_HOME");
    if (sumoPath != 0) {
        std::string newPath = std::string(sumoPath) + "/data/3D";
        if (FileHelpers::isReadable(newPath)) {
            osgDB::FilePathList path = osgDB::Registry::instance()->getDataFilePathList();
            path.push_back(newPath);
            osgDB::Registry::instance()->setDataFilePathList(path);
        }
    }

    myGreenLight = osgDB::readNodeFile("tlg.obj");
    myYellowLight = osgDB::readNodeFile("tly.obj");
    myRedLight = osgDB::readNodeFile("tlr.obj");
    myRedYellowLight = osgDB::readNodeFile("tlu.obj");
    if (myGreenLight == 0 || myYellowLight == 0 || myRedLight == 0 || myRedYellowLight == 0) {
        WRITE_ERROR("Could not load traffic light files.");
    }
    myRoot = GUIOSGBuilder::buildOSGScene(myGreenLight, myYellowLight, myRedLight, myRedYellowLight);
    // add the stats handler
    myViewer->addEventHandler(new osgViewer::StatsHandler());
    myViewer->setSceneData(myRoot);
    myViewer->setCameraManipulator(myCameraManipulator);
    osg::Vec3d lookFrom, lookAt, up;
    myCameraManipulator->getHomePosition(lookFrom, lookAt, up);
    double z = lookFrom[2];
    lookFrom[2] = -lookFrom.y();
    lookFrom[1] = z;
    myCameraManipulator->setHomePosition(lookFrom, lookAt, up);
    myViewer->home();
    getApp()->addChore(this, MID_CHORE);
}


GUIOSGView::~GUIOSGView() {
    getApp()->removeChore(this, MID_CHORE);
    myViewer->setDone(true);
    myViewer = 0;
    myRoot = 0;
    myAdapter = 0;
}


void
GUIOSGView::buildViewToolBars(GUIGlChildWindow* v) {
    // build coloring tools
    {
        const std::vector<std::string>& names = gSchemeStorage.getNames();
        for (std::vector<std::string>::const_iterator i = names.begin(); i != names.end(); ++i) {
            v->getColoringSchemesCombo()->appendItem(i->c_str());
            if ((*i) == myVisualizationSettings->name) {
                v->getColoringSchemesCombo()->setCurrentItem(v->getColoringSchemesCombo()->getNumItems() - 1);
            }
        }
        v->getColoringSchemesCombo()->setNumVisible(5);
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
                 "\tLocate POI\tLocate a POI within the network.",
                 GUIIconSubSys::getIcon(GUIIcon::LOCATEPOI), v, MID_LOCATEPOI,
                 ICON_ABOVE_TEXT | FRAME_THICK | FRAME_RAISED);
    // for polygons
    new FXButton(v->getLocatorPopup(),
                 "\tLocate Polygon\tLocate a Polygon within the network.",
                 GUIIconSubSys::getIcon(GUIIcon::LOCATEPOLY), v, MID_LOCATEPOLY,
                 ICON_ABOVE_TEXT | FRAME_THICK | FRAME_RAISED);
}


void
GUIOSGView::recenterView() {
    stopTrack();
    Position center = myGrid->getCenter();
    osg::Vec3d lookFromOSG, lookAtOSG, up;
    myViewer->getCameraManipulator()->getHomePosition(lookFromOSG, lookAtOSG, up);
    lookFromOSG[0] = center.x();
    lookFromOSG[1] = center.y();
    lookFromOSG[2] = myChanger->zoom2ZPos(100);
    lookAtOSG[0] = center.x();
    lookAtOSG[1] = center.y();
    lookAtOSG[2] = 0;
    myViewer->getCameraManipulator()->setHomePosition(lookFromOSG, lookAtOSG, up);
    myViewer->home();
}


void
GUIOSGView::centerTo(GUIGlID id, bool /* applyZoom */, double /* zoomDist */) {
    startTrack(id);
}


bool
GUIOSGView::setColorScheme(const std::string& name) {
    if (!gSchemeStorage.contains(name)) {
        return false;
    }
    if (myVisualizationChanger != 0) {
        if (myVisualizationChanger->getCurrentScheme() != name) {
            myVisualizationChanger->setCurrentScheme(name);
        }
    }
    myVisualizationSettings = &gSchemeStorage.get(name.c_str());
    myVisualizationSettings->gaming = myApp->isGaming();
    update();
    return true;
}


long
GUIOSGView::onPaint(FXObject*, FXSelector, void*) {
    if (!isEnabled()) {
        return 1;
    }
    myDecalsLock.lock();
    for (GUISUMOAbstractView::Decal& d : myDecals) {
        if (!d.initialised) {
            if (d.filename.length() == 6 && d.filename.substr(0, 5) == "light") {
                GUIOSGBuilder::buildLight(d, *myRoot);
            } else if (d.filename.length() > 3 && d.filename.substr(0, 3) == "tl:") {
                const int linkStringIdx = (int)d.filename.find(':', 3);
                GUINet* net = (GUINet*) MSNet::getInstance();
                try {
                    MSTLLogicControl::TLSLogicVariants& vars = net->getTLSControl().get(d.filename.substr(3, linkStringIdx - 3));
                    const int linkIdx = StringUtils::toInt(d.filename.substr(linkStringIdx + 1));
                    if (linkIdx < 0 || linkIdx >= static_cast<int>(vars.getActive()->getLinks().size())) {
                        throw NumberFormatException("");
                    }
                    const MSLink* const link = vars.getActive()->getLinksAt(linkIdx)[0];
                    osg::Switch* switchNode = new osg::Switch();
                    switchNode->addChild(GUIOSGBuilder::getTrafficLight(d, d.layer < 0 ? 0 : myGreenLight, osg::Vec4d(0., 1., 0., .3)), false);
                    switchNode->addChild(GUIOSGBuilder::getTrafficLight(d, d.layer < 0 ? 0 : myYellowLight, osg::Vec4d(1., 1., 0., .3)), false);
                    switchNode->addChild(GUIOSGBuilder::getTrafficLight(d, d.layer < 0 ? 0 : myRedLight, osg::Vec4d(1., 0., 0., .3)), false);
                    switchNode->addChild(GUIOSGBuilder::getTrafficLight(d, d.layer < 0 ? 0 : myRedYellowLight, osg::Vec4d(1., .5, 0., .3)), false);
                    myRoot->addChild(switchNode);
                    vars.addSwitchCommand(new Command_TLSChange(link, switchNode));
                } catch (NumberFormatException&) {
                    WRITE_ERROR("Invalid link index in '" + d.filename + "'.");
                } catch (InvalidArgument&) {
                    WRITE_ERROR("Unknown traffic light in '" + d.filename + "'.");
                }
            } else {
                GUIOSGBuilder::buildDecal(d, *myRoot);
            }
            d.initialised = true;
        }
    }
    myDecalsLock.unlock();
    MSVehicleControl::constVehIt it = MSNet::getInstance()->getVehicleControl().loadedVehBegin();
    // reset active flag
    for (auto& item : myVehicles) {
        item.second.active = false;
    }
    for (; it != MSNet::getInstance()->getVehicleControl().loadedVehEnd(); it++) {
        GUIVehicle* veh = static_cast<GUIVehicle*>(it->second);
        if (!(veh->isOnRoad() || veh->isParking() || veh->wasRemoteControlled())) {
            continue;
        }
        auto itVeh = myVehicles.find(veh);
        if (itVeh == myVehicles.end()) {
            myVehicles[veh] = GUIOSGBuilder::buildMovable(veh->getVehicleType());
            myRoot->addChild(myVehicles[veh].pos);
        } else {
            itVeh->second.active = true;
        }
        osg::PositionAttitudeTransform* n = myVehicles[veh].pos;
        n->setPosition(osg::Vec3d(veh->getPosition().x(), veh->getPosition().y(), veh->getPosition().z()));
        const double dir = veh->getAngle() + M_PI / 2.;
        const double slope = veh->getSlope();
        n->setAttitude(osg::Quat(dir, osg::Vec3d(0, 0, 1)) *
                       osg::Quat(osg::DegreesToRadians(slope), osg::Vec3d(0, 1, 0)));
        /*
        osg::ref_ptr<osg::AnimationPath> path = new osg::AnimationPath;
        // path->setLoopMode( osg::AnimationPath::NO_LOOPING );
        osg::AnimationPath::ControlPoint pointA(n->getPosition(), n->getAttitude());
        osg::AnimationPath::ControlPoint pointB(osg::Vec3(veh->getPosition().x(), veh->getPosition().y(), veh->getPosition().z()),
                                                osg::Quat(dir, osg::Vec3(0, 0, 1)) *
                                                osg::Quat(osg::DegreesToRadians(slope), osg::Vec3(0, 1, 0)));
        path->insert(0.0f, pointA);
        path->insert(0.5f, pointB);
        n->setUpdateCallback(new osg::AnimationPathCallback(path));
        */
        const RGBColor& col = myVisualizationSettings->vehicleColorer.getScheme().getColor(veh->getColorValue(*myVisualizationSettings, myVisualizationSettings->vehicleColorer.getActive()));
        myVehicles[veh].geom->setColor(osg::Vec4d(col.red() / 255., col.green() / 255., col.blue() / 255., col.alpha() / 255.));
        myVehicles[veh].lights->setValue(0, veh->signalSet(MSVehicle::VEH_SIGNAL_BLINKER_RIGHT | MSVehicle::VEH_SIGNAL_BLINKER_EMERGENCY));
        myVehicles[veh].lights->setValue(1, veh->signalSet(MSVehicle::VEH_SIGNAL_BLINKER_LEFT | MSVehicle::VEH_SIGNAL_BLINKER_EMERGENCY));
        myVehicles[veh].lights->setValue(2, veh->signalSet(MSVehicle::VEH_SIGNAL_BRAKELIGHT));
    }
    // remove inactive
    for (auto veh = myVehicles.begin(); veh != myVehicles.end();) {
        if (!veh->second.active) {
            removeVeh((veh++)->first);
        } else {
            ++veh;
        }
    }

    const SUMOTime now = MSNet::getInstance()->getCurrentTimeStep();
    if (now != myLastUpdate || (myVisualizationChanger != 0 && myVisualizationChanger->shown())) {
        GUINet::getGUIInstance()->updateColor(*myVisualizationSettings);
    }
    if (now != myLastUpdate && myTracked != 0) {
        osg::Vec3d lookFrom, lookAt, up;
        lookAt[0] = myTracked->getPosition().x();
        lookAt[1] = myTracked->getPosition().y();
        lookAt[2] = myTracked->getPosition().z();
        const double angle = myTracked->getAngle();
        lookFrom[0] = lookAt[0] + 50. * cos(angle);
        lookFrom[1] = lookAt[1] + 50. * sin(angle);
        lookFrom[2] = lookAt[2] + 10.;
        osg::Matrix m;
        m.makeLookAt(lookFrom, lookAt, osg::Z_AXIS);
        myCameraManipulator->setByInverseMatrix(m);
    }

    // reset active flag
    for (auto& item : myPersons) {
        item.second.active = false;
    }
    for (auto transIt = MSNet::getInstance()->getPersonControl().loadedBegin(); transIt != MSNet::getInstance()->getPersonControl().loadedEnd(); ++transIt) {
        MSTransportable* const person = transIt->second;
        // XXX if not departed: continue
        if (person->hasArrived() || !person->hasDeparted()) {
            //std::cout << SIMTIME << " person " << person->getID() << " is loaded but arrived\n";
            continue;
        }
        auto itPers = myPersons.find(person);
        if (itPers == myPersons.end()) {
            myPersons[person] = GUIOSGBuilder::buildMovable(person->getVehicleType());
            myRoot->addChild(myPersons[person].pos);
        } else {
            itPers->second.active = true;
        }
        osg::PositionAttitudeTransform* n = myPersons[person].pos;
        const Position pos = person->getPosition();
        n->setPosition(osg::Vec3d(pos.x(), pos.y(), pos.z()));
        const double dir = person->getAngle() + M_PI / 2.;
        n->setAttitude(osg::Quat(dir, osg::Vec3d(0, 0, 1)));
    }
    // remove inactive
    for (auto person = myPersons.begin(); person != myPersons.end();) {
        if (!person->second.active) {
            removeTransportable((person++)->first);
        } else {
            ++person;
        }
    }


    if (myAdapter->makeCurrent()) {
        myViewer->frame();
        makeNonCurrent();
    }
    myLastUpdate = now;
    return 1;
}


void
GUIOSGView::removeVeh(MSVehicle* veh) {
    if (myTracked == veh) {
        stopTrack();
    }
    std::map<MSVehicle*, OSGMovable>::iterator i = myVehicles.find(veh);
    if (i != myVehicles.end()) {
        myRoot->removeChild(i->second.pos);
        myVehicles.erase(i);
    }
}


void
GUIOSGView::removeTransportable(MSTransportable* t) {
    std::map<MSTransportable*, OSGMovable>::iterator i = myPersons.find(t);
    if (i != myPersons.end()) {
        myRoot->removeChild(i->second.pos);
        myPersons.erase(i);
    }
}


void
GUIOSGView::showViewportEditor() {
    getViewportEditor(); // make sure it exists;
    osg::Vec3d lookFromOSG, lookAtOSG, up;
    myViewer->getCameraManipulator()->getInverseMatrix().getLookAt(lookFromOSG, lookAtOSG, up);
    Position from(lookFromOSG[0], lookFromOSG[1], lookFromOSG[2]), at(lookAtOSG[0], lookAtOSG[1], lookAtOSG[2]);
    myViewportChooser->setOldValues(from, at, 0);
    myViewportChooser->show();
}


void
GUIOSGView::setViewportFromToRot(const Position& lookFrom, const Position& lookAt, double /*rotation*/) {
    osg::Vec3d lookFromOSG, lookAtOSG, up;
    myViewer->getCameraManipulator()->getHomePosition(lookFromOSG, lookAtOSG, up);
    lookFromOSG[0] = lookFrom.x();
    lookFromOSG[1] = lookFrom.y();
    lookFromOSG[2] = lookFrom.z();
    lookAtOSG[0] = lookAt.x();
    lookAtOSG[1] = lookAt.y();
    lookAtOSG[2] = lookAt.z();
    myViewer->getCameraManipulator()->setHomePosition(lookFromOSG, lookAtOSG, up);
    myViewer->home();
}



void
GUIOSGView::copyViewportTo(GUISUMOAbstractView* view) {
    osg::Vec3d lookFrom, lookAt, up;
    myCameraManipulator->getHomePosition(lookFrom, lookAt, up);
    view->setViewportFromToRot(Position(lookFrom[0], lookFrom[1], lookFrom[2]),
                               Position(lookAt[0], lookAt[1], lookAt[2]), 0);
}



void
GUIOSGView::startTrack(int id) {
    if (myTracked == 0 || (int)myTracked->getGlID() != id) {
        myTracked = 0;
        MSVehicleControl::constVehIt it = MSNet::getInstance()->getVehicleControl().loadedVehBegin();
        for (; it != MSNet::getInstance()->getVehicleControl().loadedVehEnd(); it++) {
            GUIVehicle* veh = (GUIVehicle*)(*it).second;
            if ((int)veh->getGlID() == id) {
                if (!veh->isOnRoad() || myVehicles.find(veh) == myVehicles.end()) {
                    return;
                }
                myTracked = veh;
                break;
            }
        }
        if (myTracked != 0) {
            osg::Vec3d lookFrom, lookAt, up;
            lookAt[0] = myTracked->getPosition().x();
            lookAt[1] = myTracked->getPosition().y();
            lookAt[2] = myTracked->getPosition().z();
            lookFrom[0] = lookAt[0] + 50.;
            lookFrom[1] = lookAt[1] + 50.;
            lookFrom[2] = lookAt[2] + 10.;
            osg::Matrix m;
            m.makeLookAt(lookFrom, lookAt, osg::Z_AXIS);
            myCameraManipulator->setByInverseMatrix(m);
        }
    }
}


void
GUIOSGView::stopTrack() {
    myTracked = 0;
}


GUIGlID
GUIOSGView::getTrackedID() const {
    return myTracked == 0 ? GUIGlObject::INVALID_ID : myTracked->getGlID();
}


void
GUIOSGView::onGamingClick(Position pos) {
    MSTLLogicControl& tlsControl = MSNet::getInstance()->getTLSControl();
    const MSTrafficLightLogic* minTll = nullptr;
    double minDist = std::numeric_limits<double>::infinity();
    for (const MSTrafficLightLogic* const tll : tlsControl.getAllLogics()) {
        if (tlsControl.isActive(tll)) {
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
    if (minTll != 0) {
        const MSTLLogicControl::TLSLogicVariants& vars = tlsControl.get(minTll->getID());
        const std::vector<MSTrafficLightLogic*> logics = vars.getAllLogics();
        if (logics.size() > 1) {
            MSSimpleTrafficLightLogic* l = (MSSimpleTrafficLightLogic*) logics[0];
            for (int i = 0; i < (int)logics.size() - 1; i++) {
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
}


SUMOTime
GUIOSGView::getCurrentTimeStep() const {
    return MSNet::getInstance()->getCurrentTimeStep();
}


long GUIOSGView::onConfigure(FXObject* sender, FXSelector sel, void* ptr) {
    // update the window dimensions, in case the window has been resized.
    myAdapter->getEventQueue()->windowResize(0, 0, getWidth(), getHeight());
    myAdapter->resized(0, 0, getWidth(), getHeight());

    return FXGLCanvas::onConfigure(sender, sel, ptr);
}

long GUIOSGView::onKeyPress(FXObject* sender, FXSelector sel, void* ptr) {
    int key = ((FXEvent*)ptr)->code;
    myAdapter->getEventQueue()->keyPress(key);

    return FXGLCanvas::onKeyPress(sender, sel, ptr);
}

long GUIOSGView::onKeyRelease(FXObject* sender, FXSelector sel, void* ptr) {
    int key = ((FXEvent*)ptr)->code;
    myAdapter->getEventQueue()->keyRelease(key);

    return FXGLCanvas::onKeyRelease(sender, sel, ptr);
}

long GUIOSGView::onLeftBtnPress(FXObject* sender, FXSelector sel, void* ptr) {
    handle(this, FXSEL(SEL_FOCUS_SELF, 0), ptr);

    FXEvent* event = (FXEvent*)ptr;
    myAdapter->getEventQueue()->mouseButtonPress((float)event->click_x, (float)event->click_y, 1);
    if (myApp->isGaming()) {
        onGamingClick(getPositionInformation());
    }

    return FXGLCanvas::onLeftBtnPress(sender, sel, ptr);
}

long GUIOSGView::onLeftBtnRelease(FXObject* sender, FXSelector sel, void* ptr) {
    FXEvent* event = (FXEvent*)ptr;
    myAdapter->getEventQueue()->mouseButtonRelease((float)event->click_x, (float)event->click_y, 1);

    return FXGLCanvas::onLeftBtnRelease(sender, sel, ptr);
}

long GUIOSGView::onMiddleBtnPress(FXObject* sender, FXSelector sel, void* ptr) {
    handle(this, FXSEL(SEL_FOCUS_SELF, 0), ptr);

    FXEvent* event = (FXEvent*)ptr;
    myAdapter->getEventQueue()->mouseButtonPress((float)event->click_x, (float)event->click_y, 2);

    return FXGLCanvas::onMiddleBtnPress(sender, sel, ptr);
}

long GUIOSGView::onMiddleBtnRelease(FXObject* sender, FXSelector sel, void* ptr) {
    FXEvent* event = (FXEvent*)ptr;
    myAdapter->getEventQueue()->mouseButtonRelease((float)event->click_x, (float)event->click_y, 2);

    return FXGLCanvas::onMiddleBtnRelease(sender, sel, ptr);
}

long GUIOSGView::onRightBtnPress(FXObject* sender, FXSelector sel, void* ptr) {
    handle(this, FXSEL(SEL_FOCUS_SELF, 0), ptr);

    FXEvent* event = (FXEvent*)ptr;
    myAdapter->getEventQueue()->mouseButtonPress((float)event->click_x, (float)event->click_y, 3);

    return FXGLCanvas::onRightBtnPress(sender, sel, ptr);
}

long GUIOSGView::onRightBtnRelease(FXObject* sender, FXSelector sel, void* ptr) {
    FXEvent* event = (FXEvent*)ptr;
    myAdapter->getEventQueue()->mouseButtonRelease((float)event->click_x, (float)event->click_y, 3);

    return FXGLCanvas::onRightBtnRelease(sender, sel, ptr);
}

long
GUIOSGView::onMouseMove(FXObject* sender, FXSelector sel, void* ptr) {
    FXEvent* event = (FXEvent*)ptr;
    myAdapter->getEventQueue()->mouseMotion((float)event->win_x, (float)event->win_y);

    return FXGLCanvas::onMotion(sender, sel, ptr);
}

long
GUIOSGView::OnIdle(FXObject* /* sender */, FXSelector /* sel */, void*) {
    forceRefresh();
    update();
    getApp()->addChore(this, MID_CHORE);
    return 1;
}



GUIOSGView::FXOSGAdapter::FXOSGAdapter(GUISUMOAbstractView* parent, FXCursor* cursor)
    : myParent(parent), myOldCursor(cursor) {
    _traits = new GraphicsContext::Traits();
    _traits->x = 0;
    _traits->y = 0;
    _traits->width = parent->getWidth();
    _traits->height = parent->getHeight();
    _traits->windowDecoration = false;
    _traits->doubleBuffer = true;
    _traits->sharedContext = 0;
    if (valid()) {
        setState(new osg::State());
        getState()->setGraphicsContext(this);
        if (_traits.valid() && _traits->sharedContext != 0) {
            getState()->setContextID(_traits->sharedContext->getState()->getContextID());
            incrementContextIDUsageCount(getState()->getContextID());
        } else {
            getState()->setContextID(createNewContextID());
        }
    }
}


GUIOSGView::FXOSGAdapter::~FXOSGAdapter() {
    delete myOldCursor;
}


void GUIOSGView::FXOSGAdapter::grabFocus() {
    // focus this window
    myParent->setFocus();
}

void GUIOSGView::FXOSGAdapter::useCursor(bool cursorOn) {
    if (cursorOn) {
        myParent->setDefaultCursor(myOldCursor);
    } else {
        myParent->setDefaultCursor(NULL);
    }
}

bool GUIOSGView::FXOSGAdapter::makeCurrentImplementation() {
    myParent->makeCurrent();
    return true;
}

bool GUIOSGView::FXOSGAdapter::releaseContext() {
    myParent->makeNonCurrent();
    return true;
}

void GUIOSGView::FXOSGAdapter::swapBuffersImplementation() {
    myParent->swapBuffers();
}


#endif


/****************************************************************************/
