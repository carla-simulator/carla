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

#pragma once

#include <map>
#include <vector>
#include <string>
#include <memory>

#include "veins/veins.h"

#include "veins/base/connectionManager/ChannelAccess.h"
#include "veins/base/phyLayer/DeciderToPhyInterface.h"
#include "veins/base/phyLayer/MacToPhyInterface.h"
#include "veins/base/phyLayer/Antenna.h"
#include "veins/base/phyLayer/ChannelInfo.h"

namespace veins {

class Decider;
class BaseWorldUtility;
// class omnetpp::cXMLElement;

class AirFrame;
class ChannelAccess;
class Radio;

/**
 * The BasePhyLayer represents the physical layer of a nic.
 *
 * The BasePhyLayer is directly connected to the mac layer via OMNeT channels and is able to send messages to other physical layers through sub-classing from ChannelAcces.
 * In order to implement a physical layer, subclass the BasePhyLayer and override these methods:
 *  - initializeRadio
 *  - getAnalogueModelFromName
 *  - getDeciderFromName
 *  - createAirFrame
 *  - attachSignal
 *
 * The BasePhyLayer encapsulates three sub modules:
 * The AnalogueModels, which are responsible for simulating the attenuation of received signals, and the Decider, which provides the main functionality of the physical layer like signal classification (noise or not noise) and demodulation (calculating transmission errors).
 * Furthermore, the Antenna used for gain calculation is managed by the BasePhyLayer.
 *
 * The BasePhyLayer itself is responsible for the OMNeT depended parts of the physical layer which are the following:
 *
 * Module initialization:
 * - read ned-parameters and initialize module, Decider, AnalogueModels and Antenna
 *
 * Message handling:
 * - receive messages from mac layer and hand them to the Decider or directly send them to the channel
 * - receive AirFrames from the channel, hand them to the AnalogueModels for filtering, simulate delay and transmission duration, hand it to the Decider for evaluation and send received packets to the mac layer
 * - keep track of currently active AirFrames on the channel (see ChannelInfo)
 *
 * The actual evaluation of incoming signals is done by the Decider.
 *
 * base class ChannelAccess:
 * - provides access to the channel via the ConnectionManager
 *
 * base class DeciderToPhyInterface:
 * - interface for the Decider
 *
 * base class MacToPhyInterface:
 * - interface for the Mac
 *
 * @ingroup phyLayer
 * @ingroup baseModules
 */

class VEINS_API BasePhyLayer : public ChannelAccess, public DeciderToPhyInterface, public MacToPhyInterface {

protected:
    using ParameterMap = std::map<std::string, cMsgPar>; ///< Used at initialisation to pass the parameters to the AnalogueModel and Decider.

    /** The states of the receiving process for AirFrames.*/
    enum class AirFrameState {
        start_receive = 1, ///< Start of actual receiving process of the AirFrame.
        receiving, ///< AirFrame is being received.
        end_receive ///< Receiving process over.
    };

    enum ProtocolIds {
        PROTOCOL_ID_GENERIC = 0,
    };

    /** @brief Defines the scheduling priority of AirFrames.
     *
     * AirFrames use a slightly higher priority than normal to ensure channel consistency.
     * This means that before anything else happens at a time point t every AirFrame which ended at t has been removed and every AirFrame started at t has been added to the channel.
     */
    static short airFramePriority()
    {
        return 10;
    }

    int protocolId = PROTOCOL_ID_GENERIC; ///< The ID of the protocol this phy can transceive.
    double noiseFloorValue = 0; ///< Catch-all for all factors negatively impacting SINR (e.g., thermal noise, noise figure, ...)
    double minPowerLevel; ///< The minimum receive power needed to even attempt decoding a frame.
    bool recordStats; ///< Stores if tracking of statistics (esp. cOutvectors) is enabled.
    ChannelInfo channelInfo; ///< Channel info keeps track of received AirFrames and provides information about currently active AirFrames at the channel.
    std::unique_ptr<Radio> radio; ///< The state machine storing the current radio state (TX, RX, SLEEP).

    /**
     * Shared pointer to the Antenna used for this node.
     *
     * Using a shared pointer ensures proper handling of a signal is possible, even after the sender has been destroyed.
     */
    std::shared_ptr<Antenna> antenna;

    std::unique_ptr<Decider> decider; ///< Pointer to the decider module.

    /**
     * The analogue models to use which might attenuate or amplify a signal.
     */
    AnalogueModelList analogueModels;

    /**
     * The analogue models to use which will only attenuate a signal.
     *
     * These models are not applied immediately, but only attached to the signal.
     * This enables lazy application of the models.
     */
    AnalogueModelList analogueModelsThresholding;

    int upperLayerIn; ///< The id of the in-data gate from the Mac layer.
    int upperLayerOut; ///< The id of the out-data gate to the Mac layer.
    int upperControlOut; ///< The id of the out-control gate to the Mac layer.
    int upperControlIn; ///< The id of the in-control gate from the Mac layer.

    cMessage* radioSwitchingOverTimer = nullptr; ///< Self message scheduled to the point in time when the switching process of the radio is over.
    cMessage* txOverTimer = nullptr; ///< Self message scheduled to the point in time when the transmission of an AirFrame is over.

    BaseWorldUtility* world = nullptr; ///< Pointer to the World Utility, to obtain some global information

private:
    /**
     * Read the parameters of a XML element and stores them in the passed ParameterMap reference.
     */
    void getParametersFromXML(cXMLElement* xmlData, ParameterMap& outputMap);

    /**
     * Initialize the AnalogueModels with the data from the passed XML-config data.
     */
    void initializeAnalogueModels(cXMLElement* xmlConfig);

    /**
     * Initialize the Decider with the data from the passed XML-config data.
     */
    void initializeDecider(cXMLElement* xmlConfig);

    /**
     * Initialize the Antenna with the data from the passed XML-config data.
     */
    void initializeAntenna(cXMLElement* xmlConfig);

protected:
    /**
     * Read and return the parameter with the passed name.
     *
     * If the parameter couldn't be found the value of defaultValue is returned.
     *
     * @param parName The name of the ned-parameter
     * @param defaultValue The value to be returned if the parameter couldn't be found
     */
    template <class T>
    T readPar(const char* parName, const T defaultValue);

    /**
     * Initialize members and associated objects according to parameterization.
     */
    void initialize(int stage) override;

    /**
     * Process received messages and pass to specific handlers.
     *
     * @see handleSelfMessage
     * @see handleUpperMessage
     * @see handleUpperControlMessage
     * @see handleAirFrame
     */
    void handleMessage(cMessage* msg) override;

    /**
     * Create and return the radio class to use.
     */
    virtual std::unique_ptr<Radio> initializeRadio();

    /**
     * Create and return an instance of the AnalogueModel with the
     * specified name.
     *
     * This method is used by the BasePhyLayer during initialisation to load the AnalogueModels which has been specified in the NED file.
     */
    virtual std::unique_ptr<AnalogueModel> getAnalogueModelFromName(std::string name, ParameterMap& params)
    {
        return nullptr;
    }

    /**
     * Create and return an instance of the Decider with the specified name.
     *
     * This method is used by the BasePhyLayer during initialisation to load the Decider which has been specified in the ned file.
     */
    virtual std::unique_ptr<Decider> getDeciderFromName(std::string name, ParameterMap& params);

    /**
     * Create and return an instance of the Antenna with the specified name as a shared pointer.
     *
     * This method is called during initialization to load the Antenna specified.
     * If no special antenna has been specified, an object of the base Antenna class is instantiated, representing an isotropic antenna.
     */
    virtual std::shared_ptr<Antenna> getAntennaFromName(std::string name, ParameterMap& params);

    /**
     * Creates and returns an instance of the SampledAntenna1D class as a shared pointer.
     *
     * The given parameters (i.e. samples and optional randomness parameters) are evaluated and passed to the antenna's constructor.
     */
    virtual std::shared_ptr<Antenna> initializeSampledAntenna1D(ParameterMap& params);

    /**
     * @name Handle Messages
     **/
    /*@{ */
    /**
     * Handle messages received from the channel.
     *
     * @see handleAirFrameStartReceive
     * @see handleAirFrameReceiving
     * @see handleAirFrameEndReceive
     */
    virtual void handleAirFrame(AirFrame* frame);

    /**
     * Handle messages received from the upper layer and start transmitting them.
     */
    virtual void handleUpperMessage(cMessage* msg);

    /**
     * Handle messages received from the upper layer through the control gate.
     */
    virtual void handleUpperControlMessage(cMessage* msg);

    /**
     * Handle self scheduled messages.
     */
    virtual void handleSelfMessage(cMessage* msg);

    /**
     * Handle incoming AirFrames with the state AirFrameState::start_receive.
     */
    virtual void handleAirFrameStartReceive(AirFrame* msg);

    /**
     * Handle incoming AirFrames with the state AirFrameState::receiving.
     */
    virtual void handleAirFrameReceiving(AirFrame* msg);

    /**
     * Handle incoming AirFrames with the state AirFrameState::end_receive.
     */
    virtual void handleAirFrameEndReceive(AirFrame* msg);

    /*@}*/

    /**
     * @name Send Messages
     **/
    /*@{ */

    /**
     * Send the passed control message to the upper layer.
     */
    void sendControlMessageUp(cMessage* msg);

    /**
     * Send the passed MacPkt to the upper layer.
     */
    void sendMacPktUp(cMessage* pkt);

    /**
     * Send the passed AirFrame to the channel
     */
    void sendMessageDown(AirFrame* pkt);

    /**
     * Schedule self message to passed point in time.
     */
    void sendSelfMessage(cMessage* msg, simtime_t_cref time);

    /*@}*/

    /**
     * Create a protocol-specific AirFrame
     */
    virtual std::unique_ptr<AirFrame> createAirFrame(cPacket* macPkt);

    /**
     * Create a signal corresponding to the received control information.
     */
    virtual void attachSignal(AirFrame* airFrame, cObject* ctrlInfo)
    {
        throw cRuntimeError("Not implemented in the BasePhyLayer. Override in subclass.");
    }

    /**
     * Encapsulate messages from the upper layer into an AirFrame and set all necessary attributes.
     */
    virtual std::unique_ptr<AirFrame> encapsMsg(cPacket* msg);

    /**
     * Filter the passed AirFrame's Signal by every registered AnalogueModel.
     *
     * Moreover, the antenna gains are calculated and added to the signal.
     * @note Only models from analogueModels are applied. Those referenced in analogueModelsThresholding are passed to the Signal to enable more efficient handling.
     *
     * @see analogueModels
     * @see analogueModelsThresholding
     */
    virtual void filterSignal(AirFrame* frame);

    /**
     * Called when the switching process of the Radio is finished.
     *
     * The default implementation sends a confirmation message to the mac layer.
     *
     * @see MacToPhyInterface::RADIO_SWITCHING_OVER
     */
    virtual void finishRadioSwitching();

    /**
     * Return the identifier of the protocol this phy uses to send messages.
     *
     * @return An integer representing the identifier of the used protocol.
     */
    virtual int myProtocolId()
    {
        return protocolId;
    }

    /**
     * Return whether the protocol with the passed identifier is decodeable by the decider.
     *
     * Default implementation checks only if the passed id is the same as the one returned by "myProtocolId()".
     *
     * @param id The identifier of the protocol of an AirFrame.
     * @return true if the passed protocol id is supported by this phy.
     * layer.
     */
    virtual bool isKnownProtocolId(int id)
    {
        return id == myProtocolId();
    }

    /**
     * The underlying spectrum (definition of interesting freqs) for all signals
     */
    Spectrum overallSpectrum;

public:
    ~BasePhyLayer() override;

    /** Call the deciders finish method. */
    void finish() override;

    // ---------MacToPhyInterface implementation-----------
    /**
     * @name MacToPhyInterface implementation
     * @brief These methods implement the MacToPhyInterface.
     **/
    /*@{ */

    /**
     * Return the current state the radio is in.
     *
     * @see RadioState for possible values.
     */
    int getRadioState() override;

    /**
     * Tell the BasePhyLayer to switch to the specified radio state.
     *
     * The switching process can take some time depending on the specified switching times in the ned file.
     *
     * @return Switching time from the current RadioState to the new RadioState or -1 if a state change is already in progress.
     */
    simtime_t setRadioState(int rs) override;

    /** Set the channel currently used by the radio. */
    void setCurrentRadioChannel(int newRadioChannel) override;

    /** Return the channel currently used by the radio. */
    int getCurrentRadioChannel() override;

    /** Return the number of channels available on this radio. */
    int getNbRadioChannels() override;

    /*@}*/

    // ---------DeciderToPhyInterface implementation-----------
    /**
     * @name DeciderToPhyInterface implementation
     * @brief These methods implement the DeciderToPhyInterface.
     **/
    /*@{ */

    /**
     * Fill the given AirFrameVector with all AirFrames that intersect with the given time interval.
     */
    void getChannelInfo(simtime_t_cref from, simtime_t_cref to, AirFrameVector& out) override;

    /**
     * Return noise floor level (in mW).
     */
    double getNoiseFloorValue() override;

    /**
     * Send the given message to via the control gate to the mac.
     *
     * @see upperControlOut
     */
    void sendControlMsgToMac(cMessage* msg) override;

    /**
     * Pass the given packet along with the result up to the mac.
     */
    void sendUp(AirFrame* packet, DeciderResult* result) override;

    /**
     * Return a pointer to the simulations world-utility-module.
     */
    BaseWorldUtility* getWorldUtility() override;

    /**
     * Record a double into the scalar result file.
     *
     * Implements the method from DeciderToPhyInterface, method-calls are forwarded to OMNeT-method 'recordScalar'.
     *
     * @see cComponent::recordScalar
     */
    void recordScalar(const char* name, double value, const char* unit = nullptr) override;

    /*@}*/
};

} // namespace veins
