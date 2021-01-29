//
// Copyright (C) 2015-2018 Dominik Buse <dbuse@mail.uni-paderborn.de>
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

namespace veins;

class DemoSafetyMessage;
enum TrafficLightAtrributeType {
    NONE = 0;
    LOGICID = 1;
    PHASEID = 2;
    SWITCHTIME = 3;
    STATE = 4;
};

enum TrafficLightChangeSource {
    UNKNOWN = 0;
    SUMO = 1;
    LOGIC = 2;
    RSU = 3;//If an RSU tries to change the values
};

// NOTE: Currently only supports changes of the IDs (due to variation in field types)
message TraCITrafficLightMessage {
    // traffic light id
    string tlId;
    // what field/attrbute of the traffic light changed?
    int changedAttribute @enum(TrafficLightAtrributeType);
    // value before the change
    string oldValue;
    // value that is to be set / was newly set
    string newValue;
    // where did the change originate
    int changeSource @enum(TrafficLightChangeSource);
};
