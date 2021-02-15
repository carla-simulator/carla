/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2012-2020 German Aerospace Center (DLR) and others.
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
/// @file    MSStateHandler.cpp
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @date    Thu, 13 Dec 2012
///
// Parser and output filter for routes and vehicles state saving and loading
/****************************************************************************/
#include <config.h>

#ifdef HAVE_VERSION_H
#include <version.h>
#endif

#include <sstream>
#include <utils/common/StringUtils.h>
#include <utils/options/OptionsCont.h>
#include <utils/iodevices/OutputDevice.h>
#include <utils/xml/SUMOXMLDefinitions.h>
#include <utils/vehicle/SUMOVehicleParserHelper.h>
#include <microsim/devices/MSDevice_Routing.h>
#include <microsim/devices/MSDevice_BTreceiver.h>
#include <microsim/devices/MSDevice_ToC.h>
#include <microsim/transportables/MSTransportableControl.h>
#include <microsim/MSEdge.h>
#include <microsim/MSLane.h>
#include <microsim/MSGlobals.h>
#include <microsim/MSNet.h>
#include <microsim/MSVehicleTransfer.h>
#include <microsim/MSInsertionControl.h>
#include <microsim/MSRoute.h>
#include <microsim/MSVehicleControl.h>
#include <microsim/MSDriverState.h>
#include "MSStateHandler.h"

#include <mesosim/MESegment.h>
#include <mesosim/MELoop.h>


// ===========================================================================
// method definitions
// ===========================================================================
MSStateHandler::MSStateHandler(const std::string& file, const SUMOTime offset, bool onlyReadTime) :
    MSRouteHandler(file, true),
    myOffset(offset),
    mySegment(nullptr),
    myCurrentLane(nullptr),
    myCurrentLink(nullptr),
    myAttrs(nullptr),
    myLastParameterised(nullptr),
    myOnlyReadTime(onlyReadTime) {
    myAmLoadingState = true;
    const std::vector<std::string> vehIDs = OptionsCont::getOptions().getStringVector("load-state.remove-vehicles");
    myVehiclesToRemove.insert(vehIDs.begin(), vehIDs.end());
}


MSStateHandler::~MSStateHandler() {
}


void
MSStateHandler::saveState(const std::string& file, SUMOTime step) {
    OutputDevice& out = OutputDevice::getDevice(file);
    out.writeHeader<MSEdge>(SUMO_TAG_SNAPSHOT);
    out.writeAttr("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance").writeAttr("xsi:noNamespaceSchemaLocation", "http://sumo.dlr.de/xsd/state_file.xsd");
    out.writeAttr(SUMO_ATTR_VERSION, VERSION_STRING).writeAttr(SUMO_ATTR_TIME, time2string(step));
    if (OptionsCont::getOptions().getBool("save-state.rng")) {
        saveRNGs(out);
    }
    MSRoute::dict_saveState(out);
    MSNet::getInstance()->getInsertionControl().saveState(out);
    MSNet::getInstance()->getVehicleControl().saveState(out);
    if (OptionsCont::getOptions().getBool("save-state.transportables")) {
        if (MSNet::getInstance()->hasPersons()) {
            out.openTag(SUMO_TAG_TRANSPORTABLES).writeAttr(SUMO_ATTR_TYPE, "person");
            MSNet::getInstance()->getPersonControl().saveState(out);
            out.closeTag();
        }
        if (MSNet::getInstance()->hasContainers()) {
            out.openTag(SUMO_TAG_TRANSPORTABLES).writeAttr(SUMO_ATTR_TYPE, "container");
            MSNet::getInstance()->getContainerControl().saveState(out);
            out.closeTag();
        }
    }
    MSVehicleTransfer::getInstance()->saveState(out);
    if (MSGlobals::gUseMesoSim) {
        for (int i = 0; i < MSEdge::dictSize(); i++) {
            for (MESegment* s = MSGlobals::gMesoNet->getSegmentForEdge(*MSEdge::getAllEdges()[i]); s != nullptr; s = s->getNextSegment()) {
                s->saveState(out);
            }
        }
    } else {
        for (int i = 0; i < MSEdge::dictSize(); i++) {
            const std::vector<MSLane*>& lanes = MSEdge::getAllEdges()[i]->getLanes();
            for (std::vector<MSLane*>::const_iterator it = lanes.begin(); it != lanes.end(); ++it) {
                (*it)->saveState(out);
            }
        }
    }
    out.close();
}


void
MSStateHandler::myStartElement(int element, const SUMOSAXAttributes& attrs) {
    MSRouteHandler::myStartElement(element, attrs);
    MSVehicleControl& vc = MSNet::getInstance()->getVehicleControl();
    switch (element) {
        case SUMO_TAG_SNAPSHOT: {
            myTime = string2time(attrs.getString(SUMO_ATTR_TIME));
            if (myOnlyReadTime) {
                throw AbortParsing("Abort state parsing after reading time");
            }
            const std::string& version = attrs.getString(SUMO_ATTR_VERSION);
            if (version != VERSION_STRING) {
                WRITE_WARNING("State was written with sumo version " + version + " (present: " + VERSION_STRING + ")!");
            }
            break;
        }
        case SUMO_TAG_RNGSTATE: {
            if (attrs.hasAttribute(SUMO_ATTR_RNG_DEFAULT)) {
                RandHelper::loadState(attrs.getString(SUMO_ATTR_RNG_DEFAULT));
            }
            if (attrs.hasAttribute(SUMO_ATTR_RNG_ROUTEHANDLER)) {
                RandHelper::loadState(attrs.getString(SUMO_ATTR_RNG_DEFAULT), MSRouteHandler::getParsingRNG());
            }
            if (attrs.hasAttribute(SUMO_ATTR_RNG_INSERTIONCONTROL)) {
                RandHelper::loadState(attrs.getString(SUMO_ATTR_RNG_DEFAULT), MSNet::getInstance()->getInsertionControl().getFlowRNG());
            }
            if (attrs.hasAttribute(SUMO_ATTR_RNG_DEVICE)) {
                RandHelper::loadState(attrs.getString(SUMO_ATTR_RNG_DEFAULT), MSDevice::getEquipmentRNG());
            }
            if (attrs.hasAttribute(SUMO_ATTR_RNG_DEVICE_BT)) {
                RandHelper::loadState(attrs.getString(SUMO_ATTR_RNG_DEFAULT), MSDevice_BTreceiver::getEquipmentRNG());
            }
            if (attrs.hasAttribute(SUMO_ATTR_RNG_DRIVERSTATE)) {
                RandHelper::loadState(attrs.getString(SUMO_ATTR_RNG_DEFAULT), OUProcess::getRNG());
            }
            if (attrs.hasAttribute(SUMO_ATTR_RNG_DEVICE_TOC)) {
                RandHelper::loadState(attrs.getString(SUMO_ATTR_RNG_DEFAULT), MSDevice_ToC::getResponseTimeRNG());
            }
            break;
        }
        case SUMO_TAG_RNGLANE: {
            const int index = attrs.getInt(SUMO_ATTR_INDEX);
            const std::string state = attrs.getString(SUMO_ATTR_STATE);
            MSLane::loadRNGState(index, state);
            break;
        }
        case SUMO_TAG_DELAY: {
            vc.setState(attrs.getInt(SUMO_ATTR_NUMBER),
                        attrs.getInt(SUMO_ATTR_BEGIN),
                        attrs.getInt(SUMO_ATTR_END),
                        attrs.getFloat(SUMO_ATTR_DEPART),
                        attrs.getFloat(SUMO_ATTR_TIME));
            break;
        }
        case SUMO_TAG_FLOWSTATE: {
            SUMOVehicleParameter* pars = new SUMOVehicleParameter();
            pars->id = attrs.getString(SUMO_ATTR_ID);
            bool ok;
            if (attrs.getOpt<bool>(SUMO_ATTR_REROUTE, nullptr, ok, false)) {
                pars->parametersSet |= VEHPARS_FORCE_REROUTE;
            }
            MSNet::getInstance()->getInsertionControl().addFlow(pars,
                    attrs.getInt(SUMO_ATTR_INDEX));
            break;
        }
        case SUMO_TAG_VTYPE: {
            myLastParameterised = myCurrentVType;
            break;
        }
        case SUMO_TAG_VEHICLE: {
            myLastParameterised = myVehicleParameter;
            myAttrs = attrs.clone();
            break;
        }
        case SUMO_TAG_DEVICE: {
            myDeviceAttrs.push_back(attrs.clone());
            break;
        }
        case SUMO_TAG_VEHICLETRANSFER: {
            MSVehicleTransfer::getInstance()->loadState(attrs, myOffset, vc);
            break;
        }
        case SUMO_TAG_SEGMENT: {
            if (mySegment == nullptr) {
                mySegment = MSGlobals::gMesoNet->getSegmentForEdge(*MSEdge::getAllEdges()[0]);
            } else if (mySegment->getNextSegment() == nullptr) {
                mySegment = MSGlobals::gMesoNet->getSegmentForEdge(*MSEdge::getAllEdges()[mySegment->getEdge().getNumericalID() + 1]);
            } else {
                mySegment = mySegment->getNextSegment();
            }
            myQueIndex = 0;
            break;
        }
        case SUMO_TAG_LANE: {
            bool ok;
            const std::string laneID = attrs.get<std::string>(SUMO_ATTR_ID, nullptr, ok);
            myCurrentLane = MSLane::dictionary(laneID);
            if (myCurrentLane == nullptr) {
                throw ProcessError("Unknown lane '" + laneID + "' in loaded state");
            }
            break;
        }
        case SUMO_TAG_VIEWSETTINGS_VEHICLES: {
            try {
                const std::vector<std::string>& vehIDs = attrs.getStringVector(SUMO_ATTR_VALUE);
                if (MSGlobals::gUseMesoSim) {
                    mySegment->loadState(vehIDs, MSNet::getInstance()->getVehicleControl(), StringUtils::toLong(attrs.getString(SUMO_ATTR_TIME)) - myOffset, myQueIndex);
                } else {
                    myCurrentLane->loadState(vehIDs, MSNet::getInstance()->getVehicleControl());
                }
            } catch (EmptyData&) {} // attr may be empty
            myQueIndex++;
            break;
        }
        case SUMO_TAG_LINK: {
            bool ok;
            myCurrentLink = nullptr;
            const std::string toLaneID = attrs.get<std::string>(SUMO_ATTR_TO, nullptr, ok);
            for (MSLink* link : myCurrentLane->getLinkCont()) {
                if (link->getViaLaneOrLane()->getID() == toLaneID) {
                    myCurrentLink = link;
                }
            }
            if (myCurrentLink == nullptr) {
                throw ProcessError("Unknown link from lane '" + myCurrentLane->getID() + "' to lane '" + toLaneID + "' in loaded state");
            }
            break;
        }
        case SUMO_TAG_APPROACHING: {
            bool ok;
            const std::string vehID = attrs.get<std::string>(SUMO_ATTR_ID, nullptr, ok);
            const SUMOTime arrivalTime = attrs.get<SUMOTime>(SUMO_ATTR_ARRIVALTIME, nullptr, ok);
            const double arrivalSpeed = attrs.get<double>(SUMO_ATTR_ARRIVALSPEED, nullptr, ok);
            const double leaveSpeed = attrs.get<double>(SUMO_ATTR_DEPARTSPEED, nullptr, ok);
            const bool setRequest = attrs.get<bool>(SUMO_ATTR_REQUEST, nullptr, ok);
            const SUMOTime arrivalTimeBraking = attrs.get<SUMOTime>(SUMO_ATTR_ARRIVALTIMEBRAKING, nullptr, ok);
            const double arrivalSpeedBraking = attrs.get<double>(SUMO_ATTR_ARRIVALSPEEDBRAKING, nullptr, ok);
            const SUMOTime waitingTime = attrs.get<SUMOTime>(SUMO_ATTR_WAITINGTIME, nullptr, ok);
            const double dist = attrs.get<double>(SUMO_ATTR_DISTANCE, nullptr, ok);
            SUMOVehicle* veh = vc.getVehicle(vehID);
            myCurrentLink->setApproaching(veh, arrivalTime, arrivalSpeed, leaveSpeed, setRequest, arrivalTimeBraking, arrivalSpeedBraking, waitingTime, dist);
            if (!MSGlobals::gUseMesoSim) {
                MSVehicle* microVeh = dynamic_cast<MSVehicle*>(veh);
                microVeh->loadPreviousApproaching(myCurrentLink, setRequest, arrivalTime, arrivalSpeed, arrivalTimeBraking, arrivalSpeedBraking, dist, leaveSpeed);
            }
            break;
        }
        case SUMO_TAG_PARAM: {
            bool ok;
            const std::string key = attrs.get<std::string>(SUMO_ATTR_KEY, nullptr, ok);
            // circumventing empty string test
            const std::string val = attrs.hasAttribute(SUMO_ATTR_VALUE) ? attrs.getString(SUMO_ATTR_VALUE) : "";
            assert(myLastParameterised != 0);
            if (myLastParameterised != nullptr) {
                myLastParameterised->setParameter(key, val);
            }
            break;
        }
        case SUMO_TAG_TRANSPORTABLES:
            if (attrs.getString(SUMO_ATTR_TYPE) == "person") {
                MSNet::getInstance()->getPersonControl().loadState(attrs.getString(SUMO_ATTR_STATE));
            }
            if (attrs.getString(SUMO_ATTR_TYPE) == "container") {
                MSNet::getInstance()->getContainerControl().loadState(attrs.getString(SUMO_ATTR_STATE));
            }
            break;
        case SUMO_TAG_PERSON:
        case SUMO_TAG_CONTAINER:
            myAttrs = attrs.clone();
            break;
        default:
            break;
    }
}


void
MSStateHandler::myEndElement(int element) {
    MSRouteHandler::myEndElement(element);
    switch (element) {
        case SUMO_TAG_PERSON:
        case SUMO_TAG_CONTAINER: {
            MSTransportableControl& tc = (element == SUMO_TAG_PERSON ? MSNet::getInstance()->getPersonControl() : MSNet::getInstance()->getContainerControl());
            tc.get(myAttrs->getString(SUMO_ATTR_ID))->loadState(myAttrs->getString(SUMO_ATTR_STATE));
            delete myAttrs;
            myAttrs = nullptr;
            break;
        }
        default:
            break;
    }
    if (element != SUMO_TAG_PARAM && myVehicleParameter == nullptr && myCurrentVType == nullptr) {
        myLastParameterised = nullptr;
    }
}


void
MSStateHandler::closeVehicle() {
    assert(myVehicleParameter != 0);
    myVehicleParameter->depart -= myOffset;
    // the vehicle was already counted in MSVehicleControl::setState
    MSVehicleControl& vc = MSNet::getInstance()->getVehicleControl();
    // make a copy because myVehicleParameter is reset in closeVehicle()
    const std::string vehID = myVehicleParameter->id;
    if (myVehiclesToRemove.count(vehID) == 0) {
        MSRouteHandler::closeVehicle();
        // reset depart
        vc.discountStateLoaded();
        SUMOVehicle* v = vc.getVehicle(vehID);
        if (v == nullptr) {
            throw ProcessError("Could not load vehicle '" + vehID + "' from state");
        }
        v->setChosenSpeedFactor(myAttrs->getFloat(SUMO_ATTR_SPEEDFACTOR));
        v->loadState(*myAttrs, myOffset);
        if (v->hasDeparted()) {
            // vehicle already departed: disable pre-insertion rerouting and enable regular routing behavior
            MSDevice_Routing* routingDevice = static_cast<MSDevice_Routing*>(v->getDevice(typeid(MSDevice_Routing)));
            if (routingDevice != nullptr) {
                routingDevice->notifyEnter(*v, MSMoveReminder::NOTIFICATION_DEPARTED);
            }
            MSNet::getInstance()->getInsertionControl().alreadyDeparted(v);
        }
        while (!myDeviceAttrs.empty()) {
            const std::string attrID = myDeviceAttrs.back()->getString(SUMO_ATTR_ID);
            for (MSVehicleDevice* const dev : v->getDevices()) {
                if (dev->getID() == attrID) {
                    dev->loadState(*myDeviceAttrs.back());
                }
            }
            delete myDeviceAttrs.back();
            myDeviceAttrs.pop_back();
        }
    } else {
        vc.discountStateLoaded(true);
        delete myVehicleParameter;
        myVehicleParameter = nullptr;
    }
    delete myAttrs;
}


void
MSStateHandler::saveRNGs(OutputDevice& out) {
    out.openTag(SUMO_TAG_RNGSTATE);
    out.writeAttr(SUMO_ATTR_RNG_DEFAULT, RandHelper::saveState());
    out.writeAttr(SUMO_ATTR_RNG_ROUTEHANDLER, RandHelper::saveState(MSRouteHandler::getParsingRNG()));
    out.writeAttr(SUMO_ATTR_RNG_INSERTIONCONTROL, RandHelper::saveState(MSNet::getInstance()->getInsertionControl().getFlowRNG()));
    out.writeAttr(SUMO_ATTR_RNG_DEVICE, RandHelper::saveState(MSDevice::getEquipmentRNG()));
    out.writeAttr(SUMO_ATTR_RNG_DEVICE_BT, RandHelper::saveState(MSDevice_BTreceiver::getRNG()));
    out.writeAttr(SUMO_ATTR_RNG_DRIVERSTATE, RandHelper::saveState(OUProcess::getRNG()));
    out.writeAttr(SUMO_ATTR_RNG_DEVICE_TOC, RandHelper::saveState(MSDevice_ToC::getResponseTimeRNG()));
    MSLane::saveRNGStates(out);
    out.closeTag();

}


/****************************************************************************/
