//
// Copyright (C) 2018 Christoph Sommer <sommer@ccs-labs.org>
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

#include <vector>

#include "veins_inet/veins_inet.h"

#include "inet/common/INETDefs.h"

#include "inet/applications/base/ApplicationBase.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"
#include "veins_inet/VeinsInetMobility.h"
#include "veins/modules/utility/TimerManager.h"

namespace veins {

class VEINS_INET_API VeinsInetApplicationBase : public inet::ApplicationBase, public inet::UdpSocket::ICallback {
protected:
    veins::VeinsInetMobility* mobility;
    veins::TraCICommandInterface* traci;
    veins::TraCICommandInterface::Vehicle* traciVehicle;
    veins::TimerManager timerManager{this};

    inet::L3Address destAddress;
    const int portNumber = 9001;
    inet::UdpSocket socket;

protected:
    virtual int numInitStages() const override;
    virtual void initialize(int stage) override;
    virtual void handleStartOperation(inet::LifecycleOperation* doneCallback) override;
    virtual bool startApplication();
    virtual bool stopApplication();
    virtual void handleStopOperation(inet::LifecycleOperation* doneCallback) override;
    virtual void handleCrashOperation(inet::LifecycleOperation* doneCallback) override;
    virtual void finish() override;

    virtual void refreshDisplay() const override;
    virtual void handleMessageWhenUp(inet::cMessage* msg) override;

    virtual void socketDataArrived(inet::UdpSocket* socket, inet::Packet* packet) override;
    virtual void socketErrorArrived(inet::UdpSocket* socket, inet::Indication* indication) override;
    virtual void socketClosed(inet::UdpSocket* socket) override;

    virtual std::unique_ptr<inet::Packet> createPacket(std::string name);
    virtual void processPacket(std::shared_ptr<inet::Packet> pk);
    virtual void timestampPayload(inet::Ptr<inet::Chunk> payload);
    virtual void sendPacket(std::unique_ptr<inet::Packet> pk);

public:
    VeinsInetApplicationBase();
    ~VeinsInetApplicationBase();
};

} // namespace veins
