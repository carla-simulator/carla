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

package org.car2x.veins.base.phyLayer;

import org.car2x.veins.base.connectionManager.IChannelAccess;

// Wireless physical layer module interface.
moduleinterface IWirelessPhy extends IChannelAccess
{
    parameters:
        bool recordStats; //enable/disable tracking of statistics (eg. cOutvectors)

        bool usePropagationDelay;        //Should transmission delay be simulated?
        double noiseFloor @unit(dBm); // catch-all for all factors negatively impacting SINR (e.g., thermal noise, noise figure, ...)
        bool useNoiseFloor; // should a noise floor be considered when calculating SINR?

        xml analogueModels;             //Specification of the analogue models to use and their parameters
        xml decider;                    //Specification of the decider to use and its parameters

        double minPowerLevel @unit(dBm); // The minimum receive power needed to even attempt decoding a frame

        //# switch times [s]:
        double timeRXToTX @unit(s); // Elapsed time to switch from receive to send state
        double timeRXToSleep @unit(s); // Elapsed time to switch from receive to sleep state

        double timeTXToRX @unit(s); // Elapsed time to switch from send to receive state
        double timeTXToSleep @unit(s); // Elapsed time to switch from send to sleep state

        double timeSleepToRX @unit(s); // Elapsed time to switch from sleep to receive state
        double timeSleepToTX @unit(s); // Elapsed time to switch from sleep to send state

        int initialRadioState;   // State the radio is initially in (0=RX, 1=TX, 2=Sleep)

        double radioMinAtt; //radios gain factor (attenuation) while receiving
        double radioMaxAtt; //radios gain factor (attenuation) while not receiving

        int nbRadioChannels;  // Number of available radio channels. Defaults to single channel radio.
        int initialRadioChannel;  // Initial radio channel.

    gates:
        input upperLayerIn;     // from the MAC layer
        output upperLayerOut;     // to the MAC layer

        input upperControlIn;     // control from the MAC layer
        output upperControlOut;     // control to the MAC layer

        input radioIn; // for sendDirect from other physical layers

}

