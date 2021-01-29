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

#pragma once

#include <stdint.h>
#include <iostream>
#include <algorithm>
#include <vector>
#include <iterator>

#include "veins/veins.h"

#include "veins/base/phyLayer/DeciderToPhyInterface.h"
#include "veins/base/phyLayer/Decider.h"
#include "veins/base/toolbox/Signal.h"

namespace veins {
namespace SignalUtils {

using AirFrameVector = DeciderToPhyInterface::AirFrameVector;

/**
 * @brief check if the summed power of interfererFrames's signals at freqIndex is below a given threshold.
 *
 * Only considers the signals active at time now and ignores the AirFrame given as exclude.
 *
 * This function will apply analogue models attached to the interfererFrames's signals.
 * It will skip applying analogue models once it is clear that the interferer signals are below threshold.
 * This is known as "thresholding" or some sort of short-circuit evaluation.
 *
 * TODO: apply analogue models one by one instead of group-wise.
 *   Currently assumes that all signals have the same analogue model (classes) attached to them.
 *   Each model is applied for all signals before the next check for the threshold is performed.
 *   This can be optimized to minimize the number of analog models needed to be applied.
 */
bool VEINS_API isChannelPowerBelowThreshold(simtime_t now, AirFrameVector& interfererFrames, size_t freqIndex, double threshold, AirFrame* exclude = nullptr);

/**
 * @brief return the minimal Signal to (Interference + Noise) Ratio at any data channel of signalFrame's signal
 *
 * Important: The AirFrameVector interfererFrames is assumed to be sorted by signal reception start time!
 *
 * This function ensures that all analogue models attached to the signal of each interfererFrame and the signalFrame are applied.
 * Only considers the given interval between [start, end) and assumes time-independent noise that is the same for all channels.
 */
double VEINS_API getMinSINR(simtime_t start, simtime_t end, AirFrame* signalFrame, AirFrameVector& interfererFrames, double noise);

} // namespace SignalUtils
} // namespace veins
