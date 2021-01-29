//
// Copyright (C) 2011 David Eckhoff <eckhoff@cs.fau.de>
// Copyright (C) 2012 Bastian Bloessl, Stefan Joerer, Michele Segata <{bloessl,joerer,segata}@ccs-labs.org>
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

#pragma once

#include "veins/base/phyLayer/BaseDecider.h"
#include "veins/modules/utility/Consts80211p.h"
#include "veins/modules/mac/ieee80211p/Mac80211pToPhy11pInterface.h"
#include "veins/modules/phy/Decider80211pToPhy80211pInterface.h"

namespace veins {

using veins::AirFrame;

/**
 * @brief
 * Based on Decider80211.h from Karl Wessel
 * and modifications by Christopher Saloman
 *
 * @author David Eckhoff
 *
 * @ingroup decider
 *
 * @see DemoBaseApplLayer
 * @see Mac1609_4
 * @see PhyLayer80211p
 * @see Decider80211p
 */
class VEINS_API Decider80211p : public BaseDecider {
public:
    enum Decider80211ControlKinds {
        NOTHING = 22100,
        BITERROR, // the phy has recognized a bit error in the packet
        LAST_DECIDER_80211_CONTROL_KIND,
        RECWHILESEND
    };

    /**
     * @brief tell the outcome of a packetOk() call, which might be
     * correctly decoded, discarded due to low SNR or discarder due
     * to low SINR (i.e. collision)
     */
    enum PACKET_OK_RESULT {
        DECODED,
        NOT_DECODED,
        COLLISION
    };

protected:
    // threshold value for checking a SNR-map (SNR-threshold)
    double snrThreshold;

    /** @brief Power level threshold used to declare channel busy if
     * preamble portion is missed (802.11-2012 18.3.10.6
     * CCA requirements). Notice that in 18.3.10.6, the mandatory CCA threshold
     * for a 10 MHz channel is -65 dBm. However, there is another threshold
     * called CCA-ED which requirements are defined in D.2.5. For a 10 MHz
     * channel, CCA-ED threshold shall be -75 dBm. CCA-ED is required for
     * certain operating classes that shall implement CCA-ED behavior.
     * According to Table E-4 however, 802.11p channels should not implement
     * it, so the correct threshold is -65 dBm.
     * When considering ETSI ITS G5 (ETSI TS 102 687) things change again.
     * Indeed, the DCC Sensitivity Control (DSC) part of the DCC algorithm
     * changes the CCA threshold depending on the state. Values are listed
     * in Table A.3: minimum value is -95 dBm, maximum value is -65 dBm,
     * and default value is -85 dBm.
     */
    double ccaThreshold;

    /** @brief allows/disallows interruption of current reception for txing
     *
     * For a standard 802.11 MAC, starting a transmission while currently
     * receiving a frame is forbidden, as the channel is in a BUSY state.
     * For research purposes, however, one might use a custom MAC layer on
     * top of an 802.11p OFDM PHY, and decide to interrupt an ongoing
     * reception to start a transmission, whatever the reason. If the
     * following variable is set to false, simulation will be terminated
     * if this occurs. If not, simulation will continue, aborting current
     * reception.
     */
    bool allowTxDuringRx;

    /** @brief The center frequency on which the decider listens for signals */
    double centerFrequency;

    double myBusyTime;
    double myStartTime;

    std::string myPath;
    Decider80211pToPhy80211pInterface* phy11p;
    std::map<AirFrame*, int> signalStates;

    /** @brief enable/disable statistics collection for collisions
     *
     * For collecting statistics about collisions, we compute the Packet
     * Error Rate for both SNR and SINR values. This might increase the
     * simulation time, so if statistics about collisions are not needed,
     * this variable should be set to false
     */
    bool collectCollisionStats;
    /** @brief count the number of collisions */
    unsigned int collisions;

    /** @brief notify PHY-RXSTART.indication  */
    bool notifyRxStart;

protected:
    /**
     * @brief Checks a mapping against a specific threshold (element-wise).
     *
     * @return    true    , if every entry of the mapping is above threshold
     *             false    , otherwise
     *
     *
     */
    virtual DeciderResult* checkIfSignalOk(AirFrame* frame);

    simtime_t processNewSignal(AirFrame* frame) override;

    /**
     * @brief Processes a received AirFrame.
     *
     * The SNR-mapping for the Signal is created and checked against the Deciders
     * SNR-threshold. Depending on that the received AirFrame is either sent up
     * to the MAC-Layer or dropped.
     *
     * @return    usually return a value for: 'do not pass it again'
     */
    simtime_t processSignalEnd(AirFrame* frame) override;

    /** @brief computes if packet is ok or has errors*/
    enum PACKET_OK_RESULT packetOk(double snirMin, double snrMin, int lengthMPDU, double bitrate);

public:
    /**
     * @brief Initializes the Decider with a pointer to its PhyLayer and
     * specific values for threshold and minPowerLevel
     */
    Decider80211p(cComponent* owner, DeciderToPhyInterface* phy, double minPowerLevel, double ccaThreshold, bool allowTxDuringRx, double centerFrequency, int myIndex = -1, bool collectCollisionStatistics = false)
        : BaseDecider(owner, phy, minPowerLevel, myIndex)
        , ccaThreshold(ccaThreshold)
        , allowTxDuringRx(allowTxDuringRx)
        , centerFrequency(centerFrequency)
        , myBusyTime(0)
        , myStartTime(simTime().dbl())
        , collectCollisionStats(collectCollisionStatistics)
        , collisions(0)
        , notifyRxStart(false)
    {
        phy11p = dynamic_cast<Decider80211pToPhy80211pInterface*>(phy);
        ASSERT(phy11p);
    }

    void setPath(std::string myPath)
    {
        this->myPath = myPath;
    }

    bool cca(simtime_t_cref, AirFrame*);
    int getSignalState(AirFrame* frame) override;
    ~Decider80211p() override;

    void changeFrequency(double freq);

    /**
     * @brief returns the CCA threshold in dBm
     */
    double getCCAThreshold();

    /**
     * @brief sets the CCA threshold
     */
    void setCCAThreshold(double ccaThreshold_dBm);

    void setChannelIdleStatus(bool isIdle) override;

    /**
     * @brief invoke this method when the phy layer is also finalized,
     * so that statistics recorded by the decider can be written to
     * the output file
     */
    void finish() override;

    /**
     * @brief Notifies the decider that phy layer is starting a transmission.
     *
     * This helps the decider interrupting a current reception. In a standard
     * 802.11 MAC, this should never happen, but in other MAC layers you might
     * decide to interrupt an ongoing reception and start transmitting. Thank
     * to this method, the decider can flag the ongoing frame as non received
     * because of the transmission.
     */
    void switchToTx() override;

    /**
     * @brief notify PHY-RXSTART.indication
     */
    void setNotifyRxStart(bool enable);
};

} // namespace veins
