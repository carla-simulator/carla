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
/// @file    GUINet.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @author  Laura Bieker
/// @date    Sept 2002
///
// A MSNet extended by some values for usage within the gui
/****************************************************************************/
#include <config.h>

#include <utility>
#include <set>
#include <vector>
#include <map>
#include <utils/shapes/ShapeContainer.h>
#include <utils/gui/globjects/GUIPolygon.h>
#include <utils/gui/globjects/GUIPointOfInterest.h>
#include <utils/gui/globjects/GUIGLObjectPopupMenu.h>
#include <utils/gui/div/GUIParameterTableWindow.h>
#include <utils/gui/div/GUIGlobalSelection.h>
#include <utils/gui/globjects/GUIShapeContainer.h>
#include <utils/xml/XMLSubSys.h>
#include <utils/common/StringUtils.h>
#include <utils/common/RGBColor.h>
#include <utils/gui/div/GLObjectValuePassConnector.h>
#include <microsim/MSNet.h>
#include <microsim/MSEdgeWeightsStorage.h>
#include <microsim/MSJunction.h>
#include <microsim/output/MSDetectorControl.h>
#include <microsim/MSEdge.h>
#include <microsim/transportables/MSPModel.h>
#include <microsim/MSInsertionControl.h>
#include <microsim/traffic_lights/MSTrafficLightLogic.h>
#include <microsim/traffic_lights/MSTLLogicControl.h>
#include <microsim/MSJunctionControl.h>
#include <guisim/GUIEdge.h>
#include <guisim/GUILane.h>
#include <guisim/GUITransportableControl.h>
#include <guisim/GUILaneSpeedTrigger.h>
#include <guisim/GUIDetectorWrapper.h>
#include <guisim/GUICalibrator.h>
#include <guisim/GUITrafficLightLogicWrapper.h>
#include <guisim/GUIJunctionWrapper.h>
#include <guisim/GUIVehicleControl.h>
#include <gui/GUIGlobals.h>
#include "GUINet.h"

#include <mesogui/GUIMEVehicleControl.h>


// ===========================================================================
// definition of static variables used for visualisation of objects' values
// ===========================================================================
template std::vector< GLObjectValuePassConnector<double>* > GLObjectValuePassConnector<double>::myContainer;
template FXMutex GLObjectValuePassConnector<double>::myLock;

template std::vector< GLObjectValuePassConnector<std::pair<int, class MSPhaseDefinition> >* > GLObjectValuePassConnector<std::pair<int, class MSPhaseDefinition> >::myContainer;
template FXMutex GLObjectValuePassConnector<std::pair<int, class MSPhaseDefinition> >::myLock;


// ===========================================================================
// member method definitions
// ===========================================================================
GUINet::GUINet(MSVehicleControl* vc, MSEventControl* beginOfTimestepEvents,
               MSEventControl* endOfTimestepEvents,
               MSEventControl* insertionEvents) :
    MSNet(vc, beginOfTimestepEvents, endOfTimestepEvents, insertionEvents, new GUIShapeContainer(myGrid)),
    GUIGlObject(GLO_NETWORK, ""),
    myLastSimDuration(0), /*myLastVisDuration(0),*/ myLastIdleDuration(0),
    myLastVehicleMovementCount(0), myOverallVehicleCount(0), myOverallSimDuration(0) {
    GUIGlObjectStorage::gIDStorage.setNetObject(this);
}


GUINet::~GUINet() {
    if (myLock.locked()) {
        myLock.unlock();
    }
    // delete allocated wrappers
    //  of junctions
    for (std::vector<GUIJunctionWrapper*>::iterator i1 = myJunctionWrapper.begin(); i1 != myJunctionWrapper.end(); i1++) {
        delete (*i1);
    }
    //  of additional structures
    GUIGlObject_AbstractAdd::clearDictionary();
    //  of tl-logics
    for (Logics2WrapperMap::iterator i3 = myLogics2Wrapper.begin(); i3 != myLogics2Wrapper.end(); i3++) {
        delete (*i3).second;
    }
    //  of detectors
    for (std::vector<GUIDetectorWrapper*>::iterator i = myDetectorWrapper.begin(); i != myDetectorWrapper.end(); ++i) {
        delete *i;
    }
    //  of calibrators
    for (GUICalibrator* cw : myCalibratorWrapper) {
        delete cw;
    }
    for (auto& item : myLoadedEdgeData) {
        delete item.second;
    }
}


const Boundary&
GUINet::getBoundary() const {
    return myBoundary;
}


MSTransportableControl&
GUINet::getPersonControl() {
    if (myPersonControl == nullptr) {
        myPersonControl = new GUITransportableControl(true);
    }
    return *myPersonControl;
}


MSTransportableControl&
GUINet::getContainerControl() {
    if (myContainerControl == nullptr) {
        myContainerControl = new GUITransportableControl(false);
    }
    return *myContainerControl;
}


void
GUINet::initTLMap() {
    // get the list of loaded tl-logics
    const std::vector<MSTrafficLightLogic*>& logics = getTLSControl().getAllLogics();
    // go through the logics
    for (std::vector<MSTrafficLightLogic*>::const_iterator i = logics.begin(); i != logics.end(); ++i) {
        createTLWrapper(*i);
    }
}


void
GUINet::createTLWrapper(MSTrafficLightLogic* tll) {
    if (myLogics2Wrapper.count(tll) > 0) {
        return;
    }
    // get the links
    const MSTrafficLightLogic::LinkVectorVector& links = tll->getLinks();
    if (links.size() == 0) { // @legacy this should never happen in 0.13.0+ networks
        return;
    }
    // build the wrapper
    GUITrafficLightLogicWrapper* tllw =
        new GUITrafficLightLogicWrapper(*myLogics, *tll);
    // build the association link->wrapper
    MSTrafficLightLogic::LinkVectorVector::const_iterator j;
    for (j = links.begin(); j != links.end(); ++j) {
        MSTrafficLightLogic::LinkVector::const_iterator j2;
        for (j2 = (*j).begin(); j2 != (*j).end(); ++j2) {
            myLinks2Logic[*j2] = tll->getID();
        }
    }
    myGrid.addAdditionalGLObject(tllw);
    myLogics2Wrapper[tll] = tllw;
    return;
}


Position
GUINet::getJunctionPosition(const std::string& name) const {
    // !!! no check for existance!
    return myJunctions->get(name)->getPosition();
}


bool
GUINet::vehicleExists(const std::string& name) const {
    return myVehicleControl->getVehicle(name) != nullptr;
}


int
GUINet::getLinkTLID(MSLink* link) const {
    if (myLinks2Logic.count(link) == 0) {
        assert(false);
        return 0;
    }
    MSTrafficLightLogic* tll = myLogics->getActive(myLinks2Logic.find(link)->second);
    if (myLogics2Wrapper.count(tll) == 0) {
        // tll may have been added via traci. @see ticket #459
        return 0;
    }
    return myLogics2Wrapper.find(tll)->second->getGlID();
}


int
GUINet::getLinkTLIndex(MSLink* link) const {
    Links2LogicMap::const_iterator i = myLinks2Logic.find(link);
    if (i == myLinks2Logic.end()) {
        return -1;
    }
    if (myLogics2Wrapper.find(myLogics->getActive((*i).second)) == myLogics2Wrapper.end()) {
        return -1;
    }
    return myLogics2Wrapper.find(myLogics->getActive((*i).second))->second->getLinkIndex(link);
}


void
GUINet::guiSimulationStep() {
    GLObjectValuePassConnector<double>::updateAll();
    GLObjectValuePassConnector<std::pair<SUMOTime, MSPhaseDefinition> >::updateAll();
}


void
GUINet::simulationStep() {
    FXMutexLock locker(myLock);
    MSNet::simulationStep();
}


std::vector<GUIGlID>
GUINet::getJunctionIDs(bool includeInternal) const {
    std::vector<GUIGlID> ret;
    for (std::vector<GUIJunctionWrapper*>::const_iterator i = myJunctionWrapper.begin(); i != myJunctionWrapper.end(); ++i) {
        if (!(*i)->isInternal() || includeInternal) {
            ret.push_back((*i)->getGlID());
        }
    }
    return ret;
}


std::vector<GUIGlID>
GUINet::getTLSIDs() const {
    std::vector<GUIGlID> ret;
    std::vector<std::string> ids;
    for (std::map<MSTrafficLightLogic*, GUITrafficLightLogicWrapper*>::const_iterator i = myLogics2Wrapper.begin(); i != myLogics2Wrapper.end(); ++i) {
        std::string sid = (*i).second->getMicrosimID();
        if (find(ids.begin(), ids.end(), sid) == ids.end()) {
            ret.push_back((*i).second->getGlID());
            ids.push_back(sid);
        }
    }
    return ret;
}


void
GUINet::initGUIStructures() {
    // initialise detector storage for gui
    const std::vector<SumoXMLTag> types = myDetectorControl->getAvailableTypes();
    for (std::vector<SumoXMLTag>::const_iterator i = types.begin(); i != types.end(); ++i) {
        for (const auto& j : myDetectorControl->getTypedDetectors(*i)) {
            GUIDetectorWrapper* wrapper = j.second->buildDetectorGUIRepresentation();
            if (wrapper != nullptr) {
                myDetectorWrapper.push_back(wrapper);
                myGrid.addAdditionalGLObject(wrapper);
            }
        }
    }
    // initialise calibrators
    for (auto& item : MSCalibrator::getInstances()) {
        GUICalibrator* wrapper = new GUICalibrator(item.second);
        myCalibratorWrapper.push_back(wrapper);
        myGrid.addAdditionalGLObject(wrapper);
    }
    // initialise the tl-map
    initTLMap();
    // initialise edge storage for gui
    const MSEdgeVector& edges = MSEdge::getAllEdges();
    myEdgeWrapper.reserve(edges.size());
    for (MSEdgeVector::const_iterator i = edges.begin(); i != edges.end(); ++i) {
        // VISIM connector edges shall be drawn (they have lanes)
        if (!(*i)->isTazConnector() || (*i)->getLanes().size() > 0) {
            myEdgeWrapper.push_back(static_cast<GUIEdge*>(*i));
        }
    }
    // initialise junction storage for gui
    int size = myJunctions->size();
    myJunctionWrapper.reserve(size);
    std::map<MSJunction*, std::string> junction2TLL;
    for (const auto tls : getTLSControl().getAllLogics()) {
        for (const auto& links : tls->getLinks()) {
            for (const MSLink* l : links) {
                junction2TLL[l->getJunction()] = l->getTLLogic()->getID();
            }
        }
    }
    for (const auto& i : *myJunctions) {
        myJunctionWrapper.push_back(new GUIJunctionWrapper(*i.second, junction2TLL[i.second]));
    }
    // build the visualization tree
    for (std::vector<GUIEdge*>::iterator i = myEdgeWrapper.begin(); i != myEdgeWrapper.end(); ++i) {
        GUIEdge* edge = *i;
        Boundary b;
        const std::vector<MSLane*>& lanes = edge->getLanes();
        for (std::vector<MSLane*>::const_iterator j = lanes.begin(); j != lanes.end(); ++j) {
            b.add((*j)->getShape().getBoxBoundary());
        }
        // make sure persons are always drawn and selectable since they depend on their edge being drawn
        b.grow(MSPModel::SIDEWALK_OFFSET + 1);
        const float cmin[2] = { (float)b.xmin(), (float)b.ymin() };
        const float cmax[2] = { (float)b.xmax(), (float)b.ymax() };
        myGrid.Insert(cmin, cmax, edge);
        myBoundary.add(b);
        if (myBoundary.getWidth() > 10e16 || myBoundary.getHeight() > 10e16) {
            throw ProcessError("Network size exceeds 1 Lightyear. Please reconsider your inputs.\n");
        }
    }
    for (std::vector<GUIJunctionWrapper*>::iterator i = myJunctionWrapper.begin(); i != myJunctionWrapper.end(); ++i) {
        GUIJunctionWrapper* junction = *i;
        Boundary b = junction->getBoundary();
        b.grow(2.);
        const float cmin[2] = { (float)b.xmin(), (float)b.ymin() };
        const float cmax[2] = { (float)b.xmax(), (float)b.ymax() };
        myGrid.Insert(cmin, cmax, junction);
        myBoundary.add(b);
    }
    myGrid.add(myBoundary);
}


int
GUINet::getWholeDuration() const {
    return myLastSimDuration +/*myLastVisDuration+*/myLastIdleDuration;
}


int
GUINet::getSimDuration() const {
    return myLastSimDuration;
}

/*
int
GUINet::getVisDuration() const
{
    return myLastVisDuration;
}
*/


double
GUINet::getRTFactor() const {
    if (myLastSimDuration == 0) {
        return -1;
    }
    return (double)DELTA_T / (double)myLastSimDuration;
}


double
GUINet::getUPS() const {
    if (myLastSimDuration == 0) {
        return -1;
    }
    return (double) myLastVehicleMovementCount / (double) myLastSimDuration * (double) 1000.;
}


double
GUINet::getMeanRTFactor(int duration) const {
    if (myOverallSimDuration == 0) {
        return -1;
    }
    return ((double)(duration) * (double) 1000. / (double)myOverallSimDuration);
}


double
GUINet::getMeanUPS() const {
    if (myOverallSimDuration == 0) {
        return -1;
    }
    return ((double)myVehiclesMoved / (double)myOverallSimDuration * (double) 1000.);
}


int
GUINet::getIdleDuration() const {
    return myLastIdleDuration;
}


void
GUINet::setSimDuration(int val) {
    myLastSimDuration = val;
    myOverallSimDuration += val;
    myLastVehicleMovementCount = getVehicleControl().getRunningVehicleNo();
    myOverallVehicleCount += myLastVehicleMovementCount;
}

/*
void
GUINet::setVisDuration(int val)
{
    myLastVisDuration = val;
}
*/

void
GUINet::setIdleDuration(int val) {
    myLastIdleDuration = val;
}


GUIGLObjectPopupMenu*
GUINet::getPopUpMenu(GUIMainWindow& app,
                     GUISUMOAbstractView& parent) {
    GUIGLObjectPopupMenu* ret = new GUIGLObjectPopupMenu(app, parent, *this);
    buildPopupHeader(ret, app);
    buildCenterPopupEntry(ret);
    buildShowParamsPopupEntry(ret);
    buildPositionCopyEntry(ret, false);
    return ret;
}


GUIParameterTableWindow*
GUINet::getParameterWindow(GUIMainWindow& app,
                           GUISUMOAbstractView& parent) {
    GUIParameterTableWindow* ret = new GUIParameterTableWindow(app, *this);
    // add items
    ret->mkItem("loaded vehicles [#]", true,
                new FunctionBinding<MSVehicleControl, int>(&getVehicleControl(), &MSVehicleControl::getLoadedVehicleNo));
    ret->mkItem("insertion-backlogged vehicles [#]", true,
                new FunctionBinding<MSInsertionControl, int>(&getInsertionControl(), &MSInsertionControl::getWaitingVehicleNo));
    ret->mkItem("departed vehicles [#]", true,
                new FunctionBinding<MSVehicleControl, int>(&getVehicleControl(), &MSVehicleControl::getDepartedVehicleNo));
    ret->mkItem("running vehicles [#]", true,
                new FunctionBinding<MSVehicleControl, int>(&getVehicleControl(), &MSVehicleControl::getRunningVehicleNo));
    ret->mkItem("arrived vehicles [#]", true,
                new FunctionBinding<MSVehicleControl, int>(&getVehicleControl(), &MSVehicleControl::getArrivedVehicleNo));
    ret->mkItem("discarded vehicles [#]", true,
                new FunctionBinding<MSVehicleControl, int>(&getVehicleControl(), &MSVehicleControl::getDiscardedVehicleNo));
    ret->mkItem("collisions [#]", true,
                new FunctionBinding<MSVehicleControl, int>(&getVehicleControl(), &MSVehicleControl::getCollisionCount));
    ret->mkItem("teleports [#]", true,
                new FunctionBinding<MSVehicleControl, int>(&getVehicleControl(), &MSVehicleControl::getTeleportCount));
    ret->mkItem("halting [#]", true,
                new FunctionBinding<MSVehicleControl, int>(&getVehicleControl(), &MSVehicleControl::getHaltingVehicleNo));
    ret->mkItem("avg. speed [m/s]", true,
                new FunctionBinding<MSVehicleControl, double>(&getVehicleControl(), &MSVehicleControl::getVehicleMeanSpeed));
    ret->mkItem("avg. relative speed", true,
                new FunctionBinding<MSVehicleControl, double>(&getVehicleControl(), &MSVehicleControl::getVehicleMeanSpeedRelative));
    if (myPersonControl != nullptr) {
        ret->mkItem("loaded persons [#]", true,
                    new FunctionBinding<MSTransportableControl, int>(&getPersonControl(), &MSTransportableControl::getLoadedNumber));
        ret->mkItem("running persons [#]", true,
                    new FunctionBinding<MSTransportableControl, int>(&getPersonControl(), &MSTransportableControl::getRunningNumber));
        ret->mkItem("jammed persons [#]", true,
                    new FunctionBinding<MSTransportableControl, int>(&getPersonControl(), &MSTransportableControl::getJammedNumber));
    }
    ret->mkItem("end time [s]", false, OptionsCont::getOptions().getString("end"));
    ret->mkItem("begin time [s]", false, OptionsCont::getOptions().getString("begin"));
//    ret->mkItem("time step [s]", true, new FunctionBinding<GUINet, SUMOTime>(this, &GUINet::getCurrentTimeStep));
    if (logSimulationDuration()) {
        ret->mkItem("step duration [ms]", true, new FunctionBinding<GUINet, int>(this, &GUINet::getWholeDuration));
        ret->mkItem("FPS", true, new FunctionBinding<GUISUMOAbstractView, double>(&parent, &GUISUMOAbstractView::getFPS));
        ret->mkItem("simulation duration [ms]", true, new FunctionBinding<GUINet, int>(this, &GUINet::getSimDuration));
        /*
        ret->mkItem("visualisation duration [ms]", true,
            new CastingFunctionBinding<GUINet, double, int>(
                &(getNet()), &GUINet::getVisDuration));
        */
        ret->mkItem("idle duration [ms]", true, new FunctionBinding<GUINet, int>(this, &GUINet::getIdleDuration));
        ret->mkItem("duration factor", true, new FunctionBinding<GUINet, double>(this, &GUINet::getRTFactor));
        /*
        ret->mkItem("mean duration factor []", true,
            new FuncBinding_IntParam<GUINet, double>(
                &(getNet()), &GUINet::getMeanRTFactor), 1);
                */
        ret->mkItem("updates per second", true, new FunctionBinding<GUINet, double>(this, &GUINet::getUPS));
        ret->mkItem("avg. updates per second", true, new FunctionBinding<GUINet, double>(this, &GUINet::getMeanUPS));
        if (OptionsCont::getOptions().getBool("duration-log.statistics")) {
            ret->mkItem("avg. trip length [m]", true, new FunctionBinding<GUINet, double>(this, &GUINet::getAvgRouteLength));
            ret->mkItem("avg. trip duration [s]", true, new FunctionBinding<GUINet, double>(this, &GUINet::getAvgDuration));
            ret->mkItem("avg. trip waiting time [s]", true, new FunctionBinding<GUINet, double>(this, &GUINet::getAvgWaitingTime));
            ret->mkItem("avg. trip time loss [s]", true, new FunctionBinding<GUINet, double>(this, &GUINet::getAvgTimeLoss));
            ret->mkItem("avg. trip depart delay [s]", true, new FunctionBinding<GUINet, double>(this, &GUINet::getAvgDepartDelay));
            ret->mkItem("avg. trip speed [m/s]", true, new FunctionBinding<GUINet, double>(this, &GUINet::getAvgTripSpeed));
            if (myPersonControl != nullptr) {
                ret->mkItem("avg. walk length [m]", true, new FunctionBinding<GUINet, double>(this, &GUINet::getAvgWalkRouteLength));
                ret->mkItem("avg. walk duration [s]", true, new FunctionBinding<GUINet, double>(this, &GUINet::getAvgWalkDuration));
                ret->mkItem("avg. walk time loss [s]", true, new FunctionBinding<GUINet, double>(this, &GUINet::getAvgWalkTimeLoss));
            }
        }
    }
    ret->mkItem("nodes [#]", false, (int)getJunctionIDs(false).size());
    ret->mkItem("edges [#]", false, (int)GUIEdge::getIDs(false).size());
    ret->mkItem("total edge length [km]", false, GUIEdge::getTotalLength(false, false) / 1000);
    ret->mkItem("total lane length [km]", false, GUIEdge::getTotalLength(false, true) / 1000);
    ret->mkItem("network version ", false, toString(myVersion));

    // close building
    ret->closeBuilding();
    return ret;
}


void
GUINet::drawGL(const GUIVisualizationSettings& /*s*/) const {
}

Boundary
GUINet::getCenteringBoundary() const {
    return getBoundary();
}


GUINet*
GUINet::getGUIInstance() {
    GUINet* net = dynamic_cast<GUINet*>(MSNet::getInstance());
    if (net != nullptr) {
        return net;
    }
    throw ProcessError("A gui-network was not yet constructed.");
}


GUIVehicleControl*
GUINet::getGUIVehicleControl() {
    return dynamic_cast<GUIVehicleControl*>(myVehicleControl);
}


void
GUINet::lock() {
    myLock.lock();
}


void
GUINet::unlock() {
    myLock.unlock();
}

GUIMEVehicleControl*
GUINet::getGUIMEVehicleControl() {
    return dynamic_cast<GUIMEVehicleControl*>(myVehicleControl);
}


double
GUINet::getEdgeData(const MSEdge* edge, const std::string& attr) {
    auto it = myLoadedEdgeData.find(attr);
    if (it != myLoadedEdgeData.end()) {
        double value;
        bool found = it->second->retrieveExistingEffort(edge, STEPS2TIME(getCurrentTimeStep()), value);
        if (found) {
            return value;
        } else {
            return GUIVisualizationSettings::MISSING_DATA;
        }
    } else {
        return GUIVisualizationSettings::MISSING_DATA;
    }
}


void
GUINet::DiscoverAttributes::myStartElement(int element, const SUMOSAXAttributes& attrs) {
    if (element == SUMO_TAG_EDGE || element == SUMO_TAG_LANE) {
        std::vector<std::string> tmp = attrs.getAttributeNames();
        edgeAttrs.insert(tmp.begin(), tmp.end());
    } else if (element == SUMO_TAG_EDGEREL) {
        for (const std::string& a : attrs.getAttributeNames()) {
            if (a != "from" && a != "to") {
                edgeAttrs.insert(a);
            }
        }
    } else if (element == SUMO_TAG_INTERVAL) {
        bool ok;
        lastIntervalEnd = MAX2(lastIntervalEnd, attrs.getSUMOTimeReporting(SUMO_ATTR_END, nullptr, ok));
    }
}

std::vector<std::string>
GUINet::DiscoverAttributes::getEdgeAttrs() {
    edgeAttrs.erase(toString(SUMO_ATTR_ID));
    return std::vector<std::string>(edgeAttrs.begin(), edgeAttrs.end());
}

void
GUINet::EdgeFloatTimeLineRetriever_GUI::addEdgeWeight(const std::string& id,
        double value, double begTime, double endTime) const {
    MSEdge* edge = MSEdge::dictionary(id);
    if (edge != nullptr) {
        myWeightStorage->addEffort(edge, begTime, endTime, value);
    } else {
        WRITE_ERROR("Trying to set the effort for the unknown edge '" + id + "'.");
    }
}

void
GUINet::EdgeFloatTimeLineRetriever_GUI::addEdgeRelWeight(const std::string& from, const std::string& to,
        double val, double beg, double end) const {
    MSEdge* fromEdge = MSEdge::dictionary(from);
    MSEdge* toEdge = MSEdge::dictionary(to);
    if (fromEdge != nullptr && toEdge != nullptr) {
        for (auto item : fromEdge->getViaSuccessors()) {
            if (item.first == toEdge) {
                const MSEdge* edge = item.second;
                while (edge != nullptr && edge->isInternal()) {
                    myWeightStorage->addEffort(edge, beg, end, val);
                    edge = edge->getViaSuccessors().front().second;
                }
            }
        }
    } else if (fromEdge == nullptr) {
        WRITE_ERROR("Trying to set the effort for the unknown edge '" + from + "'.");
    } else {
        WRITE_ERROR("Trying to set the effort for the unknown edge '" + to + "'.");
    }
}

bool
GUINet::loadEdgeData(const std::string& file) {
    // discover edge attributes
    DiscoverAttributes discoveryHandler(file);
    XMLSubSys::runParser(discoveryHandler, file);
    std::vector<std::string> attrs = discoveryHandler.getEdgeAttrs();
    WRITE_MESSAGE("Loading edgedata from '" + file
                  + "' Found " + toString(attrs.size())
                  + " attributes: " + toString(attrs));
    myEdgeDataEndTime = MAX2(myEdgeDataEndTime, discoveryHandler.lastIntervalEnd);
    // create a retriever for each attribute
    std::vector<EdgeFloatTimeLineRetriever_GUI> retrieverDefsInternal;
    retrieverDefsInternal.reserve(attrs.size());
    std::vector<SAXWeightsHandler::ToRetrieveDefinition*> retrieverDefs;
    for (const std::string& attr : attrs) {
        MSEdgeWeightsStorage* ws = new MSEdgeWeightsStorage();
        myLoadedEdgeData[attr] = ws;
        retrieverDefsInternal.push_back(EdgeFloatTimeLineRetriever_GUI(ws));
        retrieverDefs.push_back(new SAXWeightsHandler::ToRetrieveDefinition(attr, true, retrieverDefsInternal.back()));
    }
    SAXWeightsHandler handler(retrieverDefs, "");
    return XMLSubSys::runParser(handler, file);
}


std::vector<std::string>
GUINet::getEdgeDataAttrs() const {
    std::vector<std::string> result;
    for (const auto& item : myLoadedEdgeData) {
        result.push_back(item.first);
    }
    return result;
}

bool
GUINet::isSelected(const MSTrafficLightLogic* tll) const {
    const auto it = myLogics2Wrapper.find(const_cast<MSTrafficLightLogic*>(tll));
    return it != myLogics2Wrapper.end() && gSelected.isSelected(GLO_TLLOGIC, it->second->getGlID());
}

#ifdef HAVE_OSG
void
GUINet::updateColor(const GUIVisualizationSettings& s) {
    for (std::vector<GUIEdge*>::const_iterator i = myEdgeWrapper.begin(); i != myEdgeWrapper.end(); ++i) {
        if (!(*i)->isInternal()) {
            const std::vector<MSLane*>& lanes = (*i)->getLanes();
            for (std::vector<MSLane*>::const_iterator j = lanes.begin(); j != lanes.end(); ++j) {
                static_cast<GUILane*>(*j)->updateColor(s);
            }
        }
    }
    for (std::vector<GUIJunctionWrapper*>::iterator i = myJunctionWrapper.begin(); i != myJunctionWrapper.end(); ++i) {
        (*i)->updateColor(s);
    }
}
#endif


/****************************************************************************/
