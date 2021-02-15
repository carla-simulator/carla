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

#pragma once

#include "veins/base/phyLayer/BasePhyLayer.h"
#include "veins/base/toolbox/Spectrum.h"
#include "veins/modules/mac/ieee80211p/Mac80211pToPhy11pInterface.h"
#include "veins/modules/phy/Decider80211p.h"
#include "veins/modules/analogueModel/SimplePathlossModel.h"
#include "veins/base/connectionManager/BaseConnectionManager.h"
#include "veins/modules/phy/Decider80211pToPhy80211pInterface.h"
#include "veins/base/utils/Move.h"

namespace veins {

/**
 * @brief
 * Adaptation of the PhyLayer class for 802.11p.
 *
 * @ingroup phyLayer
 *
 * @see DemoBaseApplLayer
 * @see Mac1609_4
 * @see PhyLayer80211p
 * @see Decider80211p
 */
class VEINS_API PhyLayer80211p : public BasePhyLayer, public Mac80211pToPhy11pInterface, public Decider80211pToPhy80211pInterface {
public:
    void initialize(int stage) override;
    /**
     * @brief Set the carrier sense threshold
     * @param ccaThreshold_dBm the cca threshold in dBm
     */
    void setCCAThreshold(double ccaThreshold_dBm) override;
    /**
     * @brief Return the cca threshold in dBm
     */
    double getCCAThreshold();
    /**
     * @brief Enable notifications about PHY-RXSTART.indication in MAC
     * @param enable true if Mac needs to be notified about it
     */
    void notifyMacAboutRxStart(bool enable) override;
    /**
     * @brief Explicit request to PHY for the channel status
     */
    void requestChannelStatusIfIdle() override;

protected:
    /** @brief CCA threshold. See Decider80211p for details */
    double ccaThreshold;

    /** @brief enable/disable detection of packet collisions */
    bool collectCollisionStatistics;

    /** @brief allows/disallows interruption of current reception for txing
     *
     * See detailed description in Decider80211p
     */
    bool allowTxDuringRx;

    enum ProtocolIds {
        IEEE_80211 = 12123
    };
    /**
     * @brief Creates and returns an instance of the AnalogueModel with the
     * specified name.
     *
     * Is able to initialize the following AnalogueModels:
     */
    virtual std::unique_ptr<AnalogueModel> getAnalogueModelFromName(std::string name, ParameterMap& params) override;

    /**
     * @brief Creates and initializes a SimplePathlossModel with the
     * passed parameter values.
     */
    std::unique_ptr<AnalogueModel> initializeSimplePathlossModel(ParameterMap& params);

    /**
     * @brief Creates and initializes an AntennaModel with the
     * passed parameter values.
     */
    std::unique_ptr<AnalogueModel> initializeAntennaModel(ParameterMap& params);

    /**
     * @brief Creates and initializes a BreakpointPathlossModel with the
     * passed parameter values.
     */
    virtual std::unique_ptr<AnalogueModel> initializeBreakpointPathlossModel(ParameterMap& params);

    /**
     * @brief Creates and initializes a SimpleObstacleShadowing with the
     * passed parameter values.
     */
    std::unique_ptr<AnalogueModel> initializeSimpleObstacleShadowing(ParameterMap& params);

    /**
     * @brief Creates and initializes a VehicleObstacleShadowing with the
     * passed parameter values.
     */
    std::unique_ptr<AnalogueModel> initializeVehicleObstacleShadowing(ParameterMap& params);

    /**
     * @brief Creates a simple Packet Error Rate model that attenuates a percentage
     * of the packets to zero, and does not attenuate the other packets.
     *
     */
    virtual std::unique_ptr<AnalogueModel> initializePERModel(ParameterMap& params);

    /**
     * @brief Creates and initializes a TwoRayInterferenceModel with the
     * passed parameter values.
     */
    std::unique_ptr<AnalogueModel> initializeTwoRayInterferenceModel(ParameterMap& params);

    /**
     * @brief Creates and initializes a NakagamiFading with the
     * passed parameter values.
     */
    std::unique_ptr<AnalogueModel> initializeNakagamiFading(ParameterMap& params);

    /**
     * @brief Creates and returns an instance of the Decider with the specified
     * name.
     *
     * Is able to initialize the following Deciders:
     *
     * - Decider80211p
     */
    virtual std::unique_ptr<Decider> getDeciderFromName(std::string name, ParameterMap& params) override;

    /**
     * @brief Initializes a new Decider80211 from the passed parameter map.
     */
    virtual std::unique_ptr<Decider> initializeDecider80211p(ParameterMap& params);

    /**
     * Create a protocol-specific AirFrame
     * Overloaded to create a specialize AirFrame11p.
     */
    std::unique_ptr<AirFrame> createAirFrame(cPacket* macPkt) override;

    /**
     * Attach a signal to the given AirFrame.
     *
     * The attached Signal corresponds to the IEEE 802.11p standard.
     * Parameters for the signal are passed in the control info.
     * The indicated power levels are set up on the specified center frequency, as well as the neighboring 5MHz.
     *
     * @note The control info must be of type MacToPhyControlInfo11p
     */
    void attachSignal(AirFrame* airFrame, cObject* ctrlInfo) override;

    void changeListeningChannel(Channel channel) override;

    virtual simtime_t getFrameDuration(int payloadLengthBits, MCS mcs) const override;

    void handleSelfMessage(cMessage* msg) override;
    int getRadioState() override;
    simtime_t setRadioState(int rs) override;
};

} // namespace veins
