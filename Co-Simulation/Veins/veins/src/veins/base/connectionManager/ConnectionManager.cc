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

#include "veins/base/connectionManager/ConnectionManager.h"

#include <cmath>

#include "veins/base/modules/BaseWorldUtility.h"

using namespace veins;

Define_Module(veins::ConnectionManager);

double ConnectionManager::calcInterfDist()
{
    /* With the introduction of antenna models, calculating the maximum
     * interference distance only based on free space loss doesn't make any sense
     * any more as it could also be much bigger due to positive antenna gains.
     * Therefore, the user has to provide a reasonable maximum interference
     * distance himself. */
    if (hasPar("maxInterfDist")) {
        double interfDistance = par("maxInterfDist").doubleValue();
        EV_INFO << "max interference distance:" << interfDistance << endl;
        return interfDistance;
    }
    else {
        throw cRuntimeError("ConnectionManager: No value for maximum interference distance (maxInterfDist) provided.");
    }
}
