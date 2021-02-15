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

#include "veins/veins.h"

namespace veins {

class VEINS_API PassedMessage : public cObject {
public:
    enum gates_t {
        UPPER_DATA,
        UPPER_CONTROL,
        LOWER_DATA,
        LOWER_CONTROL
    };

    enum direction_t {
        INCOMING,
        OUTGOING
    };

public:
    static const char* gateToString(gates_t g)
    {
        const char* s;
        switch (g) {
        case UPPER_DATA:
            s = "UPPER_DATA";
            break;
        case UPPER_CONTROL:
            s = "UPPER_CONTROL";
            break;
        case LOWER_DATA:
            s = "LOWER_DATA";
            break;
        case LOWER_CONTROL:
            s = "LOWER_CONTROL";
            break;
        default:
            throw cRuntimeError("PassedMessage::gateToString: got invalid value");
            s = nullptr;
            break;
        }
        return s;
    }

public:
    // meta information
    int fromModule;
    gates_t gateType;
    direction_t direction;

    // message information
    int kind;
    const char* name;
};

} // namespace veins
