//
// Copyright (C) 2007 Technische Universitaet Berlin (TUB), Germany, Telecommunication Networks Group
// Copyright (C) 2007 Technische Universiteit Delft (TUD), Netherlands
// Copyright (C) 2007 Universitaet Paderborn (UPB), Germany
//
// Documentation for these modules is at http://veins.car2x.org/
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include "veins/base/phyLayer/BasePhyLayer.h"

#include <string>
#include <sstream>
#include <vector>
#include "veins/base/phyLayer/PhyToMacControlInfo.h"
#include "veins/base/utils/FindModule.h"
#include "veins/base/utils/POA.h"
#include "veins/modules/phy/SampledAntenna1D.h"
#include "veins/base/phyLayer/AnalogueModel.h"
#include "veins/base/phyLayer/Decider.h"
#include "veins/base/modules/BaseWorldUtility.h"
#include "veins/base/connectionManager/BaseConnectionManager.h"

using namespace veins;

using std::unique_ptr;

Define_Module(veins::BasePhyLayer);

// --Initialization----------------------------------

template <class T>
T BasePhyLayer::readPar(const char* parName, const T defaultValue)
{
    if (hasPar(parName))
        return par(parName);
    else
        return defaultValue;
}

// the following line is needed to allow linking when compiled in RELEASE mode.
// Add a declaration for each parameterization of the template used in
// code to be linked, e.g. in modules or in examples, if it is not already
// used in base (double and simtime_t). Needed with (at least): gcc 4.4.1.
template int BasePhyLayer::readPar<int>(const char* parName, const int);
template double BasePhyLayer::readPar<double>(const char* parName, const double);
template simtime_t BasePhyLayer::readPar<simtime_t>(const char* parName, const simtime_t);
template bool BasePhyLayer::readPar<bool>(const char* parName, const bool);

void BasePhyLayer::initialize(int stage)
{

    ChannelAccess::initialize(stage);

    if (stage == 0) {
        // if using sendDirect, make sure that messages arrive without delay
        gate("radioIn")->setDeliverOnReceptionStart(true);

        upperLayerIn = findGate("upperLayerIn");
        upperLayerOut = findGate("upperLayerOut");
        upperControlOut = findGate("upperControlOut");
        upperControlIn = findGate("upperControlIn");

        if (par("useNoiseFloor").boolValue()) {
            noiseFloorValue = FWMath::dBm2mW(par("noiseFloor").doubleValue());
        }
        else {
            noiseFloorValue = 0;
        }
        minPowerLevel = par("minPowerLevel").doubleValue();
        minPowerLevel = FWMath::dBm2mW(minPowerLevel);

        recordStats = par("recordStats").boolValue();

        radio = initializeRadio();

        world = FindModule<BaseWorldUtility*>::findGlobalModule();
        if (world == nullptr) {
            throw cRuntimeError("Could not find BaseWorldUtility module");
        }

        if (cc->hasPar("sat") && (minPowerLevel - FWMath::dBm2mW(cc->par("sat").doubleValue())) < -0.000001) {
            throw cRuntimeError("minPowerLevel can't be smaller than the signal attenuation threshold (sat) in ConnectionManager. Please adjust your omnetpp.ini file accordingly.");
        }

        initializeAnalogueModels(par("analogueModels").xmlValue());
        initializeDecider(par("decider").xmlValue());
        initializeAntenna(par("antenna").xmlValue());

        radioSwitchingOverTimer = new cMessage("radio switching over", RADIO_SWITCHING_OVER);
        txOverTimer = new cMessage("transmission over", TX_OVER);
    }
}

unique_ptr<Radio> BasePhyLayer::initializeRadio()
{
    int initialRadioState = par("initialRadioState");
    int nbRadioChannels = readPar("nbRadioChannels", 1);
    int initialRadioChannel = readPar("initialRadioChannel", 0);

    auto radio = Radio::createNewRadio(recordStats, initialRadioState, initialRadioChannel, nbRadioChannels);

    //    - switch times to TX
    // if no RX to TX defined asume same time as sleep to TX
    radio->setSwitchTime(Radio::RX, Radio::TX, (hasPar("timeRXToTX") ? par("timeRXToTX") : par("timeSleepToTX")).doubleValue());
    // if no sleep to TX defined asume same time as RX to TX
    radio->setSwitchTime(Radio::SLEEP, Radio::TX, (hasPar("timeSleepToTX") ? par("timeSleepToTX") : par("timeRXToTX")).doubleValue());

    //    - switch times to RX
    // if no TX to RX defined asume same time as sleep to RX
    radio->setSwitchTime(Radio::TX, Radio::RX, (hasPar("timeTXToRX") ? par("timeTXToRX") : par("timeSleepToRX")).doubleValue());
    // if no sleep to RX defined asume same time as TX to RX
    radio->setSwitchTime(Radio::SLEEP, Radio::RX, (hasPar("timeSleepToRX") ? par("timeSleepToRX") : par("timeTXToRX")).doubleValue());

    //    - switch times to sleep
    // if no TX to sleep defined asume same time as RX to sleep
    radio->setSwitchTime(Radio::TX, Radio::SLEEP, (hasPar("timeTXToSleep") ? par("timeTXToSleep") : par("timeRXToSleep")).doubleValue());
    // if no RX to sleep defined asume same time as TX to sleep
    radio->setSwitchTime(Radio::RX, Radio::SLEEP, (hasPar("timeRXToSleep") ? par("timeRXToSleep") : par("timeTXToSleep")).doubleValue());

    return radio;
}

void BasePhyLayer::getParametersFromXML(cXMLElement* xmlData, ParameterMap& outputMap)
{
    cXMLElementList parameters = xmlData->getElementsByTagName("Parameter");

    for (cXMLElementList::const_iterator it = parameters.begin(); it != parameters.end(); it++) {

        const char* name = (*it)->getAttribute("name");
        const char* type = (*it)->getAttribute("type");
        const char* value = (*it)->getAttribute("value");
        if (name == nullptr || type == nullptr || value == nullptr) throw cRuntimeError("Invalid parameter, could not find name, type or value");

        std::string sType = type; // needed for easier comparision
        std::string sValue = value; // needed for easier comparision

        cMsgPar param(name);

        // parse type of parameter and set value
        if (sType == "bool") {
            param.setBoolValue(sValue == "true" || sValue == "1");
        }
        else if (sType == "double") {
            param.setDoubleValue(strtod(value, nullptr));
        }
        else if (sType == "string") {
            param.setStringValue(value);
        }
        else if (sType == "long") {
            param.setLongValue(strtol(value, nullptr, 0));
        }
        else {
            throw cRuntimeError("Unknown parameter type: '%s'", sType.c_str());
        }

        // add parameter to output map
        outputMap[name] = param;
    }
}

void BasePhyLayer::finish()
{
    // give decider the chance to do something
    decider->finish();
}

// -----Decider initialization----------------------

void BasePhyLayer::initializeDecider(cXMLElement* xmlConfig)
{
    decider = nullptr;

    if (xmlConfig == nullptr) {
        throw cRuntimeError("No decider configuration file specified.");
    }

    cXMLElementList deciderList = xmlConfig->getElementsByTagName("Decider");

    if (deciderList.empty()) {
        throw cRuntimeError("No decider configuration found in configuration file.");
    }

    if (deciderList.size() > 1) {
        throw cRuntimeError("More than one decider configuration found in configuration file.");
    }

    cXMLElement* deciderData = deciderList.front();

    const char* name = deciderData->getAttribute("type");

    if (name == nullptr) {
        throw cRuntimeError("Could not read type of decider from configuration file.");
    }

    ParameterMap params;
    getParametersFromXML(deciderData, params);

    decider = getDeciderFromName(name, params);

    if (decider == nullptr) {
        throw cRuntimeError("Could not find a decider with the name \"%s\".", name);
    }

    EV_TRACE << "Decider \"" << name << "\" loaded." << endl;
}

unique_ptr<Decider> BasePhyLayer::getDeciderFromName(std::string name, ParameterMap& params)
{
    return nullptr;
}

// -----Antenna initialization----------------------

void BasePhyLayer::initializeAntenna(cXMLElement* xmlConfig)
{
    antenna = nullptr;

    if (xmlConfig == nullptr) {
        throw cRuntimeError("No antenna configuration file specified.");
    }

    cXMLElementList antennaList = xmlConfig->getElementsByTagName("Antenna");

    if (antennaList.empty()) {
        throw cRuntimeError("No antenna configuration found in configuration file.");
    }

    cXMLElement* antennaData;
    if (antennaList.size() > 1) {
        int num = intuniform(0, antennaList.size() - 1);
        antennaData = antennaList[num];
    }
    else {
        antennaData = antennaList.front();
    }

    const char* name = antennaData->getAttribute("type");

    if (name == nullptr) {
        throw cRuntimeError("Could not read type of antenna from configuration file.");
    }

    ParameterMap params;
    getParametersFromXML(antennaData, params);

    antenna = getAntennaFromName(name, params);

    if (antenna == nullptr) {
        throw cRuntimeError("Could not find an antenna with the name \"%s\".", name);
    }

    const char* id = antennaData->getAttribute("id");
    EV_TRACE << "Antenna \"" << name << "\" with ID \"" << id << "\" loaded." << endl;
}

std::shared_ptr<Antenna> BasePhyLayer::getAntennaFromName(std::string name, ParameterMap& params)
{
    if (name == "SampledAntenna1D") {
        return initializeSampledAntenna1D(params);
    }

    return std::make_shared<Antenna>();
}

std::shared_ptr<Antenna> BasePhyLayer::initializeSampledAntenna1D(ParameterMap& params)
{
    // get samples of the modeled antenna and put them in a vector
    ParameterMap::iterator it = params.find("samples");
    if (it == params.end()) {
        throw cRuntimeError("BasePhyLayer::initializeSampledAntenna1D(): No samples specified for this antenna. \
                Please adjust your xml file accordingly.");
    }

    std::vector<double> values;
    std::stringstream samplesStream(it->second.stringValue());
    std::copy(std::istream_iterator<double>(samplesStream), std::istream_iterator<double>(), std::back_inserter(values));

    // get optional random offsets for the antenna's samples
    std::string offsetType = "";
    std::vector<double> offsetParams;
    it = params.find("random-offsets");
    if (it != params.end()) {
        std::string buf;
        std::stringstream offsetStream(it->second.stringValue());
        offsetStream >> offsetType;
        std::copy(std::istream_iterator<double>(offsetStream), std::istream_iterator<double>(), std::back_inserter(offsetParams));
    }

    // get optional random rotation of the whole pattern
    std::string rotationType = "";
    std::vector<double> rotationParams;
    it = params.find("random-rotation");
    if (it != params.end()) {
        std::string buf;
        std::stringstream rotationStream(it->second.stringValue());
        rotationStream >> rotationType;
        std::copy(std::istream_iterator<double>(rotationStream), std::istream_iterator<double>(), std::back_inserter(rotationParams));
    }

    return std::make_shared<SampledAntenna1D>(values, offsetType, offsetParams, rotationType, rotationParams, this->getRNG(0));
}

// -----AnalogueModels initialization----------------

void BasePhyLayer::initializeAnalogueModels(cXMLElement* xmlConfig)
{

    if (xmlConfig == nullptr) {
        throw cRuntimeError("No analogue models configuration file specified.");
    }

    cXMLElementList analogueModelList = xmlConfig->getElementsByTagName("AnalogueModel");

    if (analogueModelList.empty()) {
        throw cRuntimeError("No analogue models configuration found in configuration file.");
    }

    // iterate over all AnalogueModel-entries, get a new AnalogueModel instance and add
    // it to analogueModels
    for (auto&& analogueModelData : analogueModelList) {
        const char* name = analogueModelData->getAttribute("type");
        const char* thresholdingFlag = analogueModelData->getAttribute("thresholding");

        if (name == nullptr) {
            throw cRuntimeError("Could not read name of analogue model.");
        }

        ParameterMap params;
        getParametersFromXML(analogueModelData, params);

        auto newAnalogueModel = getAnalogueModelFromName(name, params);

        if (!newAnalogueModel) {
            throw cRuntimeError("Could not find an analogue model with the name \"%s\".", name);
        }

        // attach the new AnalogueModel to the AnalogueModelList
        if (thresholdingFlag && std::string(thresholdingFlag) == "true") {
            if (!newAnalogueModel->neverIncreasesPower()) {
                throw cRuntimeError("Tried to instantiate analogue model \"%s\" with tresholding=true, but model does not support this.", name);
            }
            analogueModelsThresholding.push_back(std::move(newAnalogueModel));
        }
        else {
            analogueModels.push_back(std::move(newAnalogueModel));
        }

        EV_TRACE << "AnalogueModel \"" << name << "\" loaded." << endl;
    }
}

// --Message handling--------------------------------------

void BasePhyLayer::handleMessage(cMessage* msg)
{

    // self messages
    if (msg->isSelfMessage()) {
        handleSelfMessage(msg);

        // MacPkts <- MacToPhyControlInfo
    }
    else if (msg->getArrivalGateId() == upperLayerIn) {
        handleUpperMessage(msg);

        // controlmessages
    }
    else if (msg->getArrivalGateId() == upperControlIn) {
        handleUpperControlMessage(msg);

        // AirFrames
    }
    else if (msg->getKind() == AIR_FRAME) {
        handleAirFrame(static_cast<AirFrame*>(msg));

        // unknown message
    }
    else {
        EV << "Unknown message received." << endl;
        delete msg;
    }
}

void BasePhyLayer::handleAirFrame(AirFrame* frame)
{
    // TODO: ask jerome to set air frame priority in his UWBIRPhy
    // ASSERT(frame->getSchedulingPriority() == airFramePriority());

    switch (static_cast<AirFrameState>(frame->getState())) {
    case AirFrameState::start_receive:
        handleAirFrameStartReceive(frame);
        break;

    case AirFrameState::receiving: {
        handleAirFrameReceiving(frame);
        break;
    }
    case AirFrameState::end_receive:
        handleAirFrameEndReceive(frame);
        break;

    default:
        throw cRuntimeError("Unknown AirFrame state: %s", frame->getState());
    }
}

void BasePhyLayer::handleAirFrameStartReceive(AirFrame* frame)
{
    EV_TRACE << "Received new AirFrame " << frame << " from channel." << endl;

    channelInfo.addAirFrame(frame, simTime());
    ASSERT(!channelInfo.isChannelEmpty());

    if (usePropagationDelay) {
        Signal& s = frame->getSignal();
        simtime_t delay = simTime() - s.getSendingStart();
        s.setPropagationDelay(delay);
    }
    ASSERT(frame->getSignal().getReceptionStart() == simTime());

    filterSignal(frame);

    if (decider && isKnownProtocolId(frame->getProtocolId())) {
        frame->setState(static_cast<int>(AirFrameState::receiving));

        // pass the AirFrame the first time to the Decider
        handleAirFrameReceiving(frame);

        // if no decider is defined we will schedule the message directly to its end
    }
    else {
        Signal& signal = frame->getSignal();

        simtime_t signalEndTime = signal.getReceptionStart() + frame->getDuration();
        frame->setState(static_cast<int>(AirFrameState::end_receive));

        sendSelfMessage(frame, signalEndTime);
    }
}

void BasePhyLayer::handleAirFrameReceiving(AirFrame* frame)
{

    Signal& signal = frame->getSignal();
    simtime_t nextHandleTime = decider->processSignal(frame);

    ASSERT(signal.getDuration() == frame->getDuration());
    simtime_t signalEndTime = signal.getReceptionStart() + frame->getDuration();

    // check if this is the end of the receiving process
    if (simTime() >= signalEndTime) {
        frame->setState(static_cast<int>(AirFrameState::end_receive));
        handleAirFrameEndReceive(frame);
        return;
    }

    // smaller zero means don't give it to me again
    if (nextHandleTime < 0) {
        nextHandleTime = signalEndTime;
        frame->setState(static_cast<int>(AirFrameState::end_receive));

        // invalid point in time
    }
    else if (nextHandleTime < simTime() || nextHandleTime > signalEndTime) {
        throw cRuntimeError("Invalid next handle time returned by Decider. Expected a value between current simulation time (%.2f) and end of signal (%.2f) but got %.2f", SIMTIME_DBL(simTime()), SIMTIME_DBL(signalEndTime), SIMTIME_DBL(nextHandleTime));
    }

    EV_TRACE << "Handed AirFrame with ID " << frame->getId() << " to Decider. Next handling in " << nextHandleTime - simTime() << "s." << endl;

    sendSelfMessage(frame, nextHandleTime);
}

void BasePhyLayer::handleAirFrameEndReceive(AirFrame* frame)
{
    EV_TRACE << "End of Airframe with ID " << frame->getId() << "." << endl;

    simtime_t earliestInfoPoint = channelInfo.removeAirFrame(frame);

    /* clean information in the radio until earliest time-point
     * of information in the ChannelInfo,
     * since this time-point might have changed due to removal of
     * the AirFrame
     */
    if (channelInfo.isChannelEmpty()) {
        earliestInfoPoint = simTime();
    }
}

void BasePhyLayer::handleUpperMessage(cMessage* msg)
{

    // check if Radio is in TX state
    if (radio->getCurrentState() != Radio::TX) {
        delete msg;
        msg = nullptr;
        throw cRuntimeError("Error: message for sending received, but radio not in state TX");
    }

    // check if not already sending
    if (txOverTimer->isScheduled()) {
        delete msg;
        msg = nullptr;
        throw cRuntimeError("Error: message for sending received, but radio already sending");
    }

    // build the AirFrame to send
    ASSERT(dynamic_cast<cPacket*>(msg) != nullptr);

    unique_ptr<AirFrame> frame = encapsMsg(static_cast<cPacket*>(msg));

    // Prepare a POA object and attach it to the created Airframe
    AntennaPosition pos = antennaPosition;
    Coord orient = antennaHeading.toCoord();
    frame->setPoa({pos, orient, antenna});

    // make sure there is no self message of kind TX_OVER scheduled
    // and schedule the actual one
    ASSERT(!txOverTimer->isScheduled());
    sendSelfMessage(txOverTimer, simTime() + frame->getDuration());

    sendMessageDown(frame.release());
}

unique_ptr<AirFrame> BasePhyLayer::createAirFrame(cPacket* macPkt)
{
    return make_unique<AirFrame>(macPkt->getName(), AIR_FRAME);
}

unique_ptr<AirFrame> BasePhyLayer::encapsMsg(cPacket* macPkt)
{
    // the cMessage passed must be a MacPacket... but no cast needed here
    // MacPkt* pkt = static_cast<MacPkt*>(msg);

    // ...and must always have a ControlInfo attached (contains Signal)
    auto ctrlInfo = unique_ptr<cObject>(macPkt->removeControlInfo());
    ASSERT(ctrlInfo);

    // create the new AirFrame
    auto frame = createAirFrame(macPkt);

    // set the members
    // set priority of AirFrames above the normal priority to ensure
    // channel consistency (before any thing else happens at a time
    // point t make sure that the channel has removed every AirFrame
    // ended at t and added every AirFrame started at t)
    frame->setSchedulingPriority(airFramePriority());
    frame->setProtocolId(myProtocolId());
    frame->setId(world->getUniqueAirFrameId());
    frame->setChannel(radio->getCurrentChannel());
    frame->setBitLength(0); // as the PHY might mix multiple different encoding rates (and things that might not be bits at all)

    frame->encapsulate(macPkt);

    // attach the spectrum-dependent Signal to the airFrame
    attachSignal(frame.get(), ctrlInfo.get());

    // --- from here on, the AirFrame is the owner of the MacPacket ---
    macPkt = nullptr;
    EV_TRACE << "AirFrame encapsulated, length: " << frame->getBitLength() << "\n";

    return frame;
}

void BasePhyLayer::handleUpperControlMessage(cMessage* msg)
{

    throw cRuntimeError("Received unknown control message from upper layer!");
}

void BasePhyLayer::handleSelfMessage(cMessage* msg)
{

    switch (msg->getKind()) {
    // transmission over
    case TX_OVER:
        ASSERT(msg == txOverTimer);
        sendControlMsgToMac(new cMessage("Transmission over", TX_OVER));
        break;

    // radio switch over
    case RADIO_SWITCHING_OVER:
        ASSERT(msg == radioSwitchingOverTimer);
        finishRadioSwitching();
        break;

    // AirFrame
    case AIR_FRAME:
        handleAirFrame(static_cast<AirFrame*>(msg));
        break;

    default:
        break;
    }
}

// --Send messages------------------------------

void BasePhyLayer::sendControlMessageUp(cMessage* msg)
{
    send(msg, upperControlOut);
}

void BasePhyLayer::sendMacPktUp(cMessage* pkt)
{
    send(pkt, upperLayerOut);
}

void BasePhyLayer::sendMessageDown(AirFrame* msg)
{

    sendToChannel(msg);
}

void BasePhyLayer::sendSelfMessage(cMessage* msg, simtime_t_cref time)
{
    // TODO: maybe delete this method because it doesn't makes much sense,
    //        or change it to "scheduleIn(msg, timeDelta)" which schedules
    //        a message to +timeDelta from current time
    scheduleAt(time, msg);
}

void BasePhyLayer::filterSignal(AirFrame* frame)
{
    ASSERT(dynamic_cast<ChannelAccess* const>(frame->getArrivalModule()) == this);
    ASSERT(dynamic_cast<ChannelAccess* const>(frame->getSenderModule()));
    Signal& signal = frame->getSignal();

    // Extract position and orientation of sender and receiver (this module) first
    const AntennaPosition receiverPosition = antennaPosition;
    const Coord receiverOrientation = antennaHeading.toCoord();
    // get POA from frame with the sender's position, orientation and antenna
    POA& senderPOA = frame->getPoa();
    const AntennaPosition senderPosition = senderPOA.pos;
    const Coord senderOrientation = senderPOA.orientation;

    // add position information to signal
    signal.setSenderPoa(senderPOA);
    signal.setReceiverPoa({receiverPosition, receiverOrientation, antenna});

    // compute gains at sender and receiver antenna
    double receiverGain = antenna->getGain(receiverPosition.getPositionAt(), receiverOrientation, senderPosition.getPositionAt());
    double senderGain = senderPOA.antenna->getGain(senderPosition.getPositionAt(), senderOrientation, receiverPosition.getPositionAt());

    // add the resulting total gain to the attenuations list
    EV_TRACE << "Sender's antenna gain: " << senderGain << endl;
    EV_TRACE << "Own (receiver's) antenna gain: " << receiverGain << endl;
    signal *= receiverGain * senderGain;

    // go on with AnalogueModels
    // attach analogue models suitable for thresholding to signal (for later evaluation)
    signal.setAnalogueModelList(&analogueModelsThresholding);

    // apply all analouge models that are *not* suitable for thresholding now
    for (auto& analogueModel : analogueModels) {
        analogueModel->filterSignal(&signal);
    }
}

// --Destruction--------------------------------

BasePhyLayer::~BasePhyLayer()
{
    // get AirFrames from ChannelInfo and delete
    // (although ChannelInfo normally owns the AirFrames it
    // is not able to cancel and delete them itself
    AirFrameVector channel;
    channelInfo.getAirFrames(0, simTime(), channel);

    for (AirFrameVector::iterator it = channel.begin(); it != channel.end(); ++it) {
        cancelAndDelete(*it);
    }

    // free timer messages
    if (txOverTimer) {
        cancelAndDelete(txOverTimer);
        txOverTimer = nullptr;
    }
    if (radioSwitchingOverTimer) {
        cancelAndDelete(radioSwitchingOverTimer);
        radioSwitchingOverTimer = nullptr;
    }
}

// --MacToPhyInterface implementation-----------------------

int BasePhyLayer::getRadioState()
{
    Enter_Method_Silent();
    ASSERT(radio);
    return radio->getCurrentState();
}

void BasePhyLayer::finishRadioSwitching()
{
    radio->endSwitch(simTime());
    sendControlMsgToMac(new cMessage("Radio switching over", RADIO_SWITCHING_OVER));
}

simtime_t BasePhyLayer::setRadioState(int rs)
{
    Enter_Method_Silent();
    ASSERT(radio);

    if (txOverTimer && txOverTimer->isScheduled()) {
        EV_WARN << "Switched radio while sending an AirFrame. The effects this would have on the transmission are not simulated by the BasePhyLayer!";
    }

    simtime_t switchTime = radio->switchTo(rs, simTime());

    // invalid switch time, we are probably already switching
    if (switchTime < 0) return switchTime;

    // if switching is done in exactly zero-time no extra self-message is scheduled
    if (switchTime == 0.0) {
        // TODO: in case of zero-time-switch, send no control-message to mac!
        // maybe call a method finishRadioSwitchingSilent()
        finishRadioSwitching();
    }
    else {
        sendSelfMessage(radioSwitchingOverTimer, simTime() + switchTime);
    }

    return switchTime;
}

void BasePhyLayer::setCurrentRadioChannel(int newRadioChannel)
{
    if (txOverTimer && txOverTimer->isScheduled()) {
        EV_WARN << "Switched channel while sending an AirFrame. The effects this would have on the transmission are not simulated by the BasePhyLayer!";
    }

    radio->setCurrentChannel(newRadioChannel);
    decider->channelChanged(newRadioChannel);
    EV_TRACE << "Switched radio to channel " << newRadioChannel << endl;
}

int BasePhyLayer::getCurrentRadioChannel()
{
    return radio->getCurrentChannel();
}

int BasePhyLayer::getNbRadioChannels()
{
    return par("nbRadioChannels");
}

// --DeciderToPhyInterface implementation------------

void BasePhyLayer::getChannelInfo(simtime_t_cref from, simtime_t_cref to, AirFrameVector& out)
{
    channelInfo.getAirFrames(from, to, out);
}

double BasePhyLayer::getNoiseFloorValue()
{
    return noiseFloorValue;
}

void BasePhyLayer::sendControlMsgToMac(cMessage* msg)
{
    sendControlMessageUp(msg);
}

void BasePhyLayer::sendUp(AirFrame* frame, DeciderResult* result)
{

    EV_TRACE << "Decapsulating MacPacket from Airframe with ID " << frame->getId() << " and sending it up to MAC." << endl;

    cMessage* packet = frame->decapsulate();

    ASSERT(packet);

    PhyToMacControlInfo::setControlInfo(packet, result);

    sendMacPktUp(packet);
}

BaseWorldUtility* BasePhyLayer::getWorldUtility()
{
    return world;
}

void BasePhyLayer::recordScalar(const char* name, double value, const char* unit)
{
    ChannelAccess::recordScalar(name, value, unit);
}
