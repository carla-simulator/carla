//
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

#include "veins/base/toolbox/SignalUtils.h"

#include "veins/base/messages/AirFrame_m.h"

#include <queue>

namespace veins {
namespace SignalUtils {

namespace {

template <typename T>
struct greaterByReceptionEnd {
    bool operator()(const T& lhs, const T& rhs) const
    {
        return lhs.getReceptionEnd() > rhs.getReceptionEnd();
    };
};

Signal getMaxInterference(simtime_t start, simtime_t end, AirFrame* const referenceFrame, AirFrameVector& interfererFrames)
{
    Spectrum spectrum = referenceFrame->getSignal().getSpectrum();
    Signal maxInterference(spectrum);
    Signal currentInterference(spectrum);
    std::priority_queue<Signal, std::vector<Signal>, greaterByReceptionEnd<Signal>> signalEndings;
    simtime_t currentTime = 0;

    interfererFrames.sort([](const AirFrame* x, const AirFrame* y) { return x->getSignal().getReceptionStart() < y->getSignal().getReceptionStart(); });

    for (auto& interfererFrame : interfererFrames) {
        if (interfererFrame->getTreeId() == referenceFrame->getTreeId()) continue; // skip the signal we want to compare to
        const Signal& signal = interfererFrame->getSignal();
        if (signal.getReceptionEnd() <= start || signal.getReceptionStart() > end) continue; // skip signals outside our interval of interest
        ASSERT(signal.getReceptionEnd() > start); // fail on signals ending before start of interval of interest (should be filtered out anyways)
        ASSERT(signal.getReceptionStart() <= end); // fail on signal starting aftser the interval of interest
        ASSERT(signal.getReceptionStart() >= currentTime); // assume frames are sorted by reception start time
        ASSERT(signal.getSpectrum() == spectrum);
        // fetch next signal and advance current time to its start
        signalEndings.push(signal);
        currentTime = signal.getReceptionStart();

        // abort at end time
        if (currentTime >= end) break;

        // remove signals ending before the start of the current one
        while (signalEndings.top().getReceptionEnd() <= currentTime) {
            currentInterference -= signalEndings.top();
            signalEndings.pop();
        }

        // add curent signal to current total interference
        currentInterference += signal;

        // update maximum observed interference
        for (uint16_t spectrumIndex = signal.getDataStart(); spectrumIndex < signal.getDataEnd(); spectrumIndex++) {
            maxInterference.at(spectrumIndex) = std::max(currentInterference.at(spectrumIndex), maxInterference.at(spectrumIndex));
        }
    }

    return maxInterference;
}

double powerLevelSumAtFrequencyIndex(const std::vector<Signal*>& signals, size_t freqIndex)
{
    double powerLevelSum = 0;
    for (auto signalPtr : signals) {
        powerLevelSum += signalPtr->at(freqIndex);
    }
    return powerLevelSum;
}

} // namespace

bool VEINS_API isChannelPowerBelowThreshold(simtime_t now, AirFrameVector& interfererFrames, size_t freqIndex, double threshold, AirFrame* exclude)
{
    if (interfererFrames.empty()) {
        // No interferers, so the channel interference is below any threshold
        return true;
    }

    // extract valid signals on the channel at the time of interest
    // TODO: possibly move this filtering outside of this function
    std::vector<Signal*> interferers;
    for (auto& interfererFrame : interfererFrames) {
        Signal* interferer = &interfererFrame->getSignal();
        if (interferer->getReceptionStart() <= now && interferer->getReceptionEnd() > now && interfererFrame != exclude) {
            interferers.push_back(interferer);
        }
    }

    // check once before applying analogModels
    if (powerLevelSumAtFrequencyIndex(interferers, freqIndex) < threshold) {
        return true;
    }

    // start applying analogue models
    auto analogueModelCount = interfererFrames.front()->getSignal().getAnalogueModelList()->size();
    for (auto signalPtr : interferers) {
        ASSERT(analogueModelCount == signalPtr->getAnalogueModelList()->size());
    }
    for (size_t analogueModelIndex = 0; analogueModelIndex < analogueModelCount; ++analogueModelIndex) {
        for (auto signalPtr : interferers) {
            signalPtr->applyAnalogueModel(analogueModelIndex);
        }
        if (powerLevelSumAtFrequencyIndex(interferers, freqIndex) < threshold) {
            return true;
        }
    }

    // After all attenuation is performed, there interference is still higher than the threshold
    ASSERT(powerLevelSumAtFrequencyIndex(interferers, freqIndex) >= threshold);
    return false;
}

double VEINS_API getMinSINR(simtime_t start, simtime_t end, AirFrame* signalFrame, AirFrameVector& interfererFrames, double noise)
{
    ASSERT(start >= signalFrame->getSignal().getReceptionStart());
    ASSERT(end <= signalFrame->getSignal().getReceptionEnd());

    // Make sure all filters are applied
    signalFrame->getSignal().applyAllAnalogueModels();
    for (auto& interfererFrame : interfererFrames) {
        interfererFrame->getSignal().applyAllAnalogueModels();
    }

    Signal& signal = signalFrame->getSignal();
    Spectrum spectrum = signal.getSpectrum();

    Signal interference = getMaxInterference(start, end, signalFrame, interfererFrames);
    Signal sinr = signal / (interference + noise);

    double min_sinr = INFINITY;
    for (uint16_t i = signal.getDataStart(); i < signal.getDataEnd(); i++) {
        min_sinr = std::min(min_sinr, sinr.at(i));
    }
    return min_sinr;
}

} // namespace SignalUtils
} // namespace veins
