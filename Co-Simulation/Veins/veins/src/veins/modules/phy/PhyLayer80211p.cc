//
// Copyright (C) 2011 David Eckhoff <eckhoff@cs.fau.de>
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

/*
 * Based on PhyLayer.cc from Karl Wessel
 * and modifications by Christopher Saloman
 */

#include "veins/modules/phy/PhyLayer80211p.h"

#include "veins/modules/phy/Decider80211p.h"
#include "veins/modules/analogueModel/SimplePathlossModel.h"
#include "veins/modules/analogueModel/BreakpointPathlossModel.h"
#include "veins/modules/analogueModel/PERModel.h"
#include "veins/modules/analogueModel/SimpleObstacleShadowing.h"
#include "veins/modules/analogueModel/VehicleObstacleShadowing.h"
#include "veins/modules/analogueModel/TwoRayInterferenceModel.h"
#include "veins/modules/analogueModel/NakagamiFading.h"
#include "veins/base/connectionManager/BaseConnectionManager.h"
#include "veins/modules/utility/Consts80211p.h"
#include "veins/modules/messages/AirFrame11p_m.h"
#include "veins/modules/utility/MacToPhyControlInfo11p.h"

using namespace veins;

using std::unique_ptr;

Define_Module(veins::PhyLayer80211p);

void PhyLayer80211p::initialize(int stage)
{
    if (stage == 0) {
        // get ccaThreshold before calling BasePhyLayer::initialize() which instantiates the deciders
        ccaThreshold = pow(10, par("ccaThreshold").doubleValue() / 10);
        allowTxDuringRx = par("allowTxDuringRx").boolValue();
        collectCollisionStatistics = par("collectCollisionStatistics").boolValue();

        // Create frequency mappings and initialize spectrum for signal representation
        Spectrum::Frequencies freqs;
        for (auto& channel : IEEE80211ChannelFrequencies) {
            freqs.push_back(channel.second - 5e6);
            freqs.push_back(channel.second);
            freqs.push_back(channel.second + 5e6);
        }
        overallSpectrum = Spectrum(freqs);
    }
    BasePhyLayer::initialize(stage);
}

unique_ptr<AnalogueModel> PhyLayer80211p::getAnalogueModelFromName(std::string name, ParameterMap& params)
{

    if (name == "SimplePathlossModel") {
        return initializeSimplePathlossModel(params);
    }
    else if (name == "BreakpointPathlossModel") {
        return initializeBreakpointPathlossModel(params);
    }
    else if (name == "PERModel") {
        return initializePERModel(params);
    }
    else if (name == "SimpleObstacleShadowing") {
        return initializeSimpleObstacleShadowing(params);
    }
    else if (name == "VehicleObstacleShadowing") {
        return initializeVehicleObstacleShadowing(params);
    }
    else if (name == "TwoRayInterferenceModel") {
        if (world->use2D()) throw cRuntimeError("The TwoRayInterferenceModel uses nodes' z-position as the antenna height over ground. Refusing to work in a 2D world");
        return initializeTwoRayInterferenceModel(params);
    }
    else if (name == "NakagamiFading") {
        return initializeNakagamiFading(params);
    }
    return BasePhyLayer::getAnalogueModelFromName(name, params);
}

unique_ptr<AnalogueModel> PhyLayer80211p::initializeBreakpointPathlossModel(ParameterMap& params)
{
    double alpha1 = -1, alpha2 = -1, breakpointDistance = -1;
    double L01 = -1, L02 = -1;
    bool useTorus = world->useTorus();
    const Coord& playgroundSize = *(world->getPgs());
    ParameterMap::iterator it;

    it = params.find("alpha1");
    if (it != params.end()) { // parameter alpha1 has been specified in config.xml
        // set alpha1
        alpha1 = it->second.doubleValue();
        EV_TRACE << "createPathLossModel(): alpha1 set from config.xml to " << alpha1 << endl;
        // check whether alpha is not smaller than specified in ConnectionManager
        if (cc->hasPar("alpha") && alpha1 < cc->par("alpha").doubleValue()) {
            // throw error
            throw cRuntimeError("TestPhyLayer::createPathLossModel(): alpha can't be smaller than specified in \
                   ConnectionManager. Please adjust your config.xml file accordingly");
        }
    }
    it = params.find("L01");
    if (it != params.end()) {
        L01 = it->second.doubleValue();
    }
    it = params.find("L02");
    if (it != params.end()) {
        L02 = it->second.doubleValue();
    }

    it = params.find("alpha2");
    if (it != params.end()) { // parameter alpha1 has been specified in config.xml
        // set alpha2
        alpha2 = it->second.doubleValue();
        EV_TRACE << "createPathLossModel(): alpha2 set from config.xml to " << alpha2 << endl;
        // check whether alpha is not smaller than specified in ConnectionManager
        if (cc->hasPar("alpha") && alpha2 < cc->par("alpha").doubleValue()) {
            // throw error
            throw cRuntimeError("TestPhyLayer::createPathLossModel(): alpha can't be smaller than specified in \
                   ConnectionManager. Please adjust your config.xml file accordingly");
        }
    }
    it = params.find("breakpointDistance");
    if (it != params.end()) { // parameter alpha1 has been specified in config.xml
        breakpointDistance = it->second.doubleValue();
        EV_TRACE << "createPathLossModel(): breakpointDistance set from config.xml to " << alpha2 << endl;
        // check whether alpha is not smaller than specified in ConnectionManager
    }

    if (alpha1 == -1 || alpha2 == -1 || breakpointDistance == -1 || L01 == -1 || L02 == -1) {
        throw cRuntimeError("Undefined parameters for breakpointPathlossModel. Please check your configuration.");
    }

    return make_unique<BreakpointPathlossModel>(this, L01, L02, alpha1, alpha2, breakpointDistance, useTorus, playgroundSize);
}

unique_ptr<AnalogueModel> PhyLayer80211p::initializeTwoRayInterferenceModel(ParameterMap& params)
{
    ASSERT(params.count("DielectricConstant") == 1);

    double dielectricConstant = params["DielectricConstant"].doubleValue();

    return make_unique<TwoRayInterferenceModel>(this, dielectricConstant);
}

unique_ptr<AnalogueModel> PhyLayer80211p::initializeNakagamiFading(ParameterMap& params)
{
    bool constM = params["constM"].boolValue();
    double m = 0;
    if (constM) {
        m = params["m"].doubleValue();
    }
    return make_unique<NakagamiFading>(this, constM, m);
}

unique_ptr<AnalogueModel> PhyLayer80211p::initializeSimplePathlossModel(ParameterMap& params)
{

    // init with default value
    double alpha = 2.0;
    bool useTorus = world->useTorus();
    const Coord& playgroundSize = *(world->getPgs());

    // get alpha-coefficient from config
    ParameterMap::iterator it = params.find("alpha");

    if (it != params.end()) { // parameter alpha has been specified in config.xml
        // set alpha
        alpha = it->second.doubleValue();
        EV_TRACE << "createPathLossModel(): alpha set from config.xml to " << alpha << endl;

        // check whether alpha is not smaller than specified in ConnectionManager
        if (cc->hasPar("alpha") && alpha < cc->par("alpha").doubleValue()) {
            // throw error
            throw cRuntimeError("TestPhyLayer::createPathLossModel(): alpha can't be smaller than specified in \
                   ConnectionManager. Please adjust your config.xml file accordingly");
        }
    }
    else // alpha has not been specified in config.xml
    {
        if (cc->hasPar("alpha")) { // parameter alpha has been specified in ConnectionManager
            // set alpha according to ConnectionManager
            alpha = cc->par("alpha").doubleValue();
            EV_TRACE << "createPathLossModel(): alpha set from ConnectionManager to " << alpha << endl;
        }
        else // alpha has not been specified in ConnectionManager
        {
            // keep alpha at default value
            EV_TRACE << "createPathLossModel(): alpha set from default value to " << alpha << endl;
        }
    }

    return make_unique<SimplePathlossModel>(this, alpha, useTorus, playgroundSize);
}

unique_ptr<AnalogueModel> PhyLayer80211p::initializePERModel(ParameterMap& params)
{
    double per = params["packetErrorRate"].doubleValue();
    return make_unique<PERModel>(this, per);
}

unique_ptr<Decider> PhyLayer80211p::getDeciderFromName(std::string name, ParameterMap& params)
{
    if (name == "Decider80211p") {
        protocolId = IEEE_80211;
        return initializeDecider80211p(params);
    }
    return BasePhyLayer::getDeciderFromName(name, params);
}

unique_ptr<AnalogueModel> PhyLayer80211p::initializeSimpleObstacleShadowing(ParameterMap& params)
{

    // init with default value
    bool useTorus = world->useTorus();
    const Coord& playgroundSize = *(world->getPgs());

    ParameterMap::iterator it;

    ObstacleControl* obstacleControlP = ObstacleControlAccess().getIfExists();
    if (!obstacleControlP) throw cRuntimeError("initializeSimpleObstacleShadowing(): cannot find ObstacleControl module");
    return make_unique<SimpleObstacleShadowing>(this, *obstacleControlP, useTorus, playgroundSize);
}

unique_ptr<AnalogueModel> PhyLayer80211p::initializeVehicleObstacleShadowing(ParameterMap& params)
{

    // init with default value
    bool useTorus = world->useTorus();
    const Coord& playgroundSize = *(world->getPgs());

    ParameterMap::iterator it;

    VehicleObstacleControl* vehicleObstacleControlP = VehicleObstacleControlAccess().getIfExists();
    if (!vehicleObstacleControlP) throw cRuntimeError("initializeVehicleObstacleShadowing(): cannot find VehicleObstacleControl module");
    return make_unique<VehicleObstacleShadowing>(this, *vehicleObstacleControlP, useTorus, playgroundSize);
}

unique_ptr<Decider> PhyLayer80211p::initializeDecider80211p(ParameterMap& params)
{
    double centerFreq = params["centerFrequency"];
    auto dec = make_unique<Decider80211p>(this, this, minPowerLevel, ccaThreshold, allowTxDuringRx, centerFreq, findHost()->getIndex(), collectCollisionStatistics);
    dec->setPath(getParentModule()->getFullPath());
    return unique_ptr<Decider>(std::move(dec));
}

void PhyLayer80211p::changeListeningChannel(Channel channel)
{
    Decider80211p* dec = dynamic_cast<Decider80211p*>(decider.get());
    ASSERT(dec);

    double freq = IEEE80211ChannelFrequencies.at(channel);
    dec->changeFrequency(freq);
}

void PhyLayer80211p::handleSelfMessage(cMessage* msg)
{

    switch (msg->getKind()) {
    // transmission overBasePhyLayer::
    case TX_OVER: {
        ASSERT(msg == txOverTimer);
        sendControlMsgToMac(new cMessage("Transmission over", TX_OVER));
        // check if there is another packet on the chan, and change the chan-state to idle
        Decider80211p* dec = dynamic_cast<Decider80211p*>(decider.get());
        ASSERT(dec);
        if (dec->cca(simTime(), nullptr)) {
            // chan is idle
            EV_TRACE << "Channel idle after transmit!\n";
            dec->setChannelIdleStatus(true);
        }
        else {
            EV_TRACE << "Channel not yet idle after transmit!\n";
        }
        break;
    }
    // radio switch over
    case RADIO_SWITCHING_OVER:
        ASSERT(msg == radioSwitchingOverTimer);
        BasePhyLayer::finishRadioSwitching();
        break;

    // AirFrame
    case AIR_FRAME:
        BasePhyLayer::handleAirFrame(static_cast<AirFrame*>(msg));
        break;

    default:
        break;
    }
}

unique_ptr<AirFrame> PhyLayer80211p::createAirFrame(cPacket* macPkt)
{
    return make_unique<AirFrame11p>(macPkt->getName(), AIR_FRAME);
}

void PhyLayer80211p::attachSignal(AirFrame* airFrame, cObject* ctrlInfo)
{
    const auto ctrlInfo11p = check_and_cast<MacToPhyControlInfo11p*>(ctrlInfo);

    const auto duration = getFrameDuration(airFrame->getEncapsulatedPacket()->getBitLength(), ctrlInfo11p->mcs);
    ASSERT(duration > 0);
    Signal signal(overallSpectrum, simTime(), duration);
    auto freqIndex = overallSpectrum.indexOf(IEEE80211ChannelFrequencies.at(ctrlInfo11p->channelNr));
    signal.at(freqIndex - 1) = ctrlInfo11p->txPower_mW;
    signal.at(freqIndex) = ctrlInfo11p->txPower_mW;
    signal.at(freqIndex + 1) = ctrlInfo11p->txPower_mW;
    signal.setDataStart(freqIndex - 1);
    signal.setDataEnd(freqIndex + 1);
    signal.setCenterFrequencyIndex(freqIndex);
    // copy the signal into the AirFrame
    airFrame->setSignal(signal);
    airFrame->setDuration(signal.getDuration());
    airFrame->setMcs(static_cast<int>(ctrlInfo11p->mcs));
}

int PhyLayer80211p::getRadioState()
{
    return BasePhyLayer::getRadioState();
};

simtime_t PhyLayer80211p::setRadioState(int rs)
{
    if (rs == Radio::TX) decider->switchToTx();
    return BasePhyLayer::setRadioState(rs);
}

void PhyLayer80211p::setCCAThreshold(double ccaThreshold_dBm)
{
    ccaThreshold = pow(10, ccaThreshold_dBm / 10);
    Decider80211p* dec = dynamic_cast<Decider80211p*>(decider.get());
    ASSERT(dec);
    dec->setCCAThreshold(ccaThreshold_dBm);
}
double PhyLayer80211p::getCCAThreshold()
{
    return 10 * log10(ccaThreshold);
}

void PhyLayer80211p::notifyMacAboutRxStart(bool enable)
{
    Decider80211p* dec = dynamic_cast<Decider80211p*>(decider.get());
    ASSERT(dec);
    dec->setNotifyRxStart(enable);
}

void PhyLayer80211p::requestChannelStatusIfIdle()
{
    Enter_Method_Silent();
    Decider80211p* dec = dynamic_cast<Decider80211p*>(decider.get());
    ASSERT(dec);
    if (dec->cca(simTime(), nullptr)) {
        // chan is idle
        EV_TRACE << "Request channel status: channel idle!\n";
        dec->setChannelIdleStatus(true);
    }
}

simtime_t PhyLayer80211p::getFrameDuration(int payloadLengthBits, MCS mcs) const
{
    Enter_Method_Silent();
    ASSERT(mcs != MCS::undefined);
    auto ndbps = getNDBPS(mcs);
    // calculate frame duration according to Equation (17-29) of the IEEE 802.11-2007 standard
    return PHY_HDR_PREAMBLE_DURATION + PHY_HDR_PLCPSIGNAL_DURATION + T_SYM_80211P * ceil(static_cast<double>(16 + payloadLengthBits + 6) / (ndbps));
}
