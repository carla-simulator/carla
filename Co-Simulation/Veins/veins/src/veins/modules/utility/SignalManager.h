//
// Copyright (C) 2019-2019 Dominik S. Buse <buse@ccs-labs.org>
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

#include "veins/veins.h"

#include <functional>
#include <memory>

namespace veins {

template <typename Payload>
struct VEINS_API SignalPayload {
    cComponent* source;
    simsignal_t signalID;
    Payload p;
    cObject* details;
};

template <typename Payload>
class VEINS_API SignalCallbackListener : public cListener {
public:
    using Callback = std::function<void (SignalPayload<Payload>)>;
    SignalCallbackListener(Callback callback, cModule* receptor, simsignal_t signal)
        : callback(callback)
        , receptor(receptor)
        , signal(signal)
    {
        receptor->subscribe(signal, this);
    }

    ~SignalCallbackListener()
    {
        if (getSubscribeCount() > 0) {
            receptor->unsubscribe(signal, this);
        }
    }

    void receiveSignal(cComponent* source, simsignal_t signalID, Payload p, cObject* details) override
    {
        ASSERT(signalID == signal);
        callback({source, signalID, p, details});
    }

private:
    const Callback callback;
    cModule* const receptor;
    const simsignal_t signal;
};

class VEINS_API SignalManager {
public:
    void subscribeCallback(cModule* receptor, simsignal_t signal, const std::function<void(SignalPayload<bool>)> callback)
    {
        auto callbackListener = make_unique<SignalCallbackListener<bool>>(callback, receptor, signal);
        callbacks.emplace_back(std::move(callbackListener));
    }
    void subscribeCallback(cModule* receptor, simsignal_t signal, const std::function<void(SignalPayload<long>)> callback)
    {
        auto callbackListener = make_unique<SignalCallbackListener<long>>(callback, receptor, signal);
        callbacks.emplace_back(std::move(callbackListener));
    }
    void subscribeCallback(cModule* receptor, simsignal_t signal, const std::function<void(SignalPayload<unsigned long>)> callback)
    {
        auto callbackListener = make_unique<SignalCallbackListener<unsigned long>>(callback, receptor, signal);
        callbacks.emplace_back(std::move(callbackListener));
    }
    void subscribeCallback(cModule* receptor, simsignal_t signal, const std::function<void(SignalPayload<double>)> callback)
    {
        auto callbackListener = make_unique<SignalCallbackListener<double>>(callback, receptor, signal);
        callbacks.emplace_back(std::move(callbackListener));
    }
    void subscribeCallback(cModule* receptor, simsignal_t signal, const std::function<void(SignalPayload<const SimTime&>)> callback)
    {
        auto callbackListener = make_unique<SignalCallbackListener<const SimTime&>>(callback, receptor, signal);
        callbacks.emplace_back(std::move(callbackListener));
    }
    void subscribeCallback(cModule* receptor, simsignal_t signal, const std::function<void(SignalPayload<const char*>)> callback)
    {
        auto callbackListener = make_unique<SignalCallbackListener<const char*>>(callback, receptor, signal);
        callbacks.emplace_back(std::move(callbackListener));
    }
    void subscribeCallback(cModule* receptor, simsignal_t signal, const std::function<void(SignalPayload<cObject*>)> callback)
    {
        auto callbackListener = make_unique<SignalCallbackListener<cObject*>>(callback, receptor, signal);
        callbacks.emplace_back(std::move(callbackListener));
    }

private:
    std::vector<std::unique_ptr<cListener>> callbacks;
};

} // namespace veins
