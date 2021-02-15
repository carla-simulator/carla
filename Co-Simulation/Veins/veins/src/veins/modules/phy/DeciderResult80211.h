//
// Copyright (C) 2007 Technische Universitaet Berlin (TUB), Germany, Telecommunication Networks Group
// Copyright (C) 2007 Technische Universiteit Delft (TUD), Netherlands
// Copyright (C) 2007 Universitaet Paderborn (UPB), Germany
// Copyright (C) 2014 Michele Segata <segata@ccs-labs.org>
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
 * DeciderResult80211.h
 *
 *  Created on: 04.02.2009
 *      Author: karl
 *
 *  Modified by Michele Segata (segata@ccs-labs.org)
 */

#pragma once

#include "veins/veins.h"

#include "veins/base/phyLayer/Decider.h"

namespace veins {

/**
 * @brief Defines an extended DeciderResult for the 80211 protocol
 * which stores the bit-rate of the transmission.
 *
 * @ingroup decider
 * @ingroup ieee80211
 */
class VEINS_API DeciderResult80211 : public DeciderResult {
protected:
    /** @brief Stores the bit-rate of the transmission of the packet */
    double bitrate;

    /** @brief Stores the signal to noise ratio of the transmission */
    double snr;

    /** @brief Stores the received power in dBm
     * Please note that this is NOT the RSSI. The RSSI is an indicator
     * of the quality of the signal which is not standardized, and
     * different vendors can define different indicators. This value
     * indicates the power that the frame had when received by the
     * NIC card, WITHOUT noise floor and WITHOUT interference
     */
    double recvPower_dBm;

    /** @brief Stores whether the uncorrect decoding was due to low power or collision */
    bool collision;

public:
    /**
     * @brief Initialises with the passed values.
     *
     * "bitrate" defines the bit-rate of the transmission of the packet.
     */
    DeciderResult80211(bool isCorrect, double bitrate, double snr, double recvPower_dBm = 0, bool collision = false)
        : DeciderResult(isCorrect)
        , bitrate(bitrate)
        , snr(snr)
        , recvPower_dBm(recvPower_dBm)
        , collision(collision)
    {
    }

    /**
     * @brief Returns the bit-rate of the transmission of the packet.
     */
    double getBitrate() const
    {
        return bitrate;
    }

    /**
     * @brief Returns the signal to noise ratio of the transmission.
     */
    double getSnr() const
    {
        return snr;
    }

    /**
     * @brief Returns whether drop was due to collision, if isCorrect is false
     */
    bool isCollision() const
    {
        return collision;
    }

    /**
     * @brief Returns the signal power in dBm.
     */
    double getRecvPower_dBm() const
    {
        return recvPower_dBm;
    }
};

} // namespace veins
