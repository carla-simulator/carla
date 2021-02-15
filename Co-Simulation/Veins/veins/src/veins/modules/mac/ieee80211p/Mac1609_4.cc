//
// Copyright (C) 2016 David Eckhoff <david.eckhoff@fau.de>
// Copyright (C) 2018 Fabian Bronner <fabian.bronner@ccs-labs.org>
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

#include "veins/modules/mac/ieee80211p/Mac1609_4.h"
#include <iterator>

#include "veins/modules/phy/DeciderResult80211.h"
#include "veins/base/phyLayer/PhyToMacControlInfo.h"
#include "veins/modules/messages/PhyControlMessage_m.h"
#include "veins/modules/messages/AckTimeOutMessage_m.h"

using namespace veins;

using std::unique_ptr;

Define_Module(veins::Mac1609_4);

const simsignal_t Mac1609_4::sigChannelBusy = registerSignal("org_car2x_veins_modules_mac_sigChannelBusy");
const simsignal_t Mac1609_4::sigCollision = registerSignal("org_car2x_veins_modules_mac_sigCollision");

void Mac1609_4::initialize(int stage)
{
    BaseMacLayer::initialize(stage);
    if (stage == 0) {

        phy11p = FindModule<Mac80211pToPhy11pInterface*>::findSubModule(getParentModule());
        ASSERT(phy11p);

        // this is required to circumvent double precision issues with constants from CONST80211p.h
        ASSERT(simTime().getScaleExp() == -12);

        txPower = par("txPower").doubleValue();
        setParametersForBitrate(par("bitrate"));

        // unicast parameters
        dot11RTSThreshold = par("dot11RTSThreshold");
        dot11ShortRetryLimit = par("dot11ShortRetryLimit");
        dot11LongRetryLimit = par("dot11LongRetryLimit");
        ackLength = par("ackLength");
        useAcks = par("useAcks").boolValue();
        ackErrorRate = par("ackErrorRate").doubleValue();
        rxStartIndication = false;
        ignoreChannelState = false;
        waitUntilAckRXorTimeout = false;
        stopIgnoreChannelStateMsg = new cMessage("ChannelStateMsg");

        myId = getParentModule()->getParentModule()->getFullPath();
        // create two edca systems

        myEDCA[ChannelType::control] = make_unique<EDCA>(this, ChannelType::control, par("queueSize"));
        myEDCA[ChannelType::control]->myId = myId;
        myEDCA[ChannelType::control]->myId.append(" CCH");
        myEDCA[ChannelType::control]->createQueue(2, (((CWMIN_11P + 1) / 4) - 1), (((CWMIN_11P + 1) / 2) - 1), AC_VO);
        myEDCA[ChannelType::control]->createQueue(3, (((CWMIN_11P + 1) / 2) - 1), CWMIN_11P, AC_VI);
        myEDCA[ChannelType::control]->createQueue(6, CWMIN_11P, CWMAX_11P, AC_BE);
        myEDCA[ChannelType::control]->createQueue(9, CWMIN_11P, CWMAX_11P, AC_BK);

        myEDCA[ChannelType::service] = make_unique<EDCA>(this, ChannelType::service, par("queueSize"));
        myEDCA[ChannelType::service]->myId = myId;
        myEDCA[ChannelType::service]->myId.append(" SCH");
        myEDCA[ChannelType::service]->createQueue(2, (((CWMIN_11P + 1) / 4) - 1), (((CWMIN_11P + 1) / 2) - 1), AC_VO);
        myEDCA[ChannelType::service]->createQueue(3, (((CWMIN_11P + 1) / 2) - 1), CWMIN_11P, AC_VI);
        myEDCA[ChannelType::service]->createQueue(6, CWMIN_11P, CWMAX_11P, AC_BE);
        myEDCA[ChannelType::service]->createQueue(9, CWMIN_11P, CWMAX_11P, AC_BK);

        useSCH = par("useServiceChannel").boolValue();
        if (useSCH) {
            if (useAcks) throw cRuntimeError("Unicast model does not support channel switching");
            // set the initial service channel
            int serviceChannel = par("serviceChannel");
            switch (serviceChannel) {
            case 1:
                mySCH = Channel::sch1;
                break;
            case 2:
                mySCH = Channel::sch2;
                break;
            case 3:
                mySCH = Channel::sch3;
                break;
            case 4:
                mySCH = Channel::sch4;
                break;
            default:
                throw cRuntimeError("Service Channel must be between 1 and 4");
                break;
            }
        }

        headerLength = par("headerLength");

        nextMacEvent = new cMessage("next Mac Event");

        if (useSCH) {
            uint64_t currenTime = simTime().raw();
            uint64_t switchingTime = SWITCHING_INTERVAL_11P.raw();
            double timeToNextSwitch = (double) (switchingTime - (currenTime % switchingTime)) / simTime().getScale();
            if ((currenTime / switchingTime) % 2 == 0) {
                setActiveChannel(ChannelType::control);
            }
            else {
                setActiveChannel(ChannelType::service);
            }

            // channel switching active
            nextChannelSwitch = new cMessage("Channel Switch");
            // add a little bit of offset between all vehicles, but no more than syncOffset
            simtime_t offset = dblrand() * par("syncOffset").doubleValue();
            scheduleAt(simTime() + offset + timeToNextSwitch, nextChannelSwitch);
        }
        else {
            // no channel switching
            nextChannelSwitch = nullptr;
            setActiveChannel(ChannelType::control);
        }

        // stats
        statsReceivedPackets = 0;
        statsReceivedBroadcasts = 0;
        statsSentPackets = 0;
        statsSentAcks = 0;
        statsTXRXLostPackets = 0;
        statsSNIRLostPackets = 0;
        statsDroppedPackets = 0;
        statsNumTooLittleTime = 0;
        statsNumInternalContention = 0;
        statsNumBackoff = 0;
        statsSlotsBackoff = 0;
        statsTotalBusyTime = 0;

        idleChannel = true;
        lastBusy = simTime();
        channelIdle(true);
    }
}

void Mac1609_4::handleSelfMsg(cMessage* msg)
{
    if (msg == stopIgnoreChannelStateMsg) {
        ignoreChannelState = false;
        return;
    }

    if (AckTimeOutMessage* ackTimeOutMsg = dynamic_cast<AckTimeOutMessage*>(msg)) {
        handleAckTimeOut(ackTimeOutMsg);
        return;
    }

    if (msg == nextChannelSwitch) {
        ASSERT(useSCH);

        scheduleAt(simTime() + SWITCHING_INTERVAL_11P, nextChannelSwitch);

        switch (activeChannel) {
        case ChannelType::control:
            EV_TRACE << "CCH --> SCH" << std::endl;
            channelBusySelf(false);
            setActiveChannel(ChannelType::service);
            channelIdle(true);
            phy11p->changeListeningChannel(mySCH);
            break;
        case ChannelType::service:
            EV_TRACE << "SCH --> CCH" << std::endl;
            channelBusySelf(false);
            setActiveChannel(ChannelType::control);
            channelIdle(true);
            phy11p->changeListeningChannel(Channel::cch);
            break;
        }
        // schedule next channel switch in 50ms
    }
    else if (msg == nextMacEvent) {

        // we actually came to the point where we can send a packet
        channelBusySelf(true);
        BaseFrame1609_4* pktToSend = myEDCA[activeChannel]->initiateTransmit(lastIdle);
        ASSERT(pktToSend);

        lastAC = mapUserPriority(pktToSend->getUserPriority());
        lastWSM = pktToSend;

        EV_TRACE << "MacEvent received. Trying to send packet with priority" << lastAC << std::endl;

        // send the packet
        Mac80211Pkt* mac = new Mac80211Pkt(pktToSend->getName(), pktToSend->getKind());
        if (pktToSend->getRecipientAddress() != LAddress::L2BROADCAST()) {
            mac->setDestAddr(pktToSend->getRecipientAddress());
        }
        else {
            mac->setDestAddr(LAddress::L2BROADCAST());
        }
        mac->setSrcAddr(myMacAddr);
        mac->encapsulate(pktToSend->dup());

        MCS usedMcs = mcs;
        double txPower_mW;
        PhyControlMessage* controlInfo = dynamic_cast<PhyControlMessage*>(pktToSend->getControlInfo());
        if (controlInfo) {
            // if MCS is not specified, just use the default one
            MCS explicitMcs = static_cast<MCS>(controlInfo->getMcs());
            if (explicitMcs != MCS::undefined) {
                usedMcs = explicitMcs;
            }
            // apply the same principle to tx power
            txPower_mW = controlInfo->getTxPower_mW();
            if (txPower_mW < 0) {
                txPower_mW = txPower;
            }
        }
        else {
            txPower_mW = txPower;
        }

        simtime_t sendingDuration = RADIODELAY_11P + phy11p->getFrameDuration(mac->getBitLength(), usedMcs);
        EV_TRACE << "Sending duration will be" << sendingDuration << std::endl;
        if ((!useSCH) || (timeLeftInSlot() > sendingDuration)) {
            if (useSCH) EV_TRACE << " Time in this slot left: " << timeLeftInSlot() << std::endl;

            Channel channelNr = (activeChannel == ChannelType::control) ? Channel::cch : mySCH;
            double freq = IEEE80211ChannelFrequencies.at(channelNr);

            EV_TRACE << "Sending a Packet. Frequency " << freq << " Priority" << lastAC << std::endl;
            sendFrame(mac, RADIODELAY_11P, channelNr, usedMcs, txPower_mW);

            // schedule ack timeout for unicast packets
            if (pktToSend->getRecipientAddress() != LAddress::L2BROADCAST() && useAcks) {
                waitUntilAckRXorTimeout = true;
                // PHY-RXSTART.indication should be received within ackWaitTime
                // sifs + slot + rx_delay: see 802.11-2012 9.3.2.8 (32us + 13us + 49us = 94us)
                simtime_t ackWaitTime(94, SIMTIME_US);
                // update id in the retransmit timer
                myEDCA[activeChannel]->myQueues[lastAC].ackTimeOut->setWsmId(pktToSend->getTreeId());
                simtime_t timeOut = sendingDuration + ackWaitTime;
                scheduleAt(simTime() + timeOut, myEDCA[activeChannel]->myQueues[lastAC].ackTimeOut);
            }
        }
        else { // not enough time left now
            EV_TRACE << "Too little Time left. This packet cannot be send in this slot." << std::endl;
            statsNumTooLittleTime++;
            // revoke TXOP
            myEDCA[activeChannel]->revokeTxOPs();
            delete mac;
            channelIdle();
            // do nothing. contention will automatically start after channel switch
        }
    }
}

void Mac1609_4::handleUpperControl(cMessage* msg)
{
    ASSERT(false);
}

void Mac1609_4::handleUpperMsg(cMessage* msg)
{

    BaseFrame1609_4* thisMsg = check_and_cast<BaseFrame1609_4*>(msg);

    t_access_category ac = mapUserPriority(thisMsg->getUserPriority());

    EV_TRACE << "Received a message from upper layer for channel " << thisMsg->getChannelNumber() << " Access Category (Priority):  " << ac << std::endl;

    ChannelType chan;

    if (static_cast<Channel>(thisMsg->getChannelNumber()) == Channel::cch) {
        chan = ChannelType::control;
    }
    else {
        ASSERT(useSCH);
        thisMsg->setChannelNumber(static_cast<int>(mySCH));
        chan = ChannelType::service;
    }

    int num = myEDCA[chan]->queuePacket(ac, thisMsg);

    // packet was dropped in Mac
    if (num == -1) {
        statsDroppedPackets++;
        return;
    }

    // if this packet is not at the front of a new queue we dont have to reevaluate times
    EV_TRACE << "sorted packet into queue of EDCA " << static_cast<int>(chan) << " this packet is now at position: " << num << std::endl;

    if (chan == activeChannel) {
        EV_TRACE << "this packet is for the currently active channel" << std::endl;
    }
    else {
        EV_TRACE << "this packet is NOT for the currently active channel" << std::endl;
    }

    if (num == 1 && idleChannel == true && chan == activeChannel) {

        simtime_t nextEvent = myEDCA[chan]->startContent(lastIdle, guardActive());

        if (nextEvent != -1) {
            if ((!useSCH) || (nextEvent <= nextChannelSwitch->getArrivalTime())) {
                if (nextMacEvent->isScheduled()) {
                    cancelEvent(nextMacEvent);
                }
                scheduleAt(nextEvent, nextMacEvent);
                EV_TRACE << "Updated nextMacEvent:" << nextMacEvent->getArrivalTime().raw() << std::endl;
            }
            else {
                EV_TRACE << "Too little time in this interval. Will not schedule nextMacEvent" << std::endl;
                // it is possible that this queue has an txop. we have to revoke it
                myEDCA[activeChannel]->revokeTxOPs();
                statsNumTooLittleTime++;
            }
        }
        else {
            cancelEvent(nextMacEvent);
        }
    }
    if (num == 1 && idleChannel == false && myEDCA[chan]->myQueues[ac].currentBackoff == 0 && chan == activeChannel) {
        myEDCA[chan]->backoff(ac);
    }
}

void Mac1609_4::handleLowerControl(cMessage* msg)
{
    if (msg->getKind() == MacToPhyInterface::PHY_RX_START) {
        rxStartIndication = true;
    }
    else if (msg->getKind() == MacToPhyInterface::PHY_RX_END_WITH_SUCCESS) {
        // PHY_RX_END_WITH_SUCCESS will get packet soon! Nothing to do here
    }
    else if (msg->getKind() == MacToPhyInterface::PHY_RX_END_WITH_FAILURE) {
        // RX failed at phy. Time to retransmit
        phy11p->notifyMacAboutRxStart(false);
        rxStartIndication = false;
        handleRetransmit(lastAC);
    }
    else if (msg->getKind() == MacToPhyInterface::TX_OVER) {

        EV_TRACE << "Successfully transmitted a packet on " << lastAC << std::endl;

        phy->setRadioState(Radio::RX);

        if (!dynamic_cast<Mac80211Ack*>(lastMac.get())) {
            // message was sent
            // update EDCA queue. go into post-transmit backoff and set cwCur to cwMin
            myEDCA[activeChannel]->postTransmit(lastAC, lastWSM, useAcks);
        }
        // channel just turned idle.
        // don't set the chan to idle. the PHY layer decides, not us.

        if (guardActive()) {
            throw cRuntimeError("We shouldnt have sent a packet in guard!");
        }
    }
    else if (msg->getKind() == Mac80211pToPhy11pInterface::CHANNEL_BUSY) {
        channelBusy();
    }
    else if (msg->getKind() == Mac80211pToPhy11pInterface::CHANNEL_IDLE) {
        // Decider80211p::processSignalEnd() sends up the received packet to MAC followed by control message CHANNEL_IDLE in the same timestamp.
        // If we received a unicast frame (first event scheduled by Decider), MAC immediately schedules an ACK message and wants to switch the radio to TX mode.
        // So, the notification for channel idle from phy is undesirable and we skip it here.
        // After ACK TX is over, PHY will inform the channel status again.
        if (ignoreChannelState) {
            // Skipping channelidle because we are about to send an ack regardless of the channel state
        }
        else {
            channelIdle();
        }
    }
    else if (msg->getKind() == Decider80211p::BITERROR || msg->getKind() == Decider80211p::COLLISION) {
        statsSNIRLostPackets++;
        EV_TRACE << "A packet was not received due to biterrors" << std::endl;
    }
    else if (msg->getKind() == Decider80211p::RECWHILESEND) {
        statsTXRXLostPackets++;
        EV_TRACE << "A packet was not received because we were sending while receiving" << std::endl;
    }
    else if (msg->getKind() == MacToPhyInterface::RADIO_SWITCHING_OVER) {
        EV_TRACE << "Phylayer said radio switching is done" << std::endl;
    }
    else if (msg->getKind() == BaseDecider::PACKET_DROPPED) {
        phy->setRadioState(Radio::RX);
        EV_TRACE << "Phylayer said packet was dropped" << std::endl;
    }
    else {
        EV_WARN << "Invalid control message type (type=NOTHING) : name=" << msg->getName() << " modulesrc=" << msg->getSenderModule()->getFullPath() << "." << std::endl;
        ASSERT(false);
    }

    if (msg->getKind() == Decider80211p::COLLISION) {
        emit(sigCollision, true);
    }

    delete msg;
}

void Mac1609_4::setActiveChannel(ChannelType state)
{
    activeChannel = state;
    ASSERT(state == ChannelType::control || (useSCH && state == ChannelType::service));
}

void Mac1609_4::finish()
{
    for (auto&& p : myEDCA) {
        statsNumInternalContention += p.second->statsNumInternalContention;
        statsNumBackoff += p.second->statsNumBackoff;
        statsSlotsBackoff += p.second->statsSlotsBackoff;
    }

    recordScalar("ReceivedUnicastPackets", statsReceivedPackets);
    recordScalar("ReceivedBroadcasts", statsReceivedBroadcasts);
    recordScalar("SentPackets", statsSentPackets);
    recordScalar("SentAcknowledgements", statsSentAcks);
    recordScalar("SNIRLostPackets", statsSNIRLostPackets);
    recordScalar("RXTXLostPackets", statsTXRXLostPackets);
    recordScalar("TotalLostPackets", statsSNIRLostPackets + statsTXRXLostPackets);
    recordScalar("DroppedPacketsInMac", statsDroppedPackets);
    recordScalar("TooLittleTime", statsNumTooLittleTime);
    recordScalar("TimesIntoBackoff", statsNumBackoff);
    recordScalar("SlotsBackoff", statsSlotsBackoff);
    recordScalar("NumInternalContention", statsNumInternalContention);
    recordScalar("totalBusyTime", statsTotalBusyTime.dbl());
}

Mac1609_4::~Mac1609_4()
{
    if (nextMacEvent) {
        cancelAndDelete(nextMacEvent);
        nextMacEvent = nullptr;
    }

    if (nextChannelSwitch) {
        cancelAndDelete(nextChannelSwitch);
        nextChannelSwitch = nullptr;
    }

    if (stopIgnoreChannelStateMsg) {
        cancelAndDelete(stopIgnoreChannelStateMsg);
        stopIgnoreChannelStateMsg = nullptr;
    }
};

void Mac1609_4::sendFrame(Mac80211Pkt* frame, simtime_t delay, Channel channelNr, MCS mcs, double txPower_mW)
{
    phy->setRadioState(Radio::TX); // give time for the radio to be in Tx state before transmitting

    delay = std::max(delay, RADIODELAY_11P); // wait at least for the radio to switch

    attachControlInfo(frame, channelNr, mcs, txPower_mW);
    check_and_cast<MacToPhyControlInfo11p*>(frame->getControlInfo());

    lastMac.reset(frame->dup());
    sendDelayed(frame, delay, lowerLayerOut);

    if (dynamic_cast<Mac80211Ack*>(frame)) {
        statsSentAcks += 1;
    }
    else {
        statsSentPackets += 1;
    }
}

void Mac1609_4::attachControlInfo(Mac80211Pkt* mac, Channel channelNr, MCS mcs, double txPower_mW)
{
    auto cinfo = new MacToPhyControlInfo11p(channelNr, mcs, txPower_mW);
    mac->setControlInfo(cinfo);
}

/* checks if guard is active */
bool Mac1609_4::guardActive() const
{
    if (!useSCH) return false;
    if (simTime().dbl() - nextChannelSwitch->getSendingTime() <= GUARD_INTERVAL_11P) return true;
    return false;
}

/* returns the time until the guard is over */
simtime_t Mac1609_4::timeLeftTillGuardOver() const
{
    ASSERT(useSCH);
    simtime_t sTime = simTime();
    if (sTime - nextChannelSwitch->getSendingTime() <= GUARD_INTERVAL_11P) {
        return GUARD_INTERVAL_11P - (sTime - nextChannelSwitch->getSendingTime());
    }
    else
        return 0;
}

/* returns the time left in this channel window */
simtime_t Mac1609_4::timeLeftInSlot() const
{
    ASSERT(useSCH);
    return nextChannelSwitch->getArrivalTime() - simTime();
}

/* Will change the Service Channel on which the mac layer is listening and sending */
void Mac1609_4::changeServiceChannel(Channel cN)
{
    ASSERT(useSCH);
    mySCH = static_cast<Channel>(cN);
    if (mySCH != Channel::sch1 && mySCH != Channel::sch2 && mySCH != Channel::sch3 && mySCH != Channel::sch4) {
        throw cRuntimeError("This Service Channel doesnt exit: %d", cN);
    }

    if (activeChannel == ChannelType::service) {
        // change to new chan immediately if we are in a SCH slot,
        // otherwise it will switch to the new SCH upon next channel switch
        phy11p->changeListeningChannel(mySCH);
    }
}

void Mac1609_4::setTxPower(double txPower_mW)
{
    txPower = txPower_mW;
}
void Mac1609_4::setMCS(MCS mcs)
{
    ASSERT2(mcs != MCS::undefined, "invalid MCS selected");
    this->mcs = mcs;
}

void Mac1609_4::setCCAThreshold(double ccaThreshold_dBm)
{
    phy11p->setCCAThreshold(ccaThreshold_dBm);
}

void Mac1609_4::handleBroadcast(Mac80211Pkt* macPkt, DeciderResult80211* res)
{
    statsReceivedBroadcasts++;
    unique_ptr<BaseFrame1609_4> wsm(check_and_cast<BaseFrame1609_4*>(macPkt->decapsulate()));
    wsm->setControlInfo(new PhyToMacControlInfo(res));
    sendUp(wsm.release());
}

void Mac1609_4::handleLowerMsg(cMessage* msg)
{
    Mac80211Pkt* macPkt = check_and_cast<Mac80211Pkt*>(msg);

    // pass information about received frame to the upper layers
    DeciderResult80211* macRes = check_and_cast<DeciderResult80211*>(PhyToMacControlInfo::getDeciderResult(msg));
    DeciderResult80211* res = new DeciderResult80211(*macRes);

    long dest = macPkt->getDestAddr();

    EV_TRACE << "Received frame name= " << macPkt->getName() << ", myState= src=" << macPkt->getSrcAddr() << " dst=" << macPkt->getDestAddr() << " myAddr=" << myMacAddr << std::endl;

    if (dest == myMacAddr) {
        if (auto* ack = dynamic_cast<Mac80211Ack*>(macPkt)) {
            ASSERT(useAcks);
            handleAck(ack);
            delete res;
        }
        else {
            unique_ptr<BaseFrame1609_4> wsm(check_and_cast<BaseFrame1609_4*>(macPkt->decapsulate()));
            wsm->setControlInfo(new PhyToMacControlInfo(res));
            handleUnicast(macPkt->getSrcAddr(), std::move(wsm));
        }
    }
    else if (dest == LAddress::L2BROADCAST()) {
        handleBroadcast(macPkt, res);
    }
    else {
        EV_TRACE << "Packet not for me" << std::endl;
        delete res;
    }
    delete macPkt;

    if (rxStartIndication) {
        // We have handled/processed the incoming packet
        // Since we reached here, we were expecting an ack but we didnt get it, so retransmission should take place
        phy11p->notifyMacAboutRxStart(false);
        rxStartIndication = false;
        handleRetransmit(lastAC);
    }
}

int Mac1609_4::EDCA::queuePacket(t_access_category ac, BaseFrame1609_4* msg)
{

    if (maxQueueSize && myQueues[ac].queue.size() >= maxQueueSize) {
        delete msg;
        return -1;
    }
    myQueues[ac].queue.push(msg);
    return myQueues[ac].queue.size();
}

void Mac1609_4::EDCA::createQueue(int aifsn, int cwMin, int cwMax, t_access_category ac)
{

    if (myQueues.find(ac) != myQueues.end()) {
        throw cRuntimeError("You can only add one queue per Access Category per EDCA subsystem");
    }

    EDCAQueue newQueue(aifsn, cwMin, cwMax, ac);
    myQueues[ac] = newQueue;
}

Mac1609_4::t_access_category Mac1609_4::mapUserPriority(int prio)
{
    // Map user priority to access category, based on IEEE Std 802.11-2012, Table 9-1
    switch (prio) {
    case 1:
        return AC_BK;
    case 2:
        return AC_BK;
    case 0:
        return AC_BE;
    case 3:
        return AC_BE;
    case 4:
        return AC_VI;
    case 5:
        return AC_VI;
    case 6:
        return AC_VO;
    case 7:
        return AC_VO;
    default:
        throw cRuntimeError("MacLayer received a packet with unknown priority");
        break;
    }
    return AC_VO;
}

BaseFrame1609_4* Mac1609_4::EDCA::initiateTransmit(simtime_t lastIdle)
{

    // iterate through the queues to return the packet we want to send
    BaseFrame1609_4* pktToSend = nullptr;

    simtime_t idleTime = simTime() - lastIdle;

    EV_TRACE << "Initiating transmit at " << simTime() << ". I've been idle since " << idleTime << std::endl;

    // As t_access_category is sorted by priority, we iterate back to front.
    // This realizes the behavior documented in IEEE Std 802.11-2012 Section 9.2.4.2; that is, "data frames from the higher priority AC" win an internal collision.
    // The phrase "EDCAF of higher UP" of IEEE Std 802.11-2012 Section 9.19.2.3 is assumed to be meaningless.
    for (auto iter = myQueues.rbegin(); iter != myQueues.rend(); iter++) {
        if (iter->second.queue.size() != 0 && !iter->second.waitForAck) {
            if (idleTime >= iter->second.aifsn * SLOTLENGTH_11P + SIFS_11P && iter->second.txOP == true) {

                EV_TRACE << "Queue " << iter->first << " is ready to send!" << std::endl;

                iter->second.txOP = false;
                // this queue is ready to send
                if (pktToSend == nullptr) {
                    pktToSend = iter->second.queue.front();
                }
                else {
                    // there was already another packet ready. we have to go increase cw and go into backoff. It's called internal contention and its wonderful

                    statsNumInternalContention++;
                    iter->second.cwCur = std::min(iter->second.cwMax, (iter->second.cwCur + 1) * 2 - 1);
                    iter->second.currentBackoff = owner->intuniform(0, iter->second.cwCur);
                    EV_TRACE << "Internal contention for queue " << iter->first << " : " << iter->second.currentBackoff << ". Increase cwCur to " << iter->second.cwCur << std::endl;
                }
            }
        }
    }

    if (pktToSend == nullptr) {
        throw cRuntimeError("No packet was ready");
    }
    return pktToSend;
}

simtime_t Mac1609_4::EDCA::startContent(simtime_t idleSince, bool guardActive)
{

    EV_TRACE << "Restarting contention." << std::endl;

    simtime_t nextEvent = -1;

    simtime_t idleTime = SimTime().setRaw(std::max((int64_t) 0, (simTime() - idleSince).raw()));
    ;

    lastStart = idleSince;

    EV_TRACE << "Channel is already idle for:" << idleTime << " since " << idleSince << std::endl;

    // this returns the nearest possible event in this EDCA subsystem after a busy channel

    for (auto&& p : myQueues) {
        auto& accessCategory = p.first;
        auto& edcaQueue = p.second;
        if (edcaQueue.queue.size() != 0 && !edcaQueue.waitForAck) {

            /* 1609_4 says that when attempting to send (backoff == 0) when guard is active, a random backoff is invoked */

            if (guardActive == true && edcaQueue.currentBackoff == 0) {
                // cw is not increased
                edcaQueue.currentBackoff = owner->intuniform(0, edcaQueue.cwCur);
                statsNumBackoff++;
            }

            simtime_t DIFS = edcaQueue.aifsn * SLOTLENGTH_11P + SIFS_11P;

            // the next possible time to send can be in the past if the channel was idle for a long time, meaning we COULD have sent earlier if we had a packet
            simtime_t possibleNextEvent = DIFS + edcaQueue.currentBackoff * SLOTLENGTH_11P;

            EV_TRACE << "Waiting Time for Queue " << accessCategory << ":" << possibleNextEvent << "=" << edcaQueue.aifsn << " * " << SLOTLENGTH_11P << " + " << SIFS_11P << "+" << edcaQueue.currentBackoff << "*" << SLOTLENGTH_11P << "; Idle time: " << idleTime << std::endl;

            if (idleTime > possibleNextEvent) {
                EV_TRACE << "Could have already send if we had it earlier" << std::endl;
                // we could have already sent. round up to next boundary
                simtime_t base = idleSince + DIFS;
                possibleNextEvent = simTime() - simtime_t().setRaw((simTime() - base).raw() % SLOTLENGTH_11P.raw()) + SLOTLENGTH_11P;
            }
            else {
                // we are gonna send in the future
                EV_TRACE << "Sending in the future" << std::endl;
                possibleNextEvent = idleSince + possibleNextEvent;
            }
            nextEvent == -1 ? nextEvent = possibleNextEvent : nextEvent = std::min(nextEvent, possibleNextEvent);
        }
    }
    return nextEvent;
}

void Mac1609_4::EDCA::stopContent(bool allowBackoff, bool generateTxOp)
{
    // update all Queues

    EV_TRACE << "Stopping Contention at " << simTime().raw() << std::endl;

    simtime_t passedTime = simTime() - lastStart;

    EV_TRACE << "Channel was idle for " << passedTime << std::endl;

    lastStart = -1; // indicate that there was no last start

    for (auto&& p : myQueues) {
        auto& accessCategory = p.first;
        auto& edcaQueue = p.second;
        if ((edcaQueue.currentBackoff != 0 || edcaQueue.queue.size() != 0) && !edcaQueue.waitForAck) {
            // check how many slots we already waited until the chan became busy

            int64_t oldBackoff = edcaQueue.currentBackoff;

            std::string info;
            if (passedTime < edcaQueue.aifsn * SLOTLENGTH_11P + SIFS_11P) {
                // we didnt even make it one DIFS :(
                info.append(" No DIFS");
            }
            else {
                // decrease the backoff by one because we made it longer than one DIFS
                edcaQueue.currentBackoff -= 1;

                // check how many slots we waited after the first DIFS
                int64_t passedSlots = (int64_t)((passedTime - SimTime(edcaQueue.aifsn * SLOTLENGTH_11P + SIFS_11P)) / SLOTLENGTH_11P);

                EV_TRACE << "Passed slots after DIFS: " << passedSlots << std::endl;

                if (edcaQueue.queue.size() == 0) {
                    // this can be below 0 because of post transmit backoff -> backoff on empty queues will not generate macevents,
                    // we dont want to generate a txOP for empty queues
                    edcaQueue.currentBackoff -= std::min(edcaQueue.currentBackoff, passedSlots);
                    info.append(" PostCommit Over");
                }
                else {
                    edcaQueue.currentBackoff -= passedSlots;
                    if (edcaQueue.currentBackoff <= -1) {
                        if (generateTxOp) {
                            edcaQueue.txOP = true;
                            info.append(" TXOP");
                        }
                        // else: this packet couldnt be sent because there was too little time. we could have generated a txop, but the channel switched
                        edcaQueue.currentBackoff = 0;
                    }
                }
            }
            EV_TRACE << "Updating backoff for Queue " << accessCategory << ": " << oldBackoff << " -> " << edcaQueue.currentBackoff << info << std::endl;
        }
    }
}
void Mac1609_4::EDCA::backoff(t_access_category ac)
{
    myQueues[ac].currentBackoff = owner->intuniform(0, myQueues[ac].cwCur);
    statsSlotsBackoff += myQueues[ac].currentBackoff;
    statsNumBackoff++;
    EV_TRACE << "Going into Backoff because channel was busy when new packet arrived from upperLayer" << std::endl;
}

void Mac1609_4::EDCA::postTransmit(t_access_category ac, BaseFrame1609_4* wsm, bool useAcks)
{
    bool holBlocking = (wsm->getRecipientAddress() != LAddress::L2BROADCAST()) && useAcks;
    if (holBlocking) {
        // mac->waitUntilAckRXorTimeout = true; // set in handleselfmsg()
        // Head of line blocking, wait until ack timeout
        myQueues[ac].waitForAck = true;
        myQueues[ac].waitOnUnicastID = wsm->getTreeId();
        ((Mac1609_4*) owner)->phy11p->notifyMacAboutRxStart(true);
    }
    else {
        myQueues[ac].waitForAck = false;
        delete myQueues[ac].queue.front();
        myQueues[ac].queue.pop();
        myQueues[ac].cwCur = myQueues[ac].cwMin;
        // post transmit backoff
        myQueues[ac].currentBackoff = owner->intuniform(0, myQueues[ac].cwCur);
        statsSlotsBackoff += myQueues[ac].currentBackoff;
        statsNumBackoff++;
        EV_TRACE << "Queue " << ac << " will go into post-transmit backoff for " << myQueues[ac].currentBackoff << " slots" << std::endl;
    }
}

Mac1609_4::EDCA::EDCA(cSimpleModule* owner, ChannelType channelType, int maxQueueLength)
    : HasLogProxy(owner)
    , owner(owner)
    , maxQueueSize(maxQueueLength)
    , channelType(channelType)
    , statsNumInternalContention(0)
    , statsNumBackoff(0)
    , statsSlotsBackoff(0)
{
}

Mac1609_4::EDCA::~EDCA()
{
    for (auto& q : myQueues) {
        auto& ackTimeout = q.second.ackTimeOut;
        if (ackTimeout) {
            owner->cancelAndDelete(ackTimeout);
            ackTimeout = nullptr;
        }
    }
}

void Mac1609_4::EDCA::revokeTxOPs()
{
    for (auto&& p : myQueues) {
        auto& edcaQueue = p.second;
        if (edcaQueue.txOP == true) {
            edcaQueue.txOP = false;
            edcaQueue.currentBackoff = 0;
        }
    }
}

void Mac1609_4::channelBusySelf(bool generateTxOp)
{

    // the channel turned busy because we're sending. we don't want our queues to go into backoff
    // internal contention is already handled in initiateTransmission

    if (!idleChannel) return;
    idleChannel = false;
    EV_TRACE << "Channel turned busy: Switch or Self-Send" << std::endl;

    lastBusy = simTime();

    // channel turned busy
    if (nextMacEvent->isScheduled() == true) {
        cancelEvent(nextMacEvent);
    }
    else {
        // the edca subsystem was not doing anything anyway.
    }
    myEDCA[activeChannel]->stopContent(false, generateTxOp);

    emit(sigChannelBusy, true);
}

void Mac1609_4::channelBusy()
{

    if (!idleChannel) return;

    // the channel turned busy because someone else is sending
    idleChannel = false;
    EV_TRACE << "Channel turned busy: External sender" << std::endl;
    lastBusy = simTime();

    // channel turned busy
    if (nextMacEvent->isScheduled() == true) {
        cancelEvent(nextMacEvent);
    }
    else {
        // the edca subsystem was not doing anything anyway.
    }
    myEDCA[activeChannel]->stopContent(true, false);

    emit(sigChannelBusy, true);
}

void Mac1609_4::channelIdle(bool afterSwitch)
{

    EV_TRACE << "Channel turned idle: Switch: " << afterSwitch << std::endl;
    if (waitUntilAckRXorTimeout) {
        return;
    }

    if (nextMacEvent->isScheduled() == true) {
        // this rare case can happen when another node's time has such a big offset that the node sent a packet although we already changed the channel
        // the workaround is not trivial and requires a lot of changes to the phy and decider
        return;
        // throw cRuntimeError("channel turned idle but contention timer was scheduled!");
    }

    idleChannel = true;

    simtime_t delay = 0;

    // account for 1609.4 guards
    if (afterSwitch) {
        //    delay = GUARD_INTERVAL_11P;
    }
    if (useSCH) {
        delay += timeLeftTillGuardOver();
    }

    // channel turned idle! lets start contention!
    lastIdle = delay + simTime();
    statsTotalBusyTime += simTime() - lastBusy;

    // get next Event from current EDCA subsystem
    simtime_t nextEvent = myEDCA[activeChannel]->startContent(lastIdle, guardActive());
    if (nextEvent != -1) {
        if ((!useSCH) || (nextEvent < nextChannelSwitch->getArrivalTime())) {
            scheduleAt(nextEvent, nextMacEvent);
            EV_TRACE << "next Event is at " << nextMacEvent->getArrivalTime().raw() << std::endl;
        }
        else {
            EV_TRACE << "Too little time in this interval. will not schedule macEvent" << std::endl;
            statsNumTooLittleTime++;
            myEDCA[activeChannel]->revokeTxOPs();
        }
    }
    else {
        EV_TRACE << "I don't have any new events in this EDCA sub system" << std::endl;
    }

    emit(sigChannelBusy, false);
}

void Mac1609_4::setParametersForBitrate(uint64_t bitrate)
{
    mcs = getMCS(bitrate, BANDWIDTH_11P);
    if (mcs == MCS::undefined) {
        throw cRuntimeError("Chosen Bitrate is not valid for 802.11p: Valid rates are: 3Mbps, 4.5Mbps, 6Mbps, 9Mbps, 12Mbps, 18Mbps, 24Mbps and 27Mbps. Please adjust your omnetpp.ini file accordingly.");
    }
}

bool Mac1609_4::isChannelSwitchingActive()
{
    return useSCH;
}

simtime_t Mac1609_4::getSwitchingInterval()
{
    return SWITCHING_INTERVAL_11P;
}

bool Mac1609_4::isCurrentChannelCCH()
{
    return (activeChannel == ChannelType::control);
}

// Unicast
void Mac1609_4::sendAck(LAddress::L2Type recpAddress, unsigned long wsmId)
{
    ASSERT(useAcks);
    // 802.11-2012 9.3.2.8
    // send an ACK after SIFS without regard of busy/ idle state of channel
    ignoreChannelState = true;
    channelBusySelf(true);

    // send the packet
    auto* mac = new Mac80211Ack("ACK");
    mac->setDestAddr(recpAddress);
    mac->setSrcAddr(myMacAddr);
    mac->setMessageId(wsmId);
    mac->setBitLength(ackLength);

    simtime_t sendingDuration = RADIODELAY_11P + phy11p->getFrameDuration(mac->getBitLength(), mcs);
    EV_TRACE << "Ack sending duration will be " << sendingDuration << std::endl;

    // TODO: check ack procedure when channel switching is allowed
    // double freq = (activeChannel == ChannelType::control) ? IEEE80211ChannelFrequencies.at(Channel::cch) : IEEE80211ChannelFrequencies.at(mySCH);
    double freq = IEEE80211ChannelFrequencies.at(Channel::cch);

    EV_TRACE << "Sending an ack. Frequency " << freq << " at time : " << simTime() + SIFS_11P << std::endl;
    sendFrame(mac, SIFS_11P, Channel::cch, mcs, txPower);
    scheduleAt(simTime() + SIFS_11P, stopIgnoreChannelStateMsg);
}

void Mac1609_4::handleUnicast(LAddress::L2Type srcAddr, unique_ptr<BaseFrame1609_4> wsm)
{
    if (useAcks) {
        sendAck(srcAddr, wsm->getTreeId());
    }

    if (handledUnicastToApp.find(wsm->getTreeId()) == handledUnicastToApp.end()) {
        handledUnicastToApp.insert(wsm->getTreeId());
        EV_TRACE << "Received a data packet addressed to me." << std::endl;
        statsReceivedPackets++;
        sendUp(wsm.release());
    }
}

void Mac1609_4::handleAck(const Mac80211Ack* ack)
{
    ASSERT2(rxStartIndication, "Not expecting ack");
    phy11p->notifyMacAboutRxStart(false);
    rxStartIndication = false;

    ChannelType chan = ChannelType::control;
    bool queueUnblocked = false;
    for (auto&& p : myEDCA[chan]->myQueues) {
        auto& accessCategory = p.first;
        auto& edcaQueue = p.second;
        if (edcaQueue.queue.size() > 0 && edcaQueue.waitForAck && (edcaQueue.waitOnUnicastID == ack->getMessageId())) {
            BaseFrame1609_4* wsm = edcaQueue.queue.front();
            edcaQueue.queue.pop();
            delete wsm;
            myEDCA[chan]->myQueues[accessCategory].cwCur = myEDCA[chan]->myQueues[accessCategory].cwMin;
            myEDCA[chan]->backoff(accessCategory);
            edcaQueue.ssrc = 0;
            edcaQueue.slrc = 0;
            edcaQueue.waitForAck = false;
            edcaQueue.waitOnUnicastID = -1;
            if (myEDCA[chan]->myQueues[accessCategory].ackTimeOut->isScheduled()) {
                cancelEvent(myEDCA[chan]->myQueues[accessCategory].ackTimeOut);
            }
            queueUnblocked = true;
        }
    }
    if (!queueUnblocked) {
        throw cRuntimeError("Could not find WSM in EDCA queues with WSM ID received in ACK");
    }
    else {
        waitUntilAckRXorTimeout = false;
    }
}

void Mac1609_4::handleAckTimeOut(AckTimeOutMessage* ackTimeOutMsg)
{
    if (rxStartIndication) {
        // Rx is already in process. Wait for it to complete.
        // In case it is not an ack, we will retransmit
        // This assigning might be redundant as it was set already in handleSelfMsg but no harm in reassigning here.
        lastAC = (t_access_category)(ackTimeOutMsg->getKind());
        return;
    }
    // We did not start receiving any packet.
    // stop receiving notification for rx start as we will retransmit
    phy11p->notifyMacAboutRxStart(false);
    // back off and try retransmission again
    handleRetransmit((t_access_category)(ackTimeOutMsg->getKind()));
    // Phy was requested not to send channel idle status on TX_OVER
    // So request the channel status now. For the case when we receive ACK, decider updates channel status itself after ACK RX
    phy11p->requestChannelStatusIfIdle();
}

void Mac1609_4::handleRetransmit(t_access_category ac)
{
    // cancel the acktime out
    if (myEDCA[ChannelType::control]->myQueues[ac].ackTimeOut->isScheduled()) {
        // This case is possible if we received PHY_RX_END_WITH_SUCCESS or FAILURE even before ack timeout
        cancelEvent(myEDCA[ChannelType::control]->myQueues[ac].ackTimeOut);
    }
    if (myEDCA[ChannelType::control]->myQueues[ac].queue.size() == 0) {
        throw cRuntimeError("Trying retransmission on empty queue...");
    }
    BaseFrame1609_4* appPkt = myEDCA[ChannelType::control]->myQueues[ac].queue.front();
    bool contend = false;
    bool retriesExceeded = false;
    // page 879 of IEEE 802.11-2012
    if (appPkt->getBitLength() <= dot11RTSThreshold) {
        myEDCA[ChannelType::control]->myQueues[ac].ssrc++;
        if (myEDCA[ChannelType::control]->myQueues[ac].ssrc <= dot11ShortRetryLimit) {
            retriesExceeded = false;
        }
        else {
            retriesExceeded = true;
        }
    }
    else {
        myEDCA[ChannelType::control]->myQueues[ac].slrc++;
        if (myEDCA[ChannelType::control]->myQueues[ac].slrc <= dot11LongRetryLimit) {
            retriesExceeded = false;
        }
        else {
            retriesExceeded = true;
        }
    }
    if (!retriesExceeded) {
        // try again!
        myEDCA[ChannelType::control]->myQueues[ac].cwCur = std::min(myEDCA[ChannelType::control]->myQueues[ac].cwMax, (myEDCA[ChannelType::control]->myQueues[ac].cwCur * 2) + 1);
        myEDCA[ChannelType::control]->backoff(ac);
        contend = true;
        // no need to reset wait on id here as we are still retransmitting same packet
        myEDCA[ChannelType::control]->myQueues[ac].waitForAck = false;
    }
    else {
        // enough tries!
        myEDCA[ChannelType::control]->myQueues[ac].queue.pop();
        if (myEDCA[ChannelType::control]->myQueues[ac].queue.size() > 0) {
            // start contention only if there are more packets in the queue
            contend = true;
        }
        delete appPkt;
        myEDCA[ChannelType::control]->myQueues[ac].cwCur = myEDCA[ChannelType::control]->myQueues[ac].cwMin;
        myEDCA[ChannelType::control]->backoff(ac);
        myEDCA[ChannelType::control]->myQueues[ac].waitForAck = false;
        myEDCA[ChannelType::control]->myQueues[ac].waitOnUnicastID = -1;
        myEDCA[ChannelType::control]->myQueues[ac].ssrc = 0;
        myEDCA[ChannelType::control]->myQueues[ac].slrc = 0;
    }
    waitUntilAckRXorTimeout = false;
    if (contend && idleChannel && !ignoreChannelState) {
        // reevaluate times -- if channel is not idle, then contention would start automatically
        cancelEvent(nextMacEvent);
        simtime_t nextEvent = myEDCA[ChannelType::control]->startContent(lastIdle, guardActive());
        scheduleAt(nextEvent, nextMacEvent);
    }
}

Mac1609_4::EDCA::EDCAQueue::EDCAQueue(int aifsn, int cwMin, int cwMax, t_access_category ac)
    : aifsn(aifsn)
    , cwMin(cwMin)
    , cwMax(cwMax)
    , cwCur(cwMin)
    , currentBackoff(0)
    , txOP(false)
    , ssrc(0)
    , slrc(0)
    , waitForAck(false)
    , waitOnUnicastID(-1)
    , ackTimeOut(new AckTimeOutMessage("AckTimeOut"))
{
    ackTimeOut->setKind(ac);
}

Mac1609_4::EDCA::EDCAQueue::~EDCAQueue()
{
    while (!queue.empty()) {
        delete queue.front();
        queue.pop();
    }
    // ackTimeOut needs to be deleted in EDCA
}
